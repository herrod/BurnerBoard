#include "SPI.h"
#include "Board_ST7735.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "Print.h"


#define NUM_REAL_BOARD_PIXELS 544

#define sclk 13
#define mosi 11
#define cs   10
#define dc   8
#define rst  0

//Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, mosi, sclk, rst);
Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, rst);


Board_ST7735::Board_ST7735(uint16_t w, uint16_t h, uint8_t order) : Adafruit_GFX(ST7735_TFTWIDTH, ST7735_TFTHEIGHT){

  rgb_order = order;
  alloc(w * h);
  translationArray(NUM_REAL_BOARD_PIXELS);
  width = w;
  height = h;

}

// Allocate 3 bytes per pixel, init to RGB 'off' state:
void Board_ST7735::alloc(uint16_t n) {
  begun   = false;
  numvirtLEDs = ((pixels = (uint8_t *)calloc(n, 3)) != NULL) ? n : 0;
}

void Board_ST7735::begin(){

	  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab

	  tft.fillScreen(ST7735_BLACK);
	
}

// A cache of the pixel translations for the Burner Board LED strings
// For performance, translate and cache the result (math is more expensive than memory op)
// Cycles for load = 1
// Performance per instruction on page 10 of http://www.atmel.com/Images/doc0856.pdf
// translation is real board string pixel# = pixel_translate(virtual matrix with holes pixel#)
void Board_ST7735::translationArray(uint16_t n) {
  uint16_t virtpixel, newpixel, rgb;
  
  numboardLEDs = n;
  
  // Allocate Burner Board pix translation map
  pixel_translate = (uint16_t *)calloc(numboardLEDs, 3);

  if (pixel_translate == NULL)
	  return;

  // For each virt board pixel translate to real board matrix positions
  for(virtpixel = 0; virtpixel < numvirtLEDs; virtpixel ++) {
    // Array Pixel starts at 0,0 and translation map calc'ed with start of 1,1
	  newpixel = BoardPixel(virtpixel + 1);
    if (newpixel) {
      // Array Pixel starts at 0,0 and translation map calc'ed with start of 1,1
      newpixel--;
	    for (rgb = 0; rgb < 3; rgb ++) {
	  	    pixel_translate[(newpixel * 3) + rgb] = (virtpixel * 3) + rgb;
	    }
    }
  }
}

// Release memory (as needed):
Board_ST7735::~Board_ST7735(void) {
  if (pixels != NULL) {
    free(pixels);
  }
}



uint16_t Board_ST7735::numPixels(void) {
  return numvirtLEDs;
}

// Change strand length (see notes with empty constructor, above):
void Board_ST7735::updateLength(uint16_t n) {
  if(pixels != NULL) free(pixels); // Free existing data (if any)
  // Allocate new data -- note: ALL PIXELS ARE CLEARED
  numvirtLEDs = ((pixels = (uint8_t *)calloc(n, 3)) != NULL) ? n : 0;
  // 'begun' state does not change -- pins retain prior modes
}

// Change RGB data order (see notes with empty constructor, above):
void Board_ST7735::updateOrder(uint8_t order) {
  rgb_order = order;
  // Existing LED data, if any, is NOT reformatted to new data order.
  // Calling function should clear or fill pixel data anew.
}



// Clock out the actual Burner Board Pixels without holes
// 
void Board_ST7735::show(void) {
  uint16_t y, x;
	uint16_t color;
	unsigned char *pixel;
	uint8_t r, g, b;

  for (x = 0; x < width; x++) {
    for(y=0 ; y<height; y++) {
			pixel = &pixels[3 * (x * height + y)];
			r = *pixel++;
			g = *pixel++;
			g = *pixel;
			tft.drawPixel(x * 2 + 50, (162 - y * 2), tft.Color565(r, g, b));
			tft.drawPixel(x * 2 + 51, (162 - y * 2), tft.Color565(r, g, b));
    }
  }
}

// Set pixel color from separate 8-bit R, G, B components:
void Board_ST7735::setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
  if(n < numvirtLEDs) { // Arrays are 0-indexed, thus NOT '<='
    uint8_t *p = &pixels[n * 3];
    // See notes later regarding color order
    if(rgb_order == ST7735_RGB) {
      *p++ = r;
      *p++ = g;
    } else {
      *p++ = g;
      *p++ = r;
    }
    *p++ = b;
  }
}

// Set pixel color from separate 8-bit R, G, B components using x,y coordinate system:
void Board_ST7735::setPixelColor(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b) {
  // calculate x offset first
  uint16_t offset = y % height;
  // add x offset
  offset += x * height;
  setPixelColor(offset, r, g, b);
}

// Set pixel color from 'packed' 32-bit RGB value:
void Board_ST7735::setPixelColor(uint16_t n, uint32_t c) {
  if(n < numvirtLEDs) { // Arrays are 0-indexed, thus NOT '<='
    uint8_t *p = &pixels[n * 3];
    // To keep the show() loop as simple & fast as possible, the
    // internal color representation is native to different pixel
    // types.  For compatibility with existing code, 'packed' RGB
    // values passed in or out are always 0xRRGGBB order.
    if(rgb_order == ST7735_RGB) {
      *p++ = c >> 16; // Red
      *p++ = c >>  8; // Green
    } else {
      *p++ = c >>  8; // Green
      *p++ = c >> 16; // Red
    }
    *p++ = c;         // Blue
  }
}

