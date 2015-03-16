#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
 #include <pins_arduino.h>
#endif

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "Print.h"


// Not all LED pixels are RGB order; 36mm type expects GRB data.
// Optional flag to constructors indicates data order (default if
// unspecified is RGB).  As long as setPixelColor/getPixelColor are
// used, other code can always treat 'packed' colors as RGB; the
// library will handle any required translation internally.
#define ST7735_RGB 0
#define ST7735_GRB 1

class Board_ST7735  : public Adafruit_GFX {

 public:

  // Configurable pins:
  Board_ST7735(uint16_t n, uint8_t dpin, uint8_t cpin, uint8_t order=ST7735_RGB);
  Board_ST7735(uint16_t x, uint16_t y, uint8_t dpin, uint8_t cpin, uint8_t order=ST7735_RGB);
  Board_ST7735(uint16_t x, uint16_t y, uint8_t order=ST7735_RGB);
  // Use SPI hardware; specific pins only:
  Board_ST7735(uint16_t n, uint8_t order=ST7735_RGB);
  // Empty constructor; init pins/strand length/data order later:
  Board_ST7735();
  // Release memory (as needed):
  ~Board_ST7735();
  // GFX Library
  uint16_t Color565(uint8_t r, uint8_t g, uint8_t b);
  void drawPixel(int16_t x, int16_t y, uint16_t color);

  // These MAY be overridden by the subclass to provide device-specific
  // optimized code.  Otherwise 'generic' versions are used.
  //void
    //drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color),
    //drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color),
    //drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color),
    //drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color),
    //fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color),
    //fillScreen(uint16_t color),
    //invertDisplay(boolean i);



  void
    begin(void),
    show(void),
    setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b),
    setPixelColor(uint16_t n, uint32_t c),
    setPixelColor(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b),
    setPixelColor(uint16_t x, uint16_t y, uint32_t c),
    updatePins(uint8_t dpin, uint8_t cpin), // Change pins, configurable
    updatePins(void), // Change pins, hardware SPI
    updateLength(uint16_t n), // Change strand length
    updateOrder(uint8_t order), // Change data order
    enableSidelights(boolean haslights), // Has side lights
    print(char *string, uint8_t x, uint8_t y, uint8_t size);


  uint16_t
    numPixels(void),
    *pixel_translate;
  uint32_t
    getPixelColor(uint16_t n),
    getPixelColor(uint16_t x, uint16_t y);

 private:

  uint16_t
    numvirtLEDs,
    numboardLEDs,
    width,     // used with matrix mode
		height;    // used with matrix mode
  uint8_t
    *pixels,   // Holds color values for each LED (3 bytes each)
    rgb_order, // Color order; RGB vs GRB (or others, if needed in future)
    clkpin    , datapin,     // Clock & data pin numbers
    clkpinmask, datapinmask; // Clock & data PORT bitmasks
  volatile uint8_t
    *clkport  , *dataport;   // Clock & data PORT registers
  void
    alloc(uint16_t n),
    translationArray(uint16_t n),
    startSPI(void);
  boolean
    hardwareSPI, // If 'true', using hardware SPI
    begun,       // If 'true', begin() method was previously invoked
    hasSidelights;       // If 'true', extra lights on side

  uint32_t
    BoardPixel(uint32_t pixel);
};
