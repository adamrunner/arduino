/*
 * Basic IO
 * Use a pushbutton to toggle the onboard LED.
 *
 * If you do not have the 1 Button Shield, add a pushbutton or switch between D3 and GND
 */
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#define USE_SERIAL Serial

int inputPin = 15;  // pushbutton connected to digital pin D4
int inputPin2 = 4;
int val = 0;        // variable to store the read value
int val2 = 0;
int newVal;
int newVal2;

HTTPClient http;
unsigned long currentMillis;
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long lastDebounceTime2 = 0;
unsigned long buttonInterval = 1000;    // the debounce time; increase if the output flickers

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("");
  pinMode(inputPin, INPUT);      // set pin as input
  pinMode(inputPin2, INPUT);      // set pin as input
  WiFi.mode(WIFI_STA);
  WiFi.begin("home-network", "tacocat1234");

  setupOTA();
  while (WiFi.status() != WL_CONNECTED)

  delay(500);
  Serial.println("Waiting for connection\r\n");
}

void setupOTA() {
 // Setup OTA Uploads
  ArduinoOTA.setPassword((const char *)"boarding");

  ArduinoOTA.onStart([]() {
    Serial.println("STARTING OTA UPDATE");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nCOMPLETED OTA UPDATE");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void buttonPressed(int buttonNum, int val){
  Serial.print("Button ");
  Serial.print(buttonNum);
  Serial.println(" Pressed");
  toggleLights(buttonNum);
}

void toggleLights(int mappingId){
  String url = "";
  if(mappingId == 1){
    //ca08b1b5d3682ca4173ddce02fa9428f living room group id
    url = "https://api.lifx.com/v1/lights/group_id:ca08b1b5d3682ca4173ddce02fa9428f/toggle";
  }
  if(mappingId == 2){
    //d86764ff57e63363f9a1aa170f44ab40 bedroom group id
    url = "https://api.lifx.com/v1/lights/group_id:d86764ff57e63363f9a1aa170f44ab40/toggle";
  }


  USE_SERIAL.print("[HTTP] begin...\n");
  http.begin(url, "3B 6D ED 88 25 FD D1 B1 5F 46 38 78 1A 59 83 08 5D 5E 42 D9"); //HTTPS
  http.addHeader("Authorization", "Bearer c786d55da9d454735cf6e3a5c06dff612afdab4ba48473d09c700f62f7aae435");
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
  ArduinoOTA.handle();
  if(WiFi.status()== WL_CONNECTED){ //Check WiFi connection status
    newVal = digitalRead(inputPin);     // read the input pin
    newVal2 = digitalRead(inputPin2);
    if(newVal != val){
      val = newVal;
      if((millis() - lastDebounceTime) > buttonInterval && val == 0){
        // do the thing
        lastDebounceTime = millis();
         buttonPressed(1, val);
      }
    }
    if(newVal2 != val2){
      val2 = newVal2;
      if((millis() - lastDebounceTime2) > buttonInterval && val2 == 0){
        lastDebounceTime2 = millis();
        // do the thing
        buttonPressed(2, val2);
      }
    }
  }else{
    Serial.print("Error in Wifi connection");
  }
}