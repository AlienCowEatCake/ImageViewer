#!/bin/bash -e
PROJECT=ImageViewer
BUILDDIR=build_osx_qt4.4_universal
APPNAME="Image Viewer"
DMGNAME="${PROJECT}_qt4.4_universal"
INFO_PLIST_PATH="src/${PROJECT}/resources/platform/macosx/Info.plist"
ICON_PATH="src/${PROJECT}/resources/icon/icon_composer.icns"
SCRIPT_PATH="src/${PROJECT}/resources/platform/macosx/set_associations.sh"
LICENSE_PATH="LICENSE.GPLv3"
OUT_PATH="src/${PROJECT}"
APP_CERT="Developer ID Application: Petr Zhigalov (48535TNTA7)"
MAC_TARGET="10.3"
MAC_SDK="/Developer/SDKs/MacOSX10.4u.sdk"
GCC_VERSION="4.0"

QT_FRAMEWORKS_PATH="/Library/Frameworks"
QT_PLUGINS_PATH="/Developer/Applications/Qt/plugins"
QT_TRANSLATIONS_PATH="/Developer/Applications/Qt/translations"
CMD_QMAKE="qmake"

cd "$(dirname $0)"/..
SOURCE_PATH="${PWD}"
rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"
BUILD_PATH="${PWD}"
# @note Out-of-tree builds is not supported by Qt 4.4 with macx-xcode spec
cd "${SOURCE_PATH}"
${CMD_QMAKE} -macx -recursive -spec macx-xcode CONFIG+="release" LIBS+="-dead_strip" CONFIG+="x86 ppc" QMAKE_MAC_SDK="${MAC_SDK}" QMAKE_MACOSX_DEPLOYMENT_TARGET="${MAC_TARGET}" CONFIG+="disable_cxx11 disable_mactoolbar disable_mactouchbar" CONFIG+="enable_j40"
find "src/ThirdParty" -mindepth 2 -maxdepth 2 -name '*.xcodeproj' -print0 | while IFS= read -r -d '' SUBPROJECT_PATH ; do
    cd "${SUBPROJECT_PATH}"
    find . -maxdepth 1 -name 'project.pbxproj.*' -exec ln -s "{}" "project.pbxproj" \; || true
    # "-DGETTEXT_PACKAGE=\\"libexif-12\\"", => "-DGETTEXT_PACKAGE=\"libexif-12\"",
    sed -i '' 's|\("-D.*=\\\)\\\(".*\\\)\\\("".*\)|\1\2\3|' 'project.pbxproj'
    cd ..
    xcodebuild -sdk "${MAC_SDK}" -configuration Release USE_HEADERMAP=NO MACOSX_DEPLOYMENT_TARGET="${MAC_TARGET}" SDKROOT="${MAC_SDK}" GCC_VERSION="${GCC_VERSION}" CLANG_CXX_LIBRARY=libstdc++ ARCHS="i386 ppc"
    cd "${SOURCE_PATH}"
done
find "src/QtUtils" -maxdepth 1 -name '*.xcodeproj' -print0 | while IFS= read -r -d '' SUBPROJECT_PATH ; do
    cd "${SUBPROJECT_PATH}"
    find . -maxdepth 1 -name 'project.pbxproj.*' -exec ln -s "{}" "project.pbxproj" \; || true
    # "-DGETTEXT_PACKAGE=\\"libexif-12\\"", => "-DGETTEXT_PACKAGE=\"libexif-12\"",
    sed -i '' 's|\("-D.*=\\\)\\\(".*\\\)\\\("".*\)|\1\2\3|' 'project.pbxproj'
    cd ..
    xcodebuild -sdk "${MAC_SDK}" -configuration Release USE_HEADERMAP=NO MACOSX_DEPLOYMENT_TARGET="${MAC_TARGET}" SDKROOT="${MAC_SDK}" GCC_VERSION="${GCC_VERSION}" CLANG_CXX_LIBRARY=libstdc++ ARCHS="i386 ppc"
    cd "${SOURCE_PATH}"
done
find "src/ImageViewer" -maxdepth 1 -name '*.xcodeproj' -print0 | while IFS= read -r -d '' SUBPROJECT_PATH ; do
    cd "${SUBPROJECT_PATH}"
    find . -maxdepth 1 -name 'project.pbxproj.*' -exec ln -s "{}" "project.pbxproj" \; || true
    # "-DGETTEXT_PACKAGE=\\"libexif-12\\"", => "-DGETTEXT_PACKAGE=\"libexif-12\"",
    sed -i '' 's|\("-D.*=\\\)\\\(".*\\\)\\\("".*\)|\1\2\3|' 'project.pbxproj'
    # shellScript = "make\ -C\ /tmp/ImageViewer/src/ImageViewer\ -f\ Image\ Viewer.xcodeproj/qt_makeqmake.mak"; => shellScript = "make\ -C\ /tmp/ImageViewer/src/ImageViewer\ -f\ Image\\ Viewer.xcodeproj/qt_makeqmake.mak";
    sed -i '' 's|\(shellScript = .* Image\\\)\( Viewer.xcodeproj.*\)|\1\\\2|' 'project.pbxproj'
    cd ..
    xcodebuild -sdk "${MAC_SDK}" -configuration Release USE_HEADERMAP=NO MACOSX_DEPLOYMENT_TARGET="${MAC_TARGET}" SDKROOT="${MAC_SDK}" GCC_VERSION="${GCC_VERSION}" CLANG_CXX_LIBRARY=libstdc++ ARCHS="i386 ppc"
    cp -a "${APPNAME}.app" "${BUILD_PATH}/"
    cd "${SOURCE_PATH}"
