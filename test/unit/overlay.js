'use strict';

var fixtures = require('../fixtures');
var fs = require('fs');
var sharp = require('../../index');

sharp.cache(0);

// Main

// TODO: Enable comparison of `actual` against `expected`:

describe('Overlays', function() {
  it('Overlay transparent PNG on solid background', function(done) {
    var BASE_NAME = 'alpha-layer-01.png';
    var actual = fixtures.path('output.' + BASE_NAME);
    var expected = fixtures.expected(BASE_NAME);

    sharp(fixtures.inputPngOverlayLayer0)
      .overlayWith(fixtures.inputPngOverlayLayer1)
      .toFile(actual, function (error) {
        if (error) return done(error);

        fixtures.assertEqual(expected, actual, done);
      });
  });

  it('Overlay low-alpha transparent PNG on solid background', function(done) {
    var BASE_NAME = 'alpha-layer-01-low-alpha.png';
    var actual = fixtures.path('output.' + BASE_NAME);
    var expected = fixtures.expected(BASE_NAME);

    sharp(fixtures.inputPngOverlayLayer0)
      .overlayWith(fixtures.inputPngOverlayLayer1LowAlpha)
      .toFile(actual, function (error) {
        if (error) return done(error);

        fixtures.assertEqual(expected, actual, done);
      });
  });

  it('Composite three transparent PNGs into one', function(done) {
    var BASE_NAME = 'alpha-layer-012.png';
    var actual = fixtures.path('output.' + BASE_NAME);
    var expected = fixtures.expected(BASE_NAME);

    sharp(fixtures.inputPngOverlayLayer0)
      .overlayWith(fixtures.inputPngOverlayLayer1)
      .toBuffer(function (error, data, info) {
        if (error) return done(error);

        sharp(data)
          .overlayWith(fixtures.inputPngOverlayLayer2)
          .toFile(actual, function (error) {
            if (error) return done(error);

            fixtures.assertEqual(expected, actual, done);
          });
      });
  });

  // This tests that alpha channel unpremultiplication is correct:
  it('Composite two transparent PNGs into one', function(done) {
    var BASE_NAME = 'alpha-layer-12.png';
    var actual = fixtures.path('output.' + BASE_NAME);
    var expected = fixtures.expected(BASE_NAME);

    sharp(fixtures.inputPngOverlayLayer1)
      .overlayWith(fixtures.inputPngOverlayLayer2)
      .toFile(actual, function (error, data, info) {
        if (error) return done(error);
        fixtures.assertEqual(expected, actual, done);
      });
  });

  // This tests that alpha channel unpremultiplication is correct:
  it('Composite two low-alpha transparent PNGs into one', function(done) {
    var BASE_NAME = 'alpha-layer-12-low-alpha.png';
    var actual = fixtures.path('output.' + BASE_NAME);
    var expected = fixtures.expected(BASE_NAME);

    sharp(fixtures.inputPngOverlayLayer1LowAlpha)
      .overlayWith(fixtures.inputPngOverlayLayer2LowAlpha)
      .toFile(actual, function (error, data, info) {
        if (error) return done(error);
        fixtures.assertEqual(expected, actual, done);
      });
  });

  // This tests that alpha channel unpremultiplication is correct:
  it('Composite three low-alpha transparent PNGs into one', function(done) {
    var BASE_NAME = 'alpha-layer-012-low-alpha.png';
    var actual = fixtures.path('output.' + BASE_NAME);
    var expected = fixtures.expected(BASE_NAME);

    sharp(fixtures.inputPngOverlayLayer0)
      .overlayWith(fixtures.inputPngOverlayLayer1LowAlpha)
      .toBuffer(function (error, data, info) {
        if (error) return done(error);

        sharp(data)
          .overlayWith(fixtures.inputPngOverlayLayer2LowAlpha)
          .toFile(actual, function (error, data, info) {
            if (error) return done(error);
            fixtures.assertEqual(expected, actual, done);
          });
      });
  });

  // This tests that alpha channel unpremultiplication is correct:
  it('Composite transparent PNG onto JPEG', function(done) {
    sharp(fixtures.inputJpg)
      .overlayWith(fixtures.inputPngOverlayLayer1)
      .toBuffer(function (error, data, info) {
        if (error.message !== 'Input image must have an alpha channel') {
          return done(new Error('Unexpected error: ' + error.message));
        }

        done();
      });
  });

});
