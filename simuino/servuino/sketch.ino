//#include "SPI.h"
//#include <Adafruit_GFX.h>
#include "Board_WS2801.h"
//#include "Print.h"
//#include <MozziGuts.h>
//#include <Oscil.h>
//#include <tables/cos8192_int8.h> // table for Oscils to play
//#include <mozzi_midi.h> // for mtof

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

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define MEGA 1
#else
#define DUE 1
#endif



// PINS
// - SPI: 50 (MISO), 51 (MOSI), 52 (SCK), 53 (SS)
//- PWM: 2 to 13 and 44 to 46
//- 36, 42: relays
//- ID Pins 22, 23, 24, 25
//  connect inverted - ground the 1's
//  - 0, rmc - 0,0,0,0 - none
//  - 1, woodson - 0,0,0, 1 - 25
//  - 2, ric - 0,0,1,0 - 24
//  - 3, james - 0,0,1,1 - 24, 25
//  - 4, steve - 0,1,0,0 - 23
//  - 5, joon - 0,1,0,1 - 23, 25
//  - 6, steve x - 0,1,1,0 - 23, 24
  
//#define AUDIO 1

// Audio Stuff
#ifdef AUDIO
#define CONTROL_RATE 64 // powers of 2 please

Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos(COS8192_DATA);
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aVibrato(COS8192_DATA);

#endif

uint16_t boardId = 0;

const long intensity = 300;
#define AUDIO_MODE STANDARD
// End Audio Stuff

/*
   ISR(TIMER5_OVF_vect)        // interrupt service routine that wraps a user defined function supplied by attachInterrupt
   {
   TCNT5 = 34286;            // preload timer
   Serial.printlnX(68,"Int");
   }
 */

bool ledsOn = false;


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
#define RGB_DIM 80  

#define BATTERY_PIN A0
#define MOT_PIN A1
#ifdef MEGA
#define REMOTE_PIN A10
#define LRELAY_PIN 36
#define SRELAY_PIN 42

#define ID_0 25 
#define ID_1 24 
#define ID_2 23 
#define ID_3 22 

#else
#define REMOTE_PIN A2
#define LRELAY_PIN 3
#define SRELAY_PIN 4
#endif

uint8_t row;
uint8_t invader;

uint8_t wheel_color;

char *boards[] = {
  "PROTO",
  "PROTO",
  "AKULA",  
  "POINT B", 
  "GOOFY", 
  "STEVE", 
  "JOON",
  "ARTEMIS"};
  
char *names[] = {
  "RICHARD",
  "RICHARD",
  "WOODSON",  
  "RIC", 
  "STEVE", 
  "STEVE", 
  "JOON",
  "JAMES"};
  
  
/* Helper functions */

// Create a 24 bit color value from R,G,B
uint32_t rgbTo24BitColor(byte r, byte g, byte b)
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
uint32_t wheel(byte WheelPos)
{
        if (WheelPos < 85) {
                return rgbTo24BitColor(WheelPos * 3, 255 - WheelPos * 3, 0);
        } else if (WheelPos < 170) {
                WheelPos -= 85;
                return rgbTo24BitColor(255 - WheelPos * 3, 0, WheelPos * 3);
        } else {
          WheelPos -= 170; 
          return rgbTo24BitColor(0, WheelPos * 3, 255 - WheelPos * 3);
        }
}

// Slightly different, this one makes the rainbow wheel equally distributed 
// along the chain
void rainbowCycle(uint8_t wait) {
  int i, j;
  
  for (j=0; j < 256 * 5; j++) {     // 5 cycles of all 25 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      // tricky math! we use each pixel as a fraction of the full 96-color wheel
      // (thats the i / strip.numPixels() part)
      // Then add in j which makes the colors go around per pixel
      // the % 96 is to make the wheel cycle around
      strip.setPixelColor(i, wheel( ((i * 256 / strip.numPixels()) + j) % 256) );
    }  
    strip.show();   // write all the pixels out
    delayX(185,wait);
  }
}


//Shift Matrixrow down
void shiftMatrixLines()
{
  uint16_t x, y;

  for(y = 0; y < 69; y++)
  {
    for(byte x = 0; x < 10;x++)
    {
      strip.setPixelColor(x, y, strip.getPixelColor(x, y + 1));         
    }
  }
  delayX(202,50);
}

