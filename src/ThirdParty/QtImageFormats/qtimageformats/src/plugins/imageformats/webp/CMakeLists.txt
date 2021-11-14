# Generated from webp.pro.

#####################################################################
## QWebpPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWebpPlugin
    OUTPUT_NAME qwebp
    PLUGIN_TYPE imageformats
    SOURCES
        main.cpp
        qwebphandler.cpp qwebphandler_p.h
    LIBRARIES
        Qt::Core
        Qt::Gui
)

#### Keys ignored in scope 1:.:.:webp.pro:<TRUE>:
# OTHER_FILES = "webp.json"
# QT_FOR_CONFIG = "imageformats-private"

## Scopes:
#####################################################################

qt_internal_extend_target(QWebpPlugin CONDITION QT_FEATURE_system_webp
    LIBRARIES
        WrapWebP::WrapWebP
)

qt_internal_extend_target(QWebpPlugin CONDITION NOT QT_FEATURE_system_webp
    SOURCES
        ../../../3rdparty/libwebp/src/dec/alpha_dec.c
        ../../../3rdparty/libwebp/src/dec/buffer_dec.c
        ../../../3rdparty/libwebp/src/dec/frame_dec.c
        ../../../3rdparty/libwebp/src/dec/idec_dec.c
        ../../../3rdparty/libwebp/src/dec/io_dec.c
        ../../../3rdparty/libwebp/src/dec/quant_dec.c
        ../../../3rdparty/libwebp/src/dec/tree_dec.c
        ../../../3rdparty/libwebp/src/dec/vp8_dec.c
        ../../../3rdparty/libwebp/src/dec/vp8l_dec.c
        ../../../3rdparty/libwebp/src/dec/webp_dec.c
        ../../../3rdparty/libwebp/src/demux/anim_decode.c
        ../../../3rdparty/libwebp/src/demux/demux.c
        ../../../3rdparty/libwebp/src/dsp/alpha_processing.c
        ../../../3rdparty/libwebp/src/dsp/alpha_processing_mips_dsp_r2.c
        ../../../3rdparty/libwebp/src/dsp/alpha_processing_sse2.c
        ../../../3rdparty/libwebp/src/dsp/alpha_processing_sse41.c
        ../../../3rdparty/libwebp/src/dsp/cost.c
        ../../../3rdparty/libwebp/src/dsp/cost_mips32.c
        ../../../3rdparty/libwebp/src/dsp/cost_mips_dsp_r2.c
        ../../../3rdparty/libwebp/src/dsp/cost_neon.c
        ../../../3rdparty/libwebp/src/dsp/cost_sse2.c
        ../../../3rdparty/libwebp/src/dsp/cpu.c
        ../../../3rdparty/libwebp/src/dsp/dec.c
        ../../../3rdparty/libwebp/src/dsp/dec_clip_tables.c
        ../../../3rdparty/libwebp/src/dsp/dec_mips32.c
        ../../../3rdparty/libwebp/src/dsp/dec_mips_dsp_r2.c
        ../../../3rdparty/libwebp/src/dsp/dec_msa.c
        ../../../3rdparty/libwebp/src/dsp/dec_sse2.c
        ../../../3rdparty/libwebp/src/dsp/dec_sse41.c
        ../../../3rdparty/libwebp/src/dsp/enc.c
        ../../../3rdparty/libwebp/src/dsp/enc_mips32.c
        ../../../3rdparty/libwebp/src/dsp/enc_mips_dsp_r2.c
        ../../../3rdparty/libwebp/src/dsp/enc_msa.c
        ../../../3rdparty/libwebp/src/dsp/enc_sse2.c
        ../../../3rdparty/libwebp/src/dsp/enc_sse41.c
        ../../../3rdparty/libwebp/src/dsp/filters.c
        ../../../3rdparty/libwebp/src/dsp/filters_mips_dsp_r2.c
        ../../../3rdparty/libwebp/src/dsp/filters_msa.c
        ../../../3rdparty/libwebp/src/dsp/filters_sse2.c
        ../../../3rdparty/libwebp/src/dsp/lossless.c
        ../../../3rdparty/libwebp/src/dsp/lossless_enc.c
        ../../../3rdparty/libwebp/src/dsp/lossless_enc_mips32.c
        ../../../3rdparty/libwebp/src/dsp/lossless_enc_mips_dsp_r2.c
        ../../../3rdparty/libwebp/src/dsp/lossless_enc_msa.c
        ../../../3rdparty/libwebp/src/dsp/lossless_enc_sse2.c
        ../../../3rdparty/libwebp/src/dsp/lossless_enc_sse41.c
        ../../../3rdparty/libwebp/src/dsp/lossless_mips_dsp_r2.c
        ../../../3rdparty/libwebp/src/dsp/lossless_sse2.c
        ../../../3rdparty/libwebp/src/dsp/lossless_sse41.c
        ../../../3rdparty/libwebp/src/dsp/rescaler.c
        ../../../3rdparty/libwebp/src/dsp/rescaler_mips32.c
        ../../../3rdparty/libwebp/src/dsp/rescaler_mips_dsp_r2.c
        ../../../3rdparty/libwebp/src/dsp/rescaler_msa.c
        ../../../3rdparty/libwebp/src/dsp/rescaler_sse2.c
        ../../../3rdparty/libwebp/src/dsp/ssim.c
        ../../../3rdparty/libwebp/src/dsp/ssim_sse2.c
        ../../../3rdparty/libwebp/src/dsp/upsampling.c
        ../../../3rdparty/libwebp/src/dsp/upsampling_mips_dsp_r2.c
        ../../../3rdparty/libwebp/src/dsp/upsampling_msa.c
        ../../../3rdparty/libwebp/src/dsp/upsampling_sse2.c
        ../../../3rdparty/libwebp/src/dsp/upsampling_sse41.c
        ../../../3rdparty/libwebp/src/dsp/yuv.c
        ../../../3rdparty/libwebp/src/dsp/yuv_mips32.c
        ../../../3rdparty/libwebp/src/dsp/yuv_mips_dsp_r2.c
        ../../../3rdparty/libwebp/src/dsp/yuv_sse2.c
        ../../../3rdparty/libwebp/src/dsp/yuv_sse41.c
        ../../../3rdparty/libwebp/src/enc/alpha_enc.c
        ../../../3rdparty/libwebp/src/enc/analysis_enc.c
        ../../../3rdparty/libwebp/src/enc/backward_references_cost_enc.c
        ../../../3rdparty/libwebp/src/enc/backward_references_enc.c
        ../../../3rdparty/libwebp/src/enc/config_enc.c
        ../../../3rdparty/libwebp/src/enc/cost_enc.c
        ../../../3rdparty/libwebp/src/enc/filter_enc.c
        ../../../3rdparty/libwebp/src/enc/frame_enc.c
        ../../../3rdparty/libwebp/src/enc/histogram_enc.c
        ../../../3rdparty/libwebp/src/enc/iterator_enc.c
        ../../../3rdparty/libwebp/src/enc/near_lossless_enc.c
        ../../../3rdparty/libwebp/src/enc/picture_csp_enc.c
        ../../../3rdparty/libwebp/src/enc/picture_enc.c
        ../../../3rdparty/libwebp/src/enc/picture_psnr_enc.c
        ../../../3rdparty/libwebp/src/enc/picture_rescale_enc.c
        ../../../3rdparty/libwebp/src/enc/picture_tools_enc.c
        ../../../3rdparty/libwebp/src/enc/predictor_enc.c
        ../../../3rdparty/libwebp/src/enc/quant_enc.c
        ../../../3rdparty/libwebp/src/enc/syntax_enc.c
        ../../../3rdparty/libwebp/src/enc/token_enc.c
        ../../../3rdparty/libwebp/src/enc/tree_enc.c
        ../../../3rdparty/libwebp/src/enc/vp8l_enc.c
        ../../../3rdparty/libwebp/src/enc/webp_enc.c
        ../../../3rdparty/libwebp/src/mux/anim_encode.c
        ../../../3rdparty/libwebp/src/mux/muxedit.c
        ../../../3rdparty/libwebp/src/mux/muxinternal.c
        ../../../3rdparty/libwebp/src/mux/muxread.c
        ../../../3rdparty/libwebp/src/utils/bit_reader_utils.c
        ../../../3rdparty/libwebp/src/utils/bit_writer_utils.c
        ../../../3rdparty/libwebp/src/utils/color_cache_utils.c
        ../../../3rdparty/libwebp/src/utils/filters_utils.c
        ../../../3rdparty/libwebp/src/utils/huffman_encode_utils.c
        ../../../3rdparty/libwebp/src/utils/huffman_utils.c
        ../../../3rdparty/libwebp/src/utils/quant_levels_dec_utils.c
        ../../../3rdparty/libwebp/src/utils/quant_levels_utils.c
        ../../../3rdparty/libwebp/src/utils/random_utils.c
        ../../../3rdparty/libwebp/src/utils/rescaler_utils.c
        ../../../3rdparty/libwebp/src/utils/thread_utils.c
        ../../../3rdparty/libwebp/src/utils/utils.c
    INCLUDE_DIRECTORIES
        ../../../3rdparty/libwebp
        ../../../3rdparty/libwebp/src
        ../../../3rdparty/libwebp/src/dec
        ../../../3rdparty/libwebp/src/dsp
        ../../../3rdparty/libwebp/src/enc
        ../../../3rdparty/libwebp/src/mux
        ../../../3rdparty/libwebp/src/utils
        ../../../3rdparty/libwebp/src/webp
)

