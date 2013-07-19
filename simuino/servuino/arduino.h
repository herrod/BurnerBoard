#include <iostream>
using std::cerr;
using std::cout;
using std::endl;
using std::ostream;
using std::istream;

#include <iomanip>
using std::setw;

#include <cstring> 
using std::strcmp;
using std::strcpy;
using std::strcat;

#include <string> 
using std::string;

#include <cstdlib>
using std::exit;

#include <sstream>
using std::stringstream;


#define LOW    0
#define HIGH   1

#define INPUT  0
#define OUTPUT 1

#define CHANGE  11
#define RISING  12
#define FALLING 13

// Math function min and max
#ifndef max
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

#ifndef min
#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif

#define _BV(bit) (1 << (bit))


typedef int byte;
typedef int word;
typedef bool boolean;

//typedef unsigned int uint8_t;
//typedef unsigned int uint16_t;
//typedef unsigned int uint32_t;
//typedef unsigned int uint64_t;

// Functions
int   bitRead(int x, int n);
void  bitSet(unsigned int *x, int n);

// Wrappers
void pinModeX(int x,int pin,int mode);
void digitalWriteX(int x,int pin,int value);
int  digitalReadX(int x,int pin);
void analogWriteX(int x,int pin,int value);
int  analogReadX(int x,int pin);
void delayX(int x,int ms);
void delayMicrosecondsX(int x,int us);
void attachInterruptX(int x,int ir,void(*func)(),int mode);
void detachInterruptX(int x,int ir);

int A0;
int A1;
int A2;
int A3;
int A4;
int A5;
int A6;
int A7;
int A8;
int A9;
int A10;
int A11;
int A12;
int A13;
int A14;

unsigned int PORTA=0,PORTB=0,PORTC=0,PORTD=0,PORTE=0,PORTF=0,PORTG=0;
unsigned int PORTH=0,PORTJ=0,PORTK=0,PORTL=0,PORTM=0,PORTN=0;

unsigned int DDRA=0,DDRB=0,DDRC=0,DDRD=0,DDRE=0,DDRF=0,DDRG=0;
unsigned int DDRH=0,DDRJ=0,DDRK=0,DDRL=0,DDRM=0,DDRN=0;

unsigned int PINA=0,PINB=0,PINC=0,PIND=0,PINE=0,PINF=0,PING=0;
unsigned int PINH=0,PINJ=0,PINK=0,PINL=0,PINM=0,PINN=0;


