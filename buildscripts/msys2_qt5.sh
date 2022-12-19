#!/bin/bash -e
PROJECT="ImageViewer"
VCVARS="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
BUILDDIR="build_msys2_qt5_${MSYSTEM,,?}"
SUFFIX="_qt5_${MSYSTEM,,?}"
APP_PATH="src/${PROJECT}"

export PATH="$(cygpath -u "${WIX}\bin"):${PATH}"
CMD_QMAKE="qmake"
CMD_DEPLOY="windeployqt"

MSYSTEM_PKG_PREFIX="mingw-w64"
VCVARS_ARCH=""
CRT_ARCH=""
UCRT_ARCH=""
WIX_ARCH=""
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
# elif [ "${MSYSTEM}" == "CLANGARM64" ] ; then
#     MSYSTEM_PKG_PREFIX="${MSYSTEM_PKG_PREFIX}-clang-aarch64"
#     VCVARS_ARCH="arm64"
#     CRT_ARCH="arm64"
#     UCRT_ARCH="arm"
#     WIX_ARCH="x64"
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

function getDeps() {
    find "${DIST_PREFIX}" \( -name '*.exe' -o -name '*.dll' \) -exec objdump --private-headers \{\} \; | grep 'DLL Name:' | sort | uniq | sed 's|.* ||'
}

function checkDll() {
    find "${DIST_PREFIX}" -name "${1}" | grep -E '*' >/dev/null 2>/dev/null
}

function copyDlls() {
    local CHANGED
    local RESOLVED
    while true ; do
        CHANGED=false
        for i in $(getDeps) ; do
            RESOLVED=false
            if checkDll "${i}" ; then
                continue
            fi
            for j in "${@}" ; do
                if find "${j}" -maxdepth 1 -name "${i}" -print -exec cp -a \{\} "${DIST_PREFIX}/" \; | grep -E '*' ; then
                    CHANGED=true
                    RESOLVED=true
                    break
                fi
            done
            if ! ${RESOLVED} ; then
                echo "Unresolved: ${i}"
            fi
        done
        if ! ${CHANGED} ; then
            break
        fi
    done
}

function copyWebView2Loader() {
    local dll="$(find "${SOURCE_PATH}/src/ThirdParty/MSEdgeWebView2" -name 'WebView2Loader.dll' | grep "/native/${VCVARS_ARCH##*_}/" || true)"
    if [ ! -z "${dll}" ] ; then
        cp -a "${dll}" "${DIST_PREFIX}/"
    fi
}

function getCRTPath() {
    cat << EOF | cmd | tail -3 | head -1
set VCVARS="${VCVARS}"
call %VCVARS% ${VCVARS_ARCH}
echo %VCToolsRedistDir%${CRT_ARCH}\Microsoft.VC143.CRT
EOF
}

function getUCRTPath() {
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
make -j4
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
copyDlls "${MSYSTEM_PREFIX}/bin"
stripAll
copyWebView2Loader
copyDlls "$(cygpath -u "$(getCRTPath)")" "$(cygpath -u "$(getUCRTPath)")"
zip -9r "../${DIST_PREFIX}.zip" "${DIST_PREFIX}"
rm -rf "build_msi"
mv "${DIST_PREFIX}" "build_msi"
heat dir build_msi -cg ApplicationFiles -dr INSTALLLOCATION -gg -scom -sfrag -srd -sreg -svb6 -out appfiles.wxs
candle -out appfiles.wixobj appfiles.wxs -arch "${WIX_ARCH}"
candle -out common.wixobj "../src/ImageViewer/resources/platform/windows/common.wxs" -arch "${WIX_ARCH}"
candle -out main.wixobj "../src/ImageViewer/resources/platform/windows/${WIX_ARCH}.wxs" -ext WixUIExtension -ext WixUtilExtension -arch "${WIX_ARCH}"
light -out "${PROJECT}.msi" -b build_msi main.wixobj appfiles.wixobj common.wixobj -ext WixUIExtension -ext WixUtilExtension -dcl:high
mv "${PROJECT}.msi" "../${DIST_PREFIX}.msi"
cd ..
