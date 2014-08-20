#include "SPI.h"
#include <Adafruit_GFX.h>
#include "Board_WS2801.h"
#include "Print.h"


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
  

uint16_t boardId = 0;

const long intensity = 300;


bool ledsOn = false;


// Set the first variable to the NUMBER of pixels in a row and
// the second value to number of pixels in a column.
Board_WS2801 *strip;

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

/* Rotary encoder read example */
#define ENC_A A6
#define ENC_B A7
#define ENC_PORT PINF
 
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
  "BOADIE", 
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


/* returns change in encoder state (-1,0,1) */
int8_t read_encoder()
{
  static int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
  static uint8_t old_AB = 0;
  /**/
  old_AB <<= 2;                   //remember previous state
//  old_AB |= ( ENC_PORT & 0x03 );  //add current state
  old_AB |= (digitalRead(ENC_B) | (digitalRead(ENC_A) <<1));
  return ( enc_states[( old_AB & 0x0f )]);
}

int8_t encoder_pos = 0;

void mydelay(uint32_t del) {
  int i;
  int8_t enc;
  char mode[10];
  
  for (i = 0; i < del; i++) {
    delay(1);
    enc = read_encoder();
    if (enc) {
      Serial.print("Counter value: ");
      Serial.println(encoder_pos, DEC);
      encoder_pos += enc;
      if (enc < 0)
        enc = 0;
      clearScreen();
      sprintf(mode, "%d", encoder_pos);
      strip->print(mode, 35, 1, 1);
      strip->show();
      delay(500);
      clearScreen();
    }
  }
}


// Set sidelight left/right 
void setSideLight(uint16_t lr, uint16_t x,  uint32_t color)
{
        uint16_t pixel = lr * 79 + x;
        strip->setPixelColor(700 + pixel, color);
}

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
    for (i=0; i < strip->numPixels(); i++) {
      // tricky math! we use each pixel as a fraction of the full 96-color wheel
      // (thats the i / strip->numPixels() part)
      // Then add in j which makes the colors go around per pixel
      // the % 96 is to make the wheel cycle around
      strip->setPixelColor(i, wheel( ((i * 256 / strip->numPixels()) + j) % 256) );
    }  
    strip->show();   // write all the pixels out
    mydelay(wait);
  }
}

// Slightly different, this one makes the rainbow wheel equally distributed 
// along the chain
void rainbowCycle2() {
  int i;
  uint32_t color;
  uint8_t r, g, b;
  
    for (i=0; i < strip->numPixels(); i++) {
      strip->setPixelColor(i, wheel(wheel_color));
      wheel_color += random(255);
    }  
    strip->show();   // write all the pixels out  
}

#define FADER 50
// Fade the board
void fadeBoard() {
  int i;
  uint32_t color;
  uint8_t r, g, b;
  
    for (i=0; i < strip->numPixels(); i++) {
      color = strip->getPixelColor(i);

      r = (color & 0x00ff0000) >> 16;
      g = (color & 0x0000ff00) >> 8;
      b = (color & 0x000000ff);
      if (r)
        r-=FADER;
      if (b)
        b-=FADER;
      if (g)
        g-=FADER;
        
      if (r < FADER)
        r = 0;
      if (g < FADER)
        g = 0;        
      if (b < FADER)
        b = 0;        
        /*
      Serial.print("color  ");
      Serial.print(color, HEX);
      Serial.print("  ");
      Serial.print(r, HEX);
      Serial.print("  ");
      Serial.print(g, HEX);
      Serial.print("  ");
      Serial.print(b, HEX);
      Serial.println("");
      */
      strip->setPixelColor(i, r, g, b);
    }  
    strip->show();   // write all the pixels out  
}

//Shift Matrixrow down
void shiftMatrixLines()
{
  uint16_t x, y;

  for(y = 0; y < 69; y++)
  {
    for(byte x = 0; x < 10;x++)
    {
      strip->setPixelColor(x, y, strip->getPixelColor(x, y + 1));
    }
  }
  // Hardcoded 158 side LEDS for now
  for(y = 0; y < 78; y++)
  {
      strip->setPixelColor(700 + y, strip->getPixelColor(700 + y + 1));
      strip->setPixelColor(700 + 79 + y, strip->getPixelColor(700 + 79 + y + 1));
  }
}

