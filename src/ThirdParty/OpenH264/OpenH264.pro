# URL: https://github.com/cisco/openh264
# License: 2-Clause BSD License - https://github.com/cisco/openh264/blob/master/LICENSE

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_openh264

QT -= core gui

CONFIG -= warn_on
CONFIG += warn_off

THIRDPARTY_OPENH264_PATH = $${PWD}/openh264
THIRDPARTY_OPENH264_CONFIG_PATH = $${PWD}/config

include(../../Features.pri)
include(../CommonSettings.pri)

INCLUDEPATH = \
    $${THIRDPARTY_OPENH264_PATH}/codec/api \
    $${THIRDPARTY_OPENH264_PATH}/codec/api/wels \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/plus/inc \
    $${INCLUDEPATH}

# git archive --prefix='openh264/' -o ../openh264.tar HEAD

# (find ./codec/common/src/ -name '*.cpp' && find ./codec/decoder/core/src/ -name '*.cpp' && find ./codec/decoder/plus/src/ -name '*.cpp') | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_OPENH264_PATH}| ; s|$| \\|'
SOURCES += \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/src/WelsTaskThread.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/src/WelsThread.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/src/WelsThreadLib.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/src/WelsThreadPool.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/src/common_tables.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/src/copy_mb.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/src/cpu.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/src/crt_util_safe_x.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/src/deblocking_common.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/src/expand_pic.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/src/intra_pred_common.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/src/mc.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/src/memory_align.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/src/sad_common.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/src/utils.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/src/welsCodecTrace.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/src/au_parser.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/src/bit_stream.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/src/cabac_decoder.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/src/deblocking.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/src/decode_mb_aux.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/src/decode_slice.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/src/decoder.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/src/decoder_core.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/src/decoder_data_tables.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/src/error_concealment.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/src/fmo.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/src/get_intra_predictor.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/src/manage_dec_ref.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/src/memmgr_nal_unit.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/src/mv_pred.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/src/parse_mb_syn_cabac.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/src/parse_mb_syn_cavlc.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/src/pic_queue.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/src/rec_mb.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/src/wels_decoder_thread.cpp \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/plus/src/welsDecoderExt.cpp \
    $${THIRDPARTY_OPENH264_CONFIG_PATH}/version.cpp

# (find ./codec/api/ -name '*.h' && find ./codec/common/inc/ -name '*.h' && find ./codec/decoder/core/inc/ -name '*.h' && find ./codec/decoder/plus/inc/ -name '*.h') | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_OPENH264_PATH}| ; s|$| \\|'
HEADERS += \
    $${THIRDPARTY_OPENH264_PATH}/codec/api/wels/codec_api.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/api/wels/codec_app_def.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/api/wels/codec_def.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/api/wels/codec_ver.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/WelsList.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/WelsLock.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/WelsTask.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/WelsTaskThread.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/WelsThread.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/WelsThreadLib.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/WelsThreadPool.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/asmdefs_mmi.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/copy_mb.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/cpu.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/cpu_core.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/crt_util_safe_x.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/deblocking_common.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/expand_pic.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/golomb_common.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/intra_pred_common.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/loongson_intrinsics.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/ls_defines.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/macros.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/mc.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/measure_time.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/memory_align.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/msa_macros.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/sad_common.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/typedefs.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/utils.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/version.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/welsCodecTrace.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/wels_common_defs.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/common/inc/wels_const_common.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/au_parser.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/bit_stream.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/cabac_decoder.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/deblocking.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/dec_frame.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/dec_golomb.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/decode_mb_aux.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/decode_slice.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/decoder.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/decoder_context.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/decoder_core.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/error_code.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/error_concealment.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/fmo.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/get_intra_predictor.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/manage_dec_ref.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/mb_cache.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/memmgr_nal_unit.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/mv_pred.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/nal_prefix.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/nalu.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/parameter_sets.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/parse_mb_syn_cabac.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/parse_mb_syn_cavlc.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/pic_queue.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/picture.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/rec_mb.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/slice.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/vlc_decoder.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/wels_common_basis.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/wels_const.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/core/inc/wels_decoder_thread.h \
    $${THIRDPARTY_OPENH264_PATH}/codec/decoder/plus/inc/welsDecoderExt.h \

TR_EXCLUDE += $${THIRDPARTY_OPENH264_PATH}/*

