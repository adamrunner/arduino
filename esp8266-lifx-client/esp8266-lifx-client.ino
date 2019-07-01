//INCASE SOMEONE IN FUTURE WANTS, THIS IS THE WORKING CODE TO SEND JSON DATA TO HTTPS API USING ESP8266 (SSL)

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

void setup() {

  Serial.begin(115200);
  WiFi.begin("home-network", "tacocat1234");

  while (WiFi.status() != WL_CONNECTED)

  delay(500);
  Serial.println("Waiting for connection");

}

void loop() {

  if(WiFi.status()== WL_CONNECTED){ //Check WiFi connection status

    HTTPClient http;
    http.begin("https://api.lifx.com/v1/lights/all/toggle" , "E3 69 05 13 32 74 C0 37 F8 6C B8 A7 18 98 87 B7 CD DD 86 F0");
    http.addHeader("Authorization", "Bearer c786d55da9d454735cf6e3a5c06dff612afdab4ba48473d09c700f62f7aae435");
    http.addHeader("Content-Type", "application/json");
    String postMessage = "{}";
    int httpCode = http.POST(postMessage);
    Serial.print("http result:");
    Serial.println(httpCode);
    http.writeToStream(&Serial);
    String payload = http.getString();
    http.end();

  }else{

    Serial.print("Error in Wifi connection");

  }

  delay(30000); //Send a request every 30 seconds

}
