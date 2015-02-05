(function () {
  var ns = $.namespace("pskl.rendering.frame");

  /**
   * FrameRenderer will display a given frame inside a canvas element.
   * @param {HtmlElement} container HtmlElement to use as parentNode of the Frame
   * @param {Object} renderingOptions
   * @param {Array} classList array of strings to use for css classList
   */
  ns.FrameRenderer = function (container, renderingOptions, classList) {
    this.defaultRenderingOptions = {
      'supportGridRendering' : false,
      'zoom' : 1
    };

    renderingOptions = $.extend(true, {}, this.defaultRenderingOptions, renderingOptions);

    if(container === undefined) {
      throw 'Bad FrameRenderer initialization. <container> undefined.';
    }

    if(isNaN(renderingOptions.zoom)) {
      throw 'Bad FrameRenderer initialization. <zoom> not well defined.';
    }

    this.container = container;

    this.zoom = renderingOptions.zoom;

    this.offset = {
      x : 0,
      y : 0
    };

    this.margin = {
      x : 0,
      y : 0
    };

    this.supportGridRendering = renderingOptions.supportGridRendering;

    this.classList = classList || [];
    this.classList.push('canvas');

    /**
     * Off dom canvas, will be used to draw the frame at 1:1 ratio
     * @type {HTMLElement}
     */
    this.canvas = null;

    /**
     * Displayed canvas, scaled-up from the offdom canvas
     * @type {HTMLElement}
     */
    this.displayCanvas = null;
    this.setDisplaySize(renderingOptions.width, renderingOptions.height);

    this.setGridWidth(pskl.UserSettings.get(pskl.UserSettings.GRID_WIDTH));

    $.subscribe(Events.USER_SETTINGS_CHANGED, this.onUserSettingsChange_.bind(this));
  };

  pskl.utils.inherit(pskl.rendering.frame.FrameRenderer, pskl.rendering.AbstractRenderer);

  ns.FrameRenderer.prototype.render = function (frame) {
    if (frame) {
      this.clear();
      this.renderFrame_(frame);
    }
  };

  ns.FrameRenderer.prototype.clear = function () {
    pskl.utils.CanvasUtils.clear(this.canvas);
    pskl.utils.CanvasUtils.clear(this.displayCanvas);
  };

  ns.FrameRenderer.prototype.setZoom = function (zoom) {
    if (zoom > Constants.MINIMUM_ZOOM) {
      // back up center coordinates
      var centerX = this.offset.x + (this.displayWidth/(2*this.zoom));
      var centerY = this.offset.y + (this.displayHeight/(2*this.zoom));

      this.zoom = zoom;
      // recenter
      this.setOffset(
        centerX - (this.displayWidth/(2*this.zoom)),
        centerY - (this.displayHeight/(2*this.zoom))
      );
    }
  };

  ns.FrameRenderer.prototype.getZoom = function () {
    return this.zoom;
  };

  ns.FrameRenderer.prototype.setDisplaySize = function (width, height) {
    this.displayWidth = width;
    this.displayHeight = height;
    if (this.displayCanvas) {
      $(this.displayCanvas).remove();
      this.displayCanvas = null;
    }
    this.createDisplayCanvas_();
  };

  ns.FrameRenderer.prototype.getDisplaySize = function () {
    return {
      height : this.displayHeight,
      width : this.displayWidth
    };
  };

  ns.FrameRenderer.prototype.getOffset = function () {
    return {
      x : this.offset.x,
      y : this.offset.y
    };
  };

  ns.FrameRenderer.prototype.setOffset = function (x, y) {
    var width = pskl.app.piskelController.getWidth();
    var height = pskl.app.piskelController.getHeight();
    var maxX = width - (this.displayWidth/this.zoom);
    x = pskl.utils.Math.minmax(x, 0, maxX);
    var maxY = height - (this.displayHeight/this.zoom);
    y = pskl.utils.Math.minmax(y, 0, maxY);

    this.offset.x = x;
    this.offset.y = y;
  };

  ns.FrameRenderer.prototype.setGridWidth = function (value) {
    this.gridWidth_ = value;
  };

  ns.FrameRenderer.prototype.getGridWidth = function () {
    if (this.supportGridRendering) {
      return this.gridWidth_;
    } else {
      return 0;
    }
  };

  ns.FrameRenderer.prototype.updateMargins_ = function (frame) {
    var deltaX = this.displayWidth - (this.zoom * frame.getWidth());
    this.margin.x = Math.max(0, deltaX) / 2;

    var deltaY = this.displayHeight - (this.zoom * frame.getHeight());
    this.margin.y = Math.max(0, deltaY) / 2;
  };

  ns.FrameRenderer.prototype.createDisplayCanvas_ = function () {
    var height = this.displayHeight;
    var width = this.displayWidth;

    this.displayCanvas = pskl.utils.CanvasUtils.createCanvas(width, height, this.classList);
    pskl.utils.CanvasUtils.disableImageSmoothing(this.displayCanvas);
    this.container.append(this.displayCanvas);
  };

  ns.FrameRenderer.prototype.onUserSettingsChange_ = function (evt, settingName, settingValue) {
    if (settingName == pskl.UserSettings.GRID_WIDTH) {
      this.setGridWidth(settingValue);
    }
  };

  /**
   * Transform a screen pixel-based coordinate (relative to the top-left corner of the rendered
   * frame) into a sprite coordinate in column and row.
   * @public
   */
  ns.FrameRenderer.prototype.getCoordinates = function(x, y) {
    var containerOffset = this.container.offset();
    x = x - containerOffset.left;
    y = y - containerOffset.top;

    // apply margins
    x = x - this.margin.x;
    y = y - this.margin.y;

    var cellSize = this.zoom;
    // apply frame offset
    x = x + this.offset.x * cellSize;
    y = y + this.offset.y * cellSize;

    return {
      x : Math.floor(x / cellSize),
      y : Math.floor(y / cellSize)
    };
  };

  ns.FrameRenderer.prototype.reverseCoordinates = function(x, y) {
    var cellSize = this.zoom;

    x = x * cellSize;
    y = y * cellSize;

    x = x - this.offset.x * cellSize;
    y = y - this.offset.y * cellSize;

    x = x + this.margin.x;
    y = y + this.margin.y;

    var containerOffset = this.container.offset();
    x = x + containerOffset.left;
    y = y + containerOffset.top;

    return {
      x : x + (cellSize/2),
      y : y + (cellSize/2)
    };
  };

  /**
   * @private
   */
  ns.FrameRenderer.prototype.renderFrame_ = function (frame) {
    var virt_pixel;
    var board_x;
    var board_y;
    var bpixel;
    var sidelight;
    
    if (!this.canvas || frame.getWidth() != this.canvas.width || frame.getHeight() != this.canvas.height) {
      this.canvas = pskl.utils.CanvasUtils.createCanvas(frame.getWidth(), frame.getHeight());
    }

    var context = this.canvas.getContext('2d');
    for(var x = 0, width = frame.getWidth(); x < width; x++) {
      for(var y = 0, height = frame.getHeight(); y < height; y++) {
        var color = frame.getPixel(x, y);
        sidelight = false;
        if (y == 0 || y == 22) {
          sidelight = true;
        }
        // Board X dimension pixels are spaced double to Y
        board_x = ((y - 2) / 2);
        // Only render even pixels
        if ((board_x | 0) != board_x) {
          board_x = -1;
        }
        // Center board in frame because we also draw side lights which are longer by 5
        board_y = x - 5;
        if ((board_x >= 0) && (board_y >= 0) && (board_x < 10) && (board_y < 70)) {
          virt_pixel = board_x * 70 + board_y + 1;
        } else {
          virt_pixel = 0;
        }
        if (sidelight == false  && ((bpixel = this.board_pixel(virt_pixel)) == 0)) {
          color = "darkgrey";
        }
        var w = 1;
//        while (color === frame.getPixel(x, y+w)) {
//          w++;
//        }
        this.renderLine_(color, x, y, w, context);
        y = y + w - 1;
      }
    }

    this.updateMargins_(frame);

    var displayContext = this.displayCanvas.getContext('2d');
    displayContext.save();

    if (this.canvas.width*this.zoom < this.displayCanvas.width || this.canvas.height*this.zoom < this.displayCanvas.height) {
      displayContext.fillStyle = Constants.ZOOMED_OUT_BACKGROUND_COLOR;
      displayContext.fillRect(0,0,this.displayCanvas.width, this.displayCanvas.height);
    }

    displayContext.translate(
      this.margin.x-this.offset.x*this.zoom,
      this.margin.y-this.offset.y*this.zoom
    );

    displayContext.clearRect(0, 0, this.canvas.width*this.zoom, this.canvas.height*this.zoom);

    var isIE10 = pskl.utils.UserAgent.isIE && pskl.utils.UserAgent.version === 10;

    var gridWidth = this.getGridWidth();
    var isGridEnabled = gridWidth > 0;
    if (isGridEnabled || isIE10) {
      var scaled = pskl.utils.ImageResizer.resizeNearestNeighbour(this.canvas, this.zoom, gridWidth);
      displayContext.drawImage(scaled, 0, 0);
    } else {
      displayContext.scale(this.zoom, this.zoom);
      displayContext.drawImage(this.canvas, 0, 0);
    }
    displayContext.restore();
  };

  ns.FrameRenderer.prototype.renderPixel_ = function (color, x, y, context) {
    if(color != Constants.TRANSPARENT_COLOR) {
      context.fillStyle = color;
      context.fillRect(x, y, 1, 1);
    }
  };

  ns.FrameRenderer.prototype.renderLine_ = function (color, x, y, width, context) {
    if(color != Constants.TRANSPARENT_COLOR) {
      context.fillStyle = color;
      context.fillRect(x, y, 1, width);
    }
  };
  
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
  ns.FrameRenderer.prototype.board_pixel = function (pixel) {
    var new_pixel;
    const NUM_EDGE_PIXELS = 158;
    
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

//      if (this.has_sidelights == true) {
//        new_pixel += NUM_EDGE_PIXELS;
//      }
    }
    return new_pixel;
  };
  
})();