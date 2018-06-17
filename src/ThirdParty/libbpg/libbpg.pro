# URL: http://bellard.org/bpg/
# License: ???

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_libbpg

QT -= core gui

CONFIG -= warn_on
#CONFIG += hide_symbols
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_LIBBPG_PATH = $${PWD}/libbpg-0.9.8

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

HEADERS += \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/avcodec.h \
\#    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/bit_depth_template.c \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/bswapdsp.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/bytestream.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/cabac.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/cabac_functions.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/cabac_tablegen.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/get_bits.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/golomb.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/hevc.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/hevcdsp.h \
\#    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/hevcdsp_template.c \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/hevcpred.h \
\#    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/hevcpred_template.c \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/internal.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/thread.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/version.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/videodsp.h \
\#    $${THIRDPARTY_LIBBPG_PATH}/libavcodec/videodsp_template.c \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/atomic.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/attributes.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/avassert.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/avstring.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/avutil.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/bprint.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/bswap.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/buffer.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/buffer_internal.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/channel_layout.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/common.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/crc.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/dict.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/display.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/dynarray.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/frame.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/imgutils.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/internal.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/intreadwrite.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/lfg.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/mathematics.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/md5.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/mem.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/opt.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/pixdesc.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/pixfmt.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/samplefmt.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/stereo3d.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/timer.h \
    $${THIRDPARTY_LIBBPG_PATH}/libavutil/version.h \
    $${THIRDPARTY_LIBBPG_PATH}/config.h \
    $${THIRDPARTY_LIBBPG_PATH}/libbpg.h

TR_EXCLUDE += $${THIRDPARTY_LIBBPG_PATH}/*

