/*global jQuery */
(function($) {

	/************************************************************************
	*
	* pixie.js - The jQuery pixel editor.
	*
	************************************************************************/

	var imageDir = "../images/pixie/";

	var actions = {
		undo: {
			name: "Undo",
			undoable: false,
			hotkeys: ["ctrl+z"],
			perform: function(editor_panel) {
				editor_panel.undo();
			}
		},

		redo: {
			name: "Redo",
			undoable: false,
			hotkeys: ["ctrl+y"],
			perform: function(editor_panel) {
				editor_panel.redo();
			}
		},

		clear: {
			name: "Clear frame",
			perform: function(editor_panel) {
				editor_panel.eachPixel(function(pixel) {
					pixel.color(null);
				});
			}
		},

		preview: {
			name: "Preview",
			perform: function(editor_panel) {
				editor_panel.showPreview();
			}
		},

		save: {
			name: "Download Image",
			hotkeys: ["ctrl+s"],
			perform: function(editor_panel) {
				document.location.href = 'data:image/octet-stream;base64,' + editor_panel.toBase64();
			}
		},

		left: {
			name: "Move Left",
			menu: false,
			hotkeys: ['left'],
			perform: function(editor_panel) {
				editor_panel.eachPixel(function(pixel, x, y) {
					var rightPixel = editor_panel.getPixel(x + 1, y);

					if (rightPixel) {
						pixel.color(rightPixel.color());
					} else {
						pixel.color(null);
					}
				});
			}
		},

		right: {
			name: "Move Right",
			menu: false,
			hotkeys: ['right'],
			perform: function(editor_panel) {
				var width = editor_panel.width;
				var height = editor_panel.height;
				for (var x = width - 1; x >= 0; x--) {
					for (var y = 0; y < height; y++) {
						var currentPixel = editor_panel.getPixel(x, y);
						var leftPixel = editor_panel.getPixel(x - 1, y);

						if (leftPixel) {
							currentPixel.color(leftPixel.color());
						} else {
							currentPixel.color(null);
						}
					}
				}
			}
		},

		up: {
			name: "Move Up",
			menu: false,
			hotkeys: ['up'],
			perform: function(editor_panel) {
				editor_panel.eachPixel(function(pixel, x, y) {
					var lowerPixel = editor_panel.getPixel(x, y + 1);

					if (lowerPixel) {
						pixel.color(lowerPixel.color());
					} else {
						pixel.color(null);
					}
				});
			}
		},

		down: {
			name: "Move Down",
			menu: false,
			hotkeys: ['down'],
			perform: function(editor_panel) {
				var width = editor_panel.width;
				var height = editor_panel.height;
				for (var x = 0; x < width; x++) {
					for (var y = height - 1; y >= 0; y--) {
						var currentPixel = editor_panel.getPixel(x, y);
						var upperPixel = editor_panel.getPixel(x, y - 1);

						if (upperPixel) {
							currentPixel.color(upperPixel.color());
						} else {
							currentPixel.color(null);
						}
					}
				}
			}
		}
	};

	var CloneTool = function() {
		var cloneX, cloneY, targetX, targetY;

		return {
			name: "Clone",
			hotkeys: ['C'],
			icon: imageDir + "clone.png",
			cursor: "url(" + imageDir + "clone.png) 0 0, default",
			mousedown: function(e) {
				if (e.shiftKey) {
					cloneX = this.x;
					cloneY = this.y;
				} else {
					targetX = this.x;
					targetY = this.y;
					var selection = this.editor_panel.getPixel(cloneX, cloneY);

					if (selection) {
						this.color(selection.color());
					}
				}
			},
			mouseenter: function(e) {
				var deltaX = this.x - targetX;
				var deltaY = this.y - targetY;
				var selection = this.editor_panel.getPixel(cloneX + deltaX, cloneY + deltaY);

				if (selection) {
					this.color(selection.color());
				}
			}
		};
	};

	var tools = {
		pencil: {
			name: "Pencil",
			hotkeys: ['P'],
			icon: imageDir + "pencil.png",
			cursor: "url(" + imageDir + "pencil.png) 4 14, default",
			mousedown: function(e, color) {
				this.color(color);
			},
			mouseenter: function(e, color) {
				this.color(color);
			}
		},

		brush: {
			name: "Brush",
			hotkeys: ['B'],
			icon: imageDir + "paintbrush.png",
			cursor: "url(" + imageDir + "paintbrush.png) 4 14, default",
			mousedown: function(e, color) {
				this.color(color);

				$.each(this.editor_panel.getNeighbors(this.x, this.y), function(i, neighbor) {
					if (neighbor) {
						neighbor.color(color);
					}
				});
			},
			mouseenter: function(e, color) {
				this.color(color);

				$.each(this.editor_panel.getNeighbors(this.x, this.y), function(i, neighbor) {
					if (neighbor) {
						neighbor.color(color);
					}
				});
			}
		},

		dropper: {
			name: "Dropper",
			hotkeys: ['I'],
			icon: imageDir + "dropper.png",
			cursor: "url(" + imageDir + "dropper.png) 13 13, default",
			mousedown: function() {
				this.editor_panel.color(this.color());
				this.editor_panel.setTool(tools.pencil);
			}
		},

		eraser: {
			name: "Eraser",
			hotkeys: ['E'],
			icon: imageDir + "eraser.png",
			cursor: "url(" + imageDir + "eraser.png) 4 11, default",
			mousedown: function() {
				this.color(null);
			},
			mouseenter: function() {
				this.color(null);
			}
		},

		fill: {
			name: "Fill",
			hotkeys: ['F'],
			icon: imageDir + "fill.png",
			cursor: "url(" + imageDir + "fill.png) 12 13, default",
			mousedown: function(e, newColor, pixel) {
				// Store original pixel's color here
				var originalColor = this.color();

				// Return if original color is same as currentColor
				if (newColor === originalColor) {
					return;
				}

				var q = new Array();
				pixel.color(newColor);
				q.push(pixel);

				while (q.length > 0) {
					pixel = q.pop();

					// Add neighboring pixels to the queue
					var neighbors = this.editor_panel.getNeighbors(pixel.x, pixel.y);

					$.each(neighbors, function(index, neighbor) {
						if (neighbor && neighbor.css("backgroundColor") === originalColor) {
							neighbor.color(newColor);
							q.push(neighbor);
						}
					});
				}
			}
		},

		clone: CloneTool()
	};

	var falseFn = function() {
		return false
	};
	var div = '<div></div>';
	var clear = '<div class="clear"></div>';
	var ColorPicker = function() {
		return $('<input></input>').addClass('color').colorPicker();
	};

	var rgbParser = /^rgb\((\d{1,3}),\s*(\d{1,3}),\s*(\d{1,3})\)$/;

	var UndoStack = function() {
		var undos = [];
		var redos = [];
		var empty = true;

		return {
			popUndo: function() {
				var undo = undos.pop();

				if (undo) {
					redos.push(undo);
				}

				return undo;
			},

			popRedo: function() {
				var undo = redos.pop();

				if (undo) {
					undos.push(undo);
				}

				return undo;
			},

			next: function() {
				var last = undos[undos.length - 1];
				if (!last || !empty) {
					undos.push({});
					empty = true;
					// New future, no more redos
					redos = [];
				}
			},

			add: function(object, data) {
				var last = undos[undos.length - 1];

				// Only store this objects data if it is not already present.
				if (!last[object]) {
					last[object] = data;
					empty = false;
				}

				return this;
			}
		};
	};

	$.fn.pixie = function(options) {

		options = options || {};
		var width = options.width || 16;
		var height = options.height || 16;
		var pixelWidth = options.pixelWidth || 16;
		var pixelHeight = options.pixelHeight || 32;
		var initializer = options.initializer;
		var frames = options.frames || 10;

		return this.each(function() {
			var pixie = $(div).addClass('pixie');
			var actionsMenu = $(div).addClass('actions');

			var editor_panel = $(div).addClass('editor_panel').css({
				width: pixelWidth * width,
				height: pixelHeight * height
			});

			var toolbar = $(div).addClass('toolbar');
			var colorbar = $(div).addClass('toolbar');

			var preview = $(div).addClass('preview').css({
				width: width,
				height: height
			});

			var frameMenu = $(div).addClass('actions');

			var undoStack = UndoStack();

			var currentTool = undefined;
			var active = false;
			var mode = undefined;
			var primaryColorPicker = ColorPicker();
			var secondaryColorPicker = ColorPicker();

			colorbar.append(
				primaryColorPicker).append(
					secondaryColorPicker);

					pixie
					.bind('contextmenu', falseFn)
					.bind("mousedown", function(e) {
						var target = $(e.target);

						if (target.is(".swatch")) {
							editor_panel.color(target.css('backgroundColor'), e.button !== 0);
						}
					})
					.bind("mouseup", function(e) {
						active = false;
						mode = undefined;
					});

					var pixels = [];

		      for(var frame = 0; frame < frames; frame++) {
		        var frameDiv = $(div).addClass("frame").css("zIndex", frame);
		        pixels[frame] = [];

		        (function(currentFrame) {
		          var frameSelection = $("<a href='#' title='Frame "+ currentFrame +"'>"+ currentFrame +"</a>")
		            .addClass('tool')
		            .bind("mousedown", function(e) {
		              frame = currentFrame;
		              frameMenu.children().removeClass("active");
		              $(this).addClass("active");
		            })
		            .click(falseFn)

		          if(currentFrame === 0) {
		            frameSelection.addClass("active");
		          }

		          frameMenu.append(frameSelection);
		        })(frame);

		        for(var row = 0; row < height; row++) {
		          pixels[frame][row] = [];

		          for(var col = 0; col < width; col++) {
		            var pixel = $(div).addClass('pixel');
		            pixels[frame][row][col] = pixel;

		            $.extend(pixel, {
		              x: col,
		              y: row,
		              z: frame,
		              editor_panel: editor_panel,
		              toString: function() {
		                return "[Pixel: " + this.x + ", " + this.y + ", " + this.z + "]";
		              },
		              color: function(color) {
		                if(arguments.length >= 1) {
		                  undoStack.add(this, {pixel: this, color: this.css("backgroundColor")});
		                  this.css("backgroundColor", color);
		                  return this;
		                } else {
		                  return this.css("backgroundColor");
		                }
		              }
		            });


		            // Only the top frame should be sensitive to events
		            if(frame == frames - 1) {
		              (function(pixel, row, col){
		                pixel
		                  .bind("mousedown", function(e){
		                    undoStack.next();
		                    active = true;
		                    if(e.button === 0) {
		                      mode = "P";
		                    } else {
		                      mode = "S";
		                    }

		                    e.preventDefault();
		                  })
		                  .bind("mousedown mouseup mouseenter", function(e) {
		                    var p = pixels[frame][row][col];
		                    if(active && currentTool && currentTool[e.type]) {
		                      currentTool[e.type].call(p, e, editor_panel.color(), p);
		                    }
		                  });
		              })(pixel, row, col);
		            }

		            frameDiv.append(pixel);

		          }

		          frameDiv.append(clear);
		        }

		        editor_panel.append(frameDiv);
		      }

		      editor_panel.append(clear);


						frame = 0;

						$.extend(editor_panel, {
							eachPixel: function(fn, z) {
								if (z === undefined) {
									z = frame;
								}

								for (row = 0; row < height; row++) {
									for (col = 0; col < width; col++) {
										var pixel = pixels[z][row][col];
										fn.call(pixel, pixel, col, row);
									}
								}

								return editor_panel;
							},

							getPixel: function(x, y, z) {
								if (z === undefined) {
									z = frame;
								}

								if (y >= 0 && y < height) {
									if (x >= 0 && x < width) {
										return pixels[z][y][x];
									}
								}

								return undefined;
							},

							getNeighbors: function(x, y, z) {
								if (z === undefined) {
									z = frame;
								}

								return [
								this.getPixel(x + 1, y, z),
								this.getPixel(x, y + 1, z),
								this.getPixel(x - 1, y, z),
								this.getPixel(x, y - 1, z)];
							},

							toHex: function(bits) {
								var s = parseInt(bits).toString(16);
								if (s.length == 1) {
									s = '0' + s
								}
								return s;
							},

							parseColor: function(colorString) {
								if (!colorString || colorString == "transparent") {
									return false;
								}

								var bits = rgbParser.exec(colorString);
								return [
								this.toHex(bits[1]),
								this.toHex(bits[2]),
								this.toHex(bits[3])].join('').toUpperCase();
							},

							color: function(color, alternate) {
								// Handle cases where nothing, or only true or false is passed in
								// i.e. when getting the alternate color `editor_panel.color(true)`
								if (arguments.length === 0 || color === false) {
									return mode == "S" ?
									secondaryColorPicker.css('backgroundColor') :
									primaryColorPicker.css('backgroundColor');
								} else if (color === true) {
									// Switch color choice when alterate is true
									return mode == "S" ?
									primaryColorPicker.css('backgroundColor') :
									secondaryColorPicker.css('backgroundColor');
								}

								var parsedColor;
								if (color[0] != "#") {
									parsedColor = "#" + (this.parseColor(color) || "FFFFFF");
								} else {
									parsedColor = color;
								}

								if ((mode == "S") ^ alternate) {
									secondaryColorPicker.val(parsedColor);
									secondaryColorPicker[0].onblur();
								} else {
									primaryColorPicker.val(parsedColor);
									primaryColorPicker[0].onblur();
								}

								return this;
							},

							addSwatch: function(color) {
								colorbar.append(
									$(div)
									.addClass('swatch')
									.css({
										backgroundColor: color
										}));
									},

									addAction: function(action) {
										var titleText = action.name;
										var undoable = action.undoable;

										function doIt() {
											if (undoable !== false) {
												undoStack.next();
											}
											action.perform(editor_panel);
										}

										if (action.hotkeys) {
											titleText += " (" + action.hotkeys + ")";

											$.each(action.hotkeys, function(i, hotkey) {
												$(document).bind('keydown', hotkey, function(e) {
													doIt();
													e.preventDefault();
												});
											});
										}

										if (action.menu !== false) {
											actionsMenu.append(
												$("<a href='#' title='" + titleText + "'>" + action.name + "</a>")
												.addClass('tool')
												.bind("mousedown", function(e) {
													doIt();
												})
												.click(falseFn));
											}
										},

										addTool: function(tool) {
											var alt = tool.name;

											function setMe() {
												editor_panel.setTool(tool);
												toolbar.children().removeClass("active");
												toolDiv.addClass("active");
											}

											if (tool.hotkeys) {
												alt += " (" + tool.hotkeys + ")";

												$.each(tool.hotkeys, function(i, hotkey) {
													$(document).bind('keydown', hotkey, function(e) {
														setMe();
														e.preventDefault();
													});
												});
											}

											var toolDiv = $("<img src='" + tool.icon + "' alt='" + alt + "' title='" + alt + "'></img>")
											.addClass('tool')
											.bind('mousedown', function(e) {
												setMe();
											});

											toolbar.append(toolDiv);
										},

										setTool: function(tool) {
											currentTool = tool;
											if (tool.cursor) {
												pixie.css({
													cursor: tool.cursor
												});
											} else {
												pixie.css({
													cursor: "pointer"
												});
											}
										},


										undo: function() {
											var data = undoStack.popUndo();
											var swap;

											if (data) {
												$.each(data, function() {
													swap = this.color;
													this.color = this.pixel.css('backgroundColor');
													this.pixel.css('backgroundColor', (swap));
												});
											}
										},

										redo: function() {
											var data = undoStack.popRedo();
											var swap;

											if (data) {
												$.each(data, function() {
													swap = this.color;
													this.color = this.pixel.css('backgroundColor');
													this.pixel.css('backgroundColor', (swap));
												});
											}
										},

										width: width,
										height: height
									});

									$.each(actions, function(key, action) {
										editor_panel.addAction(action);
									});

									$.each(["#000", "#FFF", "#666", "#CCC", "#800", "#080", "#008", "#880", "#808", "#088"], function(i, color) {
										editor_panel.addSwatch(color);
									});

									$.each(tools, function(key, tool) {
										editor_panel.addTool(tool);
									});

									editor_panel.setTool(tools.pencil); toolbar.children().eq(0).addClass("active");

									if (initializer) {
										initializer(editor_panel);
									}


									pixie
									.append(actionsMenu)
									.append(toolbar)
									.append(editor_panel)
									.append(colorbar)
									.append(preview)
									.append(clear)
									.append(frameMenu);

									$(this).append(pixie);
								});
							};
							})(jQuery);