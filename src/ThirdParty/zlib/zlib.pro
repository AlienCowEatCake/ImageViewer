# URL: http://www.zlib.net/
# License: http://www.zlib.net/zlib_license.html

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_zlib

QT -= core gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_ZLIB_PATH = $${PWD}/zlib-1.2.11

INCLUDEPATH = $${THIRDPARTY_ZLIB_PATH} $${INCLUDEPATH}

DEFINES += Z_PREFIX
DEFINES += z_errmsg=tp_z_errmsg

SOURCES += \
    $${THIRDPARTY_ZLIB_PATH}/adler32.c \
    $${THIRDPARTY_ZLIB_PATH}/compress.c \
    $${THIRDPARTY_ZLIB_PATH}/crc32.c \
    $${THIRDPARTY_ZLIB_PATH}/deflate.c \
    $${THIRDPARTY_ZLIB_PATH}/gzclose.c \
    $${THIRDPARTY_ZLIB_PATH}/gzlib.c \
    $${THIRDPARTY_ZLIB_PATH}/gzread.c \
    $${THIRDPARTY_ZLIB_PATH}/gzwrite.c \
    $${THIRDPARTY_ZLIB_PATH}/infback.c \
    $${THIRDPARTY_ZLIB_PATH}/inffast.c \
    $${THIRDPARTY_ZLIB_PATH}/inflate.c \
    $${THIRDPARTY_ZLIB_PATH}/inftrees.c \
    $${THIRDPARTY_ZLIB_PATH}/trees.c \
    $${THIRDPARTY_ZLIB_PATH}/uncompr.c \
    $${THIRDPARTY_ZLIB_PATH}/zutil.c

TR_EXCLUDE += $${THIRDPARTY_ZLIB_PATH}/*

