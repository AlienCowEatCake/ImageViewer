# URL: https://github.com/strukturag/libde265
# License: GNU LGPL v3 - https://github.com/strukturag/libde265/blob/master/COPYING

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_libde265

QT -= core gui

CONFIG -= warn_on
CONFIG += warn_off

THIRDPARTY_LIBDE265_PATH = $${PWD}/libde265-1.0.19
THIRDPARTY_LIBDE265_CONFIG_PATH = $${PWD}/config

include(../../Features.pri)
include(../CommonSettings.pri)

INCLUDEPATH = $${THIRDPARTY_LIBDE265_CONFIG_PATH} $${THIRDPARTY_LIBDE265_PATH} $${THIRDPARTY_LIBDE265_PATH}/libde265 $${INCLUDEPATH}

DEFINES += HAVE_CONFIG_H LIBDE265_STATIC_BUILD

win32 {
    DEFINES += NOMINMAX
}

# find ./libde265 -name '*.cc' | egrep -v '(/arm/|/arm32/|/x86/)' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_LIBDE265_PATH}| ; s|$| \\|'
SOURCES += \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/alloc_pool.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/bitstream.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/cabac.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/contextmodel.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/de265.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/deblock.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/decctx.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/dpb.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/fallback-dct.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/fallback-motion.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/fallback.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/image-io.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/image.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/intrapred.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/md5.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/motion.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/nal-parser.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/nal.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/pps.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/quality.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/refpic.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/sao.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/scan.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/sei.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/slice.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/sps.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/threads.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/transform.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/util.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/visualize.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/vps.cc \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/vui.cc \

# find ./libde265 -name '*.h' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_LIBDE265_PATH}| ; s|$| \\|'
HEADERS += \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/acceleration.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/alloc_pool.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/arm32/arm.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/bitstream.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/cabac.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/contextmodel.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/de265.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/deblock.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/decctx.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/dpb.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/fallback-dct.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/fallback-motion.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/fallback.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/image-io.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/image.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/intrapred.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/md5.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/motion.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/nal-parser.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/nal.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/pps.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/quality.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/refpic.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/sao.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/scan.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/sei.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/slice.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/sps.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/threads.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/transform.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/util.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/visualize.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/vps.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/vui.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/x86/sse-dct.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/x86/sse-motion.h \
    $${THIRDPARTY_LIBDE265_PATH}/libde265/x86/sse.h \
    $${THIRDPARTY_LIBDE265_CONFIG_PATH}/config.h \
    $${THIRDPARTY_LIBDE265_CONFIG_PATH}/libde265/de265-version.h

win32 {
    SOURCES += \
        $${THIRDPARTY_LIBDE265_PATH}/extra/win32cond.c
    HEADERS += \
        $${THIRDPARTY_LIBDE265_PATH}/extra/win32cond.h
}

TR_EXCLUDE += $${THIRDPARTY_LIBDE265_PATH}/* $${THIRDPARTY_LIBDE265_CONFIG_PATH}/*

