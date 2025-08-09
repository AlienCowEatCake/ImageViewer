# URL: https://github.com/AOMediaCodec/libavif
# License: 2-Clause BSD License - https://github.com/AOMediaCodec/libavif/blob/master/LICENSE

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_libavif

QT -= gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_LIBAVIF_PATH = $${PWD}/libavif-1.3.0

include(../../Features.pri)
include(../CommonSettings.pri)
include(../aom/aom.pri)
include(../libyuv/libyuv.pri)
include(../LibWebP/LibWebP.pri)

*g++*|*clang*|*llvm*|*xcode* {
    QMAKE_CFLAGS += -std=gnu99
}

INCLUDEPATH = $${THIRDPARTY_LIBAVIF_PATH}/include $${THIRDPARTY_LIBAVIF_PATH} $${INCLUDEPATH}

DEFINES += AVIF_CODEC_AOM=1 AVIF_CODEC_AOM_ENCODE=1 AVIF_CODEC_AOM_DECODE=1 AVIF_LIBYUV_ENABLED=1
DEFINES += AVIF_ENABLE_EXPERIMENTAL_EXTENDED_PIXI AVIF_ENABLE_EXPERIMENTAL_SAMPLE_TRANSFORM
!disable_libwebp: DEFINES += AVIF_LIBSHARPYUV_ENABLED=1
*msvc*: DEFINES += inline=__inline

# find ./src -name '*.c' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_LIBAVIF_PATH}| ; s|$| \\|'
SOURCES += \
    $${THIRDPARTY_LIBAVIF_PATH}/src/alpha.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/avif.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/codec_aom.c \
\#    $${THIRDPARTY_LIBAVIF_PATH}/src/codec_avm.c \
\#    $${THIRDPARTY_LIBAVIF_PATH}/src/codec_dav1d.c \
\#    $${THIRDPARTY_LIBAVIF_PATH}/src/codec_libgav1.c \
\#    $${THIRDPARTY_LIBAVIF_PATH}/src/codec_rav1e.c \
\#    $${THIRDPARTY_LIBAVIF_PATH}/src/codec_svt.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/colr.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/colrconvert.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/diag.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/exif.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/gainmap.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/io.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/mem.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/obu.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/properties.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/rawdata.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/read.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/reformat.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/reformat_libsharpyuv.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/reformat_libyuv.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/sampletransform.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/scale.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/stream.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/utils.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/write.c \

# find ./include -name '*.h' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_LIBAVIF_PATH}| ; s|$| \\|'
HEADERS += \
    $${THIRDPARTY_LIBAVIF_PATH}/include/avif/avif.h \
    $${THIRDPARTY_LIBAVIF_PATH}/include/avif/avif_cxx.h \
    $${THIRDPARTY_LIBAVIF_PATH}/include/avif/internal.h \

TR_EXCLUDE += $${THIRDPARTY_LIBAVIF_PATH}/*

