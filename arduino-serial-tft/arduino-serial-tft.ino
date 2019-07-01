// *** SPFD5408 change -- Begin
#include <SPFD5408_Adafruit_GFX.h>    // Core graphics library
#include <SPFD5408_Adafruit_TFTLCD.h> // Hardware-specific library
#include <SPFD5408_TouchScreen.h>
// *** SPFD5408 change -- End

#if defined(__SAM3X8E__)
    #undef __FlashStringHelper::F(string_literal)
    #define F(string_literal) string_literal
#endif

#define YP A1  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 7   // can be a digital pin
#define XP 6   // can be a digital pin

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
// optional
#define LCD_RESET A4

// Assign human-readable names to some common 16-bit color values:
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF


Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define BOXSIZE 40

int oldcolor, currentcolor;

void setup(void) {
  Serial.begin(9600);
  Serial.println(F("Paint!"));

  tft.reset();

  tft.begin(0x9341); // SDFP5408

  tft.setRotation(0); // Need for the Mega, please changed for your choice or rotation initial

  // Border

  // drawBorder();

  // Initial screen

  // tft.setCursor (55, 50);
  // tft.setTextSize (3);
  // tft.setTextColor(RED);
  // tft.println("SPFD5408");
  // tft.setCursor (65, 85);
  // tft.println("Library");
  // tft.setCursor (55, 150);
  // tft.setTextSize (2);
  // tft.setTextColor(BLACK);
  // tft.println("TFT Paint");
  //
  // tft.setCursor (80, 250);
  // tft.setTextSize (1);
  // tft.setTextColor(BLACK);

  // Paint
  //
  tft.fillScreen(BLACK);
  //
  // tft.fillRect(0, 0, BOXSIZE, BOXSIZE, RED);
  // tft.fillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, YELLOW);
  // tft.fillRect(BOXSIZE*2, 0, BOXSIZE, BOXSIZE, GREEN);
  // tft.fillRect(BOXSIZE*3, 0, BOXSIZE, BOXSIZE, CYAN);
  // tft.fillRect(BOXSIZE*4, 0, BOXSIZE, BOXSIZE, BLUE);
  // tft.fillRect(BOXSIZE*5, 0, BOXSIZE, BOXSIZE, MAGENTA);
  // // tft.fillRect(BOXSIZE*6, 0, BOXSIZE, BOXSIZE, WHITE);
  //
  // tft.drawRect(0, 0, BOXSIZE, BOXSIZE, WHITE);
  currentcolor = RED;

  pinMode(13, OUTPUT);
}

#define MINPRESSURE 10
#define MAXPRESSURE 1000
const char terminator = ";";

void loop()
{
  String serialText = Serial.readStringUntil(terminator);
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.println(serialText);
}

// void drawBorder () {
//
//   // Draw a border
//
//   uint16_t width = tft.width() - 1;
//   uint16_t height = tft.height() - 1;
//   uint8_t border = 10;
//
//   tft.fillScreen(RED);
//   tft.fillRect(border, border, (width - border * 2), (height - border * 2), WHITE);
//
// }
