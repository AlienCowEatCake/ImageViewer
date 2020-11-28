# URL: https://github.com/AOMediaCodec/libavif + https://aomedia.googlesource.com/aom/
# License: 2-clause BSD License - https://github.com/AOMediaCodec/libavif/blob/master/LICENSE

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_libavif

QT -= gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_LIBAVIF_PATH = $${PWD}/libavif-0.8.4

include(../CommonSettings.pri)
include(../aom/aom.pri)

*g++*|*clang* {
    QMAKE_CFLAGS += -std=gnu99
}

INCLUDEPATH = $${THIRDPARTY_LIBAVIF_PATH}/include $${INCLUDEPATH}

DEFINES += AVIF_CODEC_AOM=1 AVIF_CODEC_AOM_ENCODE=1 AVIF_CODEC_AOM_DECODE=1

SOURCES += \
    $${THIRDPARTY_LIBAVIF_PATH}/src/alpha.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/avif.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/codec_aom.c \
\#    $${THIRDPARTY_LIBAVIF_PATH}/src/codec_dav1d.c \
\#    $${THIRDPARTY_LIBAVIF_PATH}/src/codec_libgav1.c \
\#    $${THIRDPARTY_LIBAVIF_PATH}/src/codec_rav1e.c \
\#    $${THIRDPARTY_LIBAVIF_PATH}/src/codec_svt.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/colr.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/io.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/mem.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/obu.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/rawdata.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/read.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/reformat.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/reformat_libyuv.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/stream.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/utils.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/write.c \

HEADERS += \
    $${THIRDPARTY_LIBAVIF_PATH}/include/avif/avif.h \
    $${THIRDPARTY_LIBAVIF_PATH}/include/avif/internal.h \

TR_EXCLUDE += $${THIRDPARTY_LIBAVIF_PATH}/*

