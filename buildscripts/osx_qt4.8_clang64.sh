#!/bin/bash -e
PROJECT=ImageViewer
BUILDDIR=build_osx_qt4.8_clang64
APPNAME="Image Viewer"
DMGNAME="${PROJECT}_qt4.8_clang64"
INFO_PLIST="src/ImageViewer/resources/platform/macosx/Info.plist"
ICON="src/ImageViewer/resources/icon/icon.icns"
OUT_PATH="src/${PROJECT}"
ALL_SDK_VERSIONS="$(xcodebuild -showsdks | grep '\-sdk macosx' | sed 's|.*-sdk macosx||')"
for SDK_VERSION in ${ALL_SDK_VERSIONS} ; do
    SDK_PATH="$(xcode-select -p)/Platforms/MacOSX.platform/Developer/SDKs/MacOSX${SDK_VERSION}.sdk"
    if [[ $(find "${SDK_PATH}/usr/lib" -name 'libstdc++*' -maxdepth 1 | wc -l | xargs) > 0 ]] ; then
        MAC_SDK="${SDK_PATH}"
    fi
done

CMD_QMAKE="qmake"
CMD_DEPLOY="macdeployqt"

echo "Using MAC_SDK=${MAC_SDK}"

cd "$(dirname $0)"/..
rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"
BUILD_PATH="${PWD}"
${CMD_QMAKE} -r CONFIG+="release" LIBS+=-dead_strip CONFIG+="x86_64" -r -spec unsupported/macx-clang QMAKE_MAC_SDK=${MAC_SDK} QMAKE_MACOSX_DEPLOYMENT_TARGET=10.5 "../${PROJECT}.pro"
make
cd "${OUT_PATH}"
cp -a "${BUILD_PATH}/../${INFO_PLIST}" "${APPNAME}.app/Contents/Info.plist"
plutil -replace LSMinimumSystemVersion -string "10.5" "${APPNAME}.app/Contents/Info.plist"
RES_PATH="${APPNAME}.app/Contents/Resources"
rm -f "${RES_PATH}/empty.lproj"
mkdir -p "${RES_PATH}/en.lproj" "${RES_PATH}/ru.lproj"
cp -a "${BUILD_PATH}/../${ICON}" "${RES_PATH}/"
${CMD_DEPLOY} "${APPNAME}.app" -verbose=2
cd "${BUILD_PATH}"

INSTALL_PATH="${PWD}/install"
ARTIFACTS_PATH="${PWD}/artifacts"
rm -rf "${INSTALL_PATH}" "${ARTIFACTS_PATH}"
mkdir -p "${INSTALL_PATH}" "${ARTIFACTS_PATH}"
mv "${OUT_PATH}/${APPNAME}.app" "${INSTALL_PATH}/"
cd "${INSTALL_PATH}"
ln -s /Applications ./Applications
cd "${BUILD_PATH}"
hdiutil create -format UDBZ -fs HFS+ -srcfolder "${INSTALL_PATH}" -volname "${APPNAME}" "${ARTIFACTS_PATH}/${DMGNAME}.dmg"

