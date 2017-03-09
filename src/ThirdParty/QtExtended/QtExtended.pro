# https://sourceforge.net/projects/qpe/files/QPE/qtopia/qt-extended-opensource-src-4.4.3.tar.gz/download

TEMPLATE = lib
CONFIG += staticlib
TARGET = QtExtended

THIRDPARTY_QTEXTENDED_PATH = $${PWD}/src

INCLUDEPATH += $${THIRDPARTY_QTEXTENDED_PATH}

QT += core gui

CONFIG -= warn_on

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

HEADERS += \
    $$files($${THIRDPARTY_QTEXTENDED_PATH}/*.h)

SOURCES += \
    $$files($${THIRDPARTY_QTEXTENDED_PATH}/*.cpp)

OBJECTS_DIR = build/objects
MOC_DIR = build/moc
RCC_DIR = build/rcc
UI_DIR = build/ui

