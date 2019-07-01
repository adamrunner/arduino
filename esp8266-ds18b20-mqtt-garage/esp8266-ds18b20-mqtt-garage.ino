/*
  MQTT Temperature Client
  DS18B20 Connected on Pin 2 / D4
  OTA Uploads - default port of 8266
  TEMP_SERVER - server IP
  MQTT_PORT   - default of 1883

  Reads temperature every 5m and posts it to the "outTopic" topic on the MQTT Server
  Subscribes to the LED topic
    - casts values to int
    - writes the int to an analog signal on D1
  Subscribes to the PSU topic
    - 0 turns off PSU
    - 1 turns on PSU
  Subscribes to the TEMP_REQ topic
    - when a 1 is received
    - it sends an immediate from the temperature sensor

  TODO: Give the topics better names - inTopic / outTopic are awful
*/

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <WifiCreds.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS D3
#define MQTT_PORT 1883
uint8_t MAC_array[6];
char MAC_char[18];

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
float temp = 0.0;
char currentHostname[14];


float getTemp(){
  sensors.requestTemperatures();
  temp = sensors.getTempFByIndex(0);
  Serial.print("Current Temp: ");
  Serial.print(temp);
  Serial.println("ºF");
  return temp;
}

float getAnalogTemp(){
 // read the value from LM35.
  // read 10 values for averaging.
  int val = 0;
  for(int i = 0; i < 10; i++) {
      val += analogRead(A0);   
      delay(100);
  }

  // convert to temp:
  // temp value is in 0-1023 range
  // LM35 outputs 10mV/degree C. ie, 1 Volt => 100 degrees C
  // So Temp = (avg_val/1023)*5 Volts * 100 degrees/Volt
  temp = val*50.0f/1023.0f;
  temp = temp * 1.8 + 32;
  // convert to string
  char buf[16];
  String strTemp = dtostrf(temp, 4, 1, buf);
  Serial.print("Current Temp: ");
  Serial.print(temp);
  Serial.println("ºF");
  return temp;  
}

void setup() {
  pinMode(D1, OUTPUT);     // Initialize the D1 pin as an output
  pinMode(D2, OUTPUT);     // Initialize the D2 pin as an output
  Serial.begin(115200);
  setup_wifi();
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
  getTemp();
  client.setServer(TEMP_SERVER, MQTT_PORT);
  client.setCallback(callback);
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
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  // Store the hostname to use later for MQTT ID
  String hostnameString = WiFi.hostname();
  hostnameString.toCharArray(currentHostname, 14);
  Serial.print("Hostname: ");
  Serial.println(WiFi.hostname());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  payload[length] = '\0';
  char *payloadS = (char *) payload;


  // TODO: Handle different topics - case statement?
  // if topic == LED
  // if topic == PSU
  // else
  // Switch on the LED if an 1 was received as first character
  if (strcmp(topic, "TEMP_REQ") == 0){
    if ((char)payload[0] == '1') {
      Serial.println("Temperature update requested!");
      sendTempUpdate();
    }
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
      client.subscribe("TEMP_REQ");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void sendTempUpdate(){
  getTemp();
  char tempChar[7];
  // dtostrf(FLOAT,WIDTH,PRECSISION,BUFFER);
  dtostrf(temp,4,2,tempChar);
  sprintf(msg, "%s,%s", currentHostname, tempChar);

  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish("outTopic", msg);
}

void loop() {
  ArduinoOTA.handle();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > SEND_TEMP_INTERVAL) {
    lastMsg = now;
    sendTempUpdate();
  }
}