unsigned int B00000000=B00000000; // 0
unsigned int B00000001=B00000001; // 1
unsigned int B00000010=B00000010; // 2
unsigned int B00000011=B00000011; // 3
unsigned int B00000100=B00000100; // 4
unsigned int B00000101=B00000101; // 5
unsigned int B00000110=B00000110; // 6
unsigned int B00000111=B00000111; // 7
unsigned int B00001000=B00001000; // 8
unsigned int B00001001=B00001001; // 9
unsigned int B00001010=B00001010; // 10
unsigned int B00001011=B00001011; // 11
unsigned int B00001100=B00001100; // 12
unsigned int B00001101=B00001101; // 13
unsigned int B00001110=B00001110; // 14
unsigned int B00001111=B00001111; // 15
unsigned int B00010000=B00010000; // 16
unsigned int B00010001=B00010001; // 17
unsigned int B00010010=B00010010; // 18
unsigned int B00010011=B00010011; // 19
unsigned int B00010100=B00010100; // 20
unsigned int B00010101=B00010101; // 21
unsigned int B00010110=B00010110; // 22
unsigned int B00010111=B00010111; // 23
unsigned int B00011000=B00011000; // 24
unsigned int B00011001=B00011001; // 25
unsigned int B00011010=B00011010; // 26
unsigned int B00011011=B00011011; // 27
unsigned int B00011100=B00011100; // 28
unsigned int B00011101=B00011101; // 29
unsigned int B00011110=B00011110; // 30
unsigned int B00011111=B00011111; // 31
unsigned int B00100000=B00100000; // 32
unsigned int B00100001=B00100001; // 33
unsigned int B00100010=B00100010; // 34
unsigned int B00100011=B00100011; // 35
unsigned int B00100100=B00100100; // 36
unsigned int B00100101=B00100101; // 37
unsigned int B00100110=B00100110; // 38
unsigned int B00100111=B00100111; // 39
unsigned int B00101000=B00101000; // 40
unsigned int B00101001=B00101001; // 41
unsigned int B00101010=B00101010; // 42
unsigned int B00101011=B00101011; // 43
unsigned int B00101100=B00101100; // 44
unsigned int B00101101=B00101101; // 45
unsigned int B00101110=B00101110; // 46
unsigned int B00101111=B00101111; // 47
unsigned int B00110000=B00110000; // 48
unsigned int B00110001=B00110001; // 49
unsigned int B00110010=B00110010; // 50
unsigned int B00110011=B00110011; // 51
unsigned int B00110100=B00110100; // 52
unsigned int B00110101=B00110101; // 53
unsigned int B00110110=B00110110; // 54
unsigned int B00110111=B00110111; // 55
unsigned int B00111000=B00111000; // 56
unsigned int B00111001=B00111001; // 57
unsigned int B00111010=B00111010; // 58
unsigned int B00111011=B00111011; // 59
unsigned int B00111100=B00111100; // 60
unsigned int B00111101=B00111101; // 61
unsigned int B00111110=B00111110; // 62
unsigned int B00111111=B00111111; // 63
unsigned int B01000000=B01000000; // 64
unsigned int B01000001=B01000001; // 65
unsigned int B01000010=B01000010; // 66
unsigned int B01000011=B01000011; // 67
unsigned int B01000100=B01000100; // 68
unsigned int B01000101=B01000101; // 69
unsigned int B01000110=B01000110; // 70
unsigned int B01000111=B01000111; // 71
unsigned int B01001000=B01001000; // 72
unsigned int B01001001=B01001001; // 73
unsigned int B01001010=B01001010; // 74
unsigned int B01001011=B01001011; // 75
unsigned int B01001100=B01001100; // 76
unsigned int B01001101=B01001101; // 77
unsigned int B01001110=B01001110; // 78
unsigned int B01001111=B01001111; // 79
unsigned int B01010000=B01010000; // 80
unsigned int B01010001=B01010001; // 81
unsigned int B01010010=B01010010; // 82
unsigned int B01010011=B01010011; // 83
unsigned int B01010100=B01010100; // 84
unsigned int B01010101=B01010101; // 85
unsigned int B01010110=B01010110; // 86
unsigned int B01010111=B01010111; // 87
unsigned int B01011000=B01011000; // 88
unsigned int B01011001=B01011001; // 89
unsigned int B01011010=B01011010; // 90
unsigned int B01011011=B01011011; // 91
unsigned int B01011100=B01011100; // 92
unsigned int B01011101=B01011101; // 93
unsigned int B01011110=B01011110; // 94
unsigned int B01011111=B01011111; // 95
unsigned int B01100000=B01100000; // 96
unsigned int B01100001=B01100001; // 97
unsigned int B01100010=B01100010; // 98
unsigned int B01100011=B01100011; // 99
unsigned int B01100100=B01100100; // 100
unsigned int B01100101=B01100101; // 101
unsigned int B01100110=B01100110; // 102
unsigned int B01100111=B01100111; // 103
unsigned int B01101000=B01101000; // 104
unsigned int B01101001=B01101001; // 105
unsigned int B01101010=B01101010; // 106
unsigned int B01101011=B01101011; // 107
unsigned int B01101100=B01101100; // 108
unsigned int B01101101=B01101101; // 109
unsigned int B01101110=B01101110; // 110
unsigned int B01101111=B01101111; // 111
unsigned int B01110000=B01110000; // 112
unsigned int B01110001=B01110001; // 113
unsigned int B01110010=B01110010; // 114
unsigned int B01110011=B01110011; // 115
unsigned int B01110100=B01110100; // 116
unsigned int B01110101=B01110101; // 117
unsigned int B01110110=B01110110; // 118
unsigned int B01110111=B01110111; // 119
unsigned int B01111000=B01111000; // 120
unsigned int B01111001=B01111001; // 121
unsigned int B01111010=B01111010; // 122
unsigned int B01111011=B01111011; // 123
unsigned int B01111100=B01111100; // 124
unsigned int B01111101=B01111101; // 125
unsigned int B01111110=B01111110; // 126
unsigned int B01111111=B01111111; // 127
unsigned int B10000000=B10000000; // 128
unsigned int B10000001=B10000001; // 129
unsigned int B10000010=B10000010; // 130
unsigned int B10000011=B10000011; // 131
unsigned int B10000100=B10000100; // 132
unsigned int B10000101=B10000101; // 133
unsigned int B10000110=B10000110; // 134
unsigned int B10000111=B10000111; // 135
unsigned int B10001000=B10001000; // 136
unsigned int B10001001=B10001001; // 137
unsigned int B10001010=B10001010; // 138
unsigned int B10001011=B10001011; // 139
unsigned int B10001100=B10001100; // 140
unsigned int B10001101=B10001101; // 141
unsigned int B10001110=B10001110; // 142
unsigned int B10001111=B10001111; // 143
unsigned int B10010000=B10010000; // 144
unsigned int B10010001=B10010001; // 145
unsigned int B10010010=B10010010; // 146
unsigned int B10010011=B10010011; // 147
unsigned int B10010100=B10010100; // 148
unsigned int B10010101=B10010101; // 149
unsigned int B10010110=B10010110; // 150
unsigned int B10010111=B10010111; // 151
unsigned int B10011000=B10011000; // 152
unsigned int B10011001=B10011001; // 153
unsigned int B10011010=B10011010; // 154
unsigned int B10011011=B10011011; // 155
unsigned int B10011100=B10011100; // 156
unsigned int B10011101=B10011101; // 157
unsigned int B10011110=B10011110; // 158
unsigned int B10011111=B10011111; // 159
unsigned int B10100000=B10100000; // 160
unsigned int B10100001=B10100001; // 161
unsigned int B10100010=B10100010; // 162
unsigned int B10100011=B10100011; // 163
unsigned int B10100100=B10100100; // 164
unsigned int B10100101=B10100101; // 165
unsigned int B10100110=B10100110; // 166
unsigned int B10100111=B10100111; // 167
unsigned int B10101000=B10101000; // 168
unsigned int B10101001=B10101001; // 169
unsigned int B10101010=B10101010; // 170
unsigned int B10101011=B10101011; // 171
unsigned int B10101100=B10101100; // 172
unsigned int B10101101=B10101101; // 173
unsigned int B10101110=B10101110; // 174
unsigned int B10101111=B10101111; // 175
unsigned int B10110000=B10110000; // 176
unsigned int B10110001=B10110001; // 177
unsigned int B10110010=B10110010; // 178
unsigned int B10110011=B10110011; // 179
unsigned int B10110100=B10110100; // 180
unsigned int B10110101=B10110101; // 181
unsigned int B10110110=B10110110; // 182
unsigned int B10110111=B10110111; // 183
unsigned int B10111000=B10111000; // 184
unsigned int B10111001=B10111001; // 185
unsigned int B10111010=B10111010; // 186
unsigned int B10111011=B10111011; // 187
unsigned int B10111100=B10111100; // 188
unsigned int B10111101=B10111101; // 189
unsigned int B10111110=B10111110; // 190
unsigned int B10111111=B10111111; // 191
unsigned int B11000000=B11000000; // 192
unsigned int B11000001=B11000001; // 193
unsigned int B11000010=B11000010; // 194
unsigned int B11000011=B11000011; // 195
unsigned int B11000100=B11000100; // 196
unsigned int B11000101=B11000101; // 197
unsigned int B11000110=B11000110; // 198
unsigned int B11000111=B11000111; // 199
unsigned int B11001000=B11001000; // 200
unsigned int B11001001=B11001001; // 201
unsigned int B11001010=B11001010; // 202
unsigned int B11001011=B11001011; // 203
unsigned int B11001100=B11001100; // 204
unsigned int B11001101=B11001101; // 205
unsigned int B11001110=B11001110; // 206
unsigned int B11001111=B11001111; // 207
unsigned int B11010000=B11010000; // 208
unsigned int B11010001=B11010001; // 209
unsigned int B11010010=B11010010; // 210
unsigned int B11010011=B11010011; // 211
unsigned int B11010100=B11010100; // 212
unsigned int B11010101=B11010101; // 213
unsigned int B11010110=B11010110; // 214
unsigned int B11010111=B11010111; // 215
unsigned int B11011000=B11011000; // 216
unsigned int B11011001=B11011001; // 217
unsigned int B11011010=B11011010; // 218
unsigned int B11011011=B11011011; // 219
unsigned int B11011100=B11011100; // 220
unsigned int B11011101=B11011101; // 221
unsigned int B11011110=B11011110; // 222
unsigned int B11011111=B11011111; // 223
unsigned int B11100000=B11100000; // 224
unsigned int B11100001=B11100001; // 225
unsigned int B11100010=B11100010; // 226
unsigned int B11100011=B11100011; // 227
unsigned int B11100100=B11100100; // 228
unsigned int B11100101=B11100101; // 229
unsigned int B11100110=B11100110; // 230
unsigned int B11100111=B11100111; // 231
unsigned int B11101000=B11101000; // 232
unsigned int B11101001=B11101001; // 233
unsigned int B11101010=B11101010; // 234
unsigned int B11101011=B11101011; // 235
unsigned int B11101100=B11101100; // 236
unsigned int B11101101=B11101101; // 237
unsigned int B11101110=B11101110; // 238
unsigned int B11101111=B11101111; // 239
unsigned int B11110000=B11110000; // 240
unsigned int B11110001=B11110001; // 241
unsigned int B11110010=B11110010; // 242
unsigned int B11110011=B11110011; // 243
unsigned int B11110100=B11110100; // 244
unsigned int B11110101=B11110101; // 245
unsigned int B11110110=B11110110; // 246
unsigned int B11110111=B11110111; // 247
unsigned int B11111000=B11111000; // 248
unsigned int B11111001=B11111001; // 249
unsigned int B11111010=B11111010; // 250
unsigned int B11111011=B11111011; // 251
unsigned int B11111100=B11111100; // 252
unsigned int B11111101=B11111101; // 253
unsigned int B11111110=B11111110; // 254
unsigned int B11111111=B11111111; // 255



