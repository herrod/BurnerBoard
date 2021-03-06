
/*****************************************************************************/

// Burner Board WS2801-based RGB LED Modules in a strand

// Modified to map virtual BurnerBoard 70x10 into 544 pixels
// with Y strip sizes of 31, 45, 60, 66, 70, 70, 66, 60, 45, 31
//
// Richard McDougall
// Rick Pruss
// Woodson Martin
// Keeton Martin
// Artistic Content from James Martin
// June 2013-2015

// The - is a hole in the virtual matrix
// The * is translated to a real LED pixel
//
// This image needs correcting -- don't forget ;-)
// But you get the idea, right?
//
// Starting at 0,0 left to right, bottom to top
//    Front
//
//       9,69
// ----**----    
// --******--
// --******--
// --******--
// --******--
// --******--
// -********-
// -********-
// -********-
// -********-
// -********-
// -********-
// -********-
// **********
// **********
// **********
// **********
// **********
// **********
// **********
// **********
// **********
// **********
// -********-
// -********-
// -********-
// -********-
// -********-
// -********-
// -********-
// --******--
// --******--
// --******--
// --******--
// --******--
// ----**----   
// 0,0 
//   Back 
// In addition to the main array, the edge pixels are prepended to the physical string,
// and logically appended to array. There are 79 pixels on each side for a total of 158.
// So, set_pixel_color(..., 544 is the start of the side).

//const IS_EMULATOR = 0;

// Was 544 with just the matrix, now 544 + 158
const NUM_EDGE_PIXELS = 158;
const NUM_REAL_BOARD_PIXELS = (544 + NUM_EDGE_PIXELS);


// Main object for board
function board() {
  this.has_sidelights = true;
  this.num_board_leds;
  this.pixels = [];
  this.pixel_translate = [];
  this.width = 10;
  this.height = 70;
  this.num_virt_leds = this.width * this.height  * 3;

  //this.pixels = new Array(w * h + NUM_EDGE_PIXELS);
  this.pixels = [];
  var total_pixels;

  if (this.has_sidelights == true) {
    total_pixels = 3 * (this.width * this.height + NUM_EDGE_PIXELS);
  } else {
    total_pixels = 3 * (this.width * this.height);  
  }
  for (pixel = 0; pixel < total_pixels; pixel++) {
    this.pixels[pixel] = 0;
  }


  // A cache of the pixel translations for the Burner Board LED strings
  // translation is real board string pixel# = pixel_translate(virtual matrix with holes pixel#)
  var new_pixel;

  console.log("setting up translator " + this.num_virt_leds)

  // Allocate Burner Board pix translation map
  //pixel_translate = new Array(3 * n);
  this.pixel_translate = [];

  // For each virt board pixel translate to real board matrix positions
  for(virtpixel = 0; virtpixel < this.num_virt_leds; virtpixel++) {
    // Array Pixel starts at 0,0 and translation map calc'ed with start of 1,1
    new_pixel = board_pixel(virtpixel + 1);
    if (new_pixel) {
      // Array Pixel starts at 0,0 and translation map calc'ed with start of 1,1
      new_pixel--;
      for (rgb = 0; rgb < 3; rgb ++) {
        this.pixel_translate[(new_pixel * 3) + rgb] = (virtpixel * 3) + rgb;
      }
    }
  }

  if (IS_EMULATOR == 1) { 
    var c = document.getElementById("board_canvas");
    this.ctx = c.getContext("2d");
  }

  function free() {
    pixels = [];
  }
}

