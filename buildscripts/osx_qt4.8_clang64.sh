#!/bin/bash -e
PROJECT=ImageViewer
BUILDDIR=build_osx_qt4.8_clang64
APPNAME="Image Viewer"
DMGNAME="${PROJECT}_qt4.8_clang64"
INFO_PLIST="src/ImageViewer/resources/platform/macosx/Info.plist"
ICON="src/ImageViewer/resources/icon/icon.icns"
OUT_PATH="src/${PROJECT}"
MAC_SDK="$(xcodebuild -showsdks | grep '\-sdk macosx' | tail -1 | sed 's|.*-sdk ||')"

CMD_QMAKE="qmake"
CMD_DEPLOY="macdeployqt"

cd "$(dirname $0)"/..
rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"
BUILD_PATH="${PWD}"
${CMD_QMAKE} -r CONFIG+="release" LIBS+=-dead_strip CONFIG+="x86_64" -r -spec unsupported/macx-clang QMAKE_MAC_SDK=${MAC_SDK} QMAKE_MACOSX_DEPLOYMENT_TARGET=10.5 "../${PROJECT}.pro"
make
cd "${OUT_PATH}"
cp -a "${BUILD_PATH}/../${INFO_PLIST}" "${APPNAME}.app/Contents/Info.plist"
sed -e 's/10.7/10.5/' -i "" "${APPNAME}.app/Contents/Info.plist"
RES_PATH="${APPNAME}.app/Contents/Resources"
rm -f "${RES_PATH}/empty.lproj"
mkdir -p "${RES_PATH}/en.lproj" "${RES_PATH}/ru.lproj"
cp -a "${BUILD_PATH}/../${ICON}" "${RES_PATH}/"
${CMD_DEPLOY} "${APPNAME}.app" -verbose=2
cd "${BUILD_PATH}"

INSTALL_PATH="${PWD}/install"
ARTIFACTS_PATH="${PWD}/artifacts"
mkdir -p "${INSTALL_PATH}" "${ARTIFACTS_PATH}"
mv "${OUT_PATH}/${APPNAME}.app" "${INSTALL_PATH}/"
cd "${INSTALL_PATH}"
ln -s /Applications ./Applications
cd "${BUILD_PATH}"
hdiutil create -format UDBZ -srcfolder "${INSTALL_PATH}" -volname "${APPNAME}" "${ARTIFACTS_PATH}/${DMGNAME}.dmg"