// Clear Screen
void clearScreen()
{
  uint16_t x, y;

  for(y = 0; y < 70; y++)
  {
    for(byte x = 0; x < 10;x++)
    {
      strip.setPixelColor(x, y, rgbTo24BitColor(0, 0, 0));         
    }
  }
}

void lines(uint8_t wait) {
  uint16_t x, y;
  uint32_t j = 0;

  for (x = 0; x < 10; x++) {
    for(y = 0; y < 70; y++) {
      strip.setPixelColor(x, y, wheel(j));
      strip.show();
      delayX(227,wait);
    }
    j += 50;
  }
}

void drawzagX(uint8_t w, uint8_t h, uint8_t wait) {
  uint16_t x, y;
  for (x=0; x<w; x++) {
    strip.setPixelColor(x, x, 255, 0, 0);
    strip.show();
    delayX(238,wait);
  }
  for (y=0; y<h; y++) {
    strip.setPixelColor(w-1-y, y, 0, 0, 255);
    strip.show();
    delayX(243,wait);
  }

}

void drawY(uint8_t startx, uint8_t starty, uint8_t length, uint32_t color) {
  uint16_t x, y;

  for (y = starty; y < starty + length; y++) {
    strip.setPixelColor(x, y, color);
  }
}

/* Helper macros */
#define HEX__(n) 0x##n##LU
#define B8__(x) ((x&0x0000000FLU)?1:0) \
  +((x&0x000000F0LU)?2:0) \
+((x&0x00000F00LU)?4:0) \
+((x&0x0000F000LU)?8:0) \
+((x&0x000F0000LU)?16:0) \
+((x&0x00F00000LU)?32:0) \
+((x&0x0F000000LU)?64:0) \
+((x&0xF0000000LU)?128:0)

/* User macros */
#define B8(d) ((unsigned char)B8__(HEX__(d)))
#define B16(dmsb,dlsb) (((unsigned short)B8(dmsb)<<8) \
    + B8(dlsb))
#define B32(dmsb,db2,db3,dlsb) (((unsigned long)B8(dmsb)<<24) \
    + ((unsigned long)B8(db2)<<16) \
    + ((unsigned long)B8(db3)<<8) \
    + B8(dlsb))



void drawDistrikt() {
  uint16_t x;
  uint16_t row;

  // DISTRIKT
  uint16_t distrikt[] = {
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(11,11111111),
    B16(10,00000001),
    B16(10,00000001),
    B16(10,11000001),
    B16(10,11111101),
    B16(10,11111101),
    B16(10,11000001),
    B16(10,00000001),
    B16(10,00000001),
    B16(10,11001101),
    B16(10,00110001),
    B16(10,11111101),
    B16(10,00000001),
    B16(10,00000001),
    B16(10,11111101),
    B16(10,00000001),
    B16(10,00000001),
    B16(10,01101101),
    B16(10,11010001),
    B16(10,11010001),
    B16(11,11111101),
    B16(10,00000001),
    B16(10,00000001),
    B16(10,11000001),
    B16(10,11111101),
    B16(10,11111101),
    B16(10,11000001),
    B16(10,00000001),
    B16(10,00000001),
    B16(10,01011001),
    B16(10,11011001),
    B16(10,11101101),
    B16(10,01101101),
    B16(10,00000001),
    B16(10,00000001),
    B16(10,11111101),
    B16(10,00000001),
    B16(10,00000001),
    B16(10,01111001),
    B16(10,10000101),
    B16(10,11111101),
    B16(10,11111101),
    B16(10,00000001),
    B16(10,00000001),
    B16(11,11111111),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000)};

  clearScreen();
  for (row = 0; row < sizeof(distrikt) / sizeof(uint16_t); row++) {
    for (x = 0; x < 10; x++) {
      strip.setPixelColor(x,  row, distrikt[row] & (1<<(x))? rgbTo24BitColor(255, 255, 255): rgbTo24BitColor(0, 0, 0));
    }
  }
  strip.show();
}


