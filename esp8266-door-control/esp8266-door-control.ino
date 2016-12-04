#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "FS.h"
//TODO: Test out OTA updates
//TODO: Enable Soft AP and Station mode
//TODO: Move HTML files to SPIFFS - or SD card / NVRAM
//TODO: Move wifi creds to EEPROM
//TODO: Use infared sensor / diode or ultrasonic sensor to determine if garage door is open or closed
//TODO: Report current state of garage door in WebUI and API

const char* ssid =MY_SSID;
const char* password =MY_PASSWORD;
const char root[] PROGMEM = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1, shrink-to-fit=no'></head><body><button type='button' onclick='toogleDoor()'>Toggle Door</button><p id='text'></p><script>function toogleDoor() {var xmlhttp;xmlhttp = new XMLHttpRequest();xmlhttp.onreadystatechange = function() {if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {document.getElementById('text').innerHtml = 'Toggled Door!'}};xmlhttp.open('GET', '/door', true);xmlhttp.send();}</script></body></html>";
ESP8266WebServer server(80);
const int led = 16;
const int door = 5;
void handleRoot(){
  digitalWrite(led, 0);
  server.send(200, "text/html", root);
  digitalWrite(led, 1);
}

void toggleDoor(){
  digitalWrite(door, 0);
  delay(75);
  digitalWrite(door, 1);
  // server.sendHeader("Location", "/");
  server.send(200, "text/plain", "ok");
}

void handleNotFound(){
  digitalWrite(led, 1);
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
  digitalWrite(led, 0);
}

void setup(void){
  pinMode(led, OUTPUT);
  pinMode(door, OUTPUT);
  digitalWrite(door, 1);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/door", toggleDoor);
  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();
}
