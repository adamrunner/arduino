/*
 *  This sketch sends data via HTTP GET requests to my local web service.
 *
 */

#include <ESP8266WiFi.h>
#include <DHT.h>
#include <WifiCreds.h>

#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

const char* ssid              = MY_SSID;
const char* password          = MY_PASSWORD;
const char* sensor_id         = "1";
const char* host              = TEMP_SERVER;
const long sendUpdateInterval = SEND_TEMP_INTERVAL;
const long readTempInterval   = READ_TEMP_INTERVAL;

// TODO: swap the host out with a remote host
// TODO: use HTTPS
// TODO: Verify the certificate is legitimate
unsigned long previousReadMillis = 0;
unsigned long previousSendMillis = 0;
float temp = 0.0;
float humidity = 0.0;

WiFiClient client;
void setup() {
  Serial.begin(115200);
  delay(20);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("Starting...");
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);
  dht.begin();
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
  temp = dht.readTemperature(true);
  Serial.print("Current Temp: ");
  Serial.print(temp);
  Serial.println("ºF");
  return temp;
}

float getHumidity() {
  humidity = dht.readHumidity();
  Serial.print("Current Humidity: ");
  Serial.print(humidity);
  Serial.println("% RH");
}

bool shouldReadTemp(){
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
  url += "&humidity=";
  url += humidity;

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
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println();
  Serial.println("closing connection");
}

void loop() {
  if(shouldReadTemp()){
    getTemp();
    getHumidity();
  }
  if(shouldSendUpdate()){
    sendUpdate();
  }
  // Call the various helper functions to run the app
}

