# URL: https://sourceforge.net/projects/libmng/
# License: zlib/libpng License

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_libmng

QT -= core gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_LIBMNG_PATH = $${PWD}/libmng-1.0.10

include(../CommonSettings.pri)
include(../libjpeg/libjpeg.pri)
include(../LittleCMS2/LittleCMS2.pri)
include(../zlib/zlib.pri)

INCLUDEPATH = $${THIRDPARTY_LIBMNG_PATH} $${INCLUDEPATH}

DEFINES += MNG_BUILD_SO
#DEFINES += MNG_INCLUDE_MPNG_PROPOSAL
DEFINES += MNG_INCLUDE_ANG_PROPOSAL

disable_libjpeg {
    DEFINES += MNG_NO_INCLUDE_JNG
}

!disable_liblcms2 {
    DEFINES += MNG_FULL_CMS
}

SOURCES += \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_callback_xs.c \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_chunk_descr.c \
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
    $${THIRDPARTY_LIBMNG_PATH}/libmng_trace.c \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_write.c \
    $${THIRDPARTY_LIBMNG_PATH}/libmng_zlib.c

TR_EXCLUDE += $${THIRDPARTY_LIBMNG_PATH}/*

