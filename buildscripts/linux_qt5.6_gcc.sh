#!/bin/bash
PROJECT="ImageViewer"
BUILDDIR="build_linux_qt5.6_gcc"
SUFFIX="_qt5.6_$(gcc -dumpmachine)"
APP_PATH="src/${PROJECT}"

QTDIR="/opt/qt-5.6.3-static"
CMD_QMAKE="${QTDIR}/bin/qmake"

cd "$(dirname $0)"/..
rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"
${CMD_QMAKE} -r CONFIG+="release" QTPLUGIN.imageformats="qico qsvg qtiff" "../${PROJECT}.pro"
make
strip --strip-all "${APP_PATH}/${PROJECT}"
cp -a "${APP_PATH}/${PROJECT}" ../"${PROJECT}${SUFFIX}.elf"