// I see blondes, brunets, redheads...
void shiftMatrixCircles()
{
  uint16_t x;

  for(x = 35; x < 69; x++)
  {
      strip->drawCircle(69 - x, 5, x - 35, strip->getPixelColor(5, x + 1));
  }
  mydelay(50);
}

// Clear Screen
void clearScreen()
{
  uint16_t x, y;

  for(y = 0; y < 70; y++)
  {
    for(byte x = 0; x < 10;x++)
    {
      strip->setPixelColor(x, y, rgbTo24BitColor(0, 0, 0));         
    }
  }
  // Hardcoded 158 side LEDS for now
  for(y = 0; y < 79; y++)
  {
      strip->setPixelColor(700 + y, rgbTo24BitColor(0, 0, 0));
      strip->setPixelColor(700 + 79 + y, rgbTo24BitColor(0, 0, 0));
  }

}

// Clear Screen
void fillScreen(uint32_t color)
{
  uint16_t x, y;

  for(y = 0; y < 70; y++)
  {
    for(byte x = 0; x < 10;x++)
    {
      strip->setPixelColor(x, y, color);         
    }
  }
}

void lines(uint8_t wait) {
  uint16_t x, y;
  uint32_t j = 0;

  for (x = 0; x < 10; x++) {
    for(y = 0; y < 70; y++) {
      strip->setPixelColor(x, y, wheel(j));
      strip->show();
      mydelay(wait);
    }
    j += 50;
  }
}

void drawzagX(uint8_t w, uint8_t h, uint8_t wait) {
  uint16_t x, y;
  for (x=0; x<w; x++) {
    strip->setPixelColor(x, x, 255, 0, 0);
    strip->show();
    mydelay(wait);
  }
  for (y=0; y<h; y++) {
    strip->setPixelColor(w-1-y, y, 0, 0, 255);
    strip->show();
    mydelay(wait);
  }
}

