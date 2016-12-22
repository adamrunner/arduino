/*
 ESP8266 MQTT Temperature Sensor

 It connects to an MQTT server then:
  - publishes the current Temperature and Humidtiy read from the DHT22 sensor to the topic "temperature" every 10 seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WifiCreds.h>
const char* mqtt_server = "192.168.1.90";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  // Serial.print("Chip ID: ");
  // Serial.println(ESP.getChipId());
  Serial.print("Hostname: ");
  Serial.println(WiFi.hostname());
}

void setup_wifi() {
  WiFi.mode(WIFI_STA);
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
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
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}


void reconnect() {
  char hostname[8];
  WiFi.hostname().toCharArray(hostname, 8);
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(hostname)) {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//TODO: Still not working - determine solution to pass the thing (a char array)
// To a function and then have the function modify it

void prepData(char msg, float h, float f){
  String dataString = "";
  char buffer[16];
  // char message[50];
  String stringH = dtostrf(h,6,2,buffer);
  dataString = stringH;
  dataString += ",";
  String stringF = dtostrf(f,6,2,buffer);
  dataString += stringF;
  dataString.toCharArray(msg, 50);
}
// void prepData(char &msg, float h, float f);

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;
    float humidity    = 75.25;
    float temperature = 75.25;
    prepData(&msg, temperature, humidity);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("temperature", msg);
  }
}
