# URL: https://www.webmproject.org/code/#webp-repositories + https://github.com/webmproject/libwebp/
# License: 3-clause BSD License - https://chromium.googlesource.com/webm/libwebp/+/refs/heads/master/COPYING

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_LibWebP

#QT -= core gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_LIBWEBP_PATH = $${PWD}/libwebp-1.2.4

include(../CommonSettings.pri)

INCLUDEPATH = \
    $${THIRDPARTY_LIBWEBP_PATH} \
    $${THIRDPARTY_LIBWEBP_PATH}/sharpyuv \
    $${THIRDPARTY_LIBWEBP_PATH}/src \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dec \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp \
    $${THIRDPARTY_LIBWEBP_PATH}/src/mux \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils \
    $${THIRDPARTY_LIBWEBP_PATH}/src/webp \
    $${INCLUDEPATH}

SOURCES += \
    $${THIRDPARTY_LIBWEBP_PATH}/sharpyuv/sharpyuv.c \
    $${THIRDPARTY_LIBWEBP_PATH}/sharpyuv/sharpyuv_csp.c \
    $${THIRDPARTY_LIBWEBP_PATH}/sharpyuv/sharpyuv_dsp.c \
    $${THIRDPARTY_LIBWEBP_PATH}/sharpyuv/sharpyuv_gamma.c \
    $${THIRDPARTY_LIBWEBP_PATH}/sharpyuv/sharpyuv_sse2.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dec/alpha_dec.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dec/buffer_dec.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dec/frame_dec.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dec/idec_dec.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dec/io_dec.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dec/quant_dec.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dec/tree_dec.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dec/vp8_dec.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dec/vp8l_dec.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dec/webp_dec.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/demux/demux.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/demux/anim_decode.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/alpha_processing.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/alpha_processing_mips_dsp_r2.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/alpha_processing_sse2.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/alpha_processing_sse41.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/cost.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/cost_mips32.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/cost_mips_dsp_r2.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/cost_neon.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/cost_sse2.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/cpu.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/dec.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/dec_clip_tables.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/dec_mips32.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/dec_mips_dsp_r2.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/dec_msa.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/dec_sse2.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/dec_sse41.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/enc_mips32.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/enc_mips_dsp_r2.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/enc_msa.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/enc_sse2.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/enc_sse41.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/filters.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/filters_mips_dsp_r2.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/filters_msa.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/filters_sse2.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/lossless.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/lossless_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/lossless_enc_mips32.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/lossless_enc_mips_dsp_r2.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/lossless_enc_msa.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/lossless_enc_sse2.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/lossless_enc_sse41.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/lossless_mips_dsp_r2.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/rescaler.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/rescaler_mips32.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/rescaler_mips_dsp_r2.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/rescaler_msa.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/rescaler_sse2.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/ssim.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/ssim_sse2.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/upsampling.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/upsampling_mips_dsp_r2.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/upsampling_msa.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/upsampling_sse2.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/upsampling_sse41.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/yuv.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/yuv_mips_dsp_r2.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/lossless_sse2.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/lossless_sse41.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/yuv_mips32.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/yuv_sse2.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/yuv_sse41.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/alpha_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/analysis_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/backward_references_cost_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/backward_references_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/config_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/cost_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/filter_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/frame_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/histogram_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/iterator_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/near_lossless_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/picture_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/picture_csp_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/picture_psnr_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/picture_rescale_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/picture_tools_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/predictor_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/quant_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/syntax_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/token_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/tree_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/vp8l_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/webp_enc.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/mux/anim_encode.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/mux/muxedit.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/mux/muxinternal.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/mux/muxread.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/bit_reader_utils.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/bit_writer_utils.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/color_cache_utils.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/filters_utils.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/huffman_utils.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/huffman_encode_utils.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/quant_levels_utils.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/quant_levels_dec_utils.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/random_utils.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/rescaler_utils.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/thread_utils.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/utils.c

