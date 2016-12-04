/*
 *  This sketch sends data via HTTP GET requests to my local web service.
 *
 */

#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "WifiCreds.h"

// Data wire is plugged into port 2 on the ESP8266
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

const char* ssid     = MY_SSID;
const char* password = MY_PASSWORD;
const char* sensor_id = "2";

// TODO: swap the host out with a remote host
// TODO: use HTTPS
// TODO: Verify the certificate is legitimate
const char* host = "192.168.1.90";
// send a temperature update every 5 minutes (300 seconds, 300k ms)
const long pollTempInterval = 300000;
const long readTempInterval = 5000;
unsigned long previousReadMillis = 0;
unsigned long previousSendMillis = 0;
float temp = 0.0;

WiFiClient client;
void setup() {
  Serial.begin(115200);
  delay(20);
  sensors.begin();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

float getTemp(){
  temp = sensors.getTempFByIndex(0);
  Serial.print("Current Temp: ");
  Serial.print(temp);
  Serial.println("ºF");
  return temp;
}


bool shouldReadTemp (){
  //TODO: can we make this function more abstract and reusable
  //TODO: Learn about passing pointers to global variables in functions in C / Arduino
  unsigned long currentMillis = millis();
  if (currentMillis - previousReadMillis >= readTempInterval) {
    previousReadMillis = currentMillis;
    return true;
  }else{
    return false;
  }
}

bool shouldSendUpdate(){
  unsigned long currentMillis = millis();
  if (currentMillis - previousSendMillis >= sendUpdateInterval) {
    previousSendMillis = currentMillis;
    return true;
  }else{
    return false;
  }
}


void sendUpdate(){
  digitalWrite(LED_BUILTIN, LOW);
  const int httpPort = 9292;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request
  String url = "/temp?sensor_id=";
  url += sensor_id;
  url += "&temp=";
  url += temp;

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  // Verify that the server responds quickly enough!
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println();
  Serial.println("closing connection");
}

void loop() {
  if(shouldReadTemp()){
    getTemp();
  }
  if(shouldSendUpdate()){
    sendUpdate();
  }
  // Call the various helper functions to run the app
}
