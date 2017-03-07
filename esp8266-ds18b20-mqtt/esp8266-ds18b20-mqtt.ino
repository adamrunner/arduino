/*
  MQTT Temperature Client
  Hardware Setup:
  ESP8266 running from 5V standby of XBOX PSU
  DS18B20 Connected on Pin 2 / D4
  XBOX PS_ON Connected on D5 - set to "HIGH" to turn on 12V supply
  Water Pump controlled by MOSFET on D7
  Water Pump power comes from +5VSB
  MOSFET for LED on D6

  OTA Uploads - default port of 8266
  TEMP_SERVER - server IP
  MQTT_PORT   - default of 1883

  Reads temperature every 5m and posts it to the "outTopic" topic on the MQTT Server
    - message has this format HOSTNAME,TEMP_F (e.g. ESP_299021,36.84)
    - HOSTNAME is used to identify each sensor uniquely
    - HOSTNAME is based off of the MAC addr of each sensor
  Subscribes to the PUMP topic
    - a 1 written to the PUMP topic will turn on the water pump
    - program monitors the amount of time that pump has been running
    - shuts pump off after RUN_PUMP_LENGTH (in ms) has elapsed
  Subscribes to the LED topic
    - casts values to int
    - writes the int to an analog signal on LED Pin (D6), values are 0-1024
  Subscribes to the PSU topic
    - 0 turns off PSU
    - 1 turns on PSU
  Subscribes to the TEMP_REQ topic
    - when a 1 is received
    - it sends an immediate update from the temperature sensor

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
#define ONE_WIRE_BUS D4
#define MQTT_PORT 1883
#define PUMP D7
#define PSU D6
#define LED D5
#define RUN_PUMP_LENGTH 240000 // amount of time to let the pump run in seconds
uint8_t MAC_array[6];
char MAC_char[18];

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
float temp   = 0.0;
char msg[50];
char currentHostname[14];
unsigned long pumpStartTime;
bool pumpIsRunning;


float getTemp(){
  sensors.requestTemperatures();
  temp = sensors.getTempFByIndex(0);
  Serial.print("Current Temp: ");
  Serial.print(temp);
  Serial.println("ºF");
  return temp;
}


void setup() {
  pinMode(LED, OUTPUT);     // Initialize the LED pin (D6) as an output
  pinMode(PSU, OUTPUT);     // Initialize the PSU pin (D5) as an output
  pinMode(PUMP, OUTPUT);     // D7 pin - Pump
  Serial.begin(115200);
  setup_wifi();
  client.setServer(TEMP_SERVER, MQTT_PORT);
  client.setCallback(callback);
  // Setup OTA Uploads
  ArduinoOTA.setPassword((const char *)"boarding");

  ArduinoOTA.onStart([]() {
    Serial.println("STARTING OTA UPDATE");
    char updateMessage[36]= "OTA UPDATE START - ";
    strcat(updateMessage, currentHostname);
    client.publish("debugMessages", updateMessage);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nCOMPLETED OTA UPDATE");
    char updateMessage[37]= "OTA UPDATE FINISH - ";
    strcat(updateMessage, currentHostname);
    client.publish("debugMessages", updateMessage);
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

  // TODO: Clean up logic - possibly by using a case statement?

  if (strcmp(topic, "TEMP_REQ") == 0){
    if ((char)payload[0] == '1') {
      Serial.println("Temperature update requested!");
      sendTempUpdate();
    }
  }

  if(strcmp(topic, "PUMP") == 0){
    if(pumpIsRunning == false && (char)payload[0] == '1'){
      startPump();
    }
    if((char)payload[0] == '0'){
      stopPump();
    }
  }

  if (strcmp(topic, "LED") == 0 ){
    char ledMessage[12]= "LED: ";
    strcat(ledMessage, payloadS);
    unsigned int ledValue = atoi(payloadS);
    client.publish("outTopic", ledMessage);
    Serial.print("Setting LED to: ");
    Serial.println((int) ledValue);
    analogWrite(LED, (int) ledValue);
  }

  if( strcmp(topic, "PSU") == 0){
    if ((char)payload[0] == '1') {
      Serial.print("Turning power supply ON!");
      client.publish("outTopic", "PSU ON");
      digitalWrite(PSU, HIGH);
    }else{
      Serial.print("Turning power supply OFF!");
      client.publish("outTopic", "PSU OFF");
      digitalWrite(PSU, LOW);
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
      client.subscribe("LED");
      client.subscribe("PSU");
      client.subscribe("PUMP");
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
  // Convert float to char array
  // dtostrf(FLOAT,WIDTH,PRECSISION,BUFFER);
  dtostrf(temp,4,2,tempChar);
  // Format msg
  // TODO: move msg to a local variable? Unneeded outside of this scope.
  sprintf(msg, "%s,%s", currentHostname, tempChar);

  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish("outTopic", msg);
}

void startPump() {
  pumpIsRunning = true;
  pumpStartTime = millis();
  client.publish("outTopic", "Starting Pump");
  digitalWrite(PUMP, HIGH);
}

void stopPump() {
  pumpIsRunning = false;
  client.publish("outTopic", "Stopping Pump");
  digitalWrite(PUMP, LOW);
}
//TODO: Send state update
//TODO: state update should send PSU, PUMP, TEMP, and other messages. 

void loop() {
  ArduinoOTA.handle();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - pumpStartTime > RUN_PUMP_LENGTH && pumpIsRunning){
    stopPump();
  }
  if (now - lastMsg > SEND_TEMP_INTERVAL) {
    lastMsg = now;
    sendTempUpdate();
  }
}
