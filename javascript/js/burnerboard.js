/*****************************************************************************

Burner Board LED and Audio Code
Richard McDougall
Rick Pruss
Woodson Martin
Keeton Martin
Artistic Content from James Martin
June 2013-2015

*****************************************************************************/

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

const IS_EMULATOR = 1;

if (IS_EMULATOR == 0) {
  var fs = require('fs');
  // Read and eval library
  filedata = fs.readFileSync('emulator.js','utf8');
  eval(filedata);
} else {

}

var board_id = 0;
const intensity = 300;
var leds_on = false;
var strip;

const RGB_MAX = 255;
const RGB_DIM = 80;

const BATTERY_PIN = 1;
const MOT_PIN = 2;
const REMOTE_PIN = 2;
const LRELAY_PIN = 3;
const SRELAY_PIN = 4;

// Rotary Encoder
const ENC_A = 'P8_13';
const ENC_A_PIN = '23';
const ENC_B = 'P8_15';
const ENC_B_PIN = '47';

const ID_0 = 25;
const ID_1 = 24;
const ID_2 = 23;
const ID_3 = 22;


var boards = [
"PROTO",
"PROTO",
"AKULA",  
"BOADIE", 
"GOOFY", 
"STEVE", 
"JOON",
"ARTEMIS"];

var names = [
"RICHARD",
"RICHARD",
"WOODSON",  
"RIC", 
"STEVE", 
"STEVE", 
"JOON",
"JAMES"];