void drawY(uint8_t startx, uint8_t starty, uint8_t length, uint32_t color) {
  uint16_t x, y;

  for (y = starty; y < starty + length; y++) {
    strip->setPixelColor(x, y, color);
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


void drawStanfordTree() {
  uint16_t x;
  uint16_t row;
  // Stanford
  uint16_t stanfordTree[] = {
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
                B16(00,00010000),
                B16(00,00100000),
                B16(00,00010000),
                B16(00,00100000),
                B16(00,00110000),
                B16(00,00111000),
                B16(00,01101100),
                B16(00,00110100),
                B16(00,01011000),
                B16(00,00010000),
                B16(00,10101000),
                B16(00,01010100),
                B16(00,10101000),
                B16(00,01101000),
                B16(00,11011100),
                B16(01,10110010),
                B16(00,00100100),
                B16(00,00110000),
                B16(00,00110000),
                B16(00,01111000),
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
                B16(00,00000000)};

  for (row = 0; row < sizeof(stanfordTree) / sizeof(uint16_t); row++) {
    for (x = 0; x < 10; x++) {
	if (stanfordTree[69 - row] & (1<<(x))) {
          strip->setPixelColor(x,  row, rgbTo24BitColor(0, 158, 118));
        }
    }

  }
  strip->show();
}

void drawStanfordLogo() {
  uint16_t x;
  uint16_t row;
  // Stanford
  uint16_t stanfordLogo[] = {
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
                B16(00,11111100),
                B16(01,11111110),
                B16(11,11111111),
                B16(11,11111111),
                B16(11,10000111),
                B16(11,10000000),
                B16(11,10000000),
                B16(11,11111000),
                B16(01,11111110),
                B16(00,11111111),
                B16(00,11111111),
                B16(00,00000111),
                B16(00,00000111),
                B16(00,00000111),
                B16(11,11111111),
                B16(11,11111111),
                B16(01,11111110),
                B16(00,11111100),
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
                B16(00,00000000)};

//  strip->drawRect(0, 0, 70, 10, rgbTo24BitColor(140, 21, 21));
  for (row = 0; row < sizeof(stanfordLogo) / sizeof(uint16_t); row++) {
    for (x = 0; x < 10; x++) {
      strip->setPixelColor(9 - x,  row, stanfordLogo[69 - row] & (1<<(x))? rgbTo24BitColor(140, 21, 21): rgbTo24BitColor(255, 255, 255));
    }
  }
  strip->show();
}



void drawStanford() {
  uint16_t x;
  uint16_t row;
  // Stanford
  uint16_t stanford[] = {
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
       B16(00,01111000),
       B16(00,10000100),
       B16(00,11111100),
       B16(00,11111100),
       B16(00,00000000),
       B16(00,00000000),
       B16(00,01101100),
       B16(00,11010000),
       B16(00,11111100),
       B16(00,11111100),
       B16(00,00000000),
       B16(00,00000000),
       B16(00,01111000),
       B16(00,10000100),
       B16(00,11111100),
       B16(00,01111000),
       B16(00,00000000),
       B16(00,00000000),
       B16(00,11010000),
       B16(00,11010000),
       B16(00,11111100),
       B16(00,11111100),
       B16(00,00000000),
       B16(00,00000000),
       B16(00,11111100),
       B16(00,00011000),
       B16(00,01100000),
       B16(00,11111100),
       B16(00,00000000),
       B16(00,00000000),
       B16(00,01111100),
       B16(00,10010000),
       B16(00,11111100),
       B16(00,01111100),
       B16(00,00000000),
       B16(00,00000000),
       B16(00,11000000),
       B16(00,11111100),
       B16(00,11111100),
       B16(00,11000000),
       B16(00,00000000),
       B16(00,00000000),
       B16(00,11011000),
       B16(00,11011100),
       B16(00,11101100),
       B16(00,01101100),
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
       B16(00,00000000)};

//  strip->drawRect(0, 0, 70, 10, rgbTo24BitColor(140, 21, 21));
  for (row = 0; row < sizeof(stanford) / sizeof(uint16_t); row++) {
    for (x = 0; x < 10; x++) {
      strip->setPixelColor(x,  row, stanford[row] & (1<<(x))? rgbTo24BitColor(64, 64, 64): rgbTo24BitColor(14, 2, 2));
    }
  }
  strip->show();
}


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
    B16(00,00000000),
    B16(00,11000000),
    B16(00,11111100),
    B16(00,11111100),
    B16(00,11000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,11001100),
    B16(00,00110000),
    B16(00,11111100),
    B16(00,11111100),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,11111100),
    B16(00,11111100),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,01101100),
    B16(00,11010000),
    B16(00,11010000),
    B16(00,11111100),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,11000000),
    B16(00,11111100),
    B16(00,11111100),
    B16(00,11000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,01011000),
    B16(00,11011000),
    B16(00,11101100),
    B16(00,01101100),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,11111100),
    B16(00,11111100),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,01111000),
    B16(00,10000100),
    B16(00,11111100),
    B16(00,11111100),
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
    B16(00,00000000)};

  clearScreen();
  for (row = 0; row < sizeof(distrikt) / sizeof(uint16_t); row++) {
    for (x = 0; x < 10; x++) {
      strip->setPixelColor(x,  row, distrikt[row] & (1<<(x))? rgbTo24BitColor(255, 255, 255): rgbTo24BitColor(0, 0, 0));
    }
  }
  strip->show();
}

void drawVMW() {
  uint16_t x;
  uint16_t row;

  // VMW
  uint16_t vmw[] = {
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,01100100),
    B16(00,11110110),
    B16(01,10010010),
    B16(01,00010010),
    B16(01,00010010),
    B16(00,10010110),
    B16(00,01111100),
    B16(01,00000000),
    B16(01,10000000),
    B16(00,10000000),
    B16(00,11111110),
    B16(01,11111110),
    B16(00,00000000),
    B16(00,01111110),
    B16(00,10100100),
    B16(01,00100010),
    B16(01,00100010),
    B16(01,00100010),
    B16(00,10010110),
    B16(00,00001100),
    B16(01,11000000),
    B16(00,11111000),
    B16(00,00001110),
    B16(00,00011100),
    B16(00,01110000),
    B16(01,11000000),
    B16(00,01110000),
    B16(00,00011100),
    B16(00,00001110),
    B16(00,11111000),
    B16(01,11000000),
    B16(00,00000000),
    B16(00,11111110),
    B16(01,11111110),
    B16(01,10000000),
    B16(01,10000000),
    B16(00,11111110),
    B16(00,11111110),
    B16(01,11000000),
    B16(01,10000000),
    B16(00,10000000),
    B16(00,11111110),
    B16(01,11111110),
    B16(01,10000000),
    B16(01,11000000),
    B16(00,11111000),
    B16(00,00111100),
    B16(00,00001110),
    B16(00,00111100),
    B16(00,11111000),
    B16(01,11000000),
    B16(00,10000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000)};

  clearScreen();
  for (row = 0; row < sizeof(vmw) / sizeof(uint16_t); row++) {
    for (x = 0; x < 10; x++) {
      strip->setPixelColor(x,  row, vmw[row] & (1<<(x))? rgbTo24BitColor(255, 255, 255): rgbTo24BitColor(0, 0, 0));
    }
  }
  strip->show();
}


