#!/bin/bash -e

cd "$(dirname $0)"

convert ../../icon/icon_48.png \
    -background white \
    -gravity Center -extent 58x58 \
    -gravity East -extent 493x58 \
    +repage +matte \
    -type Palette -compress RLE BMP3:ui_banner.bmp

convert ../../icon/icon_128.png \
    -background "#008000" \
    -gravity Center -extent 164x164 \
    -gravity NorthWest -extent 164x312 \
    -background white \
    -gravity West -extent 493x312 \
    +repage +matte \
    -type Palette -compress RLE BMP3:ui_dialog.bmp