board.prototype = {

  // Clock out the actual Burner Board Pixels without holes
  show : function() {
    var y, x;
    var color;
    var pixel;
    var r = 0, g = 0, b = 0;
    var adjusted_x;


    for (x = 0; x < this.width; x++) {
      for(y=0 ; y < this.height; y++) {
        if (this.has_sidelights == true) {
          pixel =  3 * (x * this.height + y);
        } else {
          pixel = 3 * (x * this.height + y);
        }
        r = this.pixels[pixel];
        g = this.pixels[pixel + 1];
        b = this.pixels[pixel + 2];
        this.ctx.fillStyle = "#" + componentToHex(r) + componentToHex(g) + componentToHex(b);
        if (board_pixel(pixel / 3)) {
          this.ctx.fillRect((6 * y) + 25, (12 * x) + 36 + 1, 6, 6);
          //console.log("rect " + this.ctx.fillStyle + "   " + (4 * y) + 25) + "," + (8 * x) + 10) + ", 4, 4");
          this.ctx.stroke();
        }
      }
    }
    if (IS_EMULATOR == 1) {
      if (this.has_sidelights == true) {
        for(x = 0 ; x < 2; x++) {
          for (y = 0; y < (NUM_EDGE_PIXELS / 2); y++) {
            pixel = 3 * (700 + x * (NUM_EDGE_PIXELS / 2)+ y);
            r = this.pixels[pixel];
            g = this.pixels[pixel + 1];
            b = this.pixels[pixel + 2];
            // render other strip side on screen
            if (x == 0) {
              adjusted_x = 0;
            } else {
              adjusted_x = 16;
            } 
            //console.log("side pixel " + pixel / 3 + " " + this.ctx.fillStyle + "   " + ((4 * y) + 10) + "," + ((8 * adjusted_x) + 10) + ", 4, 4");
            this.ctx.fillStyle = "#" + componentToHex(r) + componentToHex(g) + componentToHex(b);
            this.ctx.fillRect((6 * y) + 10, (12 * adjusted_x) + 10, 6, 6);
            this.ctx.stroke();
          }
        }
      }
      this.ctx.stroke();
    } else {

      var i;
      var nl3 = NUM_REAL_BOARD_PIXELS * 3; // 3 bytes per LED


      for(i=0; i<nl3; i++ ) {
        pixels[pixel_translate[i]];

        //delay(1); // Data is latched by holding clock pin low for 1 millisecond
      }
    }

  },

  // Set pixel color from separate 8-bit R, G, B components:
  set_pixel_color_rgb : function(n, r, g, b) {
    this.pixels[n * 3] = r;
    this.pixels[n * 3 + 1] = g;
    this.pixels[n * 3 + 2] = b;
  },

  // Set pixel color from separate 8-bit R, G, B components using x,y coordinate system:
  set_pixel_color_xyrgb : function(x, y, r, g, b) {
    // calculate x offset first
    var offset = y % height;
    // add x offset
    offset += x * height;
    set_pixel_color(offset, r, g, b);
  },

  // Set pixel color from 'packed' 32-bit RGB value:
  set_pixel_color : function(n, c) {
    // To keep the show() loop as simple & fast as possible, the
    // internal color representation is native to different pixel
    // types.  For compatibility with existing code, 'packed' RGB
    // values passed in or out are always 0xRRGGBB order.
    this.pixels[n * 3] = (c >> 16) & 0xFF //Red;
    this.pixels[n * 3 + 1] = (c >> 8) & 0xFF //Green;
    this.pixels[n * 3 + 2] = (c) & 0xFF; //Blue
  },

  // Set pixel color from 'packed' 32-bit RGB value using x,y coordinate system:
  set_pixel_color_xy : function(x, y, c) {
    // calculate x offset first
    var offset = y % this.height;
    // add x offset
    offset += x * this.height;
    this.set_pixel_color(offset, c);
  },

  // Query color from previously-set pixel (returns packed 32-bit RGB value)
  get_pixel_color : function(n) {
    var ofs = n * 3;
    // To keep the show() loop as simple & fast as possible, the
    // internal color representation is native to different pixel
    // types.  For compatibility with existing code, 'packed' RGB
    // values passed in or out are always 0xRRGGBB order.
    return ((this.pixels[ofs] << 16) | (this.pixels[ofs + 1] <<  8) | this.pixels[ofs + 2]);
  },

  // Query color from previously-set pixel (returns packed 32-bit RGB value)
  get_pixel_color_xy : function(x, y) {
    // calculate x offset first
    var n = y % this.height;
    // add x offset
    n += x * this.height;
    var ofs = n * 3;
    // To keep the show() loop as simple & fast as possible, the
    // internal color representation is native to different pixel
    // types.  For compatibility with existing code, 'packed' RGB
    // values passed in or out are always 0xRRGGBB order.
    return ((this.pixels[ofs] << 16) | (this.pixels[ofs + 1] <<  8) | this.pixels[ofs + 2]);
  },

  print : function(text, x, y, size) {
    console.log(text);
  },
  
  get_num_pixels : function() {
    return (this.num_virt_leds);
  }
}

// Map virtual pixels to physical pixels on the Burner Board Layout
// Emulate a 70 x 10 rectangle matrix 
// Optionally add a two 79 pixel (158) strips on the side 
// as pixels 0-157 before the real board pixels start
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
function board_pixel(pixel) {
  var new_pixel;

  // Pixel is a hole in the map, returns 0
  new_pixel = 0;

  // Virt Pixels 701-858 are the edge pixels
  if ((this.has_sidelights == true) && (pixel > 700)) {

    pixel = pixel - 701;

    // 1st strip of edge 
    if (pixel < (NUM_EDGE_PIXELS / 2)) {
      new_pixel = pixel;
    }

    // 2nd strip of edge - reverse order
    if (pixel >= (NUM_EDGE_PIXELS / 2)) {
      new_pixel = NUM_EDGE_PIXELS - 1 - (pixel - (NUM_EDGE_PIXELS / 2));
    }

    // we calc +1
    new_pixel++;

  } else {

    // Map linear row x column strip into strip with holes in grid
    // to cater for pixels that are missing from the corners of the
    // Burner Board layout

    //1  20-50->1-31
    if (pixel >= 20 && pixel <=50)
    new_pixel = pixel - 19;
    //2 83-127->76-32
    if (pixel >= 83 && pixel <= 127)
    new_pixel = 127 - pixel + 32;
    //3 146-205->77-136
    if (pixel >= 146 && pixel <= 205)
    new_pixel = pixel - 146 + 77;
    //4 213-278->202-137
    if (pixel >= 213 && pixel <= 278)
    new_pixel = 278 - pixel + 137;
    //5 281-350->203-272
    if (pixel >= 281 && pixel <= 350)
    new_pixel = pixel - 281 + 203;
    //6 351-420->342-273
    if (pixel >= 351 && pixel <=420)
    new_pixel = 420 - pixel + 273;
    //7 423-488->343-408
    if (pixel >= 423 && pixel <= 488)
    new_pixel = pixel - 423 + 343;
    //8 496-555->468-409
    if (pixel >= 496 && pixel <= 555)
    new_pixel = 555 - pixel + 409;
    //9 573-617->469-513
    if (pixel >= 573 && pixel <= 617)
    new_pixel = pixel - 573 + 469;
    //10 650-680->544-514
    if (pixel >= 650 && pixel <= 680)
    new_pixel = 680 - pixel + 514;

    if (this.has_sidelights == true) {
      new_pixel += NUM_EDGE_PIXELS;
    }
  }
  return new_pixel;
}

function componentToHex(c) {
  var hex = c.toString(16);
  return hex.length == 1 ? "0" + hex : hex;
}


//module.exports = new board();