// US flag 
void drawUSflag() {

  //drawY(0, 70, rgbTo24BitColor(255, 0, 0));
  //drawY(1, 70, rgbTo24BitColor(255, 255, 255));

  uint32_t color;
  uint16_t x;


  // Red and White for 20 rows
  for (row = 0; row < 20; row++) {

    for (x = 0; x < 10; x++) {
      strip.setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, 0, 0));
      x++;
      strip.setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, RGB_DIM, RGB_DIM));
    }
    shiftMatrixLines();
    strip.show();
  }

  // Red and White with solid blue
  for (x = 0; x < 4; x++) {
    strip.setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, 0, 0));
    x++;
    strip.setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, RGB_DIM, RGB_DIM));
  }
  shiftMatrixLines();
  strip.show();
  row++;


  // Red/white
  for (x = 0; x < 4; x++) {
    strip.setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, 0, 0));
    x++;
    strip.setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, RGB_DIM, RGB_DIM));
  }
  // Solid Blue line
  for (; x < 10; x++) {
    strip.setPixelColor(x, 69, rgbTo24BitColor(0, 0, RGB_DIM));
  }
  shiftMatrixLines();
  strip.show();

  for (row = 0; row < 20; row++) {
    // Red/white
    for (x = 0; x < 4; x++) {
      strip.setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, 0, 0));
      x++;
      strip.setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, RGB_DIM, RGB_DIM));
    }
    // White/Blue
    for (x = 4; x < 10; x++) {
      strip.setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, RGB_DIM, RGB_DIM));
      x++;
      strip.setPixelColor(x, 69, rgbTo24BitColor(0, 0, RGB_DIM));
    }    
    shiftMatrixLines();
    strip.show();  
    // Blue/white
    for (x = 4; x < 10; x++) {
      strip.setPixelColor(x, 69, rgbTo24BitColor(0, 0, RGB_DIM));
      x++;
      strip.setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, RGB_DIM, RGB_DIM));
    }
    shiftMatrixLines();
    strip.show();
    row++;

  }

  // Red/white
  for (x = 0; x < 4; x++) {
    strip.setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, 0, 0));
    x++;
    strip.setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, RGB_DIM, RGB_DIM));
  }
  // Blue line
  for (; x < 10; x++) {
    strip.setPixelColor(x, 69, rgbTo24BitColor(0, 0, RGB_DIM));
  }
  shiftMatrixLines();


  // 10 lines of blank
  for (x = 0; x < 10; x++) {
    strip.setPixelColor(x, 69, rgbTo24BitColor(0, 0, 0));
  }
  for (row = 0; row < 10; row++) {
    shiftMatrixLines();
    strip.show();
  }

}


void drawInvader(uint8_t invader) {
  uint32_t color;
  uint16_t x;
  for (row = 0; row < 8; row++) {
    for (x = 1; x < 9; x++) {
      if (invader) {
        strip.setPixelColor(x, 30 + row, ledn[row] & rgbTo24BitColor(0, 0, 0));
        strip.setPixelColor(x, 30 + row, ledo[row] & (1<<(x - 1))? rgbTo24BitColor(0, 0, 0): rgbTo24BitColor(0, 100, 0));
      } else {
        strip.setPixelColor(x, 30 + row, ledo[row] & rgbTo24BitColor(0, 0, 0));
        strip.setPixelColor(x, 30 + row, ledn[row] & (1<<(x - 1))? rgbTo24BitColor(0, 0, 0): rgbTo24BitColor(0, 100, 0));
      }
    }
    strip.show();
  }
}

void drawHeader() {
  uint32_t color;
  uint16_t x;

  for (x = 0; x < 10; x++) {
    //   color = random(2,4)%2 == 0 ? rgbTo24BitColor(0,0,0) : rgbTo24BitColor(0, 255, 0); //Chance of 1/3rd 
    color = random(2,4)%2 == 0 ? rgbTo24BitColor(0, 0, 0): wheel(wheel_color); //Chance of 1/3rd 
    //   color = random(2,4)%2 == 0 ? rgbTo24BitColor(0, 0, 0): rgbTo24BitColor(255, 255, 255); //Chance of 1/3rd 
    //   color =  rgbTo24BitColor(255, 255, 255); //Chance of 1/3rd 
    strip.setPixelColor(x, 69, color);
    wheel_color++;
  }
  strip.show();
}


uint16_t readID() {
 uint16_t bit;
 uint16_t id;
 
 bit = digitalReadX(499,ID_0);
 Serial.printX(500,bit, BIN);
 id = !bit;
 bit = digitalReadX(502,ID_1);
 Serial.printX(503,bit, BIN);
 id |= !bit << 1;
 bit = digitalReadX(505,ID_2);
 Serial.printX(506,bit, BIN);
 id |= !bit << 2; 
 bit = digitalReadX(508,ID_3);
 Serial.printX(509,bit, BIN);
 id |= !bit << 3;
 
 Serial.printX(512,"Board ID  ");
 Serial.printX(513,id, DEC);
 Serial.printlnX(514,"");

 return(id);

}


