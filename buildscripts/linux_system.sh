#!/bin/bash -e
PROJECT="ImageViewer"
IDENTIFIER="com.github.aliencoweatcake.imageviewer"
BUILDDIR="build_linux_system"
APP_PATH="src/${PROJECT}"
DESKTOP_PATH="src/${PROJECT}/resources/platform/linux/${IDENTIFIER}.desktop"
ICON_PATH="src/${PROJECT}/resources/icon/icon.svg"
ICONS_DIR_PATH="src/${PROJECT}/resources/icon"
DEBIAN_DIR_PATH="src/${PROJECT}/resources/platform/debian"
SCRIPT_PATH="src/${PROJECT}/resources/platform/linux/set_associations.sh"

CMD_QMAKE="qmake"
CONFIG_STR="release enable_pkgconfig disable_zstd disable_xzutils disable_brotli disable_highway disable_libexpat disable_libbpg disable_freetype disable_aom disable_libde265 disable_flif disable_qtextended disable_stb disable_qtimageformats disable_kimageformats"
if pkg-config zlib ; then
    CONFIG_STR="${CONFIG_STR} system_zlib"
else
    CONFIG_STR="${CONFIG_STR} disable_zlib"
fi
if pkg-config lcms2 ; then
    CONFIG_STR="${CONFIG_STR} system_liblcms2"
else
    CONFIG_STR="${CONFIG_STR} disable_liblcms2"
fi
if pkg-config libexif ; then
    CONFIG_STR="${CONFIG_STR} system_libexif"
else
    CONFIG_STR="${CONFIG_STR} disable_libexif"
fi
if pkg-config exiv2 ; then
    CONFIG_STR="${CONFIG_STR} system_exiv2"
else
    CONFIG_STR="${CONFIG_STR} disable_exiv2"
fi
if pkg-config libjpeg ; then
    CONFIG_STR="${CONFIG_STR} system_libjpeg"
else
    CONFIG_STR="${CONFIG_STR} disable_libjpeg"
fi
if echo '#include <jasper/jasper.h>' | cpp -x c++ 2>/dev/null >/dev/null ; then
    CONFIG_STR="${CONFIG_STR} system_libjasper"
else
    CONFIG_STR="${CONFIG_STR} disable_libjasper"
fi
if echo '#include <libmng.h>' | cpp -x c++ 2>/dev/null >/dev/null ; then
    CONFIG_STR="${CONFIG_STR} system_libmng"
else
    CONFIG_STR="${CONFIG_STR} disable_libmng"
fi
if pkg-config libpng ; then
    CONFIG_STR="${CONFIG_STR} system_libpng"
else
    CONFIG_STR="${CONFIG_STR} disable_libpng"
fi
if echo '#include <jbig.h>' | cpp -x c++ 2>/dev/null >/dev/null ; then
    CONFIG_STR="${CONFIG_STR} system_jbigkit"
else
    CONFIG_STR="${CONFIG_STR} disable_jbigkit"
fi
if echo '#include <Lerc_c_api.h>' | cpp -x c++ 2>/dev/null >/dev/null ; then
    CONFIG_STR="${CONFIG_STR} system_lerc"
else
    CONFIG_STR="${CONFIG_STR} disable_lerc"
fi
if pkg-config libtiff-4 ; then
    CONFIG_STR="${CONFIG_STR} system_libtiff"
else
    CONFIG_STR="${CONFIG_STR} disable_libtiff"
fi
if pkg-config libwebp libwebpdemux libwebpmux libsharpyuv ; then
    CONFIG_STR="${CONFIG_STR} system_libwebp"
else
    CONFIG_STR="${CONFIG_STR} disable_libwebp"
fi
if echo -e '#include <libwmf/api.h>\n#include <libwmf/gd.h>' | cpp -I "/usr/include/freetype2" -x c++ 2>/dev/null >/dev/null ; then
    CONFIG_STR="${CONFIG_STR} system_libwmf"
else
    CONFIG_STR="${CONFIG_STR} disable_libwmf"
fi
if pkg-config libopenjp2 ; then
    CONFIG_STR="${CONFIG_STR} system_openjpeg"
else
    CONFIG_STR="${CONFIG_STR} disable_openjpeg"
