# URL: https://www.freetype.org/
# License: FreeType License / GNU GPL v2

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_freetype

QT -= core gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_FREETYPE_PATH = $${PWD}/freetype-2.8
THIRDPARTY_FREETYPE_CONFIG_PATH = $${PWD}/config

include(../CommonSettings.pri)
include(../libpng/libpng.pri)
include(../zlib/zlib.pri)

INCLUDEPATH = $${THIRDPARTY_FREETYPE_CONFIG_PATH} $${THIRDPARTY_FREETYPE_PATH}/include $${INCLUDEPATH}

DEFINES += FT_PREFIX

DEFINES += FT2_BUILD_LIBRARY
DEFINES += FT_CONFIG_OPTION_SYSTEM_ZLIB
DEFINES += FT_CONFIG_OPTION_USE_PNG
DEFINES += TT_CONFIG_OPTION_SUBPIXEL_HINTING
DEFINES += FT_CONFIG_OPTION_NO_ASSEMBLER

SOURCES += \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/afangles.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/afdummy.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/afglobal.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/afhints.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/aflatin.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/afloader.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/afmodule.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/autofit.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/ftbase.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/ftbitmap.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/ftbbox.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/ftdebug.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/ftglyph.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/ftfntfmt.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/ftinit.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/ftlcdfil.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/ftmm.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/ftsynth.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/ftsystem.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/fttype1.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/bdf/bdf.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/cache/ftcache.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cff.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/cid/type1cid.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/gzip/ftgzip.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/lzw/ftlzw.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/otvalid/otvalid.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/otvalid/otvbase.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/otvalid/otvcommn.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/otvalid/otvgdef.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/otvalid/otvgpos.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/otvalid/otvgsub.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/otvalid/otvjstf.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/otvalid/otvmod.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/pcf/pcf.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/pfr/pfr.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/psaux/psaux.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/pshinter/pshinter.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/psnames/psmodule.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/raster/raster.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/sfnt/sfnt.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/smooth/smooth.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/truetype/truetype.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/type1/type1.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/type42/type42.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/winfonts/winfnt.c

HEADERS += \
# TODO

TR_EXCLUDE += $${THIRDPARTY_FREETYPE_PATH}/*

