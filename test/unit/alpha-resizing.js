'use strict';

var sharp = require('../../index');
var fixtures = require('../fixtures');

sharp.cache(0);

// Constants
var INTERPOLATOR = 'bicubic';
var METHOD = 'affine2';
var SCALE_FACTOR = 0.5;
var INPUT_WIDTH = 2048;
var INPUT_HEIGHT = 1536;

// Helper
var process = function (tool, callback) {
    var inputFilename = fixtures.path([
      'alpha-resizing-',
      INPUT_WIDTH, 'x', INPUT_HEIGHT, '-',
      tool, '.png'].join('')
    );
    var outputWidth = Math.floor(INPUT_WIDTH * SCALE_FACTOR);
    var outputHeight = Math.floor(INPUT_HEIGHT * SCALE_FACTOR);
    var outputFilename = fixtures.path([
      'out.alpha-resizing-',
      METHOD, '-', INTERPOLATOR, '-',
      outputWidth, 'x', outputHeight, '-',
      tool, '.png'].join('')
    );

    return function (callback) {
      sharp(inputFilename)
        .resize(outputWidth, outputHeight)
        .interpolateWith(sharp.interpolator[INTERPOLATOR])
        .toFile(outputFilename, callback);
    };
};

// Test
describe('Resizing image with alpha channel', function() {

  it('should not output black fringing around white details [Photoshop]',
    process('photoshop'));

  it('should not output black fringing around white details [Pixelmator]',
    process('pixelmator'));

  it('should not output black fringing around white details [Paper]',
    process('paper'));

  it('should not output black fringing around white details [Paper saved in Photoshop]',
    process('paper-ps'));

});