void drawTheMan(uint32_t color) {
  uint16_t x;
  uint16_t row;

  uint16_t the_man[] = {
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
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(00,00000000),
    B16(10,00000001),
    B16(11,00000011),
    B16(11,00000011),
    B16(01,10000110),
    B16(01,10000110),
    B16(01,10000110),
    B16(01,10000110),
    B16(00,11001100),
    B16(00,11001100),
    B16(00,11001100),
    B16(00,11001100),
    B16(00,11001100),
    B16(00,11001100),
    B16(00,01001000),
    B16(00,01001000),
    B16(00,01001000),
    B16(00,01001000),
    B16(00,01001000),
    B16(00,01001000),
    B16(00,11001100),
    B16(00,11001100),
    B16(00,11001100),
    B16(00,11001100),
    B16(00,11001100),
    B16(00,10000100),
    B16(01,10000110),
    B16(01,10000110),
    B16(01,10110110),
    B16(11,00110011),
    B16(11,01111011),
    B16(10,01111001),
    B16(00,00110000),
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
  for (row = 0; row < sizeof(the_man) / sizeof(uint16_t); row++) {
    for (x = 0; x < 10; x++) {
      strip->setPixelColor(x,  row, the_man[row] & (1<<(x))? color: rgbTo24BitColor(0, 0, 0));
    }
  }
  strip->show();
}

void cycleTheMan(){
  uint32_t color;
  uint8_t the_red;
  uint8_t the_green;
  uint8_t the_blue;
  uint8_t the_cycle;
  the_red = 100;
  the_green = 100;
  the_blue = 100;
  wheel_color = 255;
  for (the_cycle = 0; the_cycle < 100; the_cycle++) {
        the_red = random(2,4)%2 == 0 ? rgbTo24BitColor(80, 80, 80): wheel(wheel_color); //Chance of 1/3rd 
        the_green = random(2,4)%2 == 0 ? rgbTo24BitColor(80, 80, 80): wheel(wheel_color); //Chance of 1/3rd  
        the_blue = random(2,4)%2 == 0 ? rgbTo24BitColor(80, 80, 80): wheel(wheel_color); //Chance of 1/3rd 
        drawTheMan(rgbTo24BitColor(the_red, the_green, the_blue));
        mydelay(20);
  }
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
      strip->setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, 0, 0));
      x++;
      strip->setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, RGB_DIM, RGB_DIM));
    }
    shiftMatrixLines();
    strip->show();
  }

  // Red and White with solid blue
  for (x = 0; x < 4; x++) {
    strip->setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, 0, 0));
    x++;
    strip->setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, RGB_DIM, RGB_DIM));
  }
  shiftMatrixLines();
  strip->show();
  row++;


  // Red/white
  for (x = 0; x < 4; x++) {
    strip->setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, 0, 0));
    x++;
    strip->setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, RGB_DIM, RGB_DIM));
  }
  // Solid Blue line
  for (; x < 10; x++) {
    strip->setPixelColor(x, 69, rgbTo24BitColor(0, 0, RGB_DIM));
  }
  shiftMatrixLines();
  strip->show();

  for (row = 0; row < 20; row++) {
    // Red/white
    for (x = 0; x < 4; x++) {
      strip->setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, 0, 0));
      x++;
      strip->setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, RGB_DIM, RGB_DIM));
    }
    // White/Blue
    for (x = 4; x < 10; x++) {
      strip->setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, RGB_DIM, RGB_DIM));
      x++;
      strip->setPixelColor(x, 69, rgbTo24BitColor(0, 0, RGB_DIM));
    }    
    shiftMatrixLines();
    strip->show();  
    // Blue/white
    for (x = 4; x < 10; x++) {
      strip->setPixelColor(x, 69, rgbTo24BitColor(0, 0, RGB_DIM));
      x++;
      strip->setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, RGB_DIM, RGB_DIM));
    }
    shiftMatrixLines();
    strip->show();
    row++;

  }

  // Red/white
  for (x = 0; x < 4; x++) {
    strip->setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, 0, 0));
    x++;
    strip->setPixelColor(x, 69, rgbTo24BitColor(RGB_DIM, RGB_DIM, RGB_DIM));
  }
  // Blue line
  for (; x < 10; x++) {
    strip->setPixelColor(x, 69, rgbTo24BitColor(0, 0, RGB_DIM));
  }
  shiftMatrixLines();


  // 10 lines of blank
  for (x = 0; x < 10; x++) {
    strip->setPixelColor(x, 69, rgbTo24BitColor(0, 0, 0));
  }
  for (row = 0; row < 10; row++) {
    shiftMatrixLines();
    strip->show();
  }

}


