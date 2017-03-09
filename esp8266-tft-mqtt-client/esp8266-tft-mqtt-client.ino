#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <WifiCreds.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#define MQTT_PORT 1883
uint8_t MAC_array[6];
char MAC_char[18];

// Configure pins for TFT SPI
#define TFT_CS D4
#define TFT_DC D2
#define TFT_MOSI D8
#define TFT_CLK D7
#define TFT_RST D3
#define TFT_MISO D6
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

WiFiClient espClient;
PubSubClient client(espClient);

char currentHostname[14];

void setup() {
  Serial.begin(115200);
    pinMode(D1, OUTPUT);
    digitalWrite(D1, HIGH);


    tft.begin();
    tft.setCursor(0, 0);
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(2);
  setup_wifi();
  client.setServer(TEMP_SERVER, MQTT_PORT);
  client.setCallback(callback);
  // Setup OTA Uploads
  ArduinoOTA.setPassword((const char *)"boarding");

  ArduinoOTA.onStart([]() {
    Serial.println("STARTING OTA UPDATE");
    tft.fillScreen(ILI9341_BLACK);
    tft.println("STARTING OTA UPDATE");
    char updateMessage[36]= "OTA UPDATE START - ";
    strcat(updateMessage, currentHostname);
    client.publish("debugMessages", updateMessage);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nCOMPLETED OTA UPDATE");
    tft.println("\nCOMPLETED OTA UPDATE");
    char updateMessage[37]= "OTA UPDATE FINISH - ";
    strcat(updateMessage, currentHostname);
    client.publish("debugMessages", updateMessage);
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    tft.setCursor(0,0);
    tft.fillRect(0, 0, tft.width(), 20, ILI9341_BLACK);
    tft.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    tft.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

}

void setup_wifi() {

  delay(10);

  Serial.println();
  Serial.print("Connecting to ");
  WiFi.mode(WIFI_STA);
  Serial.println(MY_SSID);
  WiFi.begin(MY_SSID, MY_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    tft.print(".");
  }


  Serial.println("");
  tft.println("");
  Serial.println("WiFi connected");
  tft.println("WiFi connected");
  Serial.print("IP address: ");
  tft.println("IP address: ");
  Serial.println(WiFi.localIP());
  tft.println(WiFi.localIP());
  // Store the hostname to use later for MQTT ID
  String hostnameString = WiFi.hostname();
  hostnameString.toCharArray(currentHostname, 14);
  Serial.print("Hostname: ");
  Serial.println(WiFi.hostname());
}
int messagesReceived = 0;
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  messagesReceived++;
  Serial.println();
  payload[length] = '\0';
  char *payloadS = (char *) payload;
  String string = String((char *)payload);
  if (strcmp(topic, "outTopic") == 0){
    tft.println(string);
  }
  if(messagesReceived > 14){
    messagesReceived = 0;
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(0,0);
    tft.println("IP address: ");
    tft.println(WiFi.localIP());
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(currentHostname)) {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe("outTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void loop() {
  ArduinoOTA.handle();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
