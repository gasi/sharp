'use strict';

var sharp = require('../../index');
var fixtures = require('../fixtures');

sharp.cache(0);

describe('Overlays', function() {

  it('Overlay transparent PNG', function(done) {
    var outLayers01 = fixtures.path('output.overlay-alpha-out-01-sharp.png');
    var outLayers012 = fixtures.path('output.overlay-alpha-out-012-sharp.png');

    sharp(fixtures.inputPngOverlayLayer0)
      .overlayWith(fixtures.inputPngOverlayLayer1)
      .toFile(outLayers01, function (error, result) {
        if (error) {
          return done(error);
        }

        sharp(outLayers01)
          .overlayWith(fixtures.inputPngOverlayLayer2)
          .toFile(outLayers012, done);
      });
  });

});