void drawInvader(uint8_t invader) {
  uint32_t color;
  uint16_t x;
  for (row = 0; row < 8; row++) {
    for (x = 1; x < 9; x++) {
      if (invader) {
        strip->setPixelColor(x, 30 + row, ledn[row] & rgbTo24BitColor(0, 0, 0));
        strip->setPixelColor(x, 30 + row, ledo[row] & (1<<(x - 1))? rgbTo24BitColor(0, 0, 0): rgbTo24BitColor(0, 100, 0));
      } else {
        strip->setPixelColor(x, 30 + row, ledo[row] & rgbTo24BitColor(0, 0, 0));
        strip->setPixelColor(x, 30 + row, ledn[row] & (1<<(x - 1))? rgbTo24BitColor(0, 0, 0): rgbTo24BitColor(0, 100, 0));
      }
    }
    strip->show();
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
    strip->setPixelColor(x, 69, color);
    // Ripple down the side lights with the same color as the edges
    if (x == 0) {
        setSideLight(0, 78, color);
    }
    if (x == 9) {
        setSideLight(1, 78, color);
    }
    wheel_color++;
  }
  strip->show();
}

void drawHeaderLunarian() {
  uint32_t color;
  uint16_t x;

  for (x = 0; x < 10; x++) {
    //   color = random(2,4)%2 == 0 ? rgbTo24BitColor(0,0,0) : rgbTo24BitColor(0, 255, 0); //Chance of 1/3rd 
    color = random(2,4)%2 == 0 ? rgbTo24BitColor(0, 0, 0): rgbTo24BitColor(128, 128, 128); //Chance of 1/3rd 
    //   color = random(2,4)%2 == 0 ? rgbTo24BitColor(0, 0, 0): rgbTo24BitColor(255, 255, 255); //Chance of 1/3rd 
    //   color =  rgbTo24BitColor(255, 255, 255); //Chance of 1/3rd 
    strip->setPixelColor(x, 69, color);
    // Ripple down the side lights with the same color as the edges
    if (x == 0) {
        setSideLight(0, 78, color);
    }
    if (x == 9) {
        setSideLight(1, 78, color);
    }
    wheel_color++;
  }
  strip->show();
}

static int pd_x;
static int pd_y;

void drawPixelDust() {
  uint32_t color;
  uint16_t x, y;

  x = random(9);
  y = random(69);
  color = wheel(random(255));
  strip->setPixelColor(pd_x, pd_y, rgbTo24BitColor(0, 0, 0));
  strip->setPixelColor(pd_x+1, pd_y, rgbTo24BitColor(0, 0, 0));
  strip->setPixelColor(pd_x, pd_y+1, rgbTo24BitColor(0, 0, 0));
  strip->setPixelColor(pd_x+1, pd_y+1, rgbTo24BitColor(0, 0, 0));
  strip->setPixelColor(x, y, color);
  strip->setPixelColor(x+1, y, color);
  strip->setPixelColor(x, y+1, color);
  strip->setPixelColor(x+1, y+1, color);
  strip->show();
  pd_x = x;
  pd_y = y;
}

