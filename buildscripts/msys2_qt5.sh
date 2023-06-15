#!/bin/bash -e
PROJECT="ImageViewer"
VCVARS_VER="2022"
BUILDDIR="build_msys2_qt5_${MSYSTEM,,?}"
SUFFIX="_qt5_${MSYSTEM,,?}"
APP_PATH="src/${PROJECT}"

WIX_PATH="$(cygpath -u "${WIX}")"
export PATH="${WIX_PATH}/bin:${WIX_PATH}:${PATH}"
CMD_QMAKE="qmake"
CMD_DEPLOY="windeployqt"

MSYSTEM_PKG_PREFIX="mingw-w64"
VCVARS_ARCH=""
CRT_ARCH=""
UCRT_ARCH=""
WIX_ARCH=""
WIX_OS_VER="7"
if [ "${MSYSTEM}" == "UCRT64" ] ; then
    MSYSTEM_PKG_PREFIX="${MSYSTEM_PKG_PREFIX}-ucrt-x86_64"
    VCVARS_ARCH="x64"
    CRT_ARCH="x64"
    UCRT_ARCH="x64"
    WIX_ARCH="x64"
elif [ "${MSYSTEM}" == "MINGW32" ] ; then
    MSYSTEM_PKG_PREFIX="${MSYSTEM_PKG_PREFIX}-i686"
    VCVARS_ARCH="x64_x86"
    CRT_ARCH="x86"
    UCRT_ARCH="x86"
    WIX_ARCH="x86"
elif [ "${MSYSTEM}" == "MINGW64" ] ; then
    MSYSTEM_PKG_PREFIX="${MSYSTEM_PKG_PREFIX}-x86_64"
    VCVARS_ARCH="x64"
    CRT_ARCH="x64"
    UCRT_ARCH="x64"
    WIX_ARCH="x64"
elif [ "${MSYSTEM}" == "CLANG32" ] ; then
    MSYSTEM_PKG_PREFIX="${MSYSTEM_PKG_PREFIX}-clang-i686"
    VCVARS_ARCH="x64_x86"
    CRT_ARCH="x86"
    UCRT_ARCH="x86"
    WIX_ARCH="x86"
elif [ "${MSYSTEM}" == "CLANG64" ] ; then
    MSYSTEM_PKG_PREFIX="${MSYSTEM_PKG_PREFIX}-clang-x86_64"
    VCVARS_ARCH="x64"
    CRT_ARCH="x64"
    UCRT_ARCH="x64"
    WIX_ARCH="x64"
elif [ "${MSYSTEM}" == "CLANGARM64" ] ; then
    MSYSTEM_PKG_PREFIX="${MSYSTEM_PKG_PREFIX}-clang-aarch64"
    VCVARS_ARCH="arm64"
    CRT_ARCH="arm64"
    UCRT_ARCH="arm"
    WIX_ARCH="arm64"
    WIX_OS_VER="10"
else
    echo "Unknown or broken MSYSTEM: ${MSYSTEM}"
    exit 1
fi
pacman -S --needed --noconfirm \
    base-devel \
    zip \
    ${MSYSTEM_PKG_PREFIX}-toolchain \
    ${MSYSTEM_PKG_PREFIX}-openssl \
    ${MSYSTEM_PKG_PREFIX}-qt5-tools \
    ${MSYSTEM_PKG_PREFIX}-qt5-base \
    ${MSYSTEM_PKG_PREFIX}-qt5-svg \
    ${MSYSTEM_PKG_PREFIX}-qt5-imageformats \
    ${MSYSTEM_PKG_PREFIX}-qt5-translations \
    ${MSYSTEM_PKG_PREFIX}-zlib \
    ${MSYSTEM_PKG_PREFIX}-jbigkit \
    ${MSYSTEM_PKG_PREFIX}-lerc \
    ${MSYSTEM_PKG_PREFIX}-libtiff \
    ${MSYSTEM_PKG_PREFIX}-libwebp \
    ${MSYSTEM_PKG_PREFIX}-freetype \
    ${MSYSTEM_PKG_PREFIX}-librsvg

cd "$(dirname $0)"/..
SOURCE_PATH="${PWD}"
DIST_PREFIX="${PROJECT}${SUFFIX}"

function copyDlls() {
    "${SOURCE_PATH}/buildscripts/helpers/dllresolver.exe" "${DIST_PREFIX}" "${@}"
}

