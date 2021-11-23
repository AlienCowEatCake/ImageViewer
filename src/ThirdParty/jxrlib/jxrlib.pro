# URL: http://web.archive.org/web/20180116001029/http://jxrlib.codeplex.com/
# License: 2-clause BSD License

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_jxrlib

QT -= core gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_JXRLIB_PATH = $${PWD}/jxrlib-e922fa5

include(../CommonSettings.pri)

INCLUDEPATH = \
    $${THIRDPARTY_JXRLIB_PATH} \
    $${THIRDPARTY_JXRLIB_PATH}/common/include \
    $${THIRDPARTY_JXRLIB_PATH}/image/sys \
    $${THIRDPARTY_JXRLIB_PATH}/jxrgluelib \
    $${INCLUDEPATH}

DEFINES += __ANSI__ DISABLE_PERF_MEASUREMENT

SOURCES += \
    $${THIRDPARTY_JXRLIB_PATH}/image/sys/adapthuff.c \
    $${THIRDPARTY_JXRLIB_PATH}/image/sys/image.c \
    $${THIRDPARTY_JXRLIB_PATH}/image/sys/strcodec.c \
    $${THIRDPARTY_JXRLIB_PATH}/image/sys/strPredQuant.c \
    $${THIRDPARTY_JXRLIB_PATH}/image/sys/strTransform.c \
    $${THIRDPARTY_JXRLIB_PATH}/image/sys/perfTimerANSI.c \
    $${THIRDPARTY_JXRLIB_PATH}/image/decode/decode.c \
    $${THIRDPARTY_JXRLIB_PATH}/image/decode/postprocess.c \
    $${THIRDPARTY_JXRLIB_PATH}/image/decode/segdec.c \
    $${THIRDPARTY_JXRLIB_PATH}/image/decode/strdec.c \
    $${THIRDPARTY_JXRLIB_PATH}/image/decode/strInvTransform.c \
    $${THIRDPARTY_JXRLIB_PATH}/image/decode/strPredQuantDec.c \
    $${THIRDPARTY_JXRLIB_PATH}/image/decode/JXRTranscode.c \
    $${THIRDPARTY_JXRLIB_PATH}/image/encode/encode.c \
    $${THIRDPARTY_JXRLIB_PATH}/image/encode/segenc.c \
    $${THIRDPARTY_JXRLIB_PATH}/image/encode/strenc.c \
    $${THIRDPARTY_JXRLIB_PATH}/image/encode/strFwdTransform.c \
    $${THIRDPARTY_JXRLIB_PATH}/image/encode/strPredQuantEnc.c \
    $${THIRDPARTY_JXRLIB_PATH}/jxrgluelib/JXRGlue.c \
    $${THIRDPARTY_JXRLIB_PATH}/jxrgluelib/JXRMeta.c \
    $${THIRDPARTY_JXRLIB_PATH}/jxrgluelib/JXRGluePFC.c \
    $${THIRDPARTY_JXRLIB_PATH}/jxrgluelib/JXRGlueJxr.c \

HEADERS += \
    $${THIRDPARTY_JXRLIB_PATH}/common/include/guiddef.h \
    $${THIRDPARTY_JXRLIB_PATH}/common/include/wmsal.h \
    $${THIRDPARTY_JXRLIB_PATH}/common/include/wmspecstring.h \
    $${THIRDPARTY_JXRLIB_PATH}/common/include/wmspecstrings_adt.h \
    $${THIRDPARTY_JXRLIB_PATH}/common/include/wmspecstrings_strict.h \
    $${THIRDPARTY_JXRLIB_PATH}/common/include/wmspecstrings_undef.h \
    $${THIRDPARTY_JXRLIB_PATH}/image/decode/decode.h \
    $${THIRDPARTY_JXRLIB_PATH}/image/encode/encode.h \
    $${THIRDPARTY_JXRLIB_PATH}/image/sys/ansi.h \
    $${THIRDPARTY_JXRLIB_PATH}/image/sys/common.h \
    $${THIRDPARTY_JXRLIB_PATH}/image/sys/perfTimer.h \
    $${THIRDPARTY_JXRLIB_PATH}/image/sys/strTransform.h \
    $${THIRDPARTY_JXRLIB_PATH}/image/sys/strcodec.h \
    $${THIRDPARTY_JXRLIB_PATH}/image/sys/windowsmediaphoto.h \
    $${THIRDPARTY_JXRLIB_PATH}/image/sys/xplatform_image.h \
    $${THIRDPARTY_JXRLIB_PATH}/image/x86/x86.h \
    $${THIRDPARTY_JXRLIB_PATH}/jxrgluelib/JXRGlue.h \
    $${THIRDPARTY_JXRLIB_PATH}/jxrgluelib/JXRMeta.h \
    $${THIRDPARTY_JXRLIB_PATH}/jxrtestlib/JXRTest.h \

TR_EXCLUDE += $${THIRDPARTY_JXRLIB_PATH}/*

