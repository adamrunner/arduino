/***

Wifi Powered Data Logging Plant Growing Thing
Components
1 - DHT22 Temp / Humidity sensor
1 - ESP8266 ESP-12 Module
1 - OSEPP LCD keypad
4 - 10W Plant LEDs
4 - MOSFETs
1 - Constant Current power supply (for LEDs)

TBD:
2-3 PWM 80mm fans
1 - temp sensor for LED heatsinking
***/
#include <ESP8266WiFi.h>
#include <LiquidCrystal.h>
#include <DHT.h>
#include <ESP8266WebServer.h>
#define DHTPIN 5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
ESP8266WebServer server(80);
// Configure Wifi Settings
const char ssid =MY_SSID;
const char wpa  =MY_PASSWORD;

LCDKeypad lcd;

// init LED pins
// NOTE: replace numbers with actual pinouts
int led_1 = 1;
int led_2 = 2;
int led_3 = 3;
int led_4 = 4;
// LED PWM Values
unsigned int led_1_value = 0;
unsigned int led_2_value = 0;
unsigned int led_3_value = 0;
unsigned int led_4_value = 0;
int currentTemp;
int currentHumidity;
int lcdPage  = 1;
int maxPages = 2;
unsigned long previousMillisTemp = 0;
unsigned long pollTempInterval   = 10000;

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

bool updateInterval(){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisTemp >= pollTempInterval) {
    previousMillisTemp = currentMillis;
    return true;
  }else{
    return false;
  }
}

float getCurrentTemp(){
  return dht.readTemperature(true);
  //NOTE: Abstraction of whatever temperature sensor is being used
}

float getCurrentHumidity(){
  return dht.readHumidity();
  //NOTE: abstraction of the library to handle gathering humidity value
}



void sendUpdateToServer(int currentTemp, int currentHumidity){
  digitalWrite(LED_BUILTIN, HIGH);
  //TODO: send an update to http://192.168.1.90
  // Where the data will be stored temporarily and proxied to ElasticSearch
  digitalWrite(LED_BUILTIN, LOW);
}
void updateLEDInfoLCD(led_1_value, led_2_value, led_3_value, led_4_value){
  // Print the values to the screen
  lcd.setCursor(0, 1);
  lcd.print((led_1_value / 255 * 100));
  lcd.setCursor(0, 4);
  lcd.print("%");
  lcd.setCursor(12, 1);
  lcd.print((led_2_value / 255 * 100));
  lcd.print("%");
  lcd.setCursor(0, 2);
  lcd.print((led_3_value / 255 * 100));
  lcd.setCursor(0, 4);
  lcd.print("%");
  lcd.setCursor(12, 2);
  lcd.print((led_3_value / 255 * 100));
  lcd.print("%");
}

void downLeds(){
  if((led_1_value - 1) > -1  ){
    led_1_value = led_1_value - 1;
    led_2_value = led_2_value - 1;
    led_3_value = led_3_value - 1;
    led_4_value = led_4_value - 1;
    //TODO: abstract to function
    digitalWrite(led_1, led_1_value);
    digitalWrite(led_2, led_2_value);
    digitalWrite(led_3, led_3_value);
    digitalWrite(led_4, led_4_value);
  }
}

void upLeds(){
  if((led_1_value + 1) < 256) {
    led_1_value = led_1_value + 1;
    led_2_value = led_2_value + 1;
    led_3_value = led_3_value + 1;
    led_4_value = led_4_value + 1;
    //TODO: abstract to function
    digitalWrite(led_1, led_1_value);
    digitalWrite(led_2, led_2_value);
    digitalWrite(led_3, led_3_value);
    digitalWrite(led_4, led_4_value);
  }

}
void nextLcdPage() {
  if(lcdPage + 1 > maxPages) {
    lcdPage = 1;
  }else{
    lcdPage = lcdPage + 1;
  }
}

void setup() {
  pinMode(led_1, OUTPUT);
  pinMode(led_2, OUTPUT);
  pinMode(led_3, OUTPUT);
  pinMode(led_4, OUTPUT);
  Serial.begin(115200);
  dht.begin();
  setupWiFi();
  //TODO: add SD card to serve webpages
  //TODO: add events for HTTP control
  server.on("/led/down", downLeds);
  server.on("/led/up", upLeds);
  server.on("/led/off", offLeds);
  server.on("/led/on", onLeds);
  server.on("/", handleRoot);
  server.on("/temp", handleNumberOnly);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
  //TODO: determine how to abstract common function calls to other files
}


void loop() {
  //TODO: remove all LCD calls
  server.handleClient();
  // Always listen to button to visit next page


  if(updateInterval()){
    currentTemp     = getCurrentTemp();
    currentHumidity = getCurrentHumidity();
    sendUpdateToServer(currentTemp, currentHumidity);
  }


}
