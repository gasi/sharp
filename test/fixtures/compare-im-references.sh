#!/bin/bash

cd test/fixtures/expected

OPTIONS='-fuzz 1%'

# Regular
compare $OPTIONS alpha-layer-01-imagemagick.png alpha-layer-01.png alpha-layer-01-compare.png
compare $OPTIONS alpha-layer-012-imagemagick.png alpha-layer-012.png alpha-layer-012-compare.png
compare $OPTIONS alpha-layer-12-imagemagick.png alpha-layer-12.png alpha-layer-12-compare.png

# Low alpha
compare $OPTIONS alpha-layer-01-low-alpha-imagemagick.png alpha-layer-01-low-alpha.png alpha-layer-01-low-alpha-compare.png
compare $OPTIONS alpha-layer-12-low-alpha-imagemagick.png alpha-layer-12-low-alpha.png alpha-layer-12-low-alpha-compare.png
compare $OPTIONS alpha-layer-012-low-alpha-imagemagick.png alpha-layer-012-low-alpha.png alpha-layer-012-low-alpha-compare.png

cd - > /dev/null