qt_internal_extend_target(QWebpPlugin CONDITION ANDROID AND NOT ANDROID_EMBEDDED AND NOT QT_FEATURE_system_webp
    SOURCES
        ${CMAKE_ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c   # special case
    INCLUDE_DIRECTORIES
        ${CMAKE_ANDROID_NDK}/sources/android/cpufeatures                  # special case
)

# special case begin
set(neon_sources
        ../../../3rdparty/libwebp/src/dsp/alpha_processing_neon.c
        ../../../3rdparty/libwebp/src/dsp/dec_neon.c
        ../../../3rdparty/libwebp/src/dsp/enc_neon.c
        ../../../3rdparty/libwebp/src/dsp/filters_neon.c
        ../../../3rdparty/libwebp/src/dsp/lossless_enc_neon.c
        ../../../3rdparty/libwebp/src/dsp/lossless_neon.c
        ../../../3rdparty/libwebp/src/dsp/rescaler_neon.c
        ../../../3rdparty/libwebp/src/dsp/upsampling_neon.c
        ../../../3rdparty/libwebp/src/dsp/yuv_neon.c
)

if(NOT QT_FEATURE_system_webp)
    if(ANDROID)
        qt_internal_extend_target(QWebpPlugin
            CONDITION (CMAKE_ANDROID_ARM_NEON OR CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
            SOURCES ${neon_sources}
        )
    else()
        # For universal macOS and iOS, and other platforms that support neon
        qt_internal_add_simd_part(QWebpPlugin SIMD neon SOURCES ${neon_sources})
    endif()
endif()
# special case end

#### Keys ignored in scope 6:.:../../../3rdparty:../../../3rdparty/libwebp.pri:INTEGRITY:
# QMAKE_CFLAGS = "-c99"

#### Keys ignored in scope 12:.:../../../3rdparty:../../../3rdparty/libwebp.pri:else:
# QMAKE_EXTRA_COMPILERS = "neon_comp"
# neon_comp.commands = "$$QMAKE_CC" "-c" "$(CFLAGS)" "$$QMAKE_CFLAGS_NEON" "$(INCPATH)" "${QMAKE_FILE_IN}"
# neon_comp.dependency_type = "TYPE_C"
# neon_comp.input = "SOURCES_FOR_NEON"
# neon_comp.name = "compiling[neon]" "${QMAKE_FILE_IN}"
# neon_comp.output = "${QMAKE_VAR_OBJECTS_DIR}${QMAKE_FILE_BASE}$${firstQMAKE_EXT_OBJ}"
# neon_comp.variable_out = "OBJECTS"

#### Keys ignored in scope 13:.:../../../3rdparty:../../../3rdparty/libwebp.pri:MSVC:
# neon_comp.commands = "-Fo${QMAKE_FILE_OUT}"

#### Keys ignored in scope 14:.:../../../3rdparty:../../../3rdparty/libwebp.pri:else:
# neon_comp.commands = "-o" "${QMAKE_FILE_OUT}"

#### Keys ignored in scope 15:.:../../../3rdparty:../../../3rdparty/libwebp.pri:silent:
# neon_comp.commands = "@echo" "compiling[neon]" "${QMAKE_FILE_IN}" "&&"