class String
{
   friend ostream &operator<<( ostream &, const String & );
   friend istream &operator>>( istream &, String & );

public:
   String( const char * = "" ); // conversion/default constructor
   String( const String & ); // copy constructor
   ~String(); // destructor

   const String &operator=( const String & ); // assignment operator
   const String &operator+=( const String & ); // concatenation operator

   bool operator!() const; // is String empty?
   bool operator==( const String & ) const; // test s1 == s2
   bool operator<( const String & ) const; // test s1 < s2

   bool operator!=( const String &right ) const
   { 
      return !( *this == right ); 
   } 

   bool operator>( const String &right ) const
   { 
      return right < *this; 
   } 
 
   bool operator<=( const String &right ) const
   { 
      return !( right < *this ); 
   }

   bool operator>=( const String &right ) const
   { 
      return !( *this < right ); 
   } 

   char &operator[]( int ); // subscript operator (modifiable lvalue)
   char operator[]( int ) const; // subscript operator (rvalue)
   String operator()( int, int = 0 ) const; // return a substring
   int getLength() const; // return string length

   // ============ Benny adding =============
   String( const int x ); 
   String( const int x, const int base );
 
   const String &operator=( string right );
   const String &operator=( const char *right );

   String operator+( const int number );
   String operator+( const char one );
   String operator+( unsigned long number );
   String operator+( String right );