// Working on proto, but low end is over
// = 90% = 30350
// 38.1 = 100% = 102400
// 36v = 40% = 96900
// 10% = 91800
//#define LEVEL_EMPTY 91800
//#define LEVEL_FULL  102300

// New settings, 8/17/2013
// 0 = 92900
// 100 = 102300
#define LEVEL_EMPTY 92900
#define LEVEL_FULL  102300


// = 90% = 30350
// 38.1 = 100% = 102400
// 36v = 40% = 96900
// 10% = 91800

// Battery Level Meter
// This is a simple starting point
// Todo: Sample the battery level continously and maintain a rolling average
//       This will help with the varying voltages as motor load changes, which
//       will result in varing results depending on load with this current code
//
void drawBattery() {
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
  clearScreen();
  strip.show();
  delayX(563,1000);

  // Convert to level 0-28
  for (i = 0; i < 100; i++) {
    level += sample = analogReadX(567,BATTERY_PIN);
    Serial.printX(568,"Battery sample ");
    Serial.printX(569,sample, DEC);
    Serial.printlnX(570," ");
  }
  Serial.printX(572,"Battery Level ");
  Serial.printX(573,level, DEC);
  Serial.printlnX(574," ");

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

  Serial.printX(592,"Adjusted Level ");
  Serial.printX(593,level, DEC);
  Serial.printlnX(594," ");


  row = 20;

  // White Battery Shell with Green level

  // Battery Bottom
  for (x = 0; x < 10; x++) {
    strip.setPixelColor(x, row, rgbTo24BitColor(RGB_MAX, RGB_MAX, RGB_MAX));
  }
  row++;

  // Battery Sides
  for (; row < 49; row++) {
    strip.setPixelColor(0, row, rgbTo24BitColor(RGB_MAX, RGB_MAX, RGB_MAX));
    strip.setPixelColor(9, row, rgbTo24BitColor(RGB_MAX, RGB_MAX, RGB_MAX));
  }

  // Battery Top
  for (x = 0; x < 10; x++) {
    strip.setPixelColor(x, row, rgbTo24BitColor(RGB_MAX, RGB_MAX, RGB_MAX));
  }
  row++;

  // Battery button
  for (x = 3; x < 7; x++) {
    strip.setPixelColor(x, row, rgbTo24BitColor(RGB_MAX, RGB_MAX, RGB_MAX));
    strip.setPixelColor(x, row+1, rgbTo24BitColor(RGB_MAX, RGB_MAX, RGB_MAX));
  }
  row+=2;

  // Battery Level
  for (row = 21; row < 21 + level; row++) {
    for (x = 1; x < 9; x++) {
      strip.setPixelColor(x, row, rgbTo24BitColor(0, RGB_DIM, 0));
    }
  }

  strip.show();
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
    strip.setPixelColor(x, y, wheel(j));
    strip.show();
    delayX(664,wait);
    strip.setPixelColor(x, y, 0, 0, 0);
  }
}




