(function () {
  var ns = $.namespace('pskl.rendering.layer');

  ns.LayersRenderer = function (container, renderingOptions, piskelController) {
    pskl.rendering.CompositeRenderer.call(this);

    this.piskelController = piskelController;

    // Do not use CachedFrameRenderers here, since the caching will be performed in the render method of LayersRenderer
    this.belowRenderer = new pskl.rendering.frame.FrameRenderer(container, renderingOptions, ["layers-canvas", "layers-below-canvas"]);
    this.aboveRenderer = new pskl.rendering.frame.FrameRenderer(container, renderingOptions, ["layers-canvas", "layers-above-canvas"]);

    this.add(this.belowRenderer);
    this.add(this.aboveRenderer);

    this.serializedRendering = '';

    $.subscribe(Events.PISKEL_RESET, this.flush.bind(this));
  };

  pskl.utils.inherit(pskl.rendering.layer.LayersRenderer, pskl.rendering.CompositeRenderer);

  ns.LayersRenderer.prototype.render = function () {
    var offset = this.getOffset();
    var size = this.getDisplaySize();
    var layers = this.piskelController.getLayers();
    var currentFrameIndex = this.piskelController.getCurrentFrameIndex();
    var currentLayerIndex = this.piskelController.getCurrentLayerIndex();

    var downLayers = layers.slice(0, currentLayerIndex);
    var upLayers = layers.slice(currentLayerIndex + 1, layers.length);


    var serializedRendering = [
      this.getZoom(),
      this.getGridWidth(),
      offset.x,
      offset.y,
      size.width,
      size.height,
      this.getHashForLayersAt_(currentFrameIndex, downLayers),
      this.getHashForLayersAt_(currentFrameIndex, upLayers),
      layers.length
    ].join("-");


    if (this.serializedRendering != serializedRendering) {
      this.serializedRendering = serializedRendering;

      this.clear();

      if (downLayers.length > 0) {
        var downFrame = this.getFrameForLayersAt_(currentFrameIndex, downLayers);
        this.belowRenderer.render(downFrame);
      }

      if (upLayers.length > 0) {
        var upFrame = this.getFrameForLayersAt_(currentFrameIndex, upLayers);
        this.aboveRenderer.render(upFrame);
      }
    }
  };

  /**
   * See @pskl.rendering.frame.CachedFrameRenderer
   * Same issue : FrameRenderer setDisplaySize destroys the canvas
   * @param {Number} width
   * @param {Number} height
   */
  ns.LayersRenderer.prototype.setDisplaySize = function (width, height) {
    var size = this.getDisplaySize();
    if (size.width !== width || size.height !== height) {
      this.superclass.setDisplaySize.call(this, width, height);
    }
  };

  ns.LayersRenderer.prototype.getFrameForLayersAt_ = function (frameIndex, layers) {
    var frames = layers.map(function (l) {
      return l.getFrameAt(frameIndex);
    });
    return pskl.utils.FrameUtils.merge(frames);
  };

  ns.LayersRenderer.prototype.getHashForLayersAt_ = function (frameIndex, layers) {
    var hash = layers.map(function (l) {
      return l.getFrameAt(frameIndex).getHash();
    });
    return hash.join('-');
  };

  ns.LayersRenderer.prototype.flush = function () {
    this.serializedRendering = '';
  };
})();
