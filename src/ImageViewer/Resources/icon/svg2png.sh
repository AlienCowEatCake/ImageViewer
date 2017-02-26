#!/bin/bash

sizes="16 32 48 64 128 256 512 1024"

for size in ${sizes} ; do
	name="icon_${size}"
	inkscape -z -e ${PWD}/${name}.png -w ${size} -h ${size} ${PWD}/drawing.svg
	gimp -c -d -i -b "(let* ((image (car (file-png-load 0 \"${name}.png\" \"${name}.png\"))) (drawable (car (gimp-image-get-active-layer image)))) (gimp-image-convert-indexed image 0 0 255 0 1 \"\") (file-png-save 1 image drawable \"${name}.png\" \"${name}.png\" 0 9 0 0 0 0 0) )" -b "(gimp-quit 0)"
	optipng -o7 -strip all ${name}.png
done

