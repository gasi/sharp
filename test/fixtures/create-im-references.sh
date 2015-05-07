#!/bin/bash

cd test/fixtures

# Regular
composite alpha-layer-1-fill.png alpha-layer-0-background.png expected/alpha-layer-01-imagemagick.png

composite alpha-layer-1-fill.png alpha-layer-0-background.png t-alpha-layer-01.png
composite alpha-layer-2-ink.png t-alpha-layer-01.png expected/alpha-layer-012-imagemagick.png
rm t-alpha-layer-01.png

composite alpha-layer-2-ink.png alpha-layer-1-fill.png expected/alpha-layer-12-imagemagick.png

# Low alpha
composite alpha-layer-1-fill-low-alpha.png alpha-layer-0-background.png expected/alpha-layer-01-low-alpha-imagemagick.png

composite alpha-layer-2-ink-low-alpha.png alpha-layer-1-fill-low-alpha.png expected/alpha-layer-12-low-alpha-imagemagick.png

composite alpha-layer-1-fill-low-alpha.png alpha-layer-0-background.png t-alpha-layer-01-low-alpha.png
composite alpha-layer-2-ink-low-alpha.png t-alpha-layer-01-low-alpha.png expected/alpha-layer-012-low-alpha-imagemagick.png
rm t-alpha-layer-01-low-alpha.png

cd - > /dev/null
