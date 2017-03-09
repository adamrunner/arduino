/************************************/
 int RedLED_out = D7;
 int GreenLED_out = D8;
 int BlueLED_out = D6;

void setup()
{
 pinMode (RedLED_out, OUTPUT);
 pinMode (GreenLED_out, OUTPUT);
 pinMode (BlueLED_out, OUTPUT);
 digitalWrite(RedLED_out, LOW);
 digitalWrite(GreenLED_out, LOW);
 digitalWrite(BlueLED_out, LOW);
}
void loop()
{
 digitalWrite(RedLED_out, HIGH);
 digitalWrite(GreenLED_out, LOW);
 digitalWrite(BlueLED_out, LOW);
 delay(500);
 digitalWrite(RedLED_out, LOW);
 digitalWrite(GreenLED_out, HIGH);
 digitalWrite(BlueLED_out, LOW);
 delay(500);
 digitalWrite(RedLED_out, LOW);
 digitalWrite(GreenLED_out, LOW);
 digitalWrite(BlueLED_out, HIGH);
 delay(500);
} 