   char charAt(const int n);
   int compareTo(const String s);
   String concat(const String s); 	
   bool endsWith(const String s);
   bool equals(const String s);
   bool equalsIgnoreCase(const String s);	
   void getBytes(int buf[], int *len);	
   int indexOf(char val); 
   int indexOf(String val); 
   int indexOf(char val, int from); 
   int indexOf(String val, int from); 
   int lastIndexOf(char val); 
   int lastIndexOf(String val); 
   int lastIndexOf(char val, int from); 
   int lastIndexOf(String val, int from); 
   int length(); 
   String replace(String sub1, String sub2); 
   void setCharAt(int index,char c); 
   bool startsWith(String s); 
   String substring(int from);
   String substring(int from, int to);
   void toCharArray(char buf[], int *len);
   void toLowerCase(); 
   void toUpperCase(); 
   void trim(); 
   char *getPointer(); 

private:
   int lngth; // string length (not counting null terminator)
   char *sPtr; // pointer to start of pointer-based string

   void setString( const char * ); // utility function



}; // end class String

class serial {

 public:
  void begin(int baudRate);
  void end();
  int  available();
  char read();
  int  peek();
  void flush();
  void print(int x);
  void print(int x,int base);
  void print(const char *p);
  void println(int x);
  void println(const char *p);
  void println(string p);
  void println(String p);
  void println();
  void write(char *p);

  // Wrappers
  void beginX(int z,int baudRate);
  void endX(int z);
  int  availableX(int z);
  char readX(int z);
  int  peekX(int z);
  void flushX(int z);
  void printX(int z,int x);
  void printX(int z,int x,int base);
  void printX(int z,const char *p);
  void printlnX(int z,int x);
  void printlnX(int z,const char *p);
  void printlnX(int z,string p);
  void printlnX(int z,String p);
  void printlnX(int z);
  void writeX(int z,char *p);
};
serial Serial,Serial1,Serial2,Serial3;
