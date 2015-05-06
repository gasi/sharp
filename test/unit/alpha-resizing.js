'use strict';

var sharp = require('../../index');
var fixtures = require('../fixtures');

sharp.cache(0);

describe('Resizing image with alpha channel', function() {

  it('should not output black fringing around white details [Photoshop]', function(done) {
    var width = 512;
    var height = 512;
    var interpolator = 'nearest';
    var outFilename = fixtures.path('alpha-resizing-photoshop-out-premultiply-rgb-' +
      width + 'x' + height + '-' + interpolator + '.png');

    sharp(fixtures.path('alpha-resizing-photoshop-1024x1024.png'))
      .resize(width, height)
      .interpolateWith(sharp.interpolator[interpolator])
      .toFile(outFilename, done);
  });

  it('should not output black fringing around white details [Paper]', function(done) {
    var width = 1024;
    var height = 768;
    var interpolator = 'nearest';
    var outFilename = fixtures.path('alpha-resizing-paper-out-premultiply-rgb-' +
      width + 'x' + height + '-' + interpolator + '.png');

    sharp(fixtures.path('alpha-resizing-paper-2048x1536.png'))
      .resize(width, height)
      .interpolateWith(sharp.interpolator[interpolator])
      .toFile(outFilename, done);
  });

});
