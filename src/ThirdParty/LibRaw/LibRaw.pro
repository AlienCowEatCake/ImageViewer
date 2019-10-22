# URL: https://www.libraw.org/download
# License: GNU LGPL v2.1 or CDDL v1.0 - https://github.com/LibRaw/LibRaw/blob/master/COPYRIGHT

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_LibRaw

#QT -= core gui

CONFIG -= warn_on
CONFIG += warn_off

THIRDPARTY_LIBRAW_PATH = $${PWD}/LibRaw-0.19.5

include(../../Features.pri)
include(../CommonSettings.pri)
include(../libjpeg/libjpeg.pri)
include(../JasPer/JasPer.pri)
include(../LittleCMS2/LittleCMS2.pri)

INCLUDEPATH = $${THIRDPARTY_LIBRAW_PATH} $${INCLUDEPATH}

DEFINES += LIBRAW_NOTHREADS

!disable_libjpeg {
    DEFINES += USE_JPEG USE_JPEG8
} else {
    DEFINES += NO_JPEG
}

!disable_liblcms2 {
    DEFINES += USE_LCMS2
} else {
    DEFINES += NO_LCMS
}

!disable_libjasper {
    DEFINES += USE_JASPER
} else {
    DEFINES += NO_JASPER
}

win32 {
    DEFINES += LIBRAW_NODLL
}

*clang* {
    DEFINES *= _LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR
}

SOURCES += \
\#    $${THIRDPARTY_LIBRAW_PATH}/dcraw/dcraw.c \
\#    $${THIRDPARTY_LIBRAW_PATH}/internal/aahd_demosaic.cpp \
\#    $${THIRDPARTY_LIBRAW_PATH}/internal/dcb_demosaicing.c \
    $${THIRDPARTY_LIBRAW_PATH}/internal/dcraw_common.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/internal/dcraw_fileio.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/internal/demosaic_packs.cpp \
\#    $${THIRDPARTY_LIBRAW_PATH}/internal/dht_demosaic.cpp \
\#    $${THIRDPARTY_LIBRAW_PATH}/internal/libraw_x3f.cpp \
\#    $${THIRDPARTY_LIBRAW_PATH}/internal/wf_filtering.cpp \
\#    $${THIRDPARTY_LIBRAW_PATH}/RawSpeed/rawspeed_xmldata.cpp \
\#    $${THIRDPARTY_LIBRAW_PATH}/samples/4channels.cpp \
\#    $${THIRDPARTY_LIBRAW_PATH}/samples/dcraw_emu.cpp \
\#    $${THIRDPARTY_LIBRAW_PATH}/samples/dcraw_half.c \
\#    $${THIRDPARTY_LIBRAW_PATH}/samples/half_mt.c \
\#    $${THIRDPARTY_LIBRAW_PATH}/samples/half_mt_win32.c \
\#    $${THIRDPARTY_LIBRAW_PATH}/samples/mem_image.cpp \
\#    $${THIRDPARTY_LIBRAW_PATH}/samples/multirender_test.cpp \
\#    $${THIRDPARTY_LIBRAW_PATH}/samples/openbayer_sample.cpp \
\#    $${THIRDPARTY_LIBRAW_PATH}/samples/postprocessing_benchmark.cpp \
\#    $${THIRDPARTY_LIBRAW_PATH}/samples/raw-identify.cpp \
\#    $${THIRDPARTY_LIBRAW_PATH}/samples/simple_dcraw.cpp \
\#    $${THIRDPARTY_LIBRAW_PATH}/samples/unprocessed_raw.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/libraw_c_api.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/libraw_cxx.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/libraw_datastream.cpp \
\#    $${THIRDPARTY_LIBRAW_PATH}/src/libraw_fuji_compressed.cpp \

HEADERS += \
    $${THIRDPARTY_LIBRAW_PATH}/internal/defines.h \
    $${THIRDPARTY_LIBRAW_PATH}/internal/libraw_internal_funcs.h \
    $${THIRDPARTY_LIBRAW_PATH}/internal/var_defines.h \
    $${THIRDPARTY_LIBRAW_PATH}/libraw_build.h \
    $${THIRDPARTY_LIBRAW_PATH}/libraw/libraw_alloc.h \
    $${THIRDPARTY_LIBRAW_PATH}/libraw/libraw_const.h \
    $${THIRDPARTY_LIBRAW_PATH}/libraw/libraw_datastream.h \
    $${THIRDPARTY_LIBRAW_PATH}/libraw/libraw.h \
    $${THIRDPARTY_LIBRAW_PATH}/libraw/libraw_internal.h \
    $${THIRDPARTY_LIBRAW_PATH}/libraw/libraw_types.h \
    $${THIRDPARTY_LIBRAW_PATH}/libraw/libraw_version.h \

TR_EXCLUDE += $${THIRDPARTY_LIBPNG_PATH}/*

