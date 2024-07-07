# URL: https://github.com/lifthrasiir/j40
# License: MIT No Attribution License - https://github.com/lifthrasiir/j40/blob/main/LICENSE.txt

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_J40

QT -= core gui

CONFIG -= warn_on
CONFIG += exceptions_off warn_off

THIRDPARTY_J40_PATH = $${PWD}/j40-252e798
THIRDPARTY_J40_IMPL_PATH = $${PWD}/implementation

INCLUDEPATH += $${THIRDPARTY_J40_PATH}

include(../../Features.pri)
include(../CommonSettings.pri)

HEADERS += \
    $$files($${THIRDPARTY_J40_PATH}/*.h)

SOURCES += \
    $$files($${THIRDPARTY_J40_IMPL_PATH}/*.cpp)

TR_EXCLUDE += $${THIRDPARTY_J40_PATH}/* $${THIRDPARTY_J40_IMPL_PATH}/*

