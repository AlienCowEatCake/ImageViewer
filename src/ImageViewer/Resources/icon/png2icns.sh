#!/bin/bash

pngdir="${PWD}"
iconset="icon.iconset"
rm -rf "${iconset}"
mkdir -p "${iconset}"
cp -a "${pngdir}/icon_16.png" "${iconset}/icon_16x16.png"
cp -a "${pngdir}/icon_32.png" "${iconset}/icon_16x16@2x.png"
cp -a "${pngdir}/icon_32.png" "${iconset}/icon_32x32.png"
cp -a "${pngdir}/icon_64.png" "${iconset}/icon_32x32@2x.png"
cp -a "${pngdir}/icon_128.png" "${iconset}/icon_128x128.png"
cp -a "${pngdir}/icon_256.png" "${iconset}/icon_128x128@2x.png"
cp -a "${pngdir}/icon_256.png" "${iconset}/icon_256x256.png"
cp -a "${pngdir}/icon_512.png" "${iconset}/icon_256x256@2x.png"
cp -a "${pngdir}/icon_512.png" "${iconset}/icon_512x512.png"
cp -a "${pngdir}/icon_1024.png" "${iconset}/icon_512x512@2x.png"
iconutil -c icns "${iconset}"
rm -rf "${iconset}"

