#!/bin/bash -e
PROJECT=ImageViewer
BUILDDIR=build_osx_qt4.8_clang64
APPNAME="Image Viewer"
DMGNAME="${PROJECT}_qt4.8_clang64"
INFO_PLIST_PATH="src/${PROJECT}/resources/platform/macosx/Info.plist"
ICON_PATH="src/${PROJECT}/resources/icon/icon_jp2.icns"
SCRIPT_PATH="src/${PROJECT}/resources/platform/macosx/set_associations.sh"
LICENSE_PATH="LICENSE.GPLv3"
OUT_PATH="src/${PROJECT}"
APP_CERT="Developer ID Application: Petr Zhigalov (48535TNTA7)"
MAC_TARGET="10.5"
ALL_SDK_VERSIONS="$(xcodebuild -showsdks | grep '\-sdk macosx' | sed 's|.*-sdk macosx||')"
for SDK_VERSION in ${ALL_SDK_VERSIONS} ; do
    SDK_PATH="$(xcode-select -p)/Platforms/MacOSX.platform/Developer/SDKs/MacOSX${SDK_VERSION}.sdk"
    if [[ $(find "${SDK_PATH}/usr/lib" -name 'libstdc++*' -maxdepth 1 | wc -l | xargs) > 0 ]] ; then
        MAC_SDK="${SDK_PATH}"
    fi
done

QT_FRAMEWORKS_PATH="/Library/Frameworks"
QT_TRANSLATIONS_PATH="/Developer/Applications/Qt/translations"
CMD_QMAKE="qmake"
CMD_DEPLOY="macdeployqt"

echo "Using MAC_SDK=${MAC_SDK}"

cd "$(dirname $0)"/..
SOURCE_PATH="${PWD}"
rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"
BUILD_PATH="${PWD}"
${CMD_QMAKE} -recursive -spec unsupported/macx-clang CONFIG+="release" LIBS+="-dead_strip" CONFIG+="x86_64" QMAKE_MAC_SDK="${MAC_SDK}" QMAKE_MACOSX_DEPLOYMENT_TARGET="${MAC_TARGET}" CONFIG+="disable_cxx11" CONFIG+="enable_j40" "../${PROJECT}.pro"
make -j$(getconf _NPROCESSORS_ONLN)
cd "${OUT_PATH}"
cp -a "${SOURCE_PATH}/${INFO_PLIST_PATH}" "${APPNAME}.app/Contents/Info.plist"
plutil -replace LSMinimumSystemVersion -string "${MAC_TARGET}" "${APPNAME}.app/Contents/Info.plist"
RES_PATH="${APPNAME}.app/Contents/Resources"
rm -rf "${RES_PATH}/empty.lproj"
mkdir -p "${RES_PATH}/en.lproj" "${RES_PATH}/ru.lproj" "${RES_PATH}/zh_CN.lproj" "${RES_PATH}/zh_TW.lproj"
cp -a "${SOURCE_PATH}/${ICON_PATH}" "${RES_PATH}/icon.icns"
cp -a "${SOURCE_PATH}/${SCRIPT_PATH}" "${RES_PATH}/"
${CMD_DEPLOY} "${APPNAME}.app" -verbose=2
find "${APPNAME}.app/Contents" -type f -name '*.prl' -delete
/usr/bin/python "${SOURCE_PATH}/buildscripts/helpers/dylibresolver.py" "${APPNAME}.app" "${QT_FRAMEWORKS_PATH}"
TRANSLATIONS_PATH="${RES_PATH}/translations"
mkdir -p "${TRANSLATIONS_PATH}"
for lang in $(find "${RES_PATH}" -name '*.lproj' | sed 's|.*/|| ; s|\..*||') ; do
    if [ -f "${QT_TRANSLATIONS_PATH}/qt_${lang}.qm" ] ; then
        cp -a "${QT_TRANSLATIONS_PATH}/qt_${lang}.qm" "${TRANSLATIONS_PATH}/qt_${lang}.qm"
    fi
done
echo 'Translations = Resources/translations' >> "${RES_PATH}/qt.conf"
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
                    --verbose \
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
find "${INSTALL_PATH}/${APPNAME}.app/Contents/Frameworks" \( -name '*.framework' -or -name '*.dylib' \) -print0 | while IFS= read -r -d '' item ; do sign "${item}" ; done
find "${INSTALL_PATH}/${APPNAME}.app/Contents/PlugIns"       -name '*.dylib'                            -print0 | while IFS= read -r -d '' item ; do sign "${item}" ; done
sign "${INSTALL_PATH}/${APPNAME}.app"

hdiutil create -format UDBZ -fs HFS+ -srcfolder "${INSTALL_PATH}" -volname "${APPNAME}" "${ARTIFACTS_PATH}/${DMGNAME}.dmg"

