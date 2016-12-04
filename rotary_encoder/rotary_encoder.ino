/* interrupt routine for Rotary Encoders


   The average rotary encoder has three pins, seen from front: A C B
   Clockwise rotation A(on)->B(on)->A(off)->B(off)
   CounterCW rotation B(on)->A(on)->B(off)->A(off)

   and may be a push switch with another two pins, pulled low at pin 8 in this case


*/
#include <Adafruit_NeoPixel.h>
#define PIN            5
#define NUMPIXELS      240
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// usually the rotary encoders three pins have the ground pin in the middle
enum PinAssignments {
  encoderPinA = 2,   // right (labeled DT on our decoder, yellow wire)
  encoderPinB = 3,   // left (labeled CLK on our decoder, green wire)
  clearButton = 8    // switch (labeled SW on our decoder, orange wire)
// connect the +5v and gnd appropriately
};

volatile int encoderPos = 0; // a counter for the dial
int lastReportedPos     = 1; // change management
static boolean rotating =false; // debounce management
int buttonState         = 0; // button index
// interrupt service routine vars
boolean A_set = false;
boolean B_set = false;
int bright    = 0;
int red       = 0;
int green     = 0;
int blue      = 0;

void setup() {

  pinMode(encoderPinA, INPUT_PULLUP); // new method of enabling pullups
  pinMode(encoderPinB, INPUT_PULLUP);
  pinMode(clearButton, INPUT_PULLUP);

// encoder pin on interrupt 0 (pin 2)
  attachInterrupt(0, doEncoderA, CHANGE);
// encoder pin on interrupt 1 (pin 3)
  attachInterrupt(1, doEncoderB, CHANGE);

  Serial.begin(9600);  // output
  pixels.begin();
  pixels.show();
}

// Interrupt on A changing state
void doEncoderA(){
  // debounce
  if ( rotating ) delay (1);  // wait a little until the bouncing is done

  // Test transition, did things really change?
  if( digitalRead(encoderPinA) != A_set ) {  // debounce once more
    A_set = !A_set;

    // adjust counter + if A leads B
    if ( A_set && !B_set )
      encoderPos += 1;
      if(encoderPos > NUMPIXELS - 1){
        encoderPos = 0;
      }
    rotating = false;  // no more debouncing until loop() hits again
  }
}

// Interrupt on B changing state, same as A above
void doEncoderB(){
  if ( rotating ) delay (1);
  if( digitalRead(encoderPinB) != B_set ) {
    B_set = !B_set;
    //  adjust counter - 1 if B leads A
    if( B_set && !A_set )
      encoderPos -= 1;
      if(encoderPos < 0){
        encoderPos = 239;
      }
    rotating = false;
  }
}

// main loop, work is done by interrupt service routines, this one only prints stuff
void loop() {
  rotating = true;  // reset the debouncer

  if (lastReportedPos != encoderPos) {
    Serial.print("Index:");
    Serial.println(encoderPos, DEC);
    pixels.setPixelColor(lastReportedPos, pixels.Color(0,0,0));
    lastReportedPos = encoderPos;
    pixels.setPixelColor(encoderPos, pixels.Color(0,0,40));
    pixels.show();

    // Respond to the push button - change state

  }
  if (digitalRead(clearButton) == LOW )  {
    Serial.print("Button Pressed");
  }
}
