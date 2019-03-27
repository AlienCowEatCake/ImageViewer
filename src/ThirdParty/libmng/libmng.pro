# URL: https://sourceforge.net/projects/libmng/
# License: zlib/libpng License

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_libmng

QT -= core gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_LIBMNG_PATH = $${PWD}/libmng-1.0.10
THIRDPARTY_LIBMNG_CONFIG_PATH = $${PWD}/config

include(../CommonSettings.pri)
include(../libjpeg/libjpeg.pri)
include(../LittleCMS2/LittleCMS2.pri)
include(../zlib/zlib.pri)

INCLUDEPATH = $${THIRDPARTY_LIBMNG_CONFIG_PATH} $${THIRDPARTY_LIBMNG_PATH} $${INCLUDEPATH}

DEFINES += MNG_PREFIX

DEFINES += MNG_BUILD_SO

disable_libjpeg {
    DEFINES += MNG_NO_INCLUDE_JNG
}

!disable_liblcms2 {
    DEFINES += MNG_FULL_CMS
}

SOURCES += \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_callback_xs.c \
\ #    $${THIRDPARTY_LIBMNG_PATH}/libmng_chunk_descr.c \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_chunk_io.c \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_chunk_prc.c \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_chunk_xs.c \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_cms.c \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_display.c \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_dither.c \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_error.c \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_filter.c \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_hlapi.c \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_jpeg.c \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_object_prc.c \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_pixels.c \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_prop_xs.c \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_read.c \
\ #    $${THIRDPARTY_LIBMNG_PATH}/libmng_trace.c \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_write.c \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_zlib.c

HEADERS += \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_chunk_descr.h \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_chunk_io.h \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_chunk_prc.h \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_chunks.h \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_cms.h \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_conf.h \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_data.h \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_display.h \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_dither.h \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_error.h \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_filter.h \
    $${THIRDPARTY_LIBMNG_PATH}/libmng.h \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_jpeg.h \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_memory.h \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_object_prc.h \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_objects.h \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_pixels.h \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_read.h \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_trace.h \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_types.h \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_write.h \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_zlib.h \
    $${THIRDPARTY_LIBMNG_CONFIG_PATH}/mngprefix.h

TR_EXCLUDE += $${THIRDPARTY_LIBMNG_PATH}/*

