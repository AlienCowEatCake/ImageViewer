#!/bin/bash
PROJECT="ImageViewer"
BUILDDIR="build_haiku_qt5_gcc"
SUFFIX="_qt5_$(gcc -dumpmachine | sed 's|-unknown-|-|')"
APP_PATH="src/${PROJECT}"
RDEF_PATH="src/${PROJECT}/resources/platform/haiku/${PROJECT}.rdef"

CMD_QMAKE="qmake"

cd "$(dirname $0)"/..
rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"
${CMD_QMAKE} -r CONFIG+="release" QTPLUGIN.imageformats="qico qsvg qtiff" CONFIG+=c++11 "../${PROJECT}.pro"
make
strip --strip-all "${APP_PATH}/${PROJECT}"
cp -a "${APP_PATH}/${PROJECT}" ../"${PROJECT}${SUFFIX}"
rc -o "${PROJECT}.rsrc" ../"${RDEF_PATH}"
xres -o ../"${PROJECT}${SUFFIX}" "${PROJECT}.rsrc"
cd ..

