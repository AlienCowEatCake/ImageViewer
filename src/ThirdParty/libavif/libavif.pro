# URL: https://github.com/AOMediaCodec/libavif
# License: 2-Clause BSD License - https://github.com/AOMediaCodec/libavif/blob/master/LICENSE

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_libavif

QT -= gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_LIBAVIF_PATH = $${PWD}/libavif-1.1.1

include(../../Features.pri)
include(../CommonSettings.pri)
include(../aom/aom.pri)

*g++*|*clang*|*llvm*|*xcode* {
    QMAKE_CFLAGS += -std=gnu99
}

INCLUDEPATH = $${THIRDPARTY_LIBAVIF_PATH}/include $${THIRDPARTY_LIBAVIF_PATH}/third_party/libyuv/include $${THIRDPARTY_LIBAVIF_PATH} $${INCLUDEPATH}

DEFINES += AVIF_CODEC_AOM=1 AVIF_CODEC_AOM_ENCODE=1 AVIF_CODEC_AOM_DECODE=1
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
\#    $${THIRDPARTY_LIBAVIF_PATH}/src/gainmap.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/io.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/mem.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/obu.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/rawdata.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/read.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/reformat.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/reformat_libsharpyuv.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/reformat_libyuv.c \
\#    $${THIRDPARTY_LIBAVIF_PATH}/src/sampletransform.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/scale.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/stream.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/utils.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/write.c \

# find ./include -name '*.h' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_LIBAVIF_PATH}| ; s|$| \\|'
HEADERS += \
    $${THIRDPARTY_LIBAVIF_PATH}/include/avif/avif.h \
    $${THIRDPARTY_LIBAVIF_PATH}/include/avif/avif_cxx.h \
    $${THIRDPARTY_LIBAVIF_PATH}/include/avif/internal.h \

# find ./third_party/libyuv -name '*.c' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_LIBAVIF_PATH}| ; s|$| \\|'
SOURCES += \
    $${THIRDPARTY_LIBAVIF_PATH}/third_party/libyuv/source/planar_functions.c \
    $${THIRDPARTY_LIBAVIF_PATH}/third_party/libyuv/source/row_common.c \
\#    $${THIRDPARTY_LIBAVIF_PATH}/third_party/libyuv/source/scale.c \
    $${THIRDPARTY_LIBAVIF_PATH}/third_party/libyuv/source/scale_any.c \
    $${THIRDPARTY_LIBAVIF_PATH}/third_party/libyuv/source/scale_common.c \

# @note src/scale.c and third_party/libyuv/source/scale.c has same name and can't be in one target :(
SOURCES += \
    $${PWD}/workarounds/libyuv_scale.c

# find ./third_party/libyuv -name '*.h' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_LIBAVIF_PATH}| ; s|$| \\|'
HEADERS += \
    $${THIRDPARTY_LIBAVIF_PATH}/third_party/libyuv/include/libyuv.h \
    $${THIRDPARTY_LIBAVIF_PATH}/third_party/libyuv/include/libyuv/basic_types.h \
    $${THIRDPARTY_LIBAVIF_PATH}/third_party/libyuv/include/libyuv/planar_functions.h \
    $${THIRDPARTY_LIBAVIF_PATH}/third_party/libyuv/include/libyuv/row.h \
    $${THIRDPARTY_LIBAVIF_PATH}/third_party/libyuv/include/libyuv/scale.h \
    $${THIRDPARTY_LIBAVIF_PATH}/third_party/libyuv/include/libyuv/scale_row.h \
    $${THIRDPARTY_LIBAVIF_PATH}/third_party/libyuv/include/libyuv/version.h \

TR_EXCLUDE += $${THIRDPARTY_LIBAVIF_PATH}/*

