#!/bin/bash -e
PROJECT="ImageViewer"
IDENTIFIER="com.github.aliencoweatcake.imageviewer"
BUILDDIR="build_linux_qt6.5_clang"
SUFFIX="_qt6.5_$(gcc -dumpmachine)"
APP_PATH="src/${PROJECT}"
DESKTOP_PATH="src/${PROJECT}/resources/platform/linux/${IDENTIFIER}.desktop"
ICON_PATH="src/${PROJECT}/resources/icon/icon.svg"
ICONS_DIR_PATH="src/${PROJECT}/resources/icon"
DEBIAN_DIR_PATH="src/${PROJECT}/resources/platform/debian"
SCRIPT_PATH="src/${PROJECT}/resources/platform/linux/set_associations.sh"
RESVG_PATH="$(cd "$(dirname "${0}")" && pwd)/resvg/$(gcc -dumpmachine | sed 's|-.*||')-unknown-linux-gnu"

QTDIR="/opt/qt6"
CLANGDIR="/opt/clang"
CMD_QMAKE="${QTDIR}/bin/qmake"
CMD_DEPLOY="/usr/local/bin/linuxdeployqt"
CMD_APPIMAGETOOL="/usr/local/bin/appimagetool"

export PATH="${CLANGDIR}/bin:${PATH}"
export LD_LIBRARY_PATH="${CLANGDIR}/lib:${QTDIR}/lib:${RESVG_PATH}:${LD_LIBRARY_PATH}"

cd "$(dirname $0)"/..
rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"
${CMD_QMAKE} -r CONFIG+="release" CONFIG+=c++17 CONFIG+="enable_librsvg" CONFIG+="enable_update_checking" CONFIG+="enable_qtcore5compat" CONFIG+="system_resvg" INCLUDEPATH+="\"${RESVG_PATH}\"" LIBS+="-L\"${RESVG_PATH}\"" "../${PROJECT}.pro"
make -j$(getconf _NPROCESSORS_ONLN)
strip --strip-all "${APP_PATH}/${PROJECT}"

rm -rf "AppDir"
mkdir -p "AppDir/usr/bin" "AppDir/usr/lib" "AppDir/usr/share/applications" "AppDir/usr/share/icons/hicolor/scalable/apps" "AppDir/usr/share/ImageViewer"
cp -a "${APP_PATH}/${PROJECT}" "AppDir/usr/bin/"
cp -a "../${SCRIPT_PATH}" "AppDir/usr/share/ImageViewer/"
cp -a "../${DESKTOP_PATH}" "AppDir/usr/share/applications/${IDENTIFIER}.desktop"
cp -a "../${ICON_PATH}" "AppDir/usr/share/icons/hicolor/scalable/apps/${IDENTIFIER}.svg"
find "../${ICONS_DIR_PATH}" -name '*.png' -print0 | while IFS= read -r -d '' RASTER_ICON_PATH ; do
    RASTER_ICON_SIZE=$(file "${RASTER_ICON_PATH}" | sed -n 's|.* \([0-9]\+\)[ ]*x[ ]*\([0-9]\+\),.*|\1x\2|p')
    if [ ! -z "${RASTER_ICON_SIZE}" ] ; then
        mkdir -p "AppDir/usr/share/icons/hicolor/${RASTER_ICON_SIZE}/apps"
        cp -a "${RASTER_ICON_PATH}" "AppDir/usr/share/icons/hicolor/${RASTER_ICON_SIZE}/apps/${IDENTIFIER}.${RASTER_ICON_PATH##*.}"
    fi
done
"${CMD_DEPLOY}" "AppDir/usr/share/applications/${IDENTIFIER}.desktop" -always-overwrite -qmake="${CMD_QMAKE}" -extra-plugins=platforms/libqwayland-generic.so,platforms/libqwayland-egl.so,platformthemes,styles,wayland-decoration-client,wayland-graphics-integration-client,wayland-shell-integration
find "AppDir" -type d -exec chmod 755 \{\} \;
find "AppDir" -type f -exec chmod 644 \{\} \;
find "AppDir" -type f \( -name "${PROJECT}" -o -name "AppRun" -o -name "*.so*" -o -name "*.sh" -o -name "*.desktop" \) -exec chmod 755 \{\} \;
if type "${CMD_APPIMAGETOOL}" &> /dev/null ; then
    "${CMD_APPIMAGETOOL}" --no-appstream "AppDir" ../"${PROJECT}${SUFFIX}.AppImage"
fi

cd "AppDir"
cp -a "../../${DEBIAN_DIR_PATH}" ./
find "debian" -type d -exec chmod 755 \{\} \;
find "debian" -type f -exec chmod 644 \{\} \;
chmod 755 "debian/rules"
dpkg-buildpackage -rfakeroot -b -uc
cd ..
cp -a *.deb ../
cd ..
