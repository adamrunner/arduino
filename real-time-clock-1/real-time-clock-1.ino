/*
 * TimeRTC.pde
 * Example code illustrating Time library with Real Time Clock.
 * This example is identical to the example provided with the Time Library,
 * only the #include statement has been changed to include the DS3232RTC library.
 */

#include <DS3232RTC.h>    //http://github.com/JChristensen/DS3232RTC
#include <TimeLib.h>         //http://www.arduino.cc/playground/Code/Time  
#include <Wire.h>         //http://arduino.cc/en/Reference/Wire (included with Arduino IDE)
#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
void setup(void)
{
    // set up the lcd's number of columns and rows: 
    lcd.begin(16, 2);
    Serial.begin(9600);
    sensors.begin();
    setSyncProvider(RTC.get);   // the function to get the time from the RTC
    RTC.set(now());                     //set the RTC from the system time
    if(timeStatus() != timeSet) 
        Serial.println("Unable to sync with the RTC");
    else
        Serial.println("RTC has set the system time");      
}

void loop(void)
{
    temperatureDisplay();
    digitalClockDisplay();  
    delay(500);
}

void temperatureDisplay(void){
  sensors.requestTemperatures();
  lcd.setCursor(9,1);
  lcd.print(sensors.getTempFByIndex(0));
  lcd.print("\262F");
}

void digitalClockDisplay(void)
{
    lcd.setCursor(0, 1);
    // digital clock display of the time
    
    printDigits(hour());
    lcd.print(':');
    printDigits(minute());
    lcd.print(':');
    printDigits(second());

    lcd.print(' ');
   
    lcd.setCursor(0,0);
    printDigits(month()); 
    lcd.print('/');
    printDigits(day());
    lcd.print('/');
    lcd.print(year()); 
}

void printDigits(int digits)
{
    if(digits < 10)
        lcd.print('0');
    lcd.print(digits);
}
