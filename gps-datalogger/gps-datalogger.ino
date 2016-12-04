#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
//SD Card connected to pin 10
const int chipSelect = 10;
TinyGPS gps;
//GPS connected to pins 9/8
SoftwareSerial ss(9, 8);

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  ss.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
}

static String get_date(TinyGPS &gps)
{
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE)
    Serial.print("********** ******** ");
  else
  {
    //2016-10-03T22:51:10-07:00
    //ISO8601, UTC output from GPS
    char sz[32];
    sprintf(sz, "%02d/%02d/%02dT%02d:%02d:%02d+00:00",
        year, month, day, hour, minute, second);
//    Serial.println(sz);
    return sz;
  }
}

void loop() {
  bool newData = false;
  long lat, lon;
  unsigned long fix_age, time, date, speed, course;

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }  
  
  
  // make a string for assembling the data to log:
   
  // retrieves +/- lat/long in 100000ths of a degree
  gps.get_position(&lat, &lon, &fix_age);
   
  // time in hhmmsscc, date in ddmmyy
  gps.get_datetime(&date, &time, &fix_age);
   
  // returns speed in 100ths of a knot
  speed = gps.speed();
   
  // course in 100ths of a degree
  course = gps.course();
  String dataString = get_date(gps);
//  char* dataString2 = "";
//  String dataString2 = "";
//  sprintf(dataString2, "%c, %c, %d, %d", lat, lon, speed, course);
  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
//    Serial.println(dataString2);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
}









