#!/bin/bash -e

convert \
    -type PaletteAlpha -colors 16 -depth 4 icon_16.png icon_32.png icon_48.png icon_64.png icon_96.png icon_128.png \
    -type PaletteAlpha -colors 256 -depth 8 icon_16.png icon_32.png icon_48.png icon_64.png icon_96.png icon_128.png \
    -type PaletteAlpha -colors 65536 -depth 8 icon_256.png \
    icon.ico
