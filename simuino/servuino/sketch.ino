#include "Board_WS2801.h"

/*****************************************************************************
  Burner Board LED and Audio Code
  
  Richard McDougall
  June 2013
  
  Uses modified Adafruit WS2801 library, adapted for virtual rectangle matrix
  with holes where the board is missing the corners
  
  Some code from riginal matrix example Written by 
  David Kavanagh (dkavanagh@gmail.com).  
  BSD license, all text above must be included in any redistribution

*****************************************************************************/

// These are special directives for the hardware simulator
// Set the battery level to 300 on analogue pin 0
// BOARD_TYPE   MEGA
// SCENANAPIN  0  7   300



// Burner board is not using this: we are using Hardware SPI for performance
// Choose which 2 pins you will use for output.
// Can be any valid output pins.
// The colors of the wires may be totally different so
// BE SURE TO CHECK YOUR PIXELS TO SEE WHICH WIRES TO USE!
uint8_t dataPin  = 2;    // Yellow wire on Adafruit Pixels
uint8_t clockPin = 3;    // Green wire on Adafruit Pixels

// Don't forget to connect the ground wire to Arduino ground,
// and the +5V wire to a +5V supply

// Set the first variable to the NUMBER of pixels in a row and
// the second value to number of pixels in a column.
//Board_WS2801 strip = Board_WS2801((uint16_t)10, (uint16_t)70, dataPin, clockPin);
Board_WS2801 strip = Board_WS2801((uint16_t)10, (uint16_t)70, WS2801_RGB);

uint8_t led[8];
uint8_t ledo[8];
uint8_t ledn[8];

#define RGB_MAX 255
#define RGB_DIM 100  

#define BATTERY_PIN A0

uint8_t therow;
uint8_t invader;

