# URL: https://chromium.googlesource.com/libyuv/libyuv/
# License: 3-Clause BSD License - https://chromium.googlesource.com/libyuv/libyuv/+/HEAD/LICENSE

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_libyuv

QT -= gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_LIBYUV_PATH = $${PWD}/libyuv

include(../../Features.pri)
include(../CommonSettings.pri)
include(../libjpeg/libjpeg.pri)

INCLUDEPATH = $${THIRDPARTY_LIBYUV_PATH}/include $${INCLUDEPATH}

DEFINES += LIBYUV_DISABLE_LASX LIBYUV_DISABLE_LSX LIBYUV_DISABLE_MSA LIBYUV_DISABLE_NEON LIBYUV_DISABLE_RVV LIBYUV_DISABLE_SME LIBYUV_DISABLE_SVE LIBYUV_DISABLE_X86
DEFINES += __STDC_LIMIT_MACROS __STDC_CONSTANT_MACROS
!disable_libjpeg : DEFINES += HAVE_JPEG

# find ./source -name '*.cc' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_LIBYUV_PATH}| ; s|$| \\|'
SOURCES += \
    $${THIRDPARTY_LIBYUV_PATH}/source/compare.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/compare_common.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/compare_gcc.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/compare_msa.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/compare_neon.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/compare_neon64.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/compare_win.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/convert.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/convert_argb.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/convert_from.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/convert_from_argb.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/convert_jpeg.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/convert_to_argb.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/convert_to_i420.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/cpu_id.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/mjpeg_decoder.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/mjpeg_validate.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/planar_functions.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/rotate.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/rotate_any.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/rotate_argb.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/rotate_common.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/rotate_gcc.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/rotate_lsx.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/rotate_msa.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/rotate_neon.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/rotate_neon64.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/rotate_sme.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/rotate_win.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/row_any.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/row_common.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/row_gcc.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/row_lasx.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/row_lsx.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/row_msa.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/row_neon.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/row_neon64.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/row_rvv.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/row_sme.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/row_sve.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/row_win.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/scale.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/scale_any.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/scale_argb.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/scale_common.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/scale_gcc.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/scale_lsx.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/scale_msa.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/scale_neon.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/scale_neon64.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/scale_rgb.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/scale_rvv.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/scale_sme.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/scale_uv.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/scale_win.cc \
    $${THIRDPARTY_LIBYUV_PATH}/source/video_common.cc \

# find ./include -name '*.h' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_LIBYUV_PATH}| ; s|$| \\|'
HEADERS += \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/basic_types.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/compare.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/compare_row.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/convert.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/convert_argb.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/convert_from.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/convert_from_argb.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/cpu_id.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/cpu_support.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/loongson_intrinsics.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/macros_msa.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/mjpeg_decoder.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/planar_functions.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/rotate.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/rotate_argb.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/rotate_row.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/row.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/scale.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/scale_argb.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/scale_rgb.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/scale_row.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/scale_uv.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/version.h \
    $${THIRDPARTY_LIBYUV_PATH}/include/libyuv/video_common.h \

TR_EXCLUDE += $${THIRDPARTY_LIBYUV_PATH}/*

