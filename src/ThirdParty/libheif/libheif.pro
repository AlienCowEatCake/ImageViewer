# URL: https://github.com/strukturag/libheif
# License: GNU LGPL v3 - https://github.com/strukturag/libheif/blob/master/COPYING

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_libheif

QT -= core gui

CONFIG -= warn_on
CONFIG += warn_off

THIRDPARTY_LIBHEIF_PATH = $${PWD}/libheif-1.5.1
THIRDPARTY_LIBHEIF_CONFIG_PATH = $${PWD}/config

include(../CommonSettings.pri)
include(../libde265/libde265.pri)

INCLUDEPATH = $${THIRDPARTY_LIBHEIF_CONFIG_PATH} $${THIRDPARTY_LIBHEIF_PATH} $${INCLUDEPATH}

DEFINES += HAVE_CONFIG_H LIBHEIF_STATIC_BUILD

SOURCES += \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/bitstream.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/box.cc \
\ #    $${THIRDPARTY_LIBHEIF_PATH}/libheif/box_fuzzer.cc \
\ #    $${THIRDPARTY_LIBHEIF_PATH}/libheif/encoder_fuzzer.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/error.cc \
\ #    $${THIRDPARTY_LIBHEIF_PATH}/libheif/file_fuzzer.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_colorconversion.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_context.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_decoder_libde265.cc \
\ #    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_encoder_x265.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_file.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_hevc.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_image.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_plugin.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_plugin_registry.cc \

HEADERS += \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/bitstream.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/box.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/error.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_api_structs.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_colorconversion.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_context.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_cxx.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_decoder_libde265.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_emscripten.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_encoder_x265.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_file.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_hevc.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_image.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_limits.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_plugin.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_plugin_registry.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_version.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/logging.h \
    $${THIRDPARTY_LIBHEIF_CONFIG_PATH}/config.h

TR_EXCLUDE += $${THIRDPARTY_LIBHEIF_PATH}/*

