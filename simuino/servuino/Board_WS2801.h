#if (ARDUINO >= 100)
 #include <Arduino.h>
 #include <WProgram.h>
 #include <pins_arduino.h>
#endif

#include <stdlib.h>
#include <stdint.h>

// Not all LED pixels are RGB order; 36mm type expects GRB data.
// Optional flag to constructors indicates data order (default if
// unspecified is RGB).  As long as setPixelColor/getPixelColor are
// used, other code can always treat 'packed' colors as RGB; the
// library will handle any required translation internally.
#define WS2801_RGB 0
#define WS2801_GRB 1

class Board_WS2801 {

 public:

  // Configurable pins:
  Board_WS2801(uint16_t , uint8_t, uint8_t , uint8_t );
  Board_WS2801(uint16_t , uint16_t , uint8_t , uint8_t cpin, uint8_t );
  Board_WS2801(uint16_t , uint16_t , uint8_t );
  // Use SPI hardware; specific pins only:
  Board_WS2801(uint16_t , uint8_t );
  // Empty constructor; init pins/strand length/data order later:
  Board_WS2801();
  // Release memory (as needed):
  ~Board_WS2801();

  void
    begin(void),
    show(void),
    setPixelColor(uint16_t , uint8_t , uint8_t , uint8_t ),
    setPixelColor(uint16_t , uint32_t ),
    setPixelColor(uint16_t , uint16_t , uint8_t , uint8_t , uint8_t ),
    setPixelColor(uint16_t , uint16_t , uint32_t ),
    updatePins(uint8_t , uint8_t ), // Change pins, configurable
    updatePins(void), // Change pins, hardware SPI
    updateLength(uint16_t ), // Change strand length
    updateOrder(uint8_t ); // Change data order

  uint16_t
    numPixels(void),
    *pixel_translate;
  uint32_t
    getPixelColor(uint16_t ),
    getPixelColor(uint16_t , uint16_t );

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
    alloc(uint16_t ),
    translationArray(uint16_t ),
    startSPI(void);
  bool
    hardwareSPI, // If 'true', using hardware SPI
    begun;       // If 'true', begin() method was previously invoked
  uint32_t
    BoardPixel(uint32_t );
};
