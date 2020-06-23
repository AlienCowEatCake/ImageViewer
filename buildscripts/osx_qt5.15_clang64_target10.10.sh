#!/bin/bash -e
PROJECT=ImageViewer
BUILDDIR=build_osx_qt5.15_clang64_target10.10
APPNAME="Image Viewer"
DMGNAME="${PROJECT}_qt5.15_clang64_target10.10"
OUT_PATH="src/${PROJECT}"
MAC_SDK="$(xcodebuild -showsdks | grep '\-sdk macosx' | tail -1 | sed 's|.*-sdk ||')"

QT_PATH="/opt/Qt/5.15.0/clang_64_target10.10"
QTPLUGINS_PATH="${QT_PATH}/plugins"
CMD_QMAKE="${QT_PATH}/bin/qmake"
CMD_DEPLOY="${QT_PATH}/bin/macdeployqt"

echo "Using MAC_SDK=${MAC_SDK}"

cd "$(dirname $0)"/..
rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"
BUILD_PATH="${PWD}"
${CMD_QMAKE} -r CONFIG+="release" LIBS+=-dead_strip QMAKE_MAC_SDK=${MAC_SDK} QMAKE_MACOSX_DEPLOYMENT_TARGET=10.10 CONFIG+=c++2y CONFIG+="enable_update_checking" "../${PROJECT}.pro"
make -j3
cd "${OUT_PATH}"
plutil -replace LSMinimumSystemVersion -string "10.10" "${APPNAME}.app/Contents/Info.plist"
RES_PATH="${APPNAME}.app/Contents/Resources"
rm -f "${RES_PATH}/empty.lproj"
mkdir -p "${RES_PATH}/en.lproj" "${RES_PATH}/ru.lproj"
PLUGINS_PATH="${APPNAME}.app/Contents/PlugIns"
mkdir -p "${PLUGINS_PATH}/iconengines"
for iconengines_plugin in libqsvgicon.dylib ; do
	cp -a "${QTPLUGINS_PATH}/iconengines/${iconengines_plugin}" "${PLUGINS_PATH}/iconengines/"
done
${CMD_DEPLOY} "${APPNAME}.app" -verbose=2
for unused_plugins_subdir in virtualkeyboard platforminputcontexts ; do
	rm -r "${PLUGINS_PATH}/${unused_plugins_subdir}"
done
FRAMEWORKS_PATH="${APPNAME}.app/Contents/Frameworks"
for unused_framework in QtQml.framework QtQmlModels.framework QtQuick.framework QtVirtualKeyboard.framework ; do
	rm -r "${FRAMEWORKS_PATH}/${unused_framework}"
done
cd "${BUILD_PATH}"

INSTALL_PATH="${PWD}/install"
ARTIFACTS_PATH="${PWD}/artifacts"
mkdir -p "${INSTALL_PATH}" "${ARTIFACTS_PATH}"
mv "${OUT_PATH}/${APPNAME}.app" "${INSTALL_PATH}/"
cd "${INSTALL_PATH}"
ln -s /Applications ./Applications
find "${APPNAME}.app/Contents/PlugIns" -name "*_debug.dylib" -delete
cd "${BUILD_PATH}"
hdiutil create -format UDBZ -fs HFS+ -srcfolder "${INSTALL_PATH}" -volname "${APPNAME}" "${ARTIFACTS_PATH}/${DMGNAME}.dmg"

