#!/bin/bash -e
PROJECT="ImageViewer"
SUFFIX="qt5.6"
SUFFIX_FULL="${SUFFIX}_$(gcc -dumpmachine)"
BUILDDIR="build_linux_${SUFFIX_FULL}_gcc"
APP_PATH="src/${PROJECT}"

QTDIR="/opt/qt-5.6.3-static"
CMD_QMAKE="${QTDIR}/bin/qmake"

cd "$(dirname $0)"/..
rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"
${CMD_QMAKE} -r CONFIG+="release" QTPLUGIN.imageformats="qico qsvg qtiff" CONFIG+="enable_librsvg enable_j40" "../${PROJECT}.pro"
make -j$(getconf _NPROCESSORS_ONLN)
strip --strip-all "${APP_PATH}/${PROJECT}"
cp -a "${APP_PATH}/${PROJECT}" ../"${PROJECT}_${SUFFIX_FULL}.elf"
cd ..
gzip -9v "${PROJECT}_${SUFFIX_FULL}.elf"

