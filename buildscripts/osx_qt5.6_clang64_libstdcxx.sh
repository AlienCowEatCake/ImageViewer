#!/bin/bash -e
PROJECT=ImageViewer
BUILDDIR=build_osx_qt5.6_clang64_libstdcxx
APPNAME="Image Viewer"
DMGNAME="${PROJECT}_qt5.6_clang64_libstdcxx"
OUT_PATH="src/${PROJECT}"
MAC_SDK="$(xcodebuild -showsdks | grep '\-sdk macosx' | tail -1 | sed 's|.*-sdk ||')"

QT_PATH="/opt/Qt/5.6.3/clang_64_libstdc++_sdk10.10"
QTPLUGINS_PATH="${QT_PATH}/plugins"
CMD_QMAKE="${QT_PATH}/bin/qmake"
CMD_DEPLOY="${QT_PATH}/bin/macdeployqt"

cd "$(dirname $0)"/..
rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"
BUILD_PATH="${PWD}"
${CMD_QMAKE} CONFIG+="release" LIBS+=-dead_strip QMAKE_MAC_SDK=${MAC_SDK} "../${PROJECT}.pro"
make -j3
cd "${OUT_PATH}"
sed -e 's/10.7/10.6/' -i "" "${APPNAME}.app/Contents/Info.plist"
RES_PATH="${APPNAME}.app/Contents/Resources"
rm -f "${RES_PATH}/empty.lproj"
mkdir -p "${RES_PATH}/en.lproj" "${RES_PATH}/ru.lproj"
PLUGINS_PATH="${APPNAME}.app/Contents/PlugIns"
mkdir -p "${PLUGINS_PATH}/iconengines"
for iconengines_plugin in libqsvgicon.dylib ; do
	cp -a "${QTPLUGINS_PATH}/iconengines/${iconengines_plugin}" "${PLUGINS_PATH}/iconengines/"
done
${CMD_DEPLOY} "${APPNAME}.app" -verbose=2
cd "${BUILD_PATH}"

INSTALL_PATH="${PWD}/install"
ARTIFACTS_PATH="${PWD}/artifacts"
mkdir -p "${INSTALL_PATH}" "${ARTIFACTS_PATH}"
mv "${OUT_PATH}/${APPNAME}.app" "${INSTALL_PATH}/"
cd "${INSTALL_PATH}"
ln -s /Applications ./Applications
find "${APPNAME}.app/Contents/PlugIns" -name "*_debug.dylib" -delete
cd "${BUILD_PATH}"
hdiutil create -format UDBZ -srcfolder "${INSTALL_PATH}" -volname "${APPNAME}" "${ARTIFACTS_PATH}/${DMGNAME}.dmg"

