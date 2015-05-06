'use strict';

var sharp = require('../../index');
var fixtures = require('../fixtures');

sharp.cache(0);

var INTERPOLATOR = 'nearest';
var METHOD = 'premultiply-rgb';

var process = function (tool, width, height, callback) {
    var inputFilename = fixtures.path('alpha-resizing-' + tool + '-' +
      width + 'x' + height + '.png');
    var outputWidth = Math.floor(width / 2);
    var outputHeight = Math.floor(height / 2);
    var outputFilename = fixtures.path('alpha-resizing-' + tool + '-out-' +
      METHOD + '-' + outputWidth + 'x' + outputHeight + '-' +
      INTERPOLATOR + '.png');

    return function (callback) {
      sharp(inputFilename)
        .resize(outputWidth, outputHeight)
        .interpolateWith(sharp.interpolator[INTERPOLATOR])
        .toFile(outputFilename, callback);
    };
};

describe('Resizing image with alpha channel', function() {

  it('should not output black fringing around white details [Photoshop]',
    process('photoshop', 1024, 1024));

  it('should not output black fringing around white details [Pixelmator]',
    process('pixelmator', 1024, 1024));

  it('should not output black fringing around white details [Paper]',
    process('paper', 2048, 1536));

});
