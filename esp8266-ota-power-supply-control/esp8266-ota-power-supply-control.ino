#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Ticker.h>

const char* ssid = "home-network";
const char* password = "tacocat1234";
const int psu_pin = D2;
const int button_pin = D4;
const int psu_on_pin = D1;
const int led_output = D5;
volatile bool psuState = 0;
volatile bool currentPsuState = 0;
Ticker psuStateChecker;
Ticker psuStateSetter;
// attachInterrupt(pin, ISR(callback function), interrupt type/mode);
void setupOTA()
{

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
    psuStateChecker.detach();
    psuStateSetter.detach();
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
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

void setup()
{
  pinMode(led_output, OUTPUT); // front panel led
  pinMode(psu_pin, OUTPUT); // power supply PS_ON
  pinMode(button_pin, INPUT_PULLUP); // momentary button to ground
  pinMode(psu_on_pin, INPUT); // +5v when PSU is on
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  setupOTA();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  attachInterrupt(digitalPinToInterrupt(button_pin), buttonPressed, FALLING);
  psuStateChecker.attach(1.0, checkPsuState);
  psuStateSetter.attach(0.5, updatePsuState);
  digitalWrite(psu_pin, HIGH);
  digitalWrite(led_output, LOW);
}

void buttonPressed()
{
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 500)
  {
    Serial.print("Button Pressed: (ms)");
    Serial.println(millis());
  }
  last_interrupt_time = interrupt_time;
  psuState = !psuState;
}

void checkPsuState()
{
  currentPsuState = digitalRead(psu_on_pin);
  if(psuState != currentPsuState)
  {
    Serial.print("Power Supply is:");
    Serial.println(currentPsuState);
  }
}

void updatePsuState()
{
  if(psuState != currentPsuState)
  {
    Serial.print("Setting PSU to: ");
    Serial.println(psuState);
    if(psuState)
    {
      digitalWrite(psu_pin, LOW);
      digitalWrite(led_output, HIGH);
    }else{
      digitalWrite(psu_pin, HIGH);
      digitalWrite(led_output, LOW);
    }
  }
}

void loop()
{
  ArduinoOTA.handle();
}