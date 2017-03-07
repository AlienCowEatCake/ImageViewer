# https://github.com/nothings/stb

TEMPLATE = lib
CONFIG += staticlib
TARGET = STB

THIRDPARTY_STB_PATH = $${PWD}/stb

CONFIG += object_with_source object_parallel_to_source no_batch

*g++*|*clang* {
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE *= -O3
    QMAKE_CXXFLAGS_RELEASE *= -DNDEBUG
    QMAKE_CXXFLAGS_RELEASE *= -DQT_NO_DEBUG_OUTPUT
    QMAKE_CXXFLAGS += -Wno-unused-parameter -w
}

*msvc* {
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE *= -Ox
    QMAKE_CXXFLAGS_RELEASE -= -GS
    QMAKE_CXXFLAGS_RELEASE *= -GS-
    QMAKE_CXXFLAGS_RELEASE *= -DQT_NO_DEBUG_OUTPUT
    DEFINES += _CRT_SECURE_NO_WARNINGS
    DEFINES += _CRT_SECURE_NO_DEPRECATE
    QMAKE_CXXFLAGS += -w44100
}

HEADERS += \
    $$files($${THIRDPARTY_STB_PATH}/*.h)

SOURCES += \
    $$files($${THIRDPARTY_STB_PATH}/*.cpp)

TR_EXCLUDE += $${THIRDPARTY_STB_PATH}/*

DESTDIR = .
OBJECTS_DIR = build/objects
MOC_DIR = build/moc
RCC_DIR = build/rcc
UI_DIR = build/ui

