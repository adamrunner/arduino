/*
* MQTT Client
* Reads temp from a DS18B20 Sensor
* Sends the temp to MQTT Topic
* Deep Sleeps indefinitely until the RTC wakes it up (5 min)
*/

#include <Wire.h>         //http://arduino.cc/en/Reference/Wire (included with Arduino IDE)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <WifiCreds.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2
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

void setup() {
  pinMode(D0, INPUT_PULLUP);
  Serial.begin(115200);

  // ALM1_EVERY_SECOND -- causes an alarm once per second.
  // ALM1_MATCH_SECONDS -- causes an alarm when the seconds match (i.e. once per minute).
  // ALM1_MATCH_MINUTES -- causes an alarm when the minutes and seconds match.
  // ALM1_MATCH_HOURS -- causes an alarm when the hours and minutes and seconds match.
  // ALM1_MATCH_DATE -- causes an alarm when the date of the month and hours and minutes and seconds match.
  // ALM1_MATCH_DAY -- causes an alarm when the day of the week and hours and minutes and seconds match.

  // configureAlarm();

  // RTC.alarmInterrupt(ALARM_1, true);
  // RTC.alarmInterrupt(ALARM_2, false);

  // if ( RTC.alarm(ALARM_1) ) {     //has Alarm1 triggered?
  //   Serial.println(RTC.get());
  //   Serial.println("Alarm Triggered!");
  // }
  // else {
  //     //no alarm
  // }

  Serial.begin(115200);
  setup_wifi();
  getTemp();
  client.setServer(TEMP_SERVER, MQTT_PORT);
  // TODO: Don't set a callback - we're not subscribing
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

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(currentHostname)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void connectAndUpdate() {
  reconnect();
  // TODO: Is this loop needed if we're not subscribing?
  client.loop();

  getTemp();
  char tempChar[7];
  dtostrf(temp,4,2,tempChar);
  sprintf(msg, "%s,%s", currentHostname, tempChar);
  // dtostrf(FLOAT,WIDTH,PRECSISION,BUFFER);

  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish("outTopic", msg);
}
void goToSleep() {
  Serial.println("GOING TO SLEEP!");
  // client.stop();
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  // delay(100);
  ESP.deepSleep(5 * 60 * 1000 * 1000);
}
void loop() {
  // Call other functions to send / update
  getTemp();
  connectAndUpdate();
  goToSleep();
}
