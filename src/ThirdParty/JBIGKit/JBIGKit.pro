# URL: https://www.cl.cam.ac.uk/~mgk25/jbigkit/
# License: GNU GPL v2 or Commercial - https://www.cl.cam.ac.uk/~mgk25/jbigkit/#licensing

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_JBIGKit

QT -= core gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_JBIGKIT_PATH = $${PWD}/jbigkit-2.1

include(../CommonSettings.pri)

INCLUDEPATH = $${THIRDPARTY_JBIGKIT_PATH}/libjbig $${INCLUDEPATH}

SOURCES += \
    $${THIRDPARTY_JBIGKIT_PATH}/libjbig/jbig_ar.c \
    $${THIRDPARTY_JBIGKIT_PATH}/libjbig/jbig.c

HEADERS += \
    $${THIRDPARTY_JBIGKIT_PATH}/libjbig/jbig_ar.h \
    $${THIRDPARTY_JBIGKIT_PATH}/libjbig/jbig.h

TR_EXCLUDE += $${THIRDPARTY_JBIGKIT_PATH}/*

