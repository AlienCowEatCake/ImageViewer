# URL: https://www.ece.uvic.ca/~frodo/jasper/
# License: https://www.ece.uvic.ca/~frodo/jasper/LICENSE

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_jasper

QT -= core gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_JASPER_PATH = $${PWD}/jasper-2.0.12
THIRDPARTY_JASPER_CONFIG_PATH = $${PWD}/config

include(../CommonSettings.pri)
include(../libjpeg/libjpeg.pri)

INCLUDEPATH = $${THIRDPARTY_JASPER_PATH}/src/libjasper/include $${THIRDPARTY_JASPER_PATH}/include $${THIRDPARTY_JASPER_CONFIG_PATH} $${INCLUDEPATH}

*msvc*: DEFINES += JAS_WIN_MSVC_BUILD

SOURCES += \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/base/jas_cm.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/base/jas_debug.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/base/jas_getopt.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/base/jas_icc.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/base/jas_iccdata.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/base/jas_image.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/base/jas_init.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/base/jas_malloc.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/base/jas_seq.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/base/jas_stream.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/base/jas_string.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/base/jas_tmr.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/base/jas_tvp.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/base/jas_version.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/bmp/bmp_cod.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/bmp/bmp_dec.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/bmp/bmp_enc.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/jp2/jp2_cod.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/jp2/jp2_dec.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/jp2/jp2_enc.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/jpc/jpc_bs.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/jpc/jpc_cs.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/jpc/jpc_dec.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/jpc/jpc_enc.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/jpc/jpc_math.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/jpc/jpc_mct.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/jpc/jpc_mqcod.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/jpc/jpc_mqdec.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/jpc/jpc_mqenc.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/jpc/jpc_qmfb.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/jpc/jpc_t1cod.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/jpc/jpc_t1dec.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/jpc/jpc_t1enc.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/jpc/jpc_t2cod.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/jpc/jpc_t2dec.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/jpc/jpc_t2enc.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/jpc/jpc_tagtree.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/jpc/jpc_tsfb.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/jpc/jpc_util.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/jpg/jpg_val.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/mif/mif_cod.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/pgx/pgx_cod.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/pgx/pgx_dec.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/pgx/pgx_enc.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/pnm/pnm_cod.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/pnm/pnm_dec.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/pnm/pnm_enc.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/ras/ras_cod.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/ras/ras_dec.c \
    $${THIRDPARTY_JASPER_PATH}/src/libjasper/ras/ras_enc.c

!disable_libjpeg {
    SOURCES += \
        $${THIRDPARTY_JASPER_PATH}/src/libjasper/jpg/jpg_dec.c \
        $${THIRDPARTY_JASPER_PATH}/src/libjasper/jpg/jpg_enc.c
} else {
    SOURCES += \
        $${THIRDPARTY_JASPER_PATH}/src/libjasper/jpg/jpg_dummy.c
}

TR_EXCLUDE += $${THIRDPARTY_JASPER_PATH}/*