function copyWebView2Loader() {
    local dll="$(find "${SOURCE_PATH}/src/ThirdParty/MSEdgeWebView2" -name 'WebView2Loader.dll' | grep "/native/${VCVARS_ARCH##*_}/" || true)"
    if [ ! -z "${dll}" ] ; then
        cp -a "${dll}" "${DIST_PREFIX}/"
    fi
}

function getVCVARSPath() {
    local VCVARS_HELPER="$(cygpath -w "${SOURCE_PATH}/buildscripts/helpers/find_vcvarsall.bat")"
    cat << EOF | cmd | tail -3 | head -1
call "${VCVARS_HELPER}" ${VCVARS_VER}
echo %VS${VCVARS_VER}_VCVARSALL%
EOF
}

function getCRTPath() {
    local VCVARS="$(getVCVARSPath)"
    cat << EOF | cmd | tail -3 | head -1
set VCVARS="${VCVARS}"
call %VCVARS% ${VCVARS_ARCH}
echo %VCToolsRedistDir%${CRT_ARCH}\Microsoft.VC143.CRT
EOF
}

function getUCRTPath() {
    local VCVARS="$(getVCVARSPath)"
    cat << EOF | cmd | tail -3 | head -1
set VCVARS="${VCVARS}"
call %VCVARS% ${VCVARS_ARCH}
echo %UniversalCRTSdkDir%Redist\%UCRTVersion%\ucrt\DLLs\\${UCRT_ARCH}
EOF
}

function stripAll() {
    find "${DIST_PREFIX}" \( -name '*.exe' -o -name '*.dll' \) | while IFS= read -r item ; do
        strip --strip-all "${item}" || strip "${item}" || true
    done
}

rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"
${CMD_QMAKE} -r CONFIG+="release" \
    CONFIG+=c++2a \
    CONFIG+="enable_pkgconfig enable_update_checking enable_msedgewebview2" \
    CONFIG+="system_zlib system_jbigkit system_lerc system_libtiff" \
    CONFIG+="system_libwebp system_freetype system_librsvg" \
    "${SOURCE_PATH}/${PROJECT}.pro"
make -j$(getconf _NPROCESSORS_ONLN)
rm -rf "${DIST_PREFIX}"
mkdir "${DIST_PREFIX}"
cp -a "${APP_PATH}/release/${PROJECT}.exe" "${DIST_PREFIX}/"
${CMD_DEPLOY} \
    --no-compiler-runtime \
    --no-system-d3d-compiler \
    --no-virtualkeyboard \
    --no-angle \
    --no-opengl-sw \
    --translations en,ru \
    "${DIST_PREFIX}/"
find "${MSYSTEM_PREFIX}/bin" -type f \( -name 'libssl*.dll' -o -name 'libcrypto*.dll' \) -exec cp -a \{\} "${DIST_PREFIX}/" \;
find "${DIST_PREFIX}/imageformats" -type f \( -name 'kimg_*.dll' -o -name 'qjp2.dll' -o -name 'qmng.dll' \) -delete
find "${DIST_PREFIX}/platforms" -type f \( -name 'qdirect2d.dll' -o -name 'qminimal.dll' -o -name 'qoffscreen.dll' \) -delete
copyDlls "$(cygpath -w "${MSYSTEM_PREFIX}/bin")"
stripAll
copyWebView2Loader
copyDlls "$(cygpath -w "$(getCRTPath)")" "$(cygpath -w "$(getUCRTPath)")"
zip -9r "../${DIST_PREFIX}.zip" "${DIST_PREFIX}"
rm -rf "build_msi"
mv "${DIST_PREFIX}" "build_msi"
heat dir build_msi -cg ApplicationFiles -dr INSTALLLOCATION -gg -scom -sfrag -srd -sreg -svb6 -out appfiles.wxs
candle -out appfiles.wixobj appfiles.wxs -arch "${WIX_ARCH}"
candle -out common.wixobj "../src/ImageViewer/resources/platform/windows/common.wxs" -arch "${WIX_ARCH}"
candle -out main.wixobj "../src/ImageViewer/resources/platform/windows/w${WIX_OS_VER}_${WIX_ARCH}.wxs" -ext WixUIExtension -ext WixUtilExtension -arch "${WIX_ARCH}"
light -out "${PROJECT}.msi" -b build_msi main.wixobj appfiles.wixobj common.wixobj -ext WixUIExtension -ext WixUtilExtension -dcl:high
mv "${PROJECT}.msi" "../${DIST_PREFIX}.msi"
cd ..
