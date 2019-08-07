# URL: https://sourceforge.net/projects/giflib/files/
# License: MIT License - https://sourceforge.net/p/giflib/code/ci/master/tree/COPYING

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_giflib

QT -= core gui

CONFIG -= warn_on
#CONFIG += hide_symbols
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_GIFLIB_PATH = $${PWD}/giflib-5.1.4
THIRDPARTY_GIFLIB_CONFIG_PATH = $${PWD}/config

include(../CommonSettings.pri)

INCLUDEPATH = $${THIRDPARTY_GIFLIB_PATH}/lib $${INCLUDEPATH}
win32: INCLUDEPATH += $${THIRDPARTY_GIFLIB_CONFIG_PATH}/win

SOURCES += \
    $${THIRDPARTY_GIFLIB_PATH}/lib/dgif_lib.c \
    $${THIRDPARTY_GIFLIB_PATH}/lib/egif_lib.c \
    $${THIRDPARTY_GIFLIB_PATH}/lib/gifalloc.c \
    $${THIRDPARTY_GIFLIB_PATH}/lib/gif_err.c \
    $${THIRDPARTY_GIFLIB_PATH}/lib/gif_font.c \
    $${THIRDPARTY_GIFLIB_PATH}/lib/gif_hash.c \
    $${THIRDPARTY_GIFLIB_PATH}/lib/openbsd-reallocarray.c \
    $${THIRDPARTY_GIFLIB_PATH}/lib/quantize.c \

HEADERS += \
    $${THIRDPARTY_GIFLIB_PATH}/lib/gif_hash.h \
    $${THIRDPARTY_GIFLIB_PATH}/lib/gif_lib.h \
    $${THIRDPARTY_GIFLIB_PATH}/lib/gif_lib_private.h \

TR_EXCLUDE += $${THIRDPARTY_GIFLIB_PATH}/*