# (find ./src -name '*.h' && find ./sharpyuv -name '*.h') | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_LIBWEBP_PATH}| ; s|$| \\|'
HEADERS += \
    $${THIRDPARTY_LIBWEBP_PATH}/sharpyuv/sharpyuv.h \
    $${THIRDPARTY_LIBWEBP_PATH}/sharpyuv/sharpyuv_csp.h \
    $${THIRDPARTY_LIBWEBP_PATH}/sharpyuv/sharpyuv_dsp.h \
    $${THIRDPARTY_LIBWEBP_PATH}/sharpyuv/sharpyuv_gamma.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dec/alphai_dec.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dec/common_dec.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dec/vp8_dec.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dec/vp8i_dec.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dec/vp8li_dec.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dec/webpi_dec.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/common_sse2.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/common_sse41.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/cpu.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/dsp.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/lossless.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/lossless_common.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/mips_macro.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/msa_macro.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/neon.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/quant.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/yuv.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/backward_references_enc.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/cost_enc.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/histogram_enc.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/vp8i_enc.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/enc/vp8li_enc.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/mux/animi.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/mux/muxi.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/bit_reader_inl_utils.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/bit_reader_utils.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/bit_writer_utils.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/color_cache_utils.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/endian_inl_utils.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/filters_utils.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/huffman_encode_utils.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/huffman_utils.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/quant_levels_dec_utils.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/quant_levels_utils.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/random_utils.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/rescaler_utils.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/thread_utils.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/utils/utils.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/webp/decode.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/webp/demux.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/webp/encode.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/webp/format_constants.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/webp/mux.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/webp/mux_types.h \
    $${THIRDPARTY_LIBWEBP_PATH}/src/webp/types.h \

android:!android-embedded {
    SOURCES += $${NDK_ROOT}/sources/android/cpufeatures/cpu-features.c
    INCLUDEPATH += $${NDK_ROOT}/sources/android/cpufeatures
}

integrity {
    QMAKE_CFLAGS += -c99
}

SOURCES_FOR_NEON += \
    $${THIRDPARTY_LIBWEBP_PATH}/sharpyuv/sharpyuv_neon.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/alpha_processing_neon.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/dec_neon.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/enc_neon.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/filters_neon.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/lossless_enc_neon.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/lossless_neon.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/rescaler_neon.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/upsampling_neon.c \
    $${THIRDPARTY_LIBWEBP_PATH}/src/dsp/yuv_neon.c

android {
    arm64-v8a | armeabi-v7a: SOURCES += $$SOURCES_FOR_NEON
} else: equals(QT_ARCH, arm)|equals(QT_ARCH, arm64) {
    contains(QT_CPU_FEATURES.$$QT_ARCH, neon) {
        # Default compiler settings include this feature, so just add to SOURCES
        SOURCES += $$SOURCES_FOR_NEON
    } else {
        neon_comp.commands = $$QMAKE_CC -c $(CFLAGS)
        neon_comp.commands += $$QMAKE_CFLAGS_NEON
        neon_comp.commands += $(INCPATH) ${QMAKE_FILE_IN}
        msvc: neon_comp.commands += -Fo${QMAKE_FILE_OUT}
        else: neon_comp.commands += -o ${QMAKE_FILE_OUT}
        neon_comp.dependency_type = TYPE_C
        neon_comp.output = ${QMAKE_VAR_OBJECTS_DIR}${QMAKE_FILE_BASE}$${first(QMAKE_EXT_OBJ)}
        neon_comp.input = SOURCES_FOR_NEON
        neon_comp.variable_out = OBJECTS
        neon_comp.name = compiling[neon] ${QMAKE_FILE_IN}
        silent: neon_comp.commands = @echo compiling[neon] ${QMAKE_FILE_IN} && $$neon_comp.commands
        QMAKE_EXTRA_COMPILERS += neon_comp
    }
} else {
    macx {
        # Universal builds should contain neon sources
        SOURCES += $$SOURCES_FOR_NEON
    }
}

TR_EXCLUDE += $${THIRDPARTY_LIBWEBP_PATH}/*

