float tempC;
int reading;
int tempPin = 1;

void setup()
{
//  analogReference(INTERNAL);
  Serial.begin(9600);
}

void loop()
{
//  reading = analogRead(tempPin);
//  tempC = reading / 9.31;
  tempC = (5.0 * analogRead(tempPin) * 100.0) / 1024;

  Serial.println(tempC);
  delay(1000);
}