done

cd "${BUILD_PATH}"
cp -a "${SOURCE_PATH}/${INFO_PLIST_PATH}" "${APPNAME}.app/Contents/Info.plist"
/usr/libexec/PlistBuddy -c "Set :LSMinimumSystemVersion ${MAC_TARGET}" "${APPNAME}.app/Contents/Info.plist"
/usr/libexec/PlistBuddy -c "Delete :LSArchitecturePriority" "${APPNAME}.app/Contents/Info.plist"
/usr/libexec/PlistBuddy -c "Add :LSArchitecturePriority array" "${APPNAME}.app/Contents/Info.plist"
/usr/libexec/PlistBuddy -c "Add :LSArchitecturePriority: string i386" "${APPNAME}.app/Contents/Info.plist"
/usr/libexec/PlistBuddy -c "Add :LSArchitecturePriority: string ppc" "${APPNAME}.app/Contents/Info.plist"
plutil -convert xml1 "${APPNAME}.app/Contents/Info.plist" -o "${APPNAME}.app/Contents/Info.plist"
RES_PATH="${APPNAME}.app/Contents/Resources"
rm -rf "${RES_PATH}/empty.lproj"
mkdir -p "${RES_PATH}/en.lproj" "${RES_PATH}/ru.lproj" "${RES_PATH}/zh_CN.lproj" "${RES_PATH}/zh_TW.lproj"
cp -a "${SOURCE_PATH}/${ICON_PATH}" "${RES_PATH}/icon.icns"
cp -a "${SOURCE_PATH}/${SCRIPT_PATH}" "${RES_PATH}/"
PLUGINS_PATH="${APPNAME}.app/Contents/PlugIns"
mkdir -p "${PLUGINS_PATH}/accessible"
for plugin in libqtaccessiblewidgets.dylib ; do
    if [ -f "${QTPLUGINS_PATH}/accessible/${plugin}" ] ; then
        cp -a "${QTPLUGINS_PATH}/accessible/${plugin}" "${PLUGINS_PATH}/accessible/"
    fi
done
for subdir in codecs iconengines imageformats ; do
    if [ -d "${QT_PLUGINS_PATH}/${subdir}" ] ; then
        cp -a "${QT_PLUGINS_PATH}/${subdir}" "${PLUGINS_PATH}/"
    fi
done
/usr/bin/python "${SOURCE_PATH}/buildscripts/helpers/dylibresolver.py" "${APPNAME}.app" "${QT_FRAMEWORKS_PATH}"
TRANSLATIONS_PATH="${RES_PATH}/translations"
mkdir -p "${TRANSLATIONS_PATH}"
for lang in $(find "${RES_PATH}" -name '*.lproj' | sed 's|.*/|| ; s|\..*||') ; do
    if [ -f "${QT_TRANSLATIONS_PATH}/qt_${lang}.qm" ] ; then
        cp -a "${QT_TRANSLATIONS_PATH}/qt_${lang}.qm" "${TRANSLATIONS_PATH}/qt_${lang}.qm"
    fi
done
echo -e '[Paths]\nPrefix = Frameworks\nPlugins = PlugIns\nTranslations = Resources/translations' > "${RES_PATH}/qt.conf"
cd "${BUILD_PATH}"

INSTALL_PATH="${PWD}/install"
ARTIFACTS_PATH="${PWD}/artifacts"
rm -rf "${INSTALL_PATH}" "${ARTIFACTS_PATH}"
mkdir -p "${INSTALL_PATH}" "${ARTIFACTS_PATH}"
mv "${APPNAME}.app" "${INSTALL_PATH}/"
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
                    --force \
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
find "${INSTALL_PATH}/${APPNAME}.app/Contents/Frameworks" -name '*.framework' -print0 | while IFS= read -r -d '' framework ; do
    find "${framework}/Versions" -mindepth 2 -maxdepth 2 -type f -print0 | while IFS= read -r -d '' item ; do sign "${item}" ; done
done
find "${INSTALL_PATH}/${APPNAME}.app/Contents/Frameworks" -name '*.dylib' -print0 | while IFS= read -r -d '' item ; do sign "${item}" ; done
find "${INSTALL_PATH}/${APPNAME}.app/Contents/PlugIns"    -name '*.dylib' -print0 | while IFS= read -r -d '' item ; do sign "${item}" ; done
sign "${INSTALL_PATH}/${APPNAME}.app"

hdiutil create -format UDZO -fs HFS+ -srcfolder "${INSTALL_PATH}" -volname "${APPNAME}" "${ARTIFACTS_PATH}/${DMGNAME}.dmg"

