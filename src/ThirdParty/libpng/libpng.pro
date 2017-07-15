# URL: http://www.libpng.org/pub/png/libpng.html + https://sourceforge.net/projects/libpng-apng/
# License: http://www.libpng.org/pub/png/src/libpng-LICENSE.txt

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_libpng

QT -= core gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_LIBPNG_PATH = $${PWD}/libpng-1.6.30
THIRDPARTY_LIBPNG_CONFIG_PATH = $${PWD}/config

include(../CommonSettings.pri)
include(../zlib/zlib.pri)

INCLUDEPATH = $${THIRDPARTY_LIBPNG_CONFIG_PATH} $${THIRDPARTY_LIBPNG_PATH} $${INCLUDEPATH}

DEFINES += PNG_ARM_NEON_OPT=0

SOURCES += \
    $${THIRDPARTY_LIBPNG_PATH}/png.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngerror.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngget.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngmem.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngpread.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngread.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngrio.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngrtran.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngrutil.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngset.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngtrans.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngwio.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngwrite.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngwtran.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngwutil.c

HEADERS += \
    $${THIRDPARTY_LIBPNG_PATH}/pngconf.h \
    $${THIRDPARTY_LIBPNG_PATH}/pngdebug.h \
    $${THIRDPARTY_LIBPNG_PATH}/png.h \
    $${THIRDPARTY_LIBPNG_PATH}/pnginfo.h \
    $${THIRDPARTY_LIBPNG_PATH}/pngpriv.h \
    $${THIRDPARTY_LIBPNG_PATH}/pngstruct.h \
    $${THIRDPARTY_LIBPNG_CONFIG_PATH}/pnglibconf.h

TR_EXCLUDE += $${THIRDPARTY_LIBPNG_PATH}/*

