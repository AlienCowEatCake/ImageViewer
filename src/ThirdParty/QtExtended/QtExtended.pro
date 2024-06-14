# URL: https://sourceforge.net/projects/qpe/files/QPE/qtopia/qt-extended-opensource-src-4.4.3.tar.gz/download
# License: GNU GPL v2 - https://www.gnu.org/licenses/old-licenses/gpl-2.0.html

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_QtExtended

QT += core gui

CONFIG -= warn_on
CONFIG += exceptions_off warn_off

THIRDPARTY_QTEXTENDED_PATH = $${PWD}/src

include(../../Features.pri)
include(../CommonSettings.pri)

INCLUDEPATH += $${THIRDPARTY_QTEXTENDED_PATH}

HEADERS += \
    $$files($${THIRDPARTY_QTEXTENDED_PATH}/*.h)

SOURCES += \
    $$files($${THIRDPARTY_QTEXTENDED_PATH}/*.cpp)

