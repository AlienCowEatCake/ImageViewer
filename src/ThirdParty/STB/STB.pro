# URL: https://github.com/nothings/stb
# License: public domain or MIT

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_STB

THIRDPARTY_STB_PATH = $${PWD}/stb

QT -= core gui

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
    DEFINES += _CRT_SECURE_NO_WARNINGS
    DEFINES += _CRT_SECURE_NO_DEPRECATE
}

macx {
    QMAKE_CXXFLAGS += -Wno-invalid-constexpr
}

HEADERS += \
    $$files($${THIRDPARTY_STB_PATH}/*.h)

SOURCES += \
    $$files($${THIRDPARTY_STB_PATH}/*.cpp)

TR_EXCLUDE += $${THIRDPARTY_STB_PATH}/*

