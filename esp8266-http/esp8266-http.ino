/**
   BasicHTTPClient.ino
    Created on: 24.05.2015
*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#define USE_SERIAL Serial

ESP8266WiFiMulti WiFiMulti;
int count = 0;
void setup() {

  USE_SERIAL.begin(115200);
  USE_SERIAL.setDebugOutput(true);

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("home-network", "tacocat1234");

}
void toggleLights() {


  HTTPClient http;

  USE_SERIAL.print("[HTTP] begin...\n");
  // configure traged server and url
  http.begin("https://api.lifx.com/v1/lights/all/toggle", "3B 6D ED 88 25 FD D1 B1 5F 46 38 78 1A 59 83 08 5D 5E 42 D9"); //HTTPS
  http.addHeader("Authorization", "Bearer c786d55da9d454735cf6e3a5c06dff612afdab4ba48473d09c700f62f7aae435");


  USE_SERIAL.print("[HTTP] POST...\n");
  // start connection and send HTTP header
  int httpCode = http.POST("");

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    USE_SERIAL.printf("[HTTP] POST... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      USE_SERIAL.println(payload);
    }
  } else {
    USE_SERIAL.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
  
}
void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    if(count < 1){
      toggleLights();   
      count++;
    }
  }
}

