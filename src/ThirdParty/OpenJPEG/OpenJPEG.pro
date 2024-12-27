# URL: http://www.openjpeg.org/
# License: 2-Clause BSD License - https://github.com/uclouvain/openjpeg/blob/master/LICENSE

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_openjp2

#QT -= core gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_OPENJPEG_PATH = $${PWD}/openjpeg-2.5.3
THIRDPARTY_OPENJPEG_CONFIG_PATH = $${PWD}/config

include(../../Features.pri)
include(../CommonSettings.pri)

INCLUDEPATH = $${THIRDPARTY_OPENJPEG_CONFIG_PATH} $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2 $${INCLUDEPATH}

DEFINES += OPJ_STATIC

*msvc* {
    DEFINES += _CRT_FAR_MAPPINGS_NO_DEPRECATE
    DEFINES += _CRT_IS_WCTYPE_NO_DEPRECATE
    DEFINES += _CRT_MANAGED_FP_NO_DEPRECATE
    DEFINES += _CRT_NONSTDC_NO_DEPRECATE
    DEFINES += _CRT_SECURE_NO_DEPRECATE
    DEFINES += _CRT_SECURE_NO_DEPRECATE_GLOBALS
    DEFINES += _CRT_SETERRORMODE_BEEP_SLEEP_NO_DEPRECATE
    DEFINES += _CRT_TIME_FUNCTIONS_NO_DEPRECATE
    DEFINES += _CRT_VCCLRIT_NO_DEPRECATE
    DEFINES += _SCL_SECURE_NO_DEPRECATE
}

# find ./src/lib/openjp2 -name '*.c' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_OPENJPEG_PATH}| ; s|$| \\|'
SOURCES += \
\#    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/bench_dwt.c \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/bio.c \
\#    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/cidx_manager.c \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/cio.c \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/dwt.c \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/event.c \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/function_list.c \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/ht_dec.c \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/image.c \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/invert.c \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/j2k.c \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/jp2.c \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/mct.c \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/mqc.c \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/openjpeg.c \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/opj_clock.c \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/opj_malloc.c \
\#    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/phix_manager.c \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/pi.c \
\#    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/ppix_manager.c \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/sparse_array.c \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/t1.c \
\#    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/t1_generate_luts.c \
\#    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/t1_ht_generate_luts.c \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/t2.c \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/tcd.c \
\#    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/test_sparse_array.c \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/tgt.c \
\#    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/thix_manager.c \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/thread.c \
\#    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/tpix_manager.c \

# find ./src/lib/openjp2 -name '*.h' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_OPENJPEG_PATH}| ; s|$| \\|'
HEADERS += \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/bio.h \
\#    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/cidx_manager.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/cio.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/dwt.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/event.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/function_list.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/image.h \
\#    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/indexbox_manager.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/invert.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/j2k.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/jp2.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/mct.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/mqc.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/mqc_inl.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/openjpeg.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/opj_clock.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/opj_codec.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/opj_common.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/opj_includes.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/opj_intmath.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/opj_malloc.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/pi.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/sparse_array.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/t1.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/t1_ht_luts.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/t1_luts.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/t2.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/tcd.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/tgt.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/thread.h \
    $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2/tls_keys.h \
    $${THIRDPARTY_OPENJPEG_CONFIG_PATH}/opj_config.h \
    $${THIRDPARTY_OPENJPEG_CONFIG_PATH}/opj_config_private.h \

TR_EXCLUDE += $${THIRDPARTY_OPENJPEG_PATH}/* $${THIRDPARTY_OPENJPEG_CONFIG_PATH}/*

