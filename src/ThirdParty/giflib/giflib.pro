# URL: https://sourceforge.net/projects/giflib/files/
# License: MIT License - https://sourceforge.net/p/giflib/code/ci/master/tree/COPYING

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_giflib

QT -= core gui

CONFIG -= warn_on
#CONFIG += hide_symbols
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_GIFLIB_PATH = $${PWD}/giflib-5.2.2
THIRDPARTY_GIFLIB_CONFIG_PATH = $${PWD}/config

include(../../Features.pri)
include(../CommonSettings.pri)

INCLUDEPATH = $${THIRDPARTY_GIFLIB_PATH} $${INCLUDEPATH}
win32: INCLUDEPATH += $${THIRDPARTY_GIFLIB_CONFIG_PATH}/win

SOURCES += \
    $${THIRDPARTY_GIFLIB_PATH}/dgif_lib.c \
    $${THIRDPARTY_GIFLIB_PATH}/egif_lib.c \
    $${THIRDPARTY_GIFLIB_PATH}/gifalloc.c \
    $${THIRDPARTY_GIFLIB_PATH}/gif_err.c \
    $${THIRDPARTY_GIFLIB_PATH}/gif_font.c \
    $${THIRDPARTY_GIFLIB_PATH}/gif_hash.c \
    $${THIRDPARTY_GIFLIB_PATH}/openbsd-reallocarray.c \
    $${THIRDPARTY_GIFLIB_PATH}/quantize.c \

HEADERS += \
    $${THIRDPARTY_GIFLIB_PATH}/gif_hash.h \
    $${THIRDPARTY_GIFLIB_PATH}/gif_lib.h \
    $${THIRDPARTY_GIFLIB_PATH}/gif_lib_private.h \

TR_EXCLUDE += $${THIRDPARTY_GIFLIB_PATH}/* $${THIRDPARTY_GIFLIB_CONFIG_PATH}/*

