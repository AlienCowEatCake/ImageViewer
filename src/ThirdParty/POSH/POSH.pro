# URL: http://hookatooka.com/poshlib/
# License: ???

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_poshlib

QT -= core gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_POSHLIB_PATH = $${PWD}/poshlib

include(../CommonSettings.pri)

INCLUDEPATH = $${THIRDPARTY_POSHLIB_PATH} $${INCLUDEPATH}

SOURCES += \
    $${THIRDPARTY_POSHLIB_PATH}/posh.c

HEADERS += \
    $${THIRDPARTY_POSHLIB_PATH}/posh.h

TR_EXCLUDE += $${THIRDPARTY_POSHLIB_PATH}/*

