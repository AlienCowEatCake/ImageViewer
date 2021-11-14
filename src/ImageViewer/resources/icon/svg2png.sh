#!/bin/bash -e

if [ ! -z "$(uname -s | grep -E -i 'darwin')" ] ; then
    export PATH="${PATH}:/Applications/Inkscape.app/Contents/MacOS"
    export PATH="${PATH}:/Applications/GIMP-2.10.app/Contents/MacOS"
fi

sizes="16 20 22 24 32 36 40 42 48 64 72 96 128 192 256 480 512 1024"

for size in ${sizes} ; do
    name="icon_${size}"
    inkscape -z -e "${PWD}/${name}.png" -w ${size} -h ${size} "${PWD}/drawing.svg" || inkscape -C -o "${PWD}/${name}.png" -w ${size} -h ${size} "${PWD}/drawing.svg"
    #gimp -c -d -i -b "(let* ((image (car (file-png-load 0 \"${name}.png\" \"${name}.png\"))) (drawable (car (gimp-image-get-active-layer image)))) (gimp-image-convert-indexed image 0 0 255 0 1 \"\") (file-png-save 1 image drawable \"${name}.png\" \"${name}.png\" 0 9 0 0 0 0 0) )" -b "(gimp-quit 0)"
    optipng -o7 -zm1-9 -strip all ${name}.png
done