void drawPixelDust2() {
  uint32_t color;
  uint16_t x, y;

  x = random(10);
  y = random(70);
  color = wheel(random(255));
  strip->setPixelColor(x, y, color);
  pd_x = x;
  pd_y = y;
}

void drawStatic() {
  uint32_t color;
  uint16_t x, y;

  x = random(10);
  y = random(70);
  color = rgbTo24BitColor(200, 200, 200);
  strip->setPixelColor(x, y, color);
  pd_x = x;
  pd_y = y;
}


/* 
 *      |
 *     -#-
 *      |
*/
static int flake_row = 0;
static int flake_col = 0;
	
void drawSnowFlakes() {
	int x;

// Blue Background
  for (x = 0; x < 10; x++) {
		strip->setPixelColor(x, 69, rgbTo24BitColor(0, 0, 20));
	}

	switch(flake_row) {
		case 0:
		flake_col = random() % 8 + 1;
		strip->setPixelColor(flake_col, 69, rgbTo24BitColor(64, 64, 64));
		break;
		
		case 1:
		strip->setPixelColor(flake_col - 1, 69, rgbTo24BitColor(64, 64, 64));
		strip->setPixelColor(flake_col, 69, rgbTo24BitColor(255, 255, 255));
		strip->setPixelColor(flake_col + 1, 69, rgbTo24BitColor(64, 64, 64));		
		break;
		
		case 2:
		strip->setPixelColor(flake_col, 69, rgbTo24BitColor(64, 64, 64));
		break;
		
		case 3:
		break;

    default:
		break;
	}
	
	flake_row++;
	if (flake_row > 4) {
		flake_row = 0;
	}
}

void drawCenter() {
  uint32_t color;
  uint16_t x;

    color = random(2,4)%2 == 0 ? rgbTo24BitColor(0, 0, 0): wheel(wheel_color); //Chance of 1/3rd 
//    color = rgbTo24BitColor(RGB_MAX, RGB_MAX, RGB_MAX);
    strip->fillCircle(35, 5, 1, color);
    wheel_color++;
    strip->show();
}


uint16_t readID() {
 uint16_t bit;
 uint16_t id;
 
 bit = digitalRead(ID_0);
 Serial.print(bit, BIN);
 id = !bit;
 bit = digitalRead(ID_1);
 Serial.print(bit, BIN);
 id |= !bit << 1;
 bit = digitalRead(ID_2);
 Serial.print(bit, BIN);
 id |= !bit << 2; 
 bit = digitalRead(ID_3);
 Serial.print(bit, BIN);
 id |= !bit << 3;
 
 Serial.print("Board ID  ");
 Serial.print(id, DEC);
 Serial.println("");

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
  strip->show();
  mydelay(1000);

  // Convert to level 0-28
  for (i = 0; i < 100; i++) {
    level += sample = analogRead(BATTERY_PIN);
    Serial.print("Battery sample ");
    Serial.print(sample, DEC);
    Serial.println(" ");
  }
  Serial.print("Battery Level ");
  Serial.print(level, DEC);
  Serial.println(" ");

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

  Serial.print("Adjusted Level ");
  Serial.print(level, DEC);
  Serial.println(" ");


  row = 20;

  // White Battery Shell with Green level

  // Battery Bottom
  for (x = 0; x < 10; x++) {
    strip->setPixelColor(x, row, rgbTo24BitColor(RGB_MAX, RGB_MAX, RGB_MAX));
  }
  row++;

  // Battery Sides
  for (; row < 49; row++) {
    strip->setPixelColor(0, row, rgbTo24BitColor(RGB_MAX, RGB_MAX, RGB_MAX));
    strip->setPixelColor(9, row, rgbTo24BitColor(RGB_MAX, RGB_MAX, RGB_MAX));
  }

  // Battery Top
  for (x = 0; x < 10; x++) {
    strip->setPixelColor(x, row, rgbTo24BitColor(RGB_MAX, RGB_MAX, RGB_MAX));
  }
  row++;

  // Battery button
  for (x = 3; x < 7; x++) {
    strip->setPixelColor(x, row, rgbTo24BitColor(RGB_MAX, RGB_MAX, RGB_MAX));
    strip->setPixelColor(x, row+1, rgbTo24BitColor(RGB_MAX, RGB_MAX, RGB_MAX));
  }
  row+=2;

  // Battery Level
  for (row = 21; row < 21 + level; row++) {
    for (x = 1; x < 9; x++) {
      strip->setPixelColor(x, row, rgbTo24BitColor(0, RGB_DIM, 0));
    }
  }

  strip->show();
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
    strip->setPixelColor(x, y, wheel(j));
    strip->show();
    mydelay(wait);
    strip->setPixelColor(x, y, 0, 0, 0);
  }
}


