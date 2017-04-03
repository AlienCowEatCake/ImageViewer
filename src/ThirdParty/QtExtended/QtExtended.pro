# URL: https://sourceforge.net/projects/qpe/files/QPE/qtopia/qt-extended-opensource-src-4.4.3.tar.gz/download
# License: GNU General Public License version 2

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_QtExtended

QT += core gui

CONFIG -= warn_on
CONFIG += exceptions_off warn_off

THIRDPARTY_QTEXTENDED_PATH = $${PWD}/src

include(../CommonSettings.pri)

INCLUDEPATH += $${THIRDPARTY_QTEXTENDED_PATH}

HEADERS += \
    $$files($${THIRDPARTY_QTEXTENDED_PATH}/*.h)

SOURCES += \
    $$files($${THIRDPARTY_QTEXTENDED_PATH}/*.cpp)