/* Helper functions */

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
uint32_t Wheel(byte WheelPos)
{
  if (WheelPos < 85) {
   return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
   WheelPos -= 85;
   return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170; 
   return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

//Shift Matrixrow down
void ShiftMatrixLines()
{
  uint16_t x, y;
  
  for(y = 0; y < 69; y++)
  {
    for(byte x = 0; x < 10;x++)
    {
         strip.setPixelColor(x, y, strip.getPixelColor(x, y + 1));         
    }
  }
  delayX(94,50);
}

// Clear Screen
void ClearScreen()
{
  uint16_t x, y;
  
  for(y = 0; y < 70; y++)
  {
    for(byte x = 0; x < 10;x++)
    {
         strip.setPixelColor(x, y, Color(0, 0, 0));         
    }
  }
}


// US flag 
void drawUSflag() {
  
  //drawY(0, 70, Color(255, 0, 0));
  //drawY(1, 70, Color(255, 255, 255));
  
  uint32_t color;
  uint16_t x;
  

  // Red and White for 20 rows
  for (row = 0; row < 20; row++) {

    for (x = 0; x < 10; x++) {
        strip.setPixelColor(x, 69, Color(RGB_DIM, 0, 0));
        x++;
        strip.setPixelColor(x, 69, Color(RGB_DIM, RGB_DIM, RGB_DIM));
    }
    ShiftMatrixLines();
    strip.show();
  }
  
  // Red and White with solid blue
  for (x = 0; x < 4; x++) {
      strip.setPixelColor(x, 69, Color(RGB_DIM, 0, 0));
      x++;
      strip.setPixelColor(x, 69, Color(RGB_DIM, RGB_DIM, RGB_DIM));
  }
  ShiftMatrixLines();
  strip.show();
  row++;
  
  
  // Red/white
  for (x = 0; x < 4; x++) {
      strip.setPixelColor(x, 69, Color(RGB_DIM, 0, 0));
      x++;
      strip.setPixelColor(x, 69, Color(RGB_DIM, RGB_DIM, RGB_DIM));
  }
  // Solid Blue line
  for (; x < 10; x++) {
        strip.setPixelColor(x, 69, Color(0, 0, RGB_DIM));
  }
  ShiftMatrixLines();
  strip.show();

  for (row = 0; row < 20; row++) {
    // Red/white
    for (x = 0; x < 4; x++) {
        strip.setPixelColor(x, 69, Color(RGB_DIM, 0, 0));
        x++;
        strip.setPixelColor(x, 69, Color(RGB_DIM, RGB_DIM, RGB_DIM));
    }
    // White/Blue
    for (x = 4; x < 10; x++) {
        strip.setPixelColor(x, 69, Color(RGB_DIM, RGB_DIM, RGB_DIM));
        x++;
        strip.setPixelColor(x, 69, Color(0, 0, RGB_DIM));
    }    
    ShiftMatrixLines();
    strip.show();  
    // Blue/white
    for (x = 4; x < 10; x++) {
        strip.setPixelColor(x, 69, Color(0, 0, RGB_DIM));
        x++;
        strip.setPixelColor(x, 69, Color(RGB_DIM, RGB_DIM, RGB_DIM));
    }
    ShiftMatrixLines();
    strip.show();
    row++;
  
  }
  
  // Red/white
  for (x = 0; x < 4; x++) {
      strip.setPixelColor(x, 69, Color(RGB_DIM, 0, 0));
      x++;
      strip.setPixelColor(x, 69, Color(RGB_DIM, RGB_DIM, RGB_DIM));
  }
  // Blue line
  for (; x < 10; x++) {
        strip.setPixelColor(x, 69, Color(0, 0, RGB_DIM));
  }
  ShiftMatrixLines();


  // 10 lines of blank
  for (x = 0; x < 10; x++) {
        strip.setPixelColor(x, 69, Color(0, 0, 0));
  }
  for (row = 0; row < 10; row++) {
    ShiftMatrixLines();
    strip.show();
  }

}

uint8_t wheel;

void drawInvader(uint8_t invader) {
  uint32_t color;
  uint16_t x;
  for (row = 0; row < 8; row++) {
    for (x = 1; x < 9; x++) {
      if (invader) {
        strip.setPixelColor(x, 30 + row, ledn[row] & Color(0, 0, 0));
        strip.setPixelColor(x, 30 + row, ledo[row] & (1<<(x - 1))? Color(0, 0, 0): Color(0, 100, 0));
      } else {
        strip.setPixelColor(x, 30 + row, ledo[row] & Color(0, 0, 0));
        strip.setPixelColor(x, 30 + row, ledn[row] & (1<<(x - 1))? Color(0, 0, 0): Color(0, 100, 0));
      }
    }
    strip.show();
  }
}

void drawHeader() {
  uint32_t color;
  uint16_t x;
  
 for (x = 0; x < 10; x++) {
//   color = random(2,4)%2 == 0 ? Color(0,0,0) : Color(0, 255, 0); //Chance of 1/3rd 
   color = random(2,4)%2 == 0 ? Color(0, 0, 0): Wheel(wheel); //Chance of 1/3rd 
//   color = random(2,4)%2 == 0 ? Color(0, 0, 0): Color(255, 255, 255); //Chance of 1/3rd 
//   color =  Color(255, 255, 255); //Chance of 1/3rd 
   strip.setPixelColor(x, 69, color);
   wheel++;
 }
 strip.show();
}

#define LEVEL_EMPTY 2588
#define LEVEL_FULL 3040

// = 90% = 30350
// 36.0v = 50% = 29606
// 35.6v = 40% = 28862
// 10% = 25886

// Battery Level Meter
// This is a simple starting point
// Todo: Sample the battery level continously and maintain a rolling average
//       This will help with the varying voltages as motor load changes, which
//       will result in varing results depending on load with this current code
//
void DrawBattery() {
  uint32_t color;
  uint16_t x;
  uint8_t row;
  uint32_t level = 0;
  uint16_t i;
  uint16_t sample;
  
  // Read Battery Level
  // 18 * 1.75v = 31.5v for low voltage
  // Set zero to 30v
  // Set 100% to 38v

    // Clear screen and measure voltage, since screen load varies it!
  ClearScreen();
  strip.show();
  delayX(273,1000);
  
  // Convert to level 0-28
  for (i = 0; i < 10; i++) {
    level += sample = analogReadX(277,BATTERY_PIN);
//  Serial.printX(278,"Battery sample ");
//  Serial.printX(279,sample, DEC);
//  Serial.printlnX(280," ");
  }
//  Serial.printX(282,"Battery Level ");
//  Serial.printX(283,level, DEC);
//  Serial.printlnX(284," ");
  
  if (level > LEVEL_FULL) {
    level = LEVEL_FULL;
  }

  // Sometimes noise makes level just below zero
  if (level > LEVEL_EMPTY) {
    level -= LEVEL_EMPTY;
  } else {
    level = 0;
  }

  
  level *= 28;
    
  level = level / (LEVEL_FULL - LEVEL_EMPTY);
  
//  Serial.printX(302,"Adjusted Level ");
//  Serial.printX(303,level, DEC);
//  Serial.printlnX(304," ");
  

  
  row = 20;
  
  // White Battery Shell with Green level
  
  // Battery Bottom
  for (x = 0; x < 10; x++) {
    strip.setPixelColor(x, row, Color(RGB_MAX, RGB_MAX, RGB_MAX));
  }
  row++;
  
  // Battery Sides
  for (; row < 49; row++) {
    strip.setPixelColor(0, row, Color(RGB_MAX, RGB_MAX, RGB_MAX));
    strip.setPixelColor(9, row, Color(RGB_MAX, RGB_MAX, RGB_MAX));
  }
  
  // Battery Top
  for (x = 0; x < 10; x++) {
    strip.setPixelColor(x, row, Color(RGB_MAX, RGB_MAX, RGB_MAX));
  }
  row++;
  
  // Battery button
  for (x = 3; x < 7; x++) {
    strip.setPixelColor(x, row, Color(RGB_MAX, RGB_MAX, RGB_MAX));
    strip.setPixelColor(x, row+1, Color(RGB_MAX, RGB_MAX, RGB_MAX));
  }
  row+=2;
  
  // Battery Level
  for (row = 21; row < 21 + level; row++) {
    for (x = 1; x < 9; x++) {
        strip.setPixelColor(x, row, Color(0, RGB_DIM, 0));
    }
  }
  
  strip.show();
}


void lines(uint8_t wait) {
  uint16_t x, y;
  uint32_t j = 0;
  
  for (x = 0; x < 10; x++) {
    for(y = 0; y < 70; y++) {
     strip.setPixelColor(x, y, Wheel(j));
     strip.show();
     delayX(356,wait);
    }
    j+=50;
  }
}

void drawzagX(uint8_t w, uint8_t h, uint8_t wait) {
  uint16_t x, y;
  for (x=0; x<w; x++) {
    strip.setPixelColor(x, x, 255, 0, 0);
    strip.show();
    delayX(367,wait);
  }
  for (y=0; y<h; y++) {
    strip.setPixelColor(w-1-y, y, 0, 0, 255);
    strip.show();
    delayX(372,wait);
  }

}

void drawY(uint8_t startx, uint8_t starty, uint8_t length, uint32_t color) {
  uint16_t x, y;
  
  for (y = starty; y < starty + length; y++) {
    strip.setPixelColor(x, y, color);
  }
}

void bounce(uint8_t w, uint8_t h, uint8_t wait) {
  int16_t x = 1;
  int16_t y = 2;
  int8_t xdir = +1;
  int8_t ydir = -1;
  int j;
  for (j=0; j < 256; j++) {
     x = x + xdir;
     y = y + ydir;
     if (x < 0) {
       x = -x;
       xdir = - xdir;
     }
     if (y < 0) {
       y = -y;
       ydir = - ydir;
     }
     if (x == w) {
       x = w-2;
       xdir = - xdir;
     }
     if (y == h) {
       y = h-2;
       ydir = - ydir;
     }
     strip.setPixelColor(x, y, Wheel(j));
     strip.show();
     delayX(412,wait);
     strip.setPixelColor(x, y, 0, 0, 0);
  }
}



  
void setup() {
  
  uint16_t i;

  // Console for debugging
  Serial.beginX(425,9600);
  Serial.printlnX(426,"Goodnight moon!");
  
  // Set battery level analogue reference
//  analogReference(INTERNAL1V1);
//  pinModeX(430,BATTERY_PIN, INPUT);

/*  
  for (uint16_t i = 0; i < 544; i++) {
    Serial.printX(434,"Strip pixel ");
    Serial.printX(435,i, DEC);
    Serial.printX(436," = virt pixel ");
    Serial.printX(437,strip.pixel_translate[i], DEC);
    Serial.printlnX(438," ");
  }
*/
  
  // Space Invader Character
// 1st animation for the character
ledo[0] = B01000010;
ledo[1] = B00100100;
ledo[2] = B00111100;
ledo[3] = B01111110;
ledo[4] = B11111111;
ledo[5] = B10111101;
ledo[6] = B11000011;
ledo[7] = B01100110;

// 2nd animation for the character
ledn[0] = B11000011;
ledn[1] = B00100100;
ledn[2] = B00111100;
ledn[3] = B01011010;
ledn[4] = B11111111;
ledn[5] = B00111100;
ledn[6] = B00100100;
ledn[7] = B00100100;

  strip.begin();
  
  invader = 0;

  // Update LED contents, to start they are all 'off'
  ClearScreen();
  strip.show();
  
  //bounce(10, 70, 0);

  DrawBattery();
//  delayX(474,10000);
 
  ClearScreen();
}

int16_t loopcnt = 0;

void loop() {
  uint16_t i;

  drawHeader();
  ShiftMatrixLines();  

//   drawUSflag();

/*
  drawInvader(invader);
  delayX(491,500);

    if (invader == 0) {
       invader = 1;
    } else {
      invader = 0;
    }
*/

  if (loopcnt == 500) {
    loopcnt = 0;
    DrawBattery();
    delayX(503,20000);
    ClearScreen();
  }
  
  loopcnt++;
   
}

void vScreenHook() {
  pinModeX(512,30, OUTPUT);
  digitalWriteX(513,30, HIGH);
}
