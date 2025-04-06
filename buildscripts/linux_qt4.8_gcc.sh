#!/bin/bash -e
PROJECT="ImageViewer"
SUFFIX="qt4.8"
SUFFIX_FULL="${SUFFIX}_$(gcc -dumpmachine)"
BUILDDIR="build_linux_${SUFFIX_FULL}_gcc"
APP_PATH="src/${PROJECT}"

QTDIR="/opt/qt-4.8.7-static"
CMD_QMAKE="${QTDIR}/bin/qmake"

cd "$(dirname $0)"/..
rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"
${CMD_QMAKE} -r CONFIG+="release" CONFIG+="use_static_qico" CONFIG+="enable_librsvg" "../${PROJECT}.pro"
make -j$(getconf _NPROCESSORS_ONLN)
strip --strip-all "${APP_PATH}/${PROJECT}"
cp -a "${APP_PATH}/${PROJECT}" ../"${PROJECT}_${SUFFIX_FULL}.elf"
cd ..
gzip -9v "${PROJECT}_${SUFFIX_FULL}.elf"

