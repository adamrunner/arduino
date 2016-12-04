void loop() {
// Loop code for a LCD with buttons
  if(lcd.buttonPressed() == KEYPAD_SELECT){
    nextLcdPage();
  }

  if(lcdPage == 1){
    updateTempLCD(currentHumidity, currentTemp);
  }else if(lcdPage == 2) {
    // print the LED info on the display
    // updateLEDInfoLCD(led_1_value, led_2_value, led_3_value, led_4_value);
    // Pseudo Code - replace variables with
    if(lcd.buttonPressed() == KEYPAD_UP){
      upLeds();
      // brighten ALL leds
    }

    if(lcd.buttonPressed() == KEYPAD_DOWN){
      downLeds();
      // dim ALL leds
    }
  }
}

void updateTempLCD(char currentHumidity, char currentTemp){
  int row = 1;
  lcd.setCursor(0, row);
  lcd.print(currentTemp);
  // assumes we're moving over 4 digits... we should base this off of the value of currentTemp
  lcd.setCursor(4, row);
  lcd.print("ºF");
  //
  row = 2;
  lcd.setCursor(0, row);
  lcd.print(currentHumidity);
  lcd.setCursor(4, row);
  lcd.print("% RH");
}
