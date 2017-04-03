# URL: http://ijg.org/
# License: ???

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_libjpeg

QT -= core gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_LIBJPEG_PATH = $${PWD}/jpeg-9b
THIRDPARTY_LIBJPEG_CONFIG_PATH = $${PWD}/config

include(../CommonSettings.pri)

INCLUDEPATH = $${THIRDPARTY_LIBJPEG_PATH} $${THIRDPARTY_LIBJPEG_CONFIG_PATH} $${INCLUDEPATH}

DEFINES += jdiv_round_up=tp_jdiv_round_up
DEFINES += jround_up=tp_jround_up
DEFINES += jcopy_sample_rows=tp_jcopy_sample_rows
DEFINES += jcopy_block_row=tp_jcopy_block_row
DEFINES += jpeg_natural_order=tp_jpeg_natural_order
DEFINES += jpeg_natural_order2=tp_jpeg_natural_order2
DEFINES += jpeg_natural_order3=tp_jpeg_natural_order3
DEFINES += jpeg_natural_order4=tp_jpeg_natural_order4
DEFINES += jpeg_natural_order5=tp_jpeg_natural_order5
DEFINES += jpeg_natural_order6=tp_jpeg_natural_order6
DEFINES += jpeg_natural_order7=tp_jpeg_natural_order7

SOURCES += \
    $${THIRDPARTY_LIBJPEG_PATH}/jaricom.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jcapimin.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jcapistd.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jcarith.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jccoefct.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jccolor.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jcdctmgr.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jchuff.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jcinit.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jcmainct.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jcmarker.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jcmaster.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jcomapi.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jcparam.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jcprepct.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jcsample.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jctrans.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jdapimin.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jdapistd.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jdarith.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jdatadst.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jdatasrc.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jdcoefct.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jdcolor.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jddctmgr.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jdhuff.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jdinput.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jdmainct.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jdmarker.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jdmaster.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jdmerge.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jdpostct.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jdsample.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jdtrans.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jerror.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jfdctflt.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jfdctfst.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jfdctint.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jidctflt.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jidctfst.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jidctint.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jquant1.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jquant2.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jutils.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jmemmgr.c \
    $${THIRDPARTY_LIBJPEG_PATH}/jmemnobs.c

TR_EXCLUDE += $${THIRDPARTY_LIBJPEG_PATH}/*

