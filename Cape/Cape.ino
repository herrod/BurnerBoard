#include "FastSPI_LED2.h"

///////////////////////////////////////////////////////////////////////////////////////////
//
// Have fun with a strip of LEDs!
//
// Derived from the FastSPI_LED2 FirstLight example
//
// Copyright 2013 - Steve Herrod, Richard McDougall, et al.
//
///////////////////////////////////////////////////////////////////////////////////////////

// How many leds are in the strip?
#define NUM_LEDS 50

// Data pin that led data will be written out over
#define DATA_PIN 6

// Clock pin only needed for SPI based chipsets when not using hardware SPI
//#define CLOCK_PIN 8

// Simplify dealings with the colors
#define WHITE  CRGB::White
#define BLACK  CRGB::Black
#define BLUE   CRGB::Blue
#define GREEN  CRGB::Green
#define YELLOW CRGB::Yellow
#define PINK   CRGB::Pink
#define RED    CRGB::Red
#define ORANGE CRGB::Orange
#define PURPLE CRGB::Purple
#define INDIGO CRGB::Indigo
#define TEAL   CRGB::Teal

#define MAX_COLORS 11

CRGB colorArray[MAX_COLORS] = {WHITE, BLACK, BLUE, GREEN, YELLOW, PINK, RED, ORANGE, PURPLE, INDIGO, TEAL};


// We'll use these for random color patterns
#define RAND8_SEED  111
byte rand8seed = RAND8_SEED;

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];

// Global used to choose when to rotate through patterns
int loopCount;

//
// This function sets up the ledsand tells the controller about them
//
void setup() {
  
  loopCount = 0;
  
  // sanity check delay - allows reprogramming if accidently blowing power w/leds
  delay(2000);

      // Uncomment one of the following lines for your leds arrangement.
      // FastLED.addLeds<TM1803, DATA_PIN, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<TM1804, DATA_PIN, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<TM1809, DATA_PIN, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<UCS1903, DATA_PIN, RGB>(leds, NUM_LEDS);
      FastLED.addLeds<WS2801, RGB>(leds, NUM_LEDS);

      // FastLED.addLeds<SM16716, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<LPD8806, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<SM16716, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<LPD8806, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
      
  // Temporarily ratchet down so it doesn't kill the USB port
  // LEDS.setBrightness(64);

}

//
// Move a single led from LED 0 to the halfway point. Simultaneously 
// move a led from LED NUM_LEDS backwards to the halfway point.
//
int SymmetricWavePattern(CRGB color) {
  
   for(int led = 1; led < (NUM_LEDS/2); led++) {

      leds[led] = color;
      leds[(NUM_LEDS-led)] = color;

      // Show the leds (two of which are lit)
      FastLED.show();

      // Wait a little bit
      delay(100);

      // Turn the leds back to black for the next loop around
      leds[led] = BLACK;
      leds[NUM_LEDS-led] = BLACK;
   }
   return 0;
}

//
// Move a single led from LED 0 to the end.
//
int SingleLinePattern(CRGB color) {
  
   for(int i = 1; i < NUM_LEDS; i++) {

      leds[i] = color;

      FastLED.show();

      // Wait a little bit
      delay(50);

      // Turn the led back to black for the next loop around
      leds[i] = BLACK;
   }
   return 0;
}

//
// Rainbow time
//
int RainbowPattern() {
   
  SymmetricWavePattern(RED);
  SymmetricWavePattern(ORANGE);
  SymmetricWavePattern(YELLOW);
  SymmetricWavePattern(GREEN);
  SymmetricWavePattern(BLUE);
  SymmetricWavePattern(INDIGO);
  SymmetricWavePattern(PURPLE);
  
}

//
// Honor 'merica
//
int PatriotPattern() {
  
  SymmetricWavePattern(RED);
  SymmetricWavePattern(WHITE);
  SymmetricWavePattern(BLUE);
  
  SingleLinePattern(RED);
  SingleLinePattern(WHITE);
  SingleLinePattern(BLUE);
  
  // Now do a single run down the lights going patriot mode
  for(int led = 1; led < NUM_LEDS; led++) {
      switch(led%3) {
        case 1: leds[led] = RED; break;
        case 2: leds[led] = WHITE; break;
        case 0: leds[led] = BLUE; break;
      }
      FastLED.show();
      delay(300);
      leds[led] = BLACK;
    }

  // Leave a red-white-blue pattern on as it snakes down the line    
  for(int led = 1; led < NUM_LEDS; led++) {
      switch(led%3) {
        case 1: leds[led] = RED; break;
        case 2: leds[led] = WHITE; break;
        case 0: leds[led] = BLUE; break;
      }
      if (led > 3) {
        leds[led-2] = BLACK;
      }
      delay(500);
      FastLED.show();
    }
}

//
// Same thing, but random colors rather than white
//
int SymmetricRandomColorPattern() {
  
  for (int i=1; i<(NUM_LEDS/2); i++) {
    byte rand = random8() % MAX_COLORS;
    
    leds[i] = colorArray[rand];
    leds[NUM_LEDS-i] = colorArray[rand];
    delay(200);
    FastLED.show();
    leds[i] = BLACK;    
    leds[NUM_LEDS-i] = BLACK;

  }  
}

// This function runs over and over, and is where you do the magic to light
// your leds.

void loop() {
  
  loopCount++;
  SymmetricWavePattern(WHITE);
  
  if ((loopCount % 50) == 0) {
    PatriotPattern(); 
  }
  if ((loopCount % 75) == 0) {
   SymmetricRandomColorPattern();
  }
  if ((loopCount % 100) == 0) {
    RainbowPattern();
  }  
}
