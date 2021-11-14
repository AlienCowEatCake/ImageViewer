#!/bin/bash -e
PROJECT="ImageViewer"
IDENTIFIER="com.github.aliencoweatcake.imageviewer"
BUILDDIR="build_linux_qt5.15_clang"
SUFFIX="_qt5.15_$(gcc -dumpmachine)"
APP_PATH="src/${PROJECT}"
DESKTOP_PATH="src/${PROJECT}/resources/platform/linux/${IDENTIFIER}.desktop"
ICON_PATH="src/${PROJECT}/resources/icon/icon.svg"
ICONS_DIR_PATH="src/${PROJECT}/resources/icon"
DEBIAN_DIR_PATH="src/${PROJECT}/resources/platform/debian"

QTDIR="/opt/qt-5.15.2_clang"
CLANGDIR="/opt/clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-14.04"
CMD_QMAKE="${QTDIR}/bin/qmake"
CMD_DEPLOY="/opt/linuxdeployqt-7-x86_64.AppImage"
CMD_APPIMAGETOOL="/opt/appimagetool-x86_64.AppImage"

export PATH="${CLANGDIR}/bin:${PATH}"
export LD_LIBRARY_PATH="${CLANGDIR}/lib:${QTDIR}/lib:${LD_LIBRARY_PATH}"

cd "$(dirname $0)"/..
rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"
${CMD_QMAKE} -r CONFIG+="release" CONFIG+=c++2a CONFIG+="enable_update_checking" "../${PROJECT}.pro"
make
strip --strip-all "${APP_PATH}/${PROJECT}"

rm -rf "AppDir"
mkdir -p "AppDir/usr/bin" "AppDir/usr/lib" "AppDir/usr/share/applications" "AppDir/usr/share/icons/hicolor/scalable/apps"
cp -a "${APP_PATH}/${PROJECT}" "AppDir/usr/bin/"
cp -a "../${DESKTOP_PATH}" "AppDir/usr/share/applications/${IDENTIFIER}.desktop"
cp -a "../${ICON_PATH}" "AppDir/usr/share/icons/hicolor/scalable/apps/${IDENTIFIER}.svg"
find "../${ICONS_DIR_PATH}" -name '*.png' -print0 | while IFS= read -r -d '' RASTER_ICON_PATH ; do
    RASTER_ICON_SIZE=$(file "${RASTER_ICON_PATH}" | sed -n 's|.* \([0-9]\+\)[ ]*x[ ]*\([0-9]\+\),.*|\1x\2|p')
    if [ ! -z "${RASTER_ICON_SIZE}" ] ; then
        mkdir -p "AppDir/usr/share/icons/hicolor/${RASTER_ICON_SIZE}/apps"
        cp -a "${RASTER_ICON_PATH}" "AppDir/usr/share/icons/hicolor/${RASTER_ICON_SIZE}/apps/${IDENTIFIER}.${RASTER_ICON_PATH##*.}"
    fi
done
"${CMD_DEPLOY}" "AppDir/usr/share/applications/${IDENTIFIER}.desktop" -always-overwrite -qmake="${CMD_QMAKE}" -extra-plugins=styles,platformthemes
"${CMD_APPIMAGETOOL}" --no-appstream "AppDir" ../"${PROJECT}${SUFFIX}.AppImage"

cd "AppDir"
cp -a "../../${DEBIAN_DIR_PATH}" ./
dpkg-buildpackage -rfakeroot -b -uc
cd ..
cp -a *.deb ../
cd ..