// Set pixel color from 'packed' 32-bit RGB value using x,y coordinate system:
void Board_ST7735::setPixelColor(uint16_t x, uint16_t y, uint32_t c) {
  // calculate x offset first
  uint16_t offset = y % height;
  // add x offset
  offset += x * height;
  setPixelColor(offset, c);
}

// Query color from previously-set pixel (returns packed 32-bit RGB value)
uint32_t Board_ST7735::getPixelColor(uint16_t n) {
  if(n < numvirtLEDs) {
    uint16_t ofs = n * 3;
    // To keep the show() loop as simple & fast as possible, the
    // internal color representation is native to different pixel
    // types.  For compatibility with existing code, 'packed' RGB
    // values passed in or out are always 0xRRGGBB order.
    return (rgb_order == ST7735_RGB) ?
      ((uint32_t)pixels[ofs] << 16) | ((uint16_t) pixels[ofs + 1] <<  8) | pixels[ofs + 2] :
      (pixels[ofs] <<  8) | ((uint32_t)pixels[ofs + 1] << 16) | pixels[ofs + 2];
  }

  return 0; // Pixel # is out of bounds
}

// Query color from previously-set pixel (returns packed 32-bit RGB value)
uint32_t Board_ST7735::getPixelColor(uint16_t x, uint16_t y) {
  // calculate x offset first
  uint16_t n = y % height;
  // add x offset
  n += x * height;
  if(n < numvirtLEDs) {
    uint16_t ofs = n * 3;
    // To keep the show() loop as simple & fast as possible, the
    // internal color representation is native to different pixel
    // types.  For compatibility with existing code, 'packed' RGB
    // values passed in or out are always 0xRRGGBB order.
    return (rgb_order == ST7735_RGB) ?
      ((uint32_t)pixels[ofs] << 16) | ((uint16_t) pixels[ofs + 1] <<  8) | pixels[ofs + 2] :
      (pixels[ofs] <<  8) | ((uint32_t)pixels[ofs + 1] << 16) | pixels[ofs + 2];
  }

  return 0; // Pixel # is out of bounds
}



// Map virtual pixels to physical pixels on the Burner Board Layout
// Emulate a 70 x 10 rectangle matrix 
// Strip lengths are 31, 45, 60, 66, 70, 70, 66, 60, 45, 31
// format is colx: virt pixel offset -> real pixel offset
// col1: 1-19, 20-50, 51-70: 20-50->1-31
// col2: 71-140: 71-82, 83-127, 128-140: 83-127->76-32 
// col3: 141-210: 141-145, 146-205, 206-210: 146-205->77-136
// col4: 211-280: 211-212, 213->278, 279-280: 213-278->202-137
// col5: 281-350: 281-350: 281-350->203-272
// col6: 351-420: 351-420: 351-420->342-273
// col7: 421-490: 421-422, 423-488, 489-490: 423-488->343-408
// col8: 491-560: 491-495, 496-555, 556-560: 496-555->468-409
// col9: 561-630: 561-572, 573-617, 618-630: 573-617->469-513
// col10: 631-700: 631-649, 650-680, 681-700: 650-680->544-514
uint32_t Board_ST7735::BoardPixel(uint32_t pixel) {
  uint32_t newpixel;

  // Pixel is a hole in the map, returns 0
  newpixel = 0;

  // Map linear row x column strip into strip with holes in grid
  // to cater for pixels that are missing from the corners of the
  // Burner Board layout

  //1  20-50->1-31
  if (pixel >= 20 && pixel <=50)
    newpixel = pixel - 19;
  //2 83-127->76-32
  if (pixel >= 83 && pixel <= 127)
    newpixel = 127 - pixel + 32;
  //3 146-205->77-136
  if (pixel >= 146 && pixel <= 205)
    newpixel = pixel - 146 + 77;
  //4 213-278->202-137
  if (pixel >= 213 && pixel <= 278)
    newpixel = 278 - pixel + 137;
  //5 281-350->203-272
  if (pixel >= 281 && pixel <= 350)
    newpixel = pixel - 281 + 203;
  //6 351-420->342-273
  if (pixel >= 351 && pixel <=420)
    newpixel = 420 - pixel + 273;
  //7 423-488->343-408
  if (pixel >= 423 && pixel <= 488)
    newpixel = pixel - 423 + 343;
  //8 496-555->468-409
  if (pixel >= 496 && pixel <= 555)
    newpixel = 555 - pixel + 409;
  //9 573-617->469-513
  if (pixel >= 573 && pixel <= 617)
    newpixel = pixel - 573 + 469;
  //10 650-680->544-514
  if (pixel >= 650 && pixel <= 680)
    newpixel = 680 - pixel + 514;
    
  return newpixel;
}  

// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint32_t Color2rgb(uint16_t color) {
	uint8_t r, g, b;
  uint32_t return_color;

  r = color & 0xf800;
	r = r << 5;
	
  g = color & 0x7e0;
	g = g << 3;

  b = color & 0x1F;
	b = b << 2;	
	
	return_color = r & g & b;

  return r;
}

void Board_ST7735::drawPixel(int16_t x, int16_t y, uint16_t color) {

	// calculate x offset first
  uint16_t offset = y % height;
  // add x offset
  offset += x * height;
  setPixelColor(offset, Color2rgb(color));

}


void Board_ST7735::print(char *string, uint8_t x, uint8_t y, uint8_t size) {
		  setTextSize(size);
		  setTextColor(ST7735_MAGENTA);
			fillRect(x, y,  7 * size * strlen(string) + 1, 7 * size, ST7735_BLACK);
		  setCursor(x, y);
		  print(string);	
}


