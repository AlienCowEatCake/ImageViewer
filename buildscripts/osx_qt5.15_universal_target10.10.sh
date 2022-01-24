#!/bin/bash -e
PROJECT=ImageViewer
BUILDDIR=build_osx_qt5.15_universal_target10.10
APPNAME="Image Viewer"
DMGNAME="${PROJECT}_qt5.15_universal_target10.10"
OUT_PATH="src/${PROJECT}"
ENTITLEMENTS_PATH="src/${PROJECT}/resources/platform/macosx/${PROJECT}.entitlements"
APP_CERT="Developer ID Application: Petr Zhigalov (48535TNTA7)"
NOTARIZE_USERNAME="peter.zhigalov@gmail.com"
NOTARIZE_PASSWORD="@keychain:Notarize: ${NOTARIZE_USERNAME}"
NOTARIZE_ASC_PROVIDER="${APP_CERT: -11:10}"
MAC_SDK="$(xcodebuild -showsdks | grep '\-sdk macosx' | tail -1 | sed 's|.*-sdk ||')"

QT_PATH="/opt/Qt/5.15.2/clang_universal_target10.10"
QTPLUGINS_PATH="${QT_PATH}/plugins"
CMD_QMAKE="${QT_PATH}/bin/qmake"
CMD_DEPLOY="${QT_PATH}/bin/macdeployqt"

echo "Using MAC_SDK=${MAC_SDK}"

cd "$(dirname $0)"/..
SOURCE_PATH="${PWD}"
rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"
BUILD_PATH="${PWD}"
${CMD_QMAKE} -r CONFIG+="release" LIBS+=-dead_strip QMAKE_MAC_SDK=${MAC_SDK} QMAKE_MACOSX_DEPLOYMENT_TARGET=10.10 QMAKE_APPLE_DEVICE_ARCHS="x86_64 arm64" CONFIG+=c++2a CONFIG+="enable_qtwebkit" CONFIG+="enable_update_checking" "../${PROJECT}.pro"
make -j3
cd "${OUT_PATH}"
plutil -replace LSMinimumSystemVersion -string "10.10" "${APPNAME}.app/Contents/Info.plist"
plutil -replace LSArchitecturePriority -json '["arm64","x86_64"]' "${APPNAME}.app/Contents/Info.plist"
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
rm -rf "${INSTALL_PATH}" "${ARTIFACTS_PATH}"
mkdir -p "${INSTALL_PATH}" "${ARTIFACTS_PATH}"
mv "${OUT_PATH}/${APPNAME}.app" "${INSTALL_PATH}/"
cd "${INSTALL_PATH}"
ln -s /Applications ./Applications
find "${APPNAME}.app/Contents/PlugIns" -name "*_debug.dylib" -delete
cd "${BUILD_PATH}"

function sign() {
    if [ -z "${NO_SIGN+x}" ] ; then
        local max_retry=10
        local last_retry=$((${max_retry}-1))
        for ((i=0; i<${max_retry}; i++)) ; do
            if /usr/bin/codesign \
                    --sign "${APP_CERT}" \
                    --deep \
                    --force \
                    --timestamp \
                    --options runtime \
                    --entitlements "${SOURCE_PATH}/${ENTITLEMENTS_PATH}" \
                    --verbose \
                    --strict \
                    "${1}" ; then
                if [ ${i} != 0 ] ; then
                    echo "Sign completed at ${i} retry"
                fi
                break
            else
                if [ ${i} != ${last_retry} ] ; then
                    echo "Signing failed, retry ..."
                    sleep 5
                else
                    exit 2
                fi
            fi
        done
    fi
}
function notarize() {
    if [ -z "${NO_SIGN+x}" ] ; then
        /usr/bin/python3 "${SOURCE_PATH}/buildscripts/helpers/MacNotarizer.py" \
            --application "${1}" \
            --primary-bundle-id "${2}" \
            --username "${NOTARIZE_USERNAME}" \
            --password "${NOTARIZE_PASSWORD}" \
            --asc-provider "${NOTARIZE_ASC_PROVIDER}"
    fi
}
find "${INSTALL_PATH}/${APPNAME}.app/Contents/Frameworks" \( -name '*.framework' -or -name '*.dylib' \) -print0 | while IFS= read -r -d '' item ; do sign "${item}" ; done
find "${INSTALL_PATH}/${APPNAME}.app/Contents/PlugIns"       -name '*.dylib'                            -print0 | while IFS= read -r -d '' item ; do sign "${item}" ; done
sign "${INSTALL_PATH}/${APPNAME}.app"
notarize "${INSTALL_PATH}/${APPNAME}.app" "$(plutil -extract CFBundleIdentifier xml1 -o - "${INSTALL_PATH}/${APPNAME}.app/Contents/Info.plist" | sed -n 's|.*<string>\(.*\)<\/string>.*|\1|p')"

hdiutil create -format UDBZ -fs HFS+ -srcfolder "${INSTALL_PATH}" -volname "${APPNAME}" "${ARTIFACTS_PATH}/${DMGNAME}.dmg"
sign "${ARTIFACTS_PATH}/${DMGNAME}.dmg"
