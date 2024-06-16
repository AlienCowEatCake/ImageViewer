#!/bin/bash -e
PROJECT="ImageViewer"
BUILDDIR="build_linux_qt4.8_gcc"
SUFFIX="_qt4.8_$(gcc -dumpmachine)"
APP_PATH="src/${PROJECT}"

QTDIR="/opt/qt-4.8.7-static"
CMD_QMAKE="${QTDIR}/bin/qmake"

cd "$(dirname $0)"/..
rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"
${CMD_QMAKE} -r CONFIG+="release" CONFIG+="use_static_qico" CONFIG+="enable_librsvg enable_nanosvg" CONFIG+="enable_update_checking" "../${PROJECT}.pro"
make -j$(getconf _NPROCESSORS_ONLN)
strip --strip-all "${APP_PATH}/${PROJECT}"
cp -a "${APP_PATH}/${PROJECT}" ../"${PROJECT}${SUFFIX}.elf"
cd ..
gzip -9v "${PROJECT}${SUFFIX}.elf"