function setup_encoder()
{
  // Hack to make sure gpio 23,47 are available
  cmd = exec("echo " + ENC_A_PIN + " >/sys/class/gpio/unexport");
  cmd = exec("echo " + ENC_B_PIN + " >/sys/class/gpio/unexport");

  bone.pinMode(ENC_A, bone.INPUT, 7, 'pullup', 'fast');
  bone.pinMode(ENC_B', bone.INPUT, 7, 'pullup', 'fast');

  bone.attachInterrupt(ENC_A, check_encoder, bone.CHANGE);
  bone.attachInterrupt(ENC_B, check_encoder, bone.CHANGE);
}

var encoder_pos = 0;
function check_encoder() {
  enc = read_encoder();
  if (enc) {
    console.log('enc = ' + enc);
    encoder_pos += enc;
    console.log('    encoder_pos = ' + encoder_pos);
  }
  for (i = 0; i < del; i++) {
    //delay(1);
    enc = read_encoder();
    if (enc) {
      console.log("Counter value: ");
      console.log(encoder_pos);
      encoder_pos += enc;
      if (encoder_pos < 0)
      encoder_pos = 0;
      if (encoder_pos > 9)
      encoder_pos = 9;
      clear_screen();
      sprintf(mode, "%d", encoder_pos);
      strip.print(mode, 35, 1, 1);
      strip.show();
      del = 300;
      newmode = true;
    }
  }
  if (newmode == true)
  clear_screen();
}

/* returns change in encoder state (-1,0,1) */
var old_ab = 0;
function read_encoder(a, b)
{
//  var enc_states = [0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0];
  var enc_states = [0,-1,1,0,0,0,0,0,0,0,0,0,0,0,0,0];
  var a;
  var b;
  a = bone.digitalRead('P8_13');
  b = bone.digitalRead('P8_15');

  old_ab <<= 2;                   //remember previous state
  old_ab &= 0x0f;
  old_ab |= a | (b <<1);
  if (enc_states[old_ab])
    console.log('  a:' + a + ' b: ' + b + ' oldab = ' + old_ab + ' enc_states = ' + enc_states[old_ab]);
  return (enc_states[old_ab]);
}



function mydelay(del) {
  var i;
  var enc;
  var mode = [];
  var newmode = false;

}


// Set sidelight left/right 
function set_side_light(lr, x,  color) {
  pixel = lr * 79 + x;
  strip.set_pixel_color(700 + pixel, color);
}

// Create a 24 bit color value from R,G,B
function rgb_to_24b(r, g, b)
{
  var c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

function random_minmax(min,max) {
  return Math.floor((Math.random() * (max - min + 1)) + min);
}

function random(max) {
  return Math.floor(Math.random() * max);
}

var wheel_color = 0;

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g -b - back to r
function wheel(wheel_pos)
{
  wheel_pos &= 0xFF;
  if (wheel_pos < 85) {
    return rgb_to_24b(wheel_pos * 3, 255 - wheel_pos * 3, 0);
  } else if (wheel_pos < 170) {
    wheel_pos -= 85;
    return rgb_to_24b(255 - wheel_pos * 3, 0, wheel_pos * 3);
  } else {
    wheel_pos -= 170; 
    return rgb_to_24b(0, wheel_pos * 3, 255 - wheel_pos * 3);
  }
}

// Slightly different, this one makes the rainbow wheel equally distributed 
// along the chain
function rainbow_cycle(wait) {
  var i, j;

  for (j = 0; j < 256 * 5; j++) {     // 5 cycles of all 25 colors in the wheel
    for (i = 0; i < strip.get_num_pixels(); i++) {
      // tricky math! we use each pixel as a fraction of the full 96-color wheel
      // (thats the i / strip.get_num_pixels() part)
      // Then add in j which makes the colors go around per pixel
      // the % 96 is to make the wheel cycle around
      strip.set_pixel_color(i, wheel( ((i * 256 / strip.get_num_pixels()) + j) % 256) );
    }  
    strip.show();   // write all the pixels out
    mydelay(wait);
  }
}

// Slightly different, this one makes the rainbow wheel equally distributed 
// along the chain
function rainbow_cycle2() {
  var i;
  var color;
  var r, g, b;

  for (i = 0; i < strip.get_num_pixels(); i++) {
    strip.set_pixel_color(i, wheel(wheel_color));
    wheel_color += Math.floor((Math.random() * 255) + 1);
    wheel_color &= 0xFF;
  }  
  strip.show();   // write all the pixels out  
}

// Fade the board
const FADER = 50;
function fade_board() {
  var i;
  var color;
  var r, g, b;

  for (i=0; i < strip.get_num_pixels() + 158; i++) {
    color = strip.get_pixel_color(i);

    r = (color & 0x00ff0000) >> 16;
    g = (color & 0x0000ff00) >> 8;
    b = (color & 0x000000ff);
    if (r)
    r -= FADER;
    if (b)
    b -= FADER;
    if (g)
    g -= FADER;

    if (r < FADER)
    r = 0;
    if (g < FADER)
    g = 0;        
    if (b < FADER)
    b = 0;        

    /*
    console.log("color  ");
    console.log(color, HEX);
    console.log("  ");
    console.log(r, HEX);
    console.log("  ");
    console.log(g, HEX);
    console.log("  ");
    console.log(b, HEX);
    console.log("");
    */
    strip.set_pixel_color_rgb(i, r, g, b);
  }  
  strip.show();   // write all the pixels out  
}

//Shift Matrixrow down
function shift_matrix_lines()
{
  var x, y;

  for(y = 0; y < 69; y++)
  {
    for(x = 0; x < 10;x++)
    {
      strip.set_pixel_color_xy(x, y, strip.get_pixel_color_xy(x, y + 1));
    }
  }
  // Hardcoded 158 side LEDS for now
  for(y = 0; y < 78; y++)
  {
    strip.set_pixel_color(700 + y, strip.get_pixel_color(700 + y + 1));
    strip.set_pixel_color(700 + 79 + y, strip.get_pixel_color(700 + 79 + y + 1));
  }
}

// I see blondes, brunets, redheads...
function shift_matrix_circles()
{
  var x;

  for(x = 35; x < 69; x++)
  {
    strip.drawCircle(69 - x, 5, x - 35, strip.get_pixel_color_xy(5, x + 1));
  }
  mydelay(50);
}

// Clear Screen
function clear_screen()
{
  var x, y;

  for(y = 0; y < 70; y++)
  {
    for(x = 0; x < 10;x++)
    {
      strip.set_pixel_color_xy(x, y, rgb_to_24b(0, 0, 0));         
    }
  }
  // Hardcoded 158 side LEDS for now
  for(y = 0; y < 79; y++)
  {
    strip.set_pixel_color(700 + y, rgb_to_24b(0, 0, 0));
    strip.set_pixel_color(700 + 79 + y, rgb_to_24b(0, 0, 0));
  }

}

// Clear Screen
function fill_screen(color)
{
  var x, y;

  for(y = 0; y < 70; y++)
  {
    for(x = 0; x < 10;x++)
    {
      strip.set_pixel_color_xy(x, y, color);         
    }
  }
}

function lines(wait) {
  var x, y;
  var j = 0;

  for (x = 0; x < 10; x++) {
    for(y = 0; y < 70; y++) {
      strip.set_pixel_color_xy(x, y, wheel(j));
      strip.show();
      mydelay(wait);
    }
    j += 50;
  }
}

// US flag 
function draw_us_flag() {

  var color;
  var x;

  // Red and White for 20 rows
  for (row = 0; row < 20; row++) {

    for (x = 0; x < 10; x++) {
      strip.set_pixel_color_xy(x, 69, rgb_to_24b(RGB_DIM, 0, 0));
      x++;
      strip.set_pixel_color_xy(x, 69, rgb_to_24b(RGB_DIM, RGB_DIM, RGB_DIM));
    }
    shift_matrix_lines();
    strip.show();
  }

  // Red and White with solid blue
  for (x = 0; x < 4; x++) {
    strip.set_pixel_color_xy(x, 69, rgb_to_24b(RGB_DIM, 0, 0));
    x++;
    strip.set_pixel_color_xy(x, 69, rgb_to_24b(RGB_DIM, RGB_DIM, RGB_DIM));
  }
  shift_matrix_lines();
  strip.show();
  row++;


  // Red/white
  for (x = 0; x < 4; x++) {
    strip.set_pixel_color_xy(x, 69, rgb_to_24b(RGB_DIM, 0, 0));
    x++;
    strip.set_pixel_color_xy(x, 69, rgb_to_24b(RGB_DIM, RGB_DIM, RGB_DIM));
  }
  // Solid Blue line
  for (; x < 10; x++) {
    strip.set_pixel_color_xy(x, 69, rgb_to_24b(0, 0, RGB_DIM));
  }
  shift_matrix_lines();
  strip.show();

  for (row = 0; row < 20; row++) {
    // Red/white
    for (x = 0; x < 4; x++) {
      strip.set_pixel_color_xy(x, 69, rgb_to_24b(RGB_DIM, 0, 0));
      x++;
      strip.set_pixel_color_xy(x, 69, rgb_to_24b(RGB_DIM, RGB_DIM, RGB_DIM));
    }
    // White/Blue
    for (x = 4; x < 10; x++) {
      strip.set_pixel_color_xy(x, 69, rgb_to_24b(RGB_DIM, RGB_DIM, RGB_DIM));
      x++;
      strip.set_pixel_color_xy(x, 69, rgb_to_24b(0, 0, RGB_DIM));
    }    
    shift_matrix_lines();
    strip.show();  
    // Blue/white
    for (x = 4; x < 10; x++) {
      strip.set_pixel_color_xy(x, 69, rgb_to_24b(0, 0, RGB_DIM));
      x++;
      strip.set_pixel_color_xy(x, 69, rgb_to_24b(RGB_DIM, RGB_DIM, RGB_DIM));
    }
    shift_matrix_lines();
    strip.show();
    row++;

  }

  // Red/white
  for (x = 0; x < 4; x++) {
    strip.set_pixel_color_xy(x, 69, rgb_to_24b(RGB_DIM, 0, 0));
    x++;
    strip.set_pixel_color_xy(x, 69, rgb_to_24b(RGB_DIM, RGB_DIM, RGB_DIM));
  }
  // Blue line
  for (; x < 10; x++) {
    strip.set_pixel_color_xy(x, 69, rgb_to_24b(0, 0, RGB_DIM));
  }
  shift_matrix_lines();

  // 10 lines of blank
  for (x = 0; x < 10; x++) {
    strip.set_pixel_color_xy(x, 69, rgb_to_24b(0, 0, 0));
  }
  for (row = 0; row < 10; row++) {
    shift_matrix_lines();
    strip.show();
  }
}

function draw_header() {
  var color;
  var x;

  for (x = 0; x < 10; x++) {
    color = random_minmax(2,4)%2 == 0 ? rgb_to_24b(0, 0, 0): wheel(wheel_color); //Chance of 1/3rd 
    strip.set_pixel_color_xy(x, 69, color);
    // Ripple down the side lights with the same color as the edges
    if (x == 0) {
      set_side_light(0, 78, color);
    }
    if (x == 9) {
      set_side_light(1, 78, color);
    }
    wheel_color++;
    wheel_color &= 0xFF;
  }
  strip.show();
}

function draw_header_lunarian() {
  var color;
  var x;

  for (x = 0; x < 10; x++) {
    color = random_minmax(2,4)%2 == 0 ? rgb_to_24b(0, 0, 0): rgb_to_24b(128, 128, 128); //Chance of 1/3rd 
    strip.set_pixel_color_xy(x, 69, color);
    // Ripple down the side lights with the same color as the edges
    if (x == 0) {
      set_side_light(0, 78, color);
    }
    if (x == 9) {
      set_side_light(1, 78, color);
    }
    wheel_color++;
    wheel_color &= 0xFF;
  }
  strip.show();
}

function draw_header_xmas() {
  var color;
  var x;

  for (x = 0; x < 10; x++) {
    //   color = random_minmax(2,4)%2 == 0 ? rgb_to_24b(0,0,0) : rgb_to_24b(0, 255, 0); //Chance of 1/3rd 
    color = random_minmax(2,8)%2 == 0 ? rgb_to_24b(10, 10, 10): rgb_to_24b(128, 0, 0); //Chance of 1/3rd 
    //   color = random_minmax(2,4)%2 == 0 ? rgb_to_24b(0, 0, 0): rgb_to_24b(255, 255, 255); //Chance of 1/3rd 
    //   color =  rgb_to_24b(255, 255, 255); //Chance of 1/3rd 
    strip.set_pixel_color_xy(x, 69, color);
    // Ripple down the side lights with the same color as the edges
    if (x == 0) {
      set_side_light(0, 78, color);
    }
    if (x == 9) {
      set_side_light(1, 78, color);
    }
    wheel_color++;
    wheel_color &= 0xFF;
  }
  strip.show();
}

var pd_x = 0;
var pd_y = 0;
var pd_side = 0;

function draw_pixel_dust() {
  var color;
  var x, y;

  x = random(9);
  y = random(69);
  color = wheel(random(255));
  strip.set_pixel_color_xy(pd_x, pd_y, rgb_to_24b(0, 0, 0));
  strip.set_pixel_color_xy(pd_x+1, pd_y, rgb_to_24b(0, 0, 0));
  strip.set_pixel_color_xy(pd_x, pd_y+1, rgb_to_24b(0, 0, 0));
  strip.set_pixel_color_xy(pd_x+1, pd_y+1, rgb_to_24b(0, 0, 0));
  strip.set_pixel_color_xy(x, y, color);
  strip.set_pixel_color_xy(x+1, y, color);
  strip.set_pixel_color_xy(x, y+1, color);
  strip.set_pixel_color_xy(x+1, y+1, color);
  pd_x = x;
  pd_y = y;
  x = random(158);
  strip.set_pixel_color_xy(700 + pd_side, rgb_to_24b(0, 0, 0));
  strip.set_pixel_color_xy(700 + x, color);
  pd_side = x;
  strip.show();
}

function draw_pixel_dust2() {
  var color;
  var x, y;

  x = random(10);
  y = random(70);
  color = wheel(random(255));
  strip.set_pixel_color_xy(x, y, color);
  pd_x = x;
  pd_y = y;
  x = random(158);
  strip.set_pixel_color_xy(700 + x, color);
}

function draw_static() {
  var color;
  var x, y;

  x = random(10);
  y = random(70);
  color = rgb_to_24b(200, 200, 200);
  strip.set_pixel_color_xy(x, y, color);
  pd_x = x;
  pd_y = y;
}


/* 
*      |
*     -#-
*      |
*/
var flake_row = 0;
var flake_col = 0;

function draw_snow_flakes() {
  var x;
  var color;

  // Blue Background
  for (x = 0; x < 10; x++) {
    strip.set_pixel_color_xy(x, 69, rgb_to_24b(0, 0, 20));
  }
  set_side_light(0, 78, rgb_to_24b(0, 0, 20));
  set_side_light(1, 78, rgb_to_24b(0, 0, 20));

  switch(flake_row) {
    case 0:
    flake_col = random(1) % 8 + 1;
    strip.set_pixel_color_xy(flake_col, 69, rgb_to_24b(64, 64, 64));
    break;

    case 1:
    strip.set_pixel_color_xy(flake_col - 1, 69, rgb_to_24b(64, 64, 64));
    strip.set_pixel_color_xy(flake_col, 69, rgb_to_24b(255, 255, 255));
    strip.set_pixel_color_xy(flake_col + 1, 69, rgb_to_24b(64, 64, 64));    
    break;

    case 2:
    strip.set_pixel_color_xy(flake_col, 69, rgb_to_24b(64, 64, 64));
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
  color = random(2,8)%2 == 0 ? rgb_to_24b(0, 0, 50): rgb_to_24b(128, 128, 128); //Chance of 1/3rd 

  // Ripple down the side lights with the same color as the edges
  set_side_light(0, 78, color);
  set_side_light(1, 78, color);
  strip.show();

}

function drawCenter() {
  var color;
  var x;

  color = random(2,4)%2 == 0 ? rgb_to_24b(0, 0, 0): wheel(wheel_color); //Chance of 1/3rd 
  //    color = rgb_to_24b(RGB_MAX, RGB_MAX, RGB_MAX);
  strip.fillCircle(35, 5, 1, color);
  wheel_color++;
  wheel_color &= 0xFF;
  strip.show();
}


function read_id() {
  var bit;
  var id;

  bit = digitalRead(ID_0);
  console.log(bit);
  id = !bit;
  bit = digitalRead(ID_1);
  console.log(bit);
  id |= !bit << 1;
  bit = digitalRead(ID_2);
  console.log(bit);
  id |= !bit << 2; 
  bit = digitalRead(ID_3);
  console.log(bit);
  id |= !bit << 3;

  console.log("Board ID  ");
  console.log(id);
  console.log("");

  return(id);

}


// Working on proto, but low end is over
// = 90% = 30350
// 38.1 = 100% = 102400
// 36v = 40% = 96900
// 10% = 91800
//const LEVEL_EMPTY = 91800
//const LEVEL_FULL =  102300

// New settings, 8/17/2013
// 0 = 92900
// 100 = 102300
const LEVEL_EMPTY = 92900
const LEVEL_FULL =  102300


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
function draw_battery() {
  var color;
  var x;
  var row;
  var level = 0;
  var i;
  var sample;

  // Read Battery Level
  // 18 * 1.75v = 31.5v for low voltage
  // Set zero to 30v
  // Set 100% to 38v

  // Clear screen and measure voltage, since screen load varies it!
  clear_screen();
  strip.show();
  mydelay(1000);

  // Convert to level 0-28
  for (i = 0; i < 100; i++) {
    level += sample = analogRead(BATTERY_PIN);
    console.log("Battery sample " + sample);
  }
  console.log("Battery Level " + level);


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

  console.log("Adjusted Level " + level);

  row = 20;
  // White Battery Shell with Green level

  // Battery Bottom
  for (x = 0; x < 10; x++) {
    strip.set_pixel_color_xy(x, row, rgb_to_24b(RGB_MAX, RGB_MAX, RGB_MAX));
  }
  row++;

  // Battery Sides
  for (; row < 49; row++) {
    strip.set_pixel_color_xy(0, row, rgb_to_24b(RGB_MAX, RGB_MAX, RGB_MAX));
    strip.set_pixel_color_xy(9, row, rgb_to_24b(RGB_MAX, RGB_MAX, RGB_MAX));
  }

  // Battery Top
  for (x = 0; x < 10; x++) {
    strip.set_pixel_color_xy(x, row, rgb_to_24b(RGB_MAX, RGB_MAX, RGB_MAX));
  }
  row++;

  // Battery button
  for (x = 3; x < 7; x++) {
    strip.set_pixel_color_xy(x, row, rgb_to_24b(RGB_MAX, RGB_MAX, RGB_MAX));
    strip.set_pixel_color_xy(x, row+1, rgb_to_24b(RGB_MAX, RGB_MAX, RGB_MAX));
  }
  row+=2;

  // Battery Level
  for (row = 21; row < 21 + level; row++) {
    for (x = 1; x < 9; x++) {
      strip.set_pixel_color_xy(x, row, rgb_to_24b(0, RGB_DIM, 0));
    }
  }

  strip.show();
}


function main() {
  var i;

  // Console for debugging
  console.log("Goodnight moon!");



  // Set battery level analogue reference

  //analogReference(INTERNAL1V1);
  pinMode(BATTERY_PIN, INPUT);
  pinMode(MOT_PIN, INPUT);
  pinMode(REMOTE_PIN, INPUT);
  digitalWrite(REMOTE_PIN, LOW);
  pinMode(SRELAY_PIN, OUTPUT);
  pinMode(LRELAY_PIN, OUTPUT);
  digitalWrite(SRELAY_PIN, HIGH);
  digitalWrite(LRELAY_PIN, HIGH);

  //ID Pins  
  pinMode(ID_0, INPUT);
  digitalWrite(ID_0, HIGH);
  pinMode(ID_1, INPUT);
  digitalWrite(ID_1, HIGH);
  pinMode(ID_2, INPUT);
  digitalWrite(ID_2, HIGH);
  pinMode(ID_3, INPUT);
  digitalWrite(ID_3, HIGH);

  //Encoder Pins
  pinMode(ENC_A, INPUT);
  digitalWrite(ENC_A, HIGH);
  pinMode(ENC_B, INPUT);
  digitalWrite(ENC_B, HIGH);


  board_id = read_id();
  //strip = require('./board.js');
  strip = new board();

  for (i = 0; i < 544; i++) {
    //console.log("Strip pixel " + i + " = virt pixel " + strip.pixel_translate[i]);
  }

  clear_screen();
  var x;
  for (x = 0; x < 858; x++) {
    strip.set_pixel_color(x, rgb_to_24b(255,255,255));
    strip.show();
    mydelay(255);
  }
  //strip.set_pixel_color(700, rgb_to_24b(0,0,255));
  //strip.set_pixel_color(779, rgb_to_24b(0,0,255));
  //strip.show();

  mydelay(15000);


  // Update LED contents, to start they are all 'off'
  clear_screen();
  strip.show();

  strip.print(boards[board_id], 15, 1, 1);
  strip.show();
  mydelay(1000);

  clear_screen();
  strip.print(names[board_id], 15, 1, 1);
  strip.show();
  mydelay(1000);

  draw_battery();
  mydelay(1000);

  clear_screen();

  setInterval(loop, 100);

}


function loop_matrix()
{
  draw_header();
  shift_matrix_lines();
  mydelay(50);
}

function loop_snowflakes()
{
  draw_snow_flakes();
  shift_matrix_lines();
  mydelay(50);
}

function loop_matrixfast()
{
  draw_header();
  shift_matrix_lines();
  mydelay(1);
}


function loop_lunarian()
{
  draw_header_lunarian();
  shift_matrix_lines(); 
  mydelay(1);
}

function loop_xmas()
{
  draw_header_xmas();
  shift_matrix_lines(); 
  mydelay(1);
}

function loop_battery()
{
  draw_battery();
  mydelay(1000);
  clear_screen();
}


function loop_distrikt()
{
  //drawDistrikt();
  mydelay(10);
}

function loop_stanford(enc)
{
  var i;

  for (i = 0; i < 20  && encoder_pos == enc; i++) {
    //drawStanford();
    strip.show();
    mydelay(300);
    fill_screen(rgb_to_24b(14, 2, 2));
    strip.show();
    mydelay(300);
  }
}

function loop_pixeldust() {
  draw_pixel_dust();
  mydelay(5);

}

function loop_pixeldust2() {
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  draw_pixel_dust2();
  fade_board();
  strip.show();
  mydelay(1);

}

function loop_static() {
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  draw_static();
  fade_board();
  strip.show();
  mydelay(1);
}

function loop_rainbow() {
  rainbow_cycle2();
  mydelay(5);
}

var loopcnt = 0;
var state = 0;
// state = 0: lines
// state = 1: flag

function loop() {
  var i;


  //console.log("Encoder sample " + encoder_pos);

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
    loop_snowflakes();
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
    loop_xmas();
    //       loop_stanford(7);
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

main();

