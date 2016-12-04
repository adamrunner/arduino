/*
 * Arduino 1.6.6
 * Sketch uses 237,464 bytes (45%) of program storage space. Maximum is 524,288 bytes.
 */


#include <ESP8266WiFi.h>

const char* ssid     = MY_SSID;
const char* password =MY_PASSWORD;
// const char* host = "utcnist2.colorado.edu";
const char* host = "128.138.141.172";

int ln = 0;
String TimeDate = "";


void setup() {
  Serial.begin(115200);                   // diagnostic channel
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  delay(100);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

}

void loop()
{
  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 13;

  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // This will send the request to the server
  client.print("HEAD / HTTP/1.1\r\nAccept: */*\r\nUser-Agent: Mozilla/4.0 (compatible; ESP8266 NodeMcu Lua;)\r\n\r\n");

  delay(100);

  // Read all the lines of the reply from server and print them to Serial
  // expected line is like : Date: Thu, 01 Jan 2015 22:00:14 GMT
  char buffer[12];
  String dateTime = "";

  while(client.available())
  {
    String line = client.readStringUntil('\r');

    if (line.indexOf("Date") != -1)
    {
      Serial.print("=====>");
    } else
    {
      // Serial.print(line);
      // date starts at pos 7
      TimeDate = line.substring(7);
      Serial.println(TimeDate);
      // time starts at pos 14
      TimeDate = line.substring(7, 15);
      TimeDate.toCharArray(buffer, 10);
      TimeDate = line.substring(16, 24);
      TimeDate.toCharArray(buffer, 10);
    }
  }

  Serial.println();
  Serial.println("closing connection");
  delay(60000);
}
