#include <Adafruit_NeoPixel.h>
#define PIN            5
#define NUMPIXELS      240
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
// pixels.setPixelColor(0, pixels.Color(20,0,0));
// pixels.show(); 
// NOTE: you must call pixels.show() - when you want to redaw the pixels
int maxColor = 255;

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel((i+j) & 255));
    }
    pixels.show();
    delay(wait);
  }
}

void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));
    }
    pixels.show();
    delay(wait);
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < pixels.numPixels(); i=i+3) {
        pixels.setPixelColor(i+q, c);    //turn every third pixel on
      }
      pixels.show();

      delay(wait);

      for (uint16_t i=0; i < pixels.numPixels(); i=i+3) {
        pixels.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < pixels.numPixels(); i=i+3) {
        pixels.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      pixels.show();

      delay(wait);

      for (uint16_t i=0; i < pixels.numPixels(); i=i+3) {
        pixels.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<pixels.numPixels(); i++) {
    pixels.setPixelColor(i, c);
    pixels.show();
    delay(wait);
  }
}

void greenChaseToBlue() {
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, pixels.Color(0,40,0));
    if(i > 0){
      pixels.setPixelColor(i-1, pixels.Color(0,0,0));
    }
    pixels.show();
    delay(50);
  }
  for(int i=(NUMPIXELS-1);i>-1;i--){
    pixels.setPixelColor(i, pixels.Color(0,0,40));
    if(i < (NUMPIXELS-1)){
      pixels.setPixelColor(i+1, pixels.Color(0,0,0));
    }
    pixels.show();
    delay(50);
  }  
}

void greenFadeOut(){
  // Start at 0 LED
  // Each LED reduce the brightness by 10
  
  for(int brightness=200;brightness >=0; brightness=brightness-10){
    for(int led=0;led<NUMPIXELS;led++){
      pixels.setPixelColor(led, pixels.Color(0,brightness,0));  
    }
  }
}
//int brightnessLevels[20];
void setup() {
  pixels.begin();
  pixels.show();
}

void loop() {
}