void setup() {

  uint16_t i;

  // Console for debugging
  Serial.begin(9600);
  Serial.println("Goodnight moon!");

  // Set battery level analogue reference
#ifdef MEGA
  analogReference(INTERNAL1V1);
#else
  //  analogReference(INTERNAL);
#endif
  pinMode(BATTERY_PIN, INPUT);
  pinMode(MOT_PIN, INPUT);
  pinMode(REMOTE_PIN, INPUT);
  digitalWrite(REMOTE_PIN, LOW);
  pinMode(SRELAY_PIN, OUTPUT);
  pinMode(LRELAY_PIN, OUTPUT);
  digitalWrite(SRELAY_PIN, HIGH);
  
// ID Pins  
  pinMode(ID_0, INPUT);
  digitalWrite(ID_0, HIGH);
  pinMode(ID_1, INPUT);
  digitalWrite(ID_1, HIGH);
  pinMode(ID_2, INPUT);
  digitalWrite(ID_2, HIGH);
  pinMode(ID_3, INPUT);
  digitalWrite(ID_3, HIGH);
  
  // Encoder Pins
  pinMode(ENC_A, INPUT);
  digitalWrite(ENC_A, HIGH);
  pinMode(ENC_B, INPUT);
  digitalWrite(ENC_B, HIGH);

  /*
     for (uint16_t i = 0; i < 544; i++) {
     Serial.print("Strip pixel ");
     Serial.print(i, DEC);
     Serial.print(" = virt pixel ");
     Serial.print(strip->pixel_translate[i], DEC);
     Serial.println(" ");
     }
   */

  boardId = readID();
  
  if (boardId == 0 || boardId == 2) {
    strip = new Board_WS2801((uint16_t)10, (uint16_t)70, WS2801_RGB, (boolean)true);
  } else {
    strip = new Board_WS2801((uint16_t)10, (uint16_t)70, WS2801_RGB, (boolean)false);
  }

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

  strip->begin();

  invader = 0;

  // test zone
/*
  clearScreen();
  int x;
  for (x = 0; x < 858; x++) {
     strip->setPixelColor(x, rgbTo24BitColor(50,50,50));
     strip->show();
    mydelay(50);
  }
       strip->setPixelColor(700, rgbTo24BitColor(0,0,50));
       strip->setPixelColor(779, rgbTo24BitColor(0,0,50));
     strip->show();

  mydelay(15000);
  */
  
  
  // Update LED contents, to start they are all 'off'
  clearScreen();
  strip->show();
  
  strip->print(boards[boardId], 15, 1, 1);
  strip->show();
  mydelay(1000);

  clearScreen();
  strip->print(names[boardId], 15, 1, 1);
  strip->show();
  mydelay(1000);


//  strip->fillCircle(35, 5, 3, rgbTo24BitColor(RGB_MAX, RGB_MAX, RGB_MAX));
  //strip->circles(15, 5, 5);
//  strip->show();
//  mydelay(5000);
  
//  for (i = 0; i < 500; i++) {
//    rainbowCycle();
//  }

  drawBattery();
  mydelay(1000);

  clearScreen();

}


void loop_matrix()
{
  drawHeader();
  shiftMatrixLines();
  mydelay(50);
}

void loop_matrixfast()
{
  drawHeader();
  shiftMatrixLines();
  mydelay(1);
}


void loop_lunarian()
{
  drawHeaderLunarian();
  shiftMatrixLines(); 
  mydelay(1);
}


void loop_battery()
{
  drawBattery();
  mydelay(1000);
  clearScreen();
}


void loop_distrikt()
{
  drawDistrikt();
  mydelay(10);
}

void loop_stanford(uint8_t enc)
{
  int i;
  
      for (i = 0; i < 20  && encoder_pos == enc; i++) {
        drawStanford();
        strip->show();
        mydelay(300);
        fillScreen(rgbTo24BitColor(14, 2, 2u));
        strip->show();
        mydelay(300);
      }
}
  
