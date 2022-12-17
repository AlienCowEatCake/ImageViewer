# URL: http://wvware.sourceforge.net/libwmf.html
# License: GNU GPL v2 - https://www.gnu.org/licenses/old-licenses/gpl-2.0.html

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_libwmf

QT += core gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_LIBWMF_PATH = $${PWD}/libwmf-0.2.8.4
THIRDPARTY_LIBWMF_CONFIG_PATH = $${PWD}/config

include(../CommonSettings.pri)
include(../FreeType/FreeType.pri)
include(../libjpeg/libjpeg.pri)
include(../libpng/libpng.pri)
include(../zlib/zlib.pri)
include(../libexpat/libexpat.pri)

INCLUDEPATH = \
    $${THIRDPARTY_LIBWMF_CONFIG_PATH} \
    $${THIRDPARTY_LIBWMF_PATH}/include \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa \
    $${THIRDPARTY_LIBWMF_PATH}/src \
    $${INCLUDEPATH}

DEFINES += HAVE_CONFIG_H
DEFINES += HAVE_LIBFREETYPE
DEFINES += HAVE_LIBPNG
DEFINES += HAVE_LIBJPEG
win32 {
    DEFINES += MSWIN32
} else {
    DEFINES += HAVE_UNISTD_H
}
!disable_libexpat {
    DEFINES += HAVE_EXPAT
}

SOURCES += \
    $${THIRDPARTY_LIBWMF_CONFIG_PATH}/fontsprovider/fontsprovider.cpp

HEADERS += \
    $${THIRDPARTY_LIBWMF_CONFIG_PATH}/fontsprovider/fontsprovider.h

RESOURCES += \
    $${THIRDPARTY_LIBWMF_CONFIG_PATH}/fontsprovider/resources/libwmf_fontsprovider.qrc

QMAKE_RESOURCE_FLAGS += -threshold 0 -compress 9

SOURCES += \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gd.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gd_gd.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gd_gd2.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gd_io.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gd_io_dp.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gd_io_file.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gd_ss.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gd_io_ss.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gd_png.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gd_jpeg.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gdxpm.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gdfontt.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gdfonts.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gdfontmb.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gdfontl.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gdfontg.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gdtables.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gdft.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gdcache.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gdkanji.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/wbmp.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gd_wbmp.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gdhelpers.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gd_topal.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gd_clip.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/eps.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/fig.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/svg.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/xgd.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/x.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/ipa.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/plot.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/foreign.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/api.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/bbuf.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/meta.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/player.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/recorder.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/font.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/stream.c \
    $${THIRDPARTY_LIBWMF_PATH}/src/wmf.c \
\ #    $${THIRDPARTY_LIBWMF_PATH}/src/xml.c

HEADERS += \
    $${THIRDPARTY_LIBWMF_PATH}/include/libwmf/api.h \
    $${THIRDPARTY_LIBWMF_PATH}/include/libwmf/color.h \
    $${THIRDPARTY_LIBWMF_PATH}/include/libwmf/defs.h \
    $${THIRDPARTY_LIBWMF_PATH}/include/libwmf/fund.h \
    $${THIRDPARTY_LIBWMF_PATH}/include/libwmf/ipa.h \
    $${THIRDPARTY_LIBWMF_PATH}/include/libwmf/types.h \
    $${THIRDPARTY_LIBWMF_PATH}/include/libwmf/macro.h \
    $${THIRDPARTY_LIBWMF_PATH}/include/libwmf/font.h \
    $${THIRDPARTY_LIBWMF_PATH}/include/libwmf/canvas.h \
    $${THIRDPARTY_LIBWMF_PATH}/include/libwmf/foreign.h \
    $${THIRDPARTY_LIBWMF_PATH}/include/libwmf/eps.h \
    $${THIRDPARTY_LIBWMF_PATH}/include/libwmf/fig.h \
    $${THIRDPARTY_LIBWMF_PATH}/include/libwmf/svg.h \
    $${THIRDPARTY_LIBWMF_PATH}/include/libwmf/gd.h \
    $${THIRDPARTY_LIBWMF_PATH}/include/libwmf/x.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/trio/trio.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/trio/triop.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/trio/strio.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gd.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gdcache.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gd_clip.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gd_io.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gdfontg.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gdfontl.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gdfontmb.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gdfonts.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/extra/gd/gdfontt.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/ipa/bmp.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/ipa.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/eps/bmp.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/eps/device.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/eps/draw.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/eps/region.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/eps.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/fig/bmp.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/fig/color.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/fig/device.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/fig/draw.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/fig/font.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/fig/region.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/fig.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/svg/bmp.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/svg/device.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/svg/draw.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/svg/region.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/svg.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/xgd/bmp.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/xgd/device.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/xgd/draw.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/xgd/font.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/xgd/region.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/xgd.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/x/bmp.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/x/color.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/x/device.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/x/draw.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/x/font.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/x/region.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/x.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/plot/bmp.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/plot/device.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/plot/draw.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/plot/region.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/ipa/plot.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/player/clip.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/player/color.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/player/coord.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/player/dc.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/player/defaults.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/player/meta.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/player/record.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/player/region.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/wmfdefs.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/metadefs.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/player.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/recorder.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/api.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/bbuf.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/font.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/stream.h \
    $${THIRDPARTY_LIBWMF_PATH}/src/xml.h \
    $${THIRDPARTY_LIBWMF_CONFIG_PATH}/wmfconfig.h

TR_EXCLUDE += $${THIRDPARTY_LIBWMF_PATH}/* $${THIRDPARTY_LIBWMF_CONFIG_PATH}/*

