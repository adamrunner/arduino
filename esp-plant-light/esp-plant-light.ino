#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WifiCreds.h>
const int led = 2;

ESP8266WebServer server(80);
String form = "<form action='led'><input type='radio' name='state' value='1' checked>On<input type='radio' name='state' value='0'>Off<input type='submit' value='Submit'></form>";

void handleLed() {
  // get the value of request argument "state" and convert it to an int
  int state = server.arg("state").toInt();

  digitalWrite(led, state);
  server.send(200, "text/html", String("<p>LED is now ") + ((state)?"on":"off") + "</p>" + form);
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

void setup(void){
  Serial.begin(115200);
  WiFi.begin(MY_SSID, MY_PASSWORD);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(MY_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  server.on("/led", handleLed);
  server.on("/", [](){
   server.send(200, "text/html", form);
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();
}
