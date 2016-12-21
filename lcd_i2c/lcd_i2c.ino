#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <WifiCreds.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

#define DHTPIN 0
#define DHTTYPE DHT22
WiFiClient client;

const char sensor_id[2] = "1";
unsigned long previousReadMillis = 0;
unsigned long previousSendMillis = 0;
unsigned long previousApiReadMillis = 0;
char tempFromApi[530];
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
HTTPClient http;
DHT dht(DHTPIN, DHTTYPE);
struct TempData {
  char temp_f[5];
  char epoch_time[12];
  char humid[3];
};
float temp = 0.0;
float humidity = 0.0;
const int LED_PIN = LED_BUILTIN;
bool shouldReadTemp(){
  //TODO: Abstract this to a self contained function in a header file
  //TODO: can we make this function more abstract and reusable
  //TODO: Learn about passing pointers to global variables in functions in C / Arduino
  unsigned long currentMillis = millis();
  if (currentMillis - previousReadMillis >= READ_TEMP_INTERVAL || previousReadMillis == 0) {
    previousReadMillis = currentMillis;
    return true;
  }else{
    return false;
  }
}
bool shouldGetTempFromApi(){
  unsigned long currentMillis = millis();
  if (currentMillis - previousApiReadMillis >= READ_TEMP_API_INTERVAL || previousApiReadMillis == 0) {
    previousApiReadMillis = currentMillis;
    return true;
  }else{
    return false;
  }
}

bool shouldSendUpdate(){
  unsigned long currentMillis = millis();
  if (currentMillis - previousSendMillis >= SEND_TEMP_INTERVAL || previousSendMillis == 0) {
    previousSendMillis = currentMillis;
    return true;
  }else{
    return false;
  }
}
bool parseTempData(String content, struct TempData* tempData) {
  // Allocate a temporary memory pool on the stack
  // const int BUFFER_SIZE = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(8) + JSON_OBJECT_SIZE(19);
  // StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(content);

  if (!root.success()) {
    Serial.println("JSON parsing failed!");
    return false;
  }

  // Here were copy the strings we're interested in
  strcpy(tempData->temp_f, root["current"]["temp_f"]);
  strcpy(tempData->epoch_time, root["location"]["localtime_epoch"]);
  strcpy(tempData->humid, root["current"]["humidity"]);
  // It's not mandatory to make a copy, you could just use the pointers
  // Since, they are pointing inside the "content" buffer, so you need to make
  // sure it's still in memory when you read the string

  return true;
}

String readTempFromApi(){
  const char forecastAPI[82] = "http://api.apixu.com/v1/current.json?key=a31548a7f00741d580364340160712&q=97239";
  String payload;
  http.begin(forecastAPI);
  int httpCode = http.GET();
  if(httpCode == HTTP_CODE_OK) {
    payload = http.getString();
    // Serial.println(payload);
  }else{
    Serial.println("Invalid response from server");
    Serial.print("HTTP CODE: ");
    Serial.println(httpCode);
    payload = "";
  }
  http.end();
  return payload;
}

void printTempData(const struct TempData* tempData){
  lcd.setCursor(0,2);
  lcd.print("Outside Temp: ");
  lcd.print(tempData->temp_f);
  lcd.print("F");
  lcd.setCursor(0,3);
  lcd.print("Outside Humid: ");
  lcd.print(tempData->humid);
  lcd.print("%");
  // Serial.println("");
  Serial.print("Outside Temp: ");
  Serial.print(tempData->temp_f);
  Serial.println("ºF");
  // Serial.print("Timestamp: ");
  // Serial.println(tempData->epoch_time);
}

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  // activate LCD module
  lcd.begin (20,4);
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home(); // set cursor to 0,0
  lcd.print("Connecting");
  Serial.println("");
  Serial.println("Connecting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(MY_SSID, MY_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    lcd.setCursor(10, 0);
    for(int i=0;i<4;i++){
      Serial.print(".");
      lcd.print(".");
      delay(500);
    }
    lcd.setCursor(10,0);
    for(int i=0;i<4;i++){
      Serial.print(".");
      lcd.print(" ");
      delay(500);
    }
  }

  lcd.clear();
  lcd.print("SSID: ");
  lcd.print(MY_SSID);
  lcd.setCursor(0,1);
  lcd.print("IP: ");
  lcd.print(WiFi.localIP());
  delay(2000);
  lcd.clear();

}

float getTemp(){
  float temp = dht.readTemperature(true);
  Serial.print("Current Indoor Temp: ");
  Serial.print(temp);
  Serial.println("ºF");
  return temp;
}

float getHumidity() {
  float humidity = dht.readHumidity();
  Serial.print("Current Indoor Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
  return humidity;
}
void updateLCD(float temp, float humidity){
  lcd.setCursor(0,0);
  lcd.print("Indoor Temp: ");
  lcd.print(temp);
  lcd.print("F");
  lcd.setCursor(0, 1);
  lcd.print("Indoor Hum: ");
  lcd.print(humidity);
  lcd.print("%");
}
void sendUpdate(float temp, float humidity, int led_pin){
  digitalWrite(led_pin, LOW);
  const int httpPort = 9292;
  if (!client.connect(TEMP_SERVER, httpPort)) {
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
               "Host: " + TEMP_SERVER + "\r\n" +
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
  digitalWrite(led_pin, HIGH);
  Serial.println();
  Serial.println("closing connection");
}

void loop()
{
  if(shouldReadTemp()){
    temp     = getTemp();
    humidity = getHumidity();
    updateLCD(temp, humidity);
  }
  if(shouldSendUpdate()){
    sendUpdate(temp, humidity, LED_PIN);
  }
  if(shouldGetTempFromApi()){
    String apiResponse = readTempFromApi();
    TempData tempData;
    //TODO: in the parse data, when we get the local epoch_time - update the system time
    if(parseTempData(apiResponse, &tempData)){
      printTempData(&tempData);
    }
  }
}
