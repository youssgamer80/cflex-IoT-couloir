#include <WiFi.h>
#include "ThingSpeak.h"
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>


//------------------parametre thingspeak------------------
WiFiClient  client;

unsigned long myChannelNumber = 1786759 ;
String apiKey = "QWMD47ATTQMWG3R3";
const char* serverName = "http://api.thingspeak.com/update";

unsigned long lastTime = 0;
unsigned long timerDelay = 2000;
AsyncWebServer server(80); // Object of WebServer(HTTP port, 80 is defult)

//------------------------------------



//------------------parametre wifi------------------
const char* ssid = "WIFI ODC";
const char* password = "Digital1";
//------------------------------------
LiquidCrystal_I2C lcd(0x27, 20, 4);

//------------------parametre capteur------------------
const int entre = 4 ;
const int sortie = 5 ;

int nb_person = 0 ;
int nb_place = 20 ;
uint64_t idcouloir;
String id_couloir1;
String id_couloir = "";

int tamp1 = 0 ;
int tamp2 = 0 ;
//------------------------------------
String couloirId() {
  idcouloir = ESP.getEfuseMac();
  Serial.printf("ESP32 Chip ID = %04X", (uint16_t)(idcouloir >> 32));
  id_couloir1 = Serial.printf("%80X\n", (uint32_t)idcouloir);
  return id_couloir1;
}


// Replaces placeholder with LED state value
String processor(const String& var) {
  Serial.println(var);
  if (var == "NBPE") {
    return String(nb_person);
  }
  if (var == "NBPL") {
    return String(nb_place);
  }
  return String();
}


void setup() {
  id_couloir = couloirId();
  Serial.begin(115200);
  pinMode(entre, INPUT) ;
  pinMode(sortie, INPUT) ;
  connect_wifi() ;
  lcd.init();
  ThingSpeak.begin(client);
  lcd.backlight();
  lcd.setCursor(3, 0);
  lcd.print("C'Flex Tracker");

  // Initialize SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("Une erreur s'est produite lors du montage de SPIFFS");
    return;
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });



  // Start server
  server.begin();
}

void loop() {


  if ((millis() - lastTime) > timerDelay) {






    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      HTTPClient http;

      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);

      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      // Data to send with HTTP POST
      String httpRequestData = "api_key=" + apiKey + "&field3=" + String(id_couloir) + "&field2=" + String(nb_place) + "&field1=" + String(nb_person);
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);

      /*
        // If you need an HTTP request with a content type: application/json, use the following:
        http.addHeader("Content-Type", "application/json");
        // JSON data to send with HTTP POST
        String httpRequestData = "{\"api_key\":\"" + apiKey + "\",\"field1\":\"" + String(random(40)) + "\"}";
        // Send HTTP POST request
        int httpResponseCode = http.POST(httpRequestData);*/

      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }

    /*ThingSpeak.setField(1, nb_person);
      ThingSpeak.setField(2, nb_place);
      ThingSpeak.setField(3, id_couloir);

      int x = ThingSpeak.writeFields(myChannelNumber,myWriteAPIKey);
      if(x == 200){
        Serial.println("Channel update successful.");
      }
      else{
        Serial.println("Problem updating channel. HTTP error code " + String(x));
      }*/
    lastTime = millis();
  }


  tamp1 = digitalRead(entre);
  tamp2 = digitalRead(sortie) ;

  if (tamp1 == 0) {
    if (nb_place > 0) {
      //Serial.println(" ") ;
      //Serial.println("--------------Entre-----------------") ;
      nb_place -- ;
      nb_person ++ ;
      lcd.setCursor(0, 2);
      lcd.print("nombre de place:") ;
      lcd.print(nb_place) ;
      lcd.setCursor(0, 3);
      lcd.print("nombre de personne:") ;
      lcd.print(nb_person) ;
      //Serial.println("-------------------------------") ;
      //Serial.println(" ") ;

    }
    else {
      //Serial.println(" ") ;
      //Serial.println("--------------Entre-----------------") ;
      lcd.setCursor(3, 2);
      lcd.print("plus de place disponible") ;
      //Serial.println("-------------------------------") ;
      //Serial.println(" ") ;
    }



  }

  if (tamp2 == 0) {
    if (nb_person > 0) {
      //Serial.println(" ") ;
      //Serial.println("--------------Sortie-----------------") ;
      nb_place ++ ;
      nb_person -- ;
      lcd.setCursor(0, 2);
      lcd.print("nombre de place:") ;
      lcd.print(nb_place) ;
      lcd.setCursor(0, 3);
      lcd.print("nombre de personne:") ;
      lcd.print(nb_person) ;
      //Serial.println("-------------------------------") ;
      //Serial.println(" ") ;
    }
    else {
      //Serial.println(" ") ;
      //Serial.println("--------------Sortie-----------------") ;
      lcd.setCursor(3, 2);
      lcd.println("Couloir vide") ;
      //Serial.println("-------------------------------") ;
      //Serial.println(" ") ;
    }
  }

  delay(2000);


}


void connect_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Timer set to 2 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");

}
