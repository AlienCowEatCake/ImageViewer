#!/bin/bash -e
PROJECT="ImageViewer"
BUILDDIR="build_haiku_qt5_gcc"
SUFFIX="_qt5_$(gcc -dumpmachine | sed 's|-unknown-|-| ; s|-pc-|-|')"
APP_PATH="src/${PROJECT}"
RDEF_PATH="src/${PROJECT}/resources/platform/haiku/${PROJECT}.rdef"

export PATH="/bin/x86:${PATH}"
CMD_QMAKE="qmake"

cd "$(dirname $0)"/..
rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"
${CMD_QMAKE} -spec haiku-g++ -r CONFIG+="release" CONFIG+=c++11 CONFIG+="enable_update_checking" CONFIG+="enable_nanosvg" "../${PROJECT}.pro"
make
strip --strip-all "${APP_PATH}/${PROJECT}"
cp -a "${APP_PATH}/${PROJECT}" ../"${PROJECT}${SUFFIX}"
rc -o "${PROJECT}.rsrc" ../"${RDEF_PATH}"
xres -o ../"${PROJECT}${SUFFIX}" "${PROJECT}.rsrc"
cd ..

