# URL: https://github.com/strukturag/libheif
# License: GNU LGPL v3 - https://github.com/strukturag/libheif/blob/master/COPYING

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_libheif

QT -= core gui

CONFIG -= warn_on
CONFIG += warn_off

THIRDPARTY_LIBHEIF_PATH = $${PWD}/libheif-1.14.1

include(../CommonSettings.pri)
include(../aom/aom.pri)
include(../libde265/libde265.pri)

INCLUDEPATH = $${THIRDPARTY_LIBHEIF_PATH} $${INCLUDEPATH}

DEFINES += LIBHEIF_STATIC_BUILD
DEFINES += HAVE_INTTYPES_H HAVE_LIBDE265=1 HAVE_STDDEF_H
!*msvc*: DEFINES += HAVE_UNISTD_H

# find ./libheif -name '*.cc' | egrep -v '(_fuzzer|_aom|_dav1d|_rav1e|_svt|_x265)' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_LIBHEIF_PATH}| ; s|$| \\|'
SOURCES += \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/bitstream.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/box.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/error.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/exif.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_avif.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_colorconversion.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_context.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_file.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_hevc.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_image.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_init.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_plugin.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_plugin_registry.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/metadata_compression.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/nclx.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/heif_decoder_libde265.cc \

# find ./libheif -name '*.cc' | egrep '(_aom)' | LANG=C sort | sed 's|^\.|        $${THIRDPARTY_LIBHEIF_PATH}| ; s|$| \\|'
!disable_aom {
    SOURCES += \
        $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/heif_decoder_aom.cc \
        $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/heif_encoder_aom.cc \

    DEFINES += HAVE_AOM=1 HAVE_AOM_DECODER=1 HAVE_AOM_ENCODER=1
}

# find ./libheif -name '*.h' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_LIBHEIF_PATH}| ; s|$| \\|'
HEADERS += \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/bitstream.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/box.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/error.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/exif.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_api_structs.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_avif.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_colorconversion.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_context.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_cxx.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_emscripten.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_file.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_hevc.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_image.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_init.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_limits.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_plugin.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_plugin_registry.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/heif_version.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/logging.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/metadata_compression.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/nclx.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/heif_decoder_aom.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/heif_decoder_dav1d.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/heif_decoder_libde265.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/heif_encoder_aom.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/heif_encoder_rav1e.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/heif_encoder_svt.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/heif_encoder_x265.h \

TR_EXCLUDE += $${THIRDPARTY_LIBHEIF_PATH}/*

