#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "DHT.h"
#define DHTPIN 5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
const char* ssid =MY_SSID;
const char* password =MY_PASSWORD;
const long pollTempInterval = 10000;
String displayedTempF = "n/a";
unsigned long previousMillisTemp = 0;
ESP8266WebServer server(80);

void setupWiFi(){
  Serial.println("Connecting to Wifi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());
}

bool updateInterval(){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisTemp >= pollTempInterval) {
    previousMillisTemp = currentMillis;
    return true;
  }else{
    return false;
  }
}


void handleRoot() {
  server.send(200, "text/plain", "Current Temp: " + displayedTempF + (char)176 + "F");
}

void handleNumberOnly() {
  server.send(200, "text/plain", displayedTempF);
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void) {
  Serial.begin(115200);
  dht.begin();
  setupWiFi();
  server.on("/", handleRoot);
  server.on("/temp", handleNumberOnly);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  if(updateInterval()){
    displayedTempF = dht.readTemperature(true);
  }
}
