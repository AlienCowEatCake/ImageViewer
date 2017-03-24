# URL: https://sourceforge.net/projects/qpe/files/QPE/qtopia/qt-extended-opensource-src-4.4.3.tar.gz/download
# License: GNU General Public License version 2

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_QtExtended

THIRDPARTY_QTEXTENDED_PATH = $${PWD}/src

INCLUDEPATH += $${THIRDPARTY_QTEXTENDED_PATH}

QT += core gui

CONFIG -= warn_on
CONFIG += exceptions_off warn_off

*g++*|*clang* {
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE *= -O3
    QMAKE_CXXFLAGS_RELEASE *= -DNDEBUG
    QMAKE_CXXFLAGS_RELEASE *= -DQT_NO_DEBUG_OUTPUT
}

*msvc* {
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE *= -Ox
    QMAKE_CXXFLAGS_RELEASE -= -GS
    QMAKE_CXXFLAGS_RELEASE *= -GS-
    QMAKE_CXXFLAGS_RELEASE *= -DQT_NO_DEBUG_OUTPUT
}

macx {
    QMAKE_CXXFLAGS += -Wno-invalid-constexpr
}

HEADERS += \
    $$files($${THIRDPARTY_QTEXTENDED_PATH}/*.h)

SOURCES += \
    $$files($${THIRDPARTY_QTEXTENDED_PATH}/*.cpp)

