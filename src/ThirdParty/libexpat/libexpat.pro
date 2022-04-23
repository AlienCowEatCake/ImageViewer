# URL: https://github.com/libexpat/libexpat
# License: MIT License - https://github.com/libexpat/libexpat/blob/master/expat/COPYING

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_libexpat

#QT -= core gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_LIBEXPAT_PATH = $${PWD}/expat-2.4.8
THIRDPARTY_LIBEXPAT_CONFIG_PATH = $${PWD}/config

include(../CommonSettings.pri)
include(../zlib/zlib.pri)

INCLUDEPATH = $${THIRDPARTY_LIBEXPAT_CONFIG_PATH} $${THIRDPARTY_LIBEXPAT_PATH}/lib $${INCLUDEPATH}

DEFINES += XML_POOR_ENTROPY XML_STATIC

# find ./lib -name '*.c' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_LIBEXPAT_PATH}| ; s|$| \\|'
SOURCES += \
    $${THIRDPARTY_LIBEXPAT_PATH}/lib/xmlparse.c \
    $${THIRDPARTY_LIBEXPAT_PATH}/lib/xmlrole.c \
    $${THIRDPARTY_LIBEXPAT_PATH}/lib/xmltok.c \
    $${THIRDPARTY_LIBEXPAT_PATH}/lib/xmltok_impl.c \
    $${THIRDPARTY_LIBEXPAT_PATH}/lib/xmltok_ns.c \

# find ./lib -name '*.h' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_LIBEXPAT_PATH}| ; s|$| \\|'
HEADERS += \
    $${THIRDPARTY_LIBEXPAT_PATH}/lib/ascii.h \
    $${THIRDPARTY_LIBEXPAT_PATH}/lib/asciitab.h \
    $${THIRDPARTY_LIBEXPAT_PATH}/lib/expat.h \
    $${THIRDPARTY_LIBEXPAT_PATH}/lib/expat_external.h \
    $${THIRDPARTY_LIBEXPAT_PATH}/lib/iasciitab.h \
    $${THIRDPARTY_LIBEXPAT_PATH}/lib/internal.h \
    $${THIRDPARTY_LIBEXPAT_PATH}/lib/latin1tab.h \
    $${THIRDPARTY_LIBEXPAT_PATH}/lib/nametab.h \
    $${THIRDPARTY_LIBEXPAT_PATH}/lib/siphash.h \
    $${THIRDPARTY_LIBEXPAT_PATH}/lib/utf8tab.h \
    $${THIRDPARTY_LIBEXPAT_PATH}/lib/winconfig.h \
    $${THIRDPARTY_LIBEXPAT_PATH}/lib/xmlrole.h \
    $${THIRDPARTY_LIBEXPAT_PATH}/lib/xmltok.h \
    $${THIRDPARTY_LIBEXPAT_PATH}/lib/xmltok_impl.h \
    $${THIRDPARTY_LIBEXPAT_CONFIG_PATH}/expat_config.h

TR_EXCLUDE += $${THIRDPARTY_LIBEXPAT_PATH}/* $${THIRDPARTY_LIBEXPAT_CONFIG_PATH}/*

