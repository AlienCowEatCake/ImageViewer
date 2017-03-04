#!/bin/bash
PROJECT=ImageViewer
BUILDDIR=build_osx_qt4.8_clang64
APPNAME="Image Viewer"
DMGNAME="${PROJECT}_qt4.8_clang64"
INFO_PLIST="src/ImageViewer/Resources/platform/macosx/Info.plist"
ICON="src/ImageViewer/Resources/icon/icon.icns"

CMD_QMAKE="qmake"
CMD_DEPLOY="macdeployqt"

cd "$(dirname $0)"/..
rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"
${CMD_QMAKE} CONFIG+="release" CONFIG+="x86_64" -r -spec unsupported/macx-clang QMAKE_MACOSX_DEPLOYMENT_TARGET=10.5 "../${PROJECT}.pro"
make
cp -a "../${INFO_PLIST}" "${APPNAME}.app/Contents/Info.plist"
sed -e 's/10.7/10.5/' -i "" "${APPNAME}.app/Contents/Info.plist"
RES_PATH="${APPNAME}.app/Contents/Resources"
rm -f "${RES_PATH}/empty.lproj"
mkdir -p "${RES_PATH}/en.lproj" "${RES_PATH}/ru.lproj"
cp -a "../${ICON}" "${RES_PATH}/"
${CMD_DEPLOY} "${APPNAME}.app" -dmg -verbose=2

hdiutil convert -format UDRW -o "${APPNAME}_rw.dmg" "${APPNAME}.dmg"
mkdir "${APPNAME}_rw_mount"
hdiutil attach -mountpoint "${APPNAME}_rw_mount" -noautoopen "${APPNAME}_rw.dmg"
cd "${APPNAME}_rw_mount"
ln -s /Applications ./Applications
cd ..
hdiutil detach "${APPNAME}_rw_mount"
hdiutil convert -format UDRO -o "${APPNAME}_ro.dmg" "${APPNAME}_rw.dmg"
cp "${APPNAME}_ro.dmg" ../"${DMGNAME}.dmg"

