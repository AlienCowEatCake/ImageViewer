#!/bin/bash -e
PROJECT=ImageViewer
BUILDDIR=build_osx_qt4.8_clang64
APPNAME="Image Viewer"
DMGNAME="${PROJECT}_qt4.8_clang64"
INFO_PLIST_PATH="src/${PROJECT}/resources/platform/macosx/Info.plist"
ICON_PATH="src/${PROJECT}/resources/icon/icon.icns"
SCRIPT_PATH="src/${PROJECT}/resources/platform/macosx/set_associations.sh"
LICENSE_PATH="LICENSE.GPLv3"
OUT_PATH="src/${PROJECT}"
MAC_TARGET="10.5"
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
SOURCE_PATH="${PWD}"
rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"
BUILD_PATH="${PWD}"
arch -x86_64 ${CMD_QMAKE} -r CONFIG+="release" LIBS+=-dead_strip CONFIG+="x86_64" -r -spec unsupported/macx-clang QMAKE_MAC_SDK=${MAC_SDK} QMAKE_MACOSX_DEPLOYMENT_TARGET=${MAC_TARGET} CONFIG+="enable_macwebview enable_macwkwebview enable_nanosvg" "../${PROJECT}.pro"
arch -x86_64 make -j$(getconf _NPROCESSORS_ONLN)
cd "${OUT_PATH}"
cp -a "${SOURCE_PATH}/${INFO_PLIST_PATH}" "${APPNAME}.app/Contents/Info.plist"
plutil -replace LSMinimumSystemVersion -string "${MAC_TARGET}" "${APPNAME}.app/Contents/Info.plist"
RES_PATH="${APPNAME}.app/Contents/Resources"
rm -f "${RES_PATH}/empty.lproj"
mkdir -p "${RES_PATH}/en.lproj" "${RES_PATH}/ru.lproj" "${RES_PATH}/zh_CN.lproj"
cp -a "${SOURCE_PATH}/${ICON_PATH}" "${RES_PATH}/"
cp -a "${SOURCE_PATH}/${SCRIPT_PATH}" "${RES_PATH}/"
arch -x86_64 ${CMD_DEPLOY} "${APPNAME}.app" -verbose=2
find "${APPNAME}.app/Contents" -type f -name '*.prl' -delete
cd "${BUILD_PATH}"

INSTALL_PATH="${PWD}/install"
ARTIFACTS_PATH="${PWD}/artifacts"
rm -rf "${INSTALL_PATH}" "${ARTIFACTS_PATH}"
mkdir -p "${INSTALL_PATH}" "${ARTIFACTS_PATH}"
mv "${OUT_PATH}/${APPNAME}.app" "${INSTALL_PATH}/"
cd "${INSTALL_PATH}"
ln -s /Applications ./Applications
cp -a "${SOURCE_PATH}/${LICENSE_PATH}" "./"
cd "${BUILD_PATH}"
hdiutil create -format UDBZ -fs HFS+ -srcfolder "${INSTALL_PATH}" -volname "${APPNAME}" "${ARTIFACTS_PATH}/${DMGNAME}.dmg"

