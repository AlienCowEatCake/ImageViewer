# URL: https://github.com/libjxl/libjxl
# License: BSD-style License - https://github.com/libjxl/libjxl/blob/main/LICENSE

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_libjxl

QT -= core gui

CONFIG -= warn_on
CONFIG += warn_off

THIRDPARTY_LIBJXL_PATH = $${PWD}/libjxl-0.6.1
THIRDPARTY_LIBJXL_CONFIG_PATH = $${PWD}/config

include(../CommonSettings.pri)
include(../brotli/brotli.pri)
include(../highway/highway.pri)

INCLUDEPATH = \
    $${THIRDPARTY_LIBJXL_PATH}/lib/include \
    $${THIRDPARTY_LIBJXL_PATH} \
    $${THIRDPARTY_LIBJXL_CONFIG_PATH} \
    $${INCLUDEPATH}

DEFINES += JPEGXL_MAJOR_VERSION=0 JPEGXL_MINOR_VERSION=6 JPEGXL_PATCH_VERSION=1

HEADERS += \
    $${THIRDPARTY_LIBJXL_PATH}/lib/include/jxl/butteraugli.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/include/jxl/butteraugli_cxx.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/include/jxl/codestream_header.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/include/jxl/color_encoding.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/include/jxl/decode.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/include/jxl/decode_cxx.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/include/jxl/encode.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/include/jxl/encode_cxx.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/include/jxl/memory_manager.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/include/jxl/parallel_runner.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/include/jxl/types.h \
    $${THIRDPARTY_LIBJXL_CONFIG_PATH}/jxl/jxl_export.h

SOURCES += \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/ac_strategy.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/alpha.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/ans_common.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/aux_out.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/base/cache_aligned.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/base/data_parallel.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/base/descriptive_statistics.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/base/padded_bytes.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/base/status.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/blending.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/chroma_from_luma.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/coeff_order.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/color_encoding_internal.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/color_management.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/compressed_dc.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/convolve.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dct_scales.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_ans.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_cache.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_context_map.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_external_image.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_frame.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_group.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_group_border.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_huffman.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_modular.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_noise.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_patch_dictionary.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_reconstruct.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_upsample.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_xyb.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/decode.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/decode_to_jpeg.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/enc_bit_writer.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/entropy_coder.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/epf.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/fields.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/filters.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/frame_header.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/gauss_blur.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/headers.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/huffman_table.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/icc_codec.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/icc_codec_common.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/image.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/image_bundle.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/image_metadata.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/jpeg/dec_jpeg_data.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/jpeg/dec_jpeg_data_writer.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/jpeg/jpeg_data.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/loop_filter.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/luminance.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/memory_manager_internal.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/modular/encoding/dec_ma.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/modular/encoding/encoding.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/modular/modular_image.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/modular/transform/squeeze.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/modular/transform/transform.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/opsin_params.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/passes_state.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/quant_weights.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/quantizer.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/splines.cc \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/toc.cc \

HEADERS += \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/ac_context.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/ac_strategy.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/alpha.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/ans_common.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/ans_params.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/aux_out.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/aux_out_fwd.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/base/arch_macros.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/base/bits.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/base/byte_order.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/base/cache_aligned.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/base/compiler_specific.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/base/data_parallel.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/base/descriptive_statistics.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/base/file_io.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/base/iaca.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/base/os_macros.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/base/override.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/base/padded_bytes.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/base/profiler.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/base/robust_statistics.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/base/span.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/base/status.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/base/thread_pool_internal.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/blending.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/chroma_from_luma.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/codec_in_out.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/coeff_order.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/coeff_order_fwd.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/color_encoding_internal.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/color_management.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/common.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/compressed_dc.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/convolve-inl.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/convolve.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dct-inl.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dct_block-inl.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dct_scales.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dct_util.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_ans.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_bit_reader.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_cache.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_context_map.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_external_image.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_frame.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_group.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_group_border.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_huffman.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_modular.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_noise.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_params.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_patch_dictionary.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_reconstruct.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_render_pipeline.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_transforms-inl.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_upsample.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_xyb-inl.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/dec_xyb.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/decode_to_jpeg.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/enc_bit_writer.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/entropy_coder.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/epf.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/fast_math-inl.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/field_encodings.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/fields.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/filters.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/filters_internal.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/frame_header.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/gauss_blur.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/headers.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/huffman_table.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/icc_codec.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/icc_codec_common.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/image.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/image_bundle.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/image_metadata.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/image_ops.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/jpeg/dec_jpeg_data.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/jpeg/dec_jpeg_data_writer.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/jpeg/dec_jpeg_output_chunk.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/jpeg/dec_jpeg_serialization_state.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/jpeg/jpeg_data.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/jxl_inspection.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/lehmer_code.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/linalg.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/loop_filter.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/luminance.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/memory_manager_internal.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/modular/encoding/context_predict.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/modular/encoding/dec_ma.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/modular/encoding/encoding.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/modular/encoding/ma_common.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/modular/modular_image.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/modular/options.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/modular/transform/palette.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/modular/transform/rct.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/modular/transform/squeeze.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/modular/transform/transform.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/noise.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/noise_distributions.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/opsin_params.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/passes_state.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/patch_dictionary_internal.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/quant_weights.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/quantizer-inl.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/quantizer.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/rational_polynomial-inl.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/sanitizers.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/splines.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/toc.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/transfer_functions-inl.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/transpose-inl.h \
    $${THIRDPARTY_LIBJXL_PATH}/lib/jxl/xorshift128plus-inl.h \

TR_EXCLUDE += $${THIRDPARTY_LIBJXL_PATH}/*