fi
if echo '#include <gif_lib.h>' | cpp -x c++ 2>/dev/null >/dev/null ; then
    CONFIG_STR="${CONFIG_STR} system_giflib"
else
    CONFIG_STR="${CONFIG_STR} disable_giflib"
fi
if pkg-config libraw ; then
    CONFIG_STR="${CONFIG_STR} system_libraw"
else
    CONFIG_STR="${CONFIG_STR} disable_libraw"
fi
if pkg-config librsvg-2.0 ; then
    CONFIG_STR="${CONFIG_STR} system_librsvg"
else
    CONFIG_STR="${CONFIG_STR} disable_librsvg"
fi
if echo '#include <resvg.h>' | cpp -x c++ 2>/dev/null >/dev/null ; then
    CONFIG_STR="${CONFIG_STR} system_resvg"
else
    CONFIG_STR="${CONFIG_STR} disable_resvg"
fi
if pkg-config libheif ; then
    CONFIG_STR="${CONFIG_STR} system_libheif"
else
    CONFIG_STR="${CONFIG_STR} disable_libheif"
fi
if pkg-config OpenEXR IlmBase ; then
    CONFIG_STR="${CONFIG_STR} system_openexr"
else
    CONFIG_STR="${CONFIG_STR} disable_openexr"
fi
if pkg-config libavif ; then
    CONFIG_STR="${CONFIG_STR} system_libavif"
else
    CONFIG_STR="${CONFIG_STR} disable_libavif"
fi
if echo '#include <JXRGlue.h>' | cpp -I "/usr/include/jxrlib" -x c++ 2>/dev/null >/dev/null ; then
    CONFIG_STR="${CONFIG_STR} system_jxrlib"
else
    CONFIG_STR="${CONFIG_STR} disable_jxrlib"
fi
if pkg-config libjxl ; then
    CONFIG_STR="${CONFIG_STR} system_libjxl"
else
    CONFIG_STR="${CONFIG_STR} disable_libjxl"
fi
echo "Config: ${CONFIG_STR}"
echo

cd "$(dirname $0)"/..
rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"
${CMD_QMAKE} -r CONFIG+="${CONFIG_STR}" CONFIG+="enable_update_checking" INCLUDEPATH+="/usr/include/freetype2" INCLUDEPATH+="/usr/include/jxrlib" "../${PROJECT}.pro"
make -j$(getconf _NPROCESSORS_ONLN)
strip --strip-all "${APP_PATH}/${PROJECT}"

if type "dpkg-buildpackage" &> /dev/null ; then
    rm -rf "AppDir"
    mkdir -p "AppDir/usr/bin" "AppDir/usr/share/applications" "AppDir/usr/share/icons/hicolor/scalable/apps" "AppDir/usr/share/ImageViewer"
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
    find "AppDir" -type d -exec chmod 755 \{\} \;
    find "AppDir" -type f -exec chmod 644 \{\} \;
    find "AppDir" -type f \( -name "${PROJECT}" -o -name "AppRun" -o -name "*.so*" -o -name "*.sh" -o -name "*.desktop" \) -exec chmod 755 \{\} \;
    cd "AppDir"
    cp -a "../../${DEBIAN_DIR_PATH}" ./
    sed -i 's|^opt/ImageViewer$|usr/bin|' "debian/dirs"
    sed -i -n '1,/^\tcp -a usr\/\*/p;/^\tdh_fixperms/,$p' "debian/rules"
    sed -i -n '1,/^\tdh_shlibdeps/p;/^\tdh_gencontrol/,$p' "debian/rules"
    sed -i 's|^\(\tdh_shlibdeps\).*$|\1|' "debian/rules"
    sed -i 's|^\(\tcp -a usr/\* debian/imageviewer\).*$|\1/usr/|' "debian/rules"
    find "debian" -type d -exec chmod 755 \{\} \;
    find "debian" -type f -exec chmod 644 \{\} \;
    chmod 755 "debian/rules"
    dpkg-buildpackage -rfakeroot -b -uc
    cd ..
    cp -a *.deb ../
fi

cp -a "${APP_PATH}/${PROJECT}" ../"${PROJECT}.elf"
cd ..
gzip -9v "${PROJECT}.elf"
