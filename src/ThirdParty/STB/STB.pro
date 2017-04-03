# URL: https://github.com/nothings/stb
# License: public domain or MIT

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_STB

QT -= core gui

CONFIG -= warn_on
CONFIG += exceptions_off warn_off

THIRDPARTY_STB_PATH = $${PWD}/stb

include(../CommonSettings.pri)

HEADERS += \
    $$files($${THIRDPARTY_STB_PATH}/*.h)

SOURCES += \
    $$files($${THIRDPARTY_STB_PATH}/*.cpp)

TR_EXCLUDE += $${THIRDPARTY_STB_PATH}/*

