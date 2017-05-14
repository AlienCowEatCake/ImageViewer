# URL: http://bellard.org/bpg/
# License: ???

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_libbpg

QT -= core gui

CONFIG -= warn_on
#CONFIG += hide_symbols
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_LIBBPG_PATH = $${PWD}/libbpg-0.9.7

include(../CommonSettings.pri)

INCLUDEPATH = $${THIRDPARTY_LIBBPG_PATH} $${INCLUDEPATH}

DEFINES += HAVE_AV_CONFIG_H
DEFINES += USE_VAR_BIT_DEPTH
DEFINES += USE_PRED
*msvc*: DEFINES += inline=__inline

SOURCES += \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/hevc_cabac.c \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/hevc_filter.c \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/hevc.c \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/hevcpred.c \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/hevc_refs.c \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/hevcdsp.c \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/hevc_mvs.c \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/hevc_ps.c \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/hevc_sei.c \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/utils.c \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/cabac.c \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/golomb.c \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/videodsp.c \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/mem.c \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/buffer.c \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/log2_tab.c \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/frame.c \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/pixdesc.c \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/md5.c \
    $${THIRDPARTY_LIBBPG_PATH}/libbpg.c

TR_EXCLUDE += $${THIRDPARTY_LIBBPG_PATH}/*

