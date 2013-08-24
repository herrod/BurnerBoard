#include "FastSPI_LED2.h"

///////////////////////////////////////////////////////////////////////////////////////////
//
// Take a strip of LEDS and have the lights flash moving from the two ends of the strip 
// into the middle. 
// 
// Derived from the FastSPI_LED2 FirstLight example
//
///////////////////////////////////////////////////////////////////////////////////////////

// How many leds are in the strip?
#define NUM_LEDS 50

// Data pin that led data will be written out over
#define DATA_PIN 6

// Clock pin only needed for SPI based chipsets when not using hardware SPI
//#define CLOCK_PIN 8

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];

// This function sets up the ledsand tells the controller about them
void setup() {
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
}

// This function runs over and over, and is where you do the magic to light
// your leds.

void loop() {

   // Move a single white led from LED 0 to the halfway point. Simultaneously 
   // move a white led from LED NUM_LEDS backwards to the halfway point.

   for(int whiteLed = 1; whiteLed < (NUM_LEDS/2); whiteLed = whiteLed + 1) {

      // Turn our current led on to white, then show the leds
      leds[whiteLed] = CRGB::White;

      // Do the same thing counting backwards from the end
      leds[(NUM_LEDS-whiteLed)] = CRGB::White;

      // Show the leds (two of which are lit)
      FastLED.show();

      // Wait a little bit
      delay(150);

      // Turn the leds back to black for the next loop around
      leds[whiteLed] = CRGB::Black;
      leds[NUM_LEDS-whiteLed] = CRGB::Black;
   }
}
