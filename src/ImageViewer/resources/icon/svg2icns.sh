#!/bin/bash -e

pngdir="${PWD}"
iconset="icon.iconset"

if [ ! -z "$(uname -s | grep -E -i 'darwin')" ] ; then
    export PATH="${PATH}:/Applications/Inkscape.app/Contents/MacOS"
    export PATH="${PATH}:/Applications/GIMP-2.10.app/Contents/MacOS"
fi

sizes="16 32 64 128 256 512 1024"

for size in ${sizes} ; do
    name="icon_macOS_${size}"
    inkscape -z -e "${PWD}/${name}.png" -w ${size} -h ${size} "${PWD}/drawing_macOS.svg" || inkscape -C -o "${PWD}/${name}.png" -w ${size} -h ${size} "${PWD}/drawing_macOS.svg"
    #gimp -c -d -i -b "(let* ((image (car (file-png-load 0 \"${name}.png\" \"${name}.png\"))) (drawable (car (gimp-image-get-active-layer image)))) (gimp-image-convert-indexed image 0 0 255 0 1 \"\") (file-png-save 1 image drawable \"${name}.png\" \"${name}.png\" 0 9 0 0 0 0 0) )" -b "(gimp-quit 0)"
    optipng -o7 -zm1-9 -strip all ${name}.png
done

rm -rf "${iconset}"
mkdir -p "${iconset}"
cp -a "${pngdir}/icon_macOS_16.png" "${iconset}/icon_16x16.png"
cp -a "${pngdir}/icon_macOS_32.png" "${iconset}/icon_16x16@2x.png"
cp -a "${pngdir}/icon_macOS_32.png" "${iconset}/icon_32x32.png"
cp -a "${pngdir}/icon_macOS_64.png" "${iconset}/icon_32x32@2x.png"
cp -a "${pngdir}/icon_macOS_128.png" "${iconset}/icon_128x128.png"
cp -a "${pngdir}/icon_macOS_256.png" "${iconset}/icon_128x128@2x.png"
cp -a "${pngdir}/icon_macOS_256.png" "${iconset}/icon_256x256.png"
cp -a "${pngdir}/icon_macOS_512.png" "${iconset}/icon_256x256@2x.png"
cp -a "${pngdir}/icon_macOS_512.png" "${iconset}/icon_512x512.png"
cp -a "${pngdir}/icon_macOS_1024.png" "${iconset}/icon_512x512@2x.png"
iconutil -c icns "${iconset}"
rm -rf "${iconset}"

for size in ${sizes} ; do
    rm -f "icon_macOS_${size}.png"
done

