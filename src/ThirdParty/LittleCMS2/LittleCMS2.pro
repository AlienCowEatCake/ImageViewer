# URL: http://www.littlecms.com/
# License:  MIT License

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_liblcms2

QT -= core gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_LIBLCMS2_PATH = $${PWD}/lcms2-2.8

include(../CommonSettings.pri)

INCLUDEPATH = $${THIRDPARTY_LIBLCMS2_PATH}/include $${THIRDPARTY_LIBLCMS2_PATH}/src $${INCLUDEPATH}

SOURCES += \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmsalpha.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmscam02.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmscgats.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmscnvrt.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmserr.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmsgamma.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmsgmt.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmshalf.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmsintrp.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmsio0.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmsio1.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmslut.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmsmd5.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmsmtrx.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmsnamed.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmsopt.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmspack.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmspcs.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmsplugin.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmsps2.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmssamp.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmssm.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmstypes.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmsvirt.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmswtpnt.c \
    $${THIRDPARTY_LIBLCMS2_PATH}/src/cmsxform.c

TR_EXCLUDE += $${THIRDPARTY_LIBLCMS2_PATH}/*

