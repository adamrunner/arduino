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
// NOTE: READ_TEMP_INTERVAL and SEND_TEMP_INTERVAL come from WifiCreds.h
const int LED_PIN = 0;
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
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.println("Starting...");
  delay(1000);
  digitalWrite(LED_PIN, HIGH);
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
  if (currentMillis - previousReadMillis >= READ_TEMP_INTERVAL) {
    previousReadMillis = currentMillis;
    return true;
  }else{
    return false;
  }
}

bool shouldSendUpdate(){
  unsigned long currentMillis = millis();
  if (currentMillis - previousSendMillis >= SEND_TEMP_INTERVAL) {
    previousSendMillis = currentMillis;
    return true;
  }else{
    return false;
  }
}

void loop() {
  if(shouldReadTemp()){
    temp     = getTemp();
    humidity = getHumidity();
  }
  if(shouldSendUpdate()){
    sendUpdate(temp, humidity, LED_PIN);
  }
  // Call the various helper functions to run the app
}