void setup() {

  uint16_t i;

  // Console for debugging
  Serial.beginX(677,9600);
  Serial.printlnX(678,"Goodnight moon!");

  // Set battery level analogue reference
#ifdef MEGA
  analogReference(INTERNAL1V1);
#else
  //  analogReference(INTERNAL);
#endif
  pinModeX(686,BATTERY_PIN, INPUT);
  pinModeX(687,MOT_PIN, INPUT);
  pinModeX(688,REMOTE_PIN, INPUT);
  digitalWriteX(689,REMOTE_PIN, LOW);
  pinModeX(690,SRELAY_PIN, OUTPUT);
  pinModeX(691,LRELAY_PIN, OUTPUT);
  digitalWriteX(692,SRELAY_PIN, HIGH);
  
// ID Pins  
  pinModeX(695,ID_0, INPUT);
  digitalWriteX(696,ID_0, HIGH);
  pinModeX(697,ID_1, INPUT);
  digitalWriteX(698,ID_1, HIGH);
  pinModeX(699,ID_2, INPUT);
  digitalWriteX(700,ID_2, HIGH);
  pinModeX(701,ID_3, INPUT);
  digitalWriteX(702,ID_3, HIGH);
  
  
  /*
     TCCR5A = 0;
     TCCR5B = 0;
     TCNT5  = 0;

     OCR5A = 31250;            // compare match register 16MHz/256/2Hz
     TCCR5B |= (1 << WGM12);   // CTC mode
     TCCR5B |= (1 << CS12);    // 256 prescaler 
     TIMSK5 |= (1 << OCIE1A);  // enable timer compare interrupt
   */



  /*
     for (uint16_t i = 0; i < 544; i++) {
     Serial.printX(720,"Strip pixel ");
     Serial.printX(721,i, DEC);
     Serial.printX(722," = virt pixel ");
     Serial.printX(723,strip.pixel_translate[i], DEC);
     Serial.printlnX(724," ");
     }
   */

  boardId = readID();

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
  clearScreen();
  strip.show();

  strip.print(boards[boardId], 15, 1, 1);
  strip.show();
  delayX(761,5000);

  clearScreen();
  strip.print(names[boardId], 15, 1, 1);
  strip.show();
  delayX(766,5000);
  
  
//  for (i = 0; i < 500; i++) {
//    rainbowCycle(1);
//  }

  //bounce(10, 70, 0);

  drawBattery();
  delayX(776,10000);

  clearScreen();
  

  
#ifdef AUDIO
  startMozzi(64);
  aCos.setFreq(25);
  aVibrato.setFreq(4);
#endif

}

int16_t loopcnt = 0;

int16_t state = 0;
// state = 0: lines
// state = 1: flag


void loop() {

#ifdef AUDIO
  audioHook();

#else
  if (ledsOn) {


    if (loopcnt > 5000) {
      loopcnt = 0;
      state++;
    }


    if (state == 0) {
      drawDistrikt();
      delayX(814,2000);
      state = 1;    
    }

    if (state == 1) {
      drawHeader();
      shiftMatrixLines();
    }  

    if (state == 2) {
      drawUSflag();
      loopcnt += 50;
    }

    if (state == 3) {
      loopcnt = 0;
      state = 0;
      drawBattery();
      delayX(832,2000);
      clearScreen();
    }

    /*
       drawInvader(invader);
       delayX(838,500);

       if (invader == 0) {
       invader = 1;
       } else {
       invader = 0;
       }
     */
  } else {
    clearScreen();
    strip.show();
  }
#endif

  loopcnt++;

}

#ifdef AUDIO
uint32_t updateTone = 100;

void updateControl() {
  uint16_t motSpeed;
  float f1, f2;


  if (updateTone > 5) {
    motSpeed = analogReadX(865,MOT_PIN);

    f1 = motSpeed / 21.0;
    f1 += 20;
    f2 = f1 / 4.0;

    //  Serial.printX(871,"update ");
    //  Serial.printlnX(872,motSpeed);
    aCos.setFreq(mtof(f1));
    aVibrato.setFreq(f2);
    updateTone = 0;
  }


  updateTone++;  
}

int updateAudio(){
  long vibrato = intensity * aVibrato.next();
  return (int)aCos.phMod(vibrato);
}

#endif



uint16_t buttonPress = 0;
bool lightState = false;

#define HOLD_COUNT 30

void checkButton() {
  uint16_t remotePosition;

#ifdef MEGA
  remotePosition = analogReadX(900,REMOTE_PIN);
  //  Serial.printX(901,"Remote position ");
  //  Serial.printlnX(902,remotePosition);



  //  Serial.printX(906,"Button position ");
  //  Serial.printlnX(907,buttonPress);

  if (remotePosition > 100) {
    ledsOn = true;
  } else {
    ledsOn = false;
  }

  if (buttonPress == HOLD_COUNT) {
    if (lightState == false) {
      lightState = true;
      digitalWriteX(918,LRELAY_PIN, HIGH);
    } else {
      if (lightState == true) {
        lightState = false;
        digitalWriteX(922,LRELAY_PIN, LOW);
      }  
    }
    buttonPress = 0;
  }


  if (remotePosition < 400 && remotePosition > 100) {
    buttonPress++;
    if (buttonPress > HOLD_COUNT) {
      buttonPress = HOLD_COUNT;
    }
  } else {
    if (buttonPress > 0) {
      buttonPress--;
    }
  } 
#else
  ledsOn = true;
#endif
}

void screenHook() {
  checkButton();
}


