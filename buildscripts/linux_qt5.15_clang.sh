#!/bin/bash -e
PROJECT="ImageViewer"
IDENTIFIER="ru.codefreak.fami.imageviewer"
BUILDDIR="build_linux_qt5.15_clang"
SUFFIX="_qt5.15_$(gcc -dumpmachine)"
APP_PATH="src/${PROJECT}"
DESKTOP_PATH="src/${PROJECT}/resources/platform/linux/${IDENTIFIER}.desktop"
ICON_PATH="src/${PROJECT}/resources/icon/icon.svg"
DEBIAN_DIR_PATH="src/${PROJECT}/resources/platform/debian"

QTDIR="/opt/qt-5.15.2_clang"
CLANGDIR="/opt/clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-14.04"
CMD_QMAKE="${QTDIR}/bin/qmake"
CMD_DEPLOY="/opt/linuxdeployqt-6-x86_64.AppImage"
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
"${CMD_DEPLOY}" "AppDir/usr/share/applications/${IDENTIFIER}.desktop" -always-overwrite -qmake="${CMD_QMAKE}" -extra-plugins=styles,platformthemes
"${CMD_APPIMAGETOOL}" --no-appstream "AppDir" ../"${PROJECT}${SUFFIX}.AppImage"

cd "AppDir"
cp -a "../../${DEBIAN_DIR_PATH}" ./
dpkg-buildpackage -rfakeroot -b -uc
cd ..
cp -a *.deb ../
cd ..
