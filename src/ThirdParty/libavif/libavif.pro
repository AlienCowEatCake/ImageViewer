# URL: https://github.com/AOMediaCodec/libavif + https://aomedia.googlesource.com/aom/
# License: 2-clause BSD License - https://github.com/AOMediaCodec/libavif/blob/master/LICENSE

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_libavif

QT -= gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_LIBAVIF_PATH = $${PWD}/libavif-0.7.3
THIRDPARTY_AOM_PATH = $${PWD}/aom-v1.0.0-errata1-avif
THIRDPARTY_AOM_CONFIG_PATH = $${PWD}/aom_config

include(../CommonSettings.pri)
include(../aom/aom.pri)

INCLUDEPATH = $${THIRDPARTY_LIBAVIF_PATH}/include $${INCLUDEPATH}

DEFINES += AVIF_CODEC_AOM=1

SOURCES += \
    $${THIRDPARTY_LIBAVIF_PATH}/src/alpha.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/avif.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/codec_aom.c \
\#    $${THIRDPARTY_LIBAVIF_PATH}/src/codec_dav1d.c \
\#    $${THIRDPARTY_LIBAVIF_PATH}/src/codec_libgav1.c \
\#    $${THIRDPARTY_LIBAVIF_PATH}/src/codec_rav1e.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/colr.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/mem.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/rawdata.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/read.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/reformat.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/stream.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/utils.c \
    $${THIRDPARTY_LIBAVIF_PATH}/src/write.c \

HEADERS += \
    $${THIRDPARTY_LIBAVIF_PATH}/include/avif/avif.h \
    $${THIRDPARTY_LIBAVIF_PATH}/include/avif/internal.h \

TR_EXCLUDE += $${THIRDPARTY_LIBAVIF_PATH}/*