void loop_pixeldust() {
  drawPixelDust();
  mydelay(5);

}

void loop_pixeldust2() {
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  drawPixelDust2();
  fadeBoard();
  strip->show();
  mydelay(1);

}

void loop_static() {
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  drawStatic();
  fadeBoard();
  strip->show();
  mydelay(1);

}


void loop_rainbow() {
  rainbowCycle2();
  mydelay(5);
}



int16_t loopcnt = 0;

int16_t state = 0;
// state = 0: lines
// state = 1: flag


void loop() {
  int i;
  

   
   /*
   Serial.print("Encoder sample ");
   Serial.print(encoder_pos, DEC);
   Serial.println(" ");
   */
   
   switch (encoder_pos) {
     
     case 0:
       loop_matrix();
       break;
 
      case 1:
       loop_matrixfast();
       break;
       
     case 2:
       loop_lunarian();
       break;
       
     case 3:
       loop_distrikt();
       break;
       
     case 4:
       loop_pixeldust();
       break;
       
     case 5:
       loop_pixeldust2();
       break;
       
     case 6:
       loop_rainbow();
       break;

     case 7:
       loop_stanford(7);
       break;
       
     case 8:
       loop_battery();
       break;

     case 9:
       loop_static();
       encoder_pos = 9;     
       
     default:
       mydelay(1);
       break;
   }  

}




uint16_t buttonPress = 0;
bool lightState = false;

#define HOLD_COUNT 30

void checkButton() {
  uint16_t remotePosition;

#ifdef MEGA
  remotePosition = analogRead(REMOTE_PIN);
  //  Serial.print("Remote position ");
  //  Serial.println(remotePosition);

  //  Serial.print("Button position ");
  //  Serial.println(buttonPress);

  if (remotePosition > 100) {
    ledsOn = true;
  } else {
    ledsOn = false;
  }

  if (buttonPress == HOLD_COUNT) {
    if (lightState == false) {
      lightState = true;
      digitalWrite(LRELAY_PIN, HIGH);
    } else {
      if (lightState == true) {
        lightState = false;
        digitalWrite(LRELAY_PIN, LOW);
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


#ifdef CRAP


   if (encoder_pos == 2) {
      drawUSflag();
      loopcnt += 50;
   }

#ifdef FOO

   if (ledsOn) {

    if (loopcnt > 3000) {
      loopcnt = 0;
      state++;
    }

    if (encoder_pos == 1) {
 /*
      for (i = 0; i < 5; i++) {
        drawVMW();
        strip->show();
        mydelay(2000);
        clearScreen();
        strip->show();
        mydelay(2000);
      }
 */
    
      drawDistrikt();
      state = 1;    

/*
      for (i = 0; i < 20; i++) {
        drawStanford();
        strip->show();
        mydelay(300);
        fillScreen(rgbTo24BitColor(14, 2, 2u));
        strip->show();
        mydelay(300);
      }

      state = 1;    
      drawSnowFlakes();
      shiftMatrixLines();

*/
    }



//    if (state == 0) {
//      drawCenter();
//      shiftMatrixCircles();
//    }  

    if (encoder_pos == 0) {
//      state = 2;
      drawHeader();
      shiftMatrixLines();
    }  

    if (encoder_pos == 2) {
      drawUSflag();
      loopcnt += 50;
    }


    if (encoder_pos == 3) {
      drawStanfordLogo();
      drawStanfordTree();
      mydelay(10000);
      state = 3;    
    }



// The Man
    if (encoder_pos == 4) {
        for (row = 0; row < 10; row++) {
          strip->setPixelColor(row, 69, 0);
        }
        strip->show();
        for (row = 0; row < 70; row++) {
          shiftMatrixLines();
          strip->show();
        }
 
        cycleTheMan();
        state = 3;    
    }
 

    if (encoder_pos == 5) {
      loopcnt = 0;
      state = 0;
      drawBattery();
      mydelay(1000);
      clearScreen();
    }


    /*
       drawInvader(invader);
       mydelay(500);

       if (invader == 0) {
       invader = 1;
       } else {
       invader = 0;
       }
     */
  } else {
    clearScreen();
    strip->show();
  }

  loopcnt++;
  
#endif
#endif

