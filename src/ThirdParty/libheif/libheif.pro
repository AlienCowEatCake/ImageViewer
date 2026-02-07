# URL: https://github.com/strukturag/libheif
# License: GNU LGPL v3 - https://github.com/strukturag/libheif/blob/master/COPYING

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_libheif

#QT -= core gui

CONFIG -= warn_on
CONFIG += warn_off

THIRDPARTY_LIBHEIF_PATH = $${PWD}/libheif-1.21.2
THIRDPARTY_LIBHEIF_INCLUDE_PATH = $${PWD}/include

include(../../Features.pri)
include(../CommonSettings.pri)
include(../aom/aom.pri)
include(../libjpeg/libjpeg.pri)
include(../OpenJPEG/OpenJPEG.pri)
include(../libde265/libde265.pri)
include(../zlib/zlib.pri)
include(../brotli/brotli.pri)
include(../LibWebP/LibWebP.pri)
include(../OpenJPH/OpenJPH.pri)
include(../OpenH264/OpenH264.pri)
include(../VVdeC/VVdeC.pri)

INCLUDEPATH = $${THIRDPARTY_LIBHEIF_PATH} $${THIRDPARTY_LIBHEIF_PATH}/libheif $${THIRDPARTY_LIBHEIF_PATH}/libheif/api $${THIRDPARTY_LIBHEIF_INCLUDE_PATH} $${INCLUDEPATH}

DEFINES += LIBHEIF_STATIC_BUILD
DEFINES += HAVE_INTTYPES_H WITH_UNCOMPRESSED_CODEC=1 HAVE_STDDEF_H
!*msvc*: DEFINES += HAVE_UNISTD_H
!disable_zlib: DEFINES += HAVE_ZLIB=1
!disable_brotli: DEFINES += HAVE_BROTLI=1
!disable_libwebp: DEFINES += HAVE_LIBSHARPYUV=1

# find ./libheif -name '*.cc' | egrep -v '(_fuzzer|_libde265|_aom|_dav1d|_rav1e|_svt|_x265|_ffmpeg|_jpeg|_openjpeg|_kvazaar|_unix|_windows|_vvdec|_vvenc|_uvg266|_openjph|_openh264|_webcodecs|_x264)' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_LIBHEIF_PATH}| ; s|$| \\|'
SOURCES += \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_aux_images.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_brands.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_color.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_context.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_decoding.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_encoding.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_entity_groups.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_experimental.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_image.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_image_handle.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_items.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_library.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_metadata.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_plugin.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_properties.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_regions.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_security.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_sequences.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_tai_timestamps.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_text.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_tiling.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_uncompressed.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/bitstream.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/box.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/brands.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/avc_boxes.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/avc_dec.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/avc_enc.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/avif_boxes.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/avif_dec.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/avif_enc.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/decoder.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/encoder.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/hevc_boxes.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/hevc_dec.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/hevc_enc.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/jpeg2000_boxes.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/jpeg2000_dec.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/jpeg2000_enc.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/jpeg_boxes.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/jpeg_dec.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/jpeg_enc.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/uncompressed/decoder_abstract.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/uncompressed/decoder_component_interleave.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/uncompressed/decoder_mixed_interleave.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/uncompressed/decoder_pixel_interleave.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/uncompressed/decoder_row_interleave.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/uncompressed/decoder_tile_component_interleave.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/uncompressed/unc_boxes.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/uncompressed/unc_codec.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/uncompressed/unc_dec.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/uncompressed/unc_enc.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/vvc_boxes.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/vvc_dec.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/vvc_enc.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/color-conversion/alpha.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/color-conversion/chroma_sampling.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/color-conversion/colorconversion.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/color-conversion/hdr_sdr.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/color-conversion/monochrome.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/color-conversion/rgb2rgb.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/color-conversion/rgb2yuv.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/color-conversion/rgb2yuv_sharp.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/color-conversion/yuv2rgb.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/common_utils.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/compression_brotli.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/compression_zlib.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/context.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/error.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/file.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/file_layout.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/avc.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/avif.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/grid.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/hevc.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/iden.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/image_item.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/jpeg.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/jpeg2000.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/mask_image.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/overlay.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/tiled.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/unc_image.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/vvc.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/init.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/logging.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/mini.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/nclx.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/pixelimage.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugin_registry.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/decoder_uncompressed.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/encoder_mask.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/encoder_uncompressed.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/nalu_utils.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/region.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/security_limits.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/sequences/chunk.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/sequences/seq_boxes.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/sequences/track.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/sequences/track_metadata.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/sequences/track_visual.cc \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/text.cc \

# find ./libheif -name '*.cc' | egrep '(_windows)' | LANG=C sort | sed 's|^\.|        $${THIRDPARTY_LIBHEIF_PATH}| ; s|$| \\|'
# find ./libheif -name '*.cc' | egrep '(_unix)' | LANG=C sort | sed 's|^\.|        $${THIRDPARTY_LIBHEIF_PATH}| ; s|$| \\|'
#win32 {
#    SOURCES += \
#        $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins_windows.cc
#} else {
#    SOURCES += \
#        $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins_unix.cc
#}

# find ./libheif -name '*.cc' | egrep '(_libde265)' | LANG=C sort | sed 's|^\.|        $${THIRDPARTY_LIBHEIF_PATH}| ; s|$| \\|'
!disable_libde265 {
    SOURCES += \
        $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/decoder_libde265.cc \

    DEFINES += HAVE_LIBDE265=1
}

# find ./libheif -name '*.cc' | egrep '(_aom)' | LANG=C sort | sed 's|^\.|        $${THIRDPARTY_LIBHEIF_PATH}| ; s|$| \\|'
!disable_aom {
    SOURCES += \
        $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/decoder_aom.cc \
        $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/encoder_aom.cc \

    DEFINES += HAVE_AOM=1 HAVE_AOM_DECODER=1 HAVE_AOM_ENCODER=1
}

# find ./libheif -name '*.cc' | egrep '(_jpeg)' | LANG=C sort | sed 's|^\.|        $${THIRDPARTY_LIBHEIF_PATH}| ; s|$| \\|'
!disable_libjpeg {
    SOURCES += \
        $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/decoder_jpeg.cc \
        $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/encoder_jpeg.cc \

    DEFINES += HAVE_JPEG_DECODER=1 HAVE_JPEG_ENCODER=1
}

# find ./libheif -name '*.cc' | egrep '(_openjpeg)' | LANG=C sort | sed 's|^\.|        $${THIRDPARTY_LIBHEIF_PATH}| ; s|$| \\|'
!disable_openjpeg {
    SOURCES += \
        $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/decoder_openjpeg.cc \
        $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/encoder_openjpeg.cc \

    DEFINES += HAVE_OPENJPEG_DECODER=1 HAVE_OPENJPEG_ENCODER=1
}

# find ./libheif -name '*.cc' | egrep '(_openjph)' | LANG=C sort | sed 's|^\.|        $${THIRDPARTY_LIBHEIF_PATH}| ; s|$| \\|'
!disable_openjph {
    SOURCES += \
        $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/encoder_openjph.cc \

    DEFINES += HAVE_OPENJPH_ENCODER=1
}

# find ./libheif -name '*.cc' | egrep '(_openh264)' | LANG=C sort | sed 's|^\.|        $${THIRDPARTY_LIBHEIF_PATH}| ; s|$| \\|'
!disable_openh264 {
    SOURCES += \
        $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/decoder_openh264.cc \

    DEFINES += HAVE_OpenH264_DECODER=1
}

# find ./libheif -name '*.cc' | egrep '(_vvdec)' | LANG=C sort | sed 's|^\.|        $${THIRDPARTY_LIBHEIF_PATH}| ; s|$| \\|'
!disable_vvdec {
    SOURCES += \
        $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/decoder_vvdec.cc \

    DEFINES += HAVE_VVDEC=1
}

# find ./libheif -name '*.h' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_LIBHEIF_PATH}| ; s|$| \\|'
HEADERS += \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_aux_images.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_brands.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_color.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_context.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_cxx.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_decoding.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_emscripten.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_encoding.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_entity_groups.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_error.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_experimental.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_image.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_image_handle.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_items.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_library.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_metadata.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_plugin.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_properties.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_regions.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_security.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_sequences.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_tai_timestamps.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_text.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_tiling.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api/libheif/heif_uncompressed.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/api_structs.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/bitstream.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/box.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/brands.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/avc_boxes.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/avc_dec.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/avc_enc.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/avif_boxes.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/avif_dec.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/avif_enc.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/decoder.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/encoder.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/hevc_boxes.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/hevc_dec.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/hevc_enc.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/jpeg2000_boxes.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/jpeg2000_dec.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/jpeg2000_enc.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/jpeg_boxes.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/jpeg_dec.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/jpeg_enc.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/uncompressed/decoder_abstract.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/uncompressed/decoder_component_interleave.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/uncompressed/decoder_mixed_interleave.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/uncompressed/decoder_pixel_interleave.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/uncompressed/decoder_row_interleave.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/uncompressed/decoder_tile_component_interleave.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/uncompressed/unc_boxes.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/uncompressed/unc_codec.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/uncompressed/unc_dec.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/uncompressed/unc_enc.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/uncompressed/unc_types.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/vvc_boxes.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/vvc_dec.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/codecs/vvc_enc.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/color-conversion/alpha.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/color-conversion/chroma_sampling.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/color-conversion/colorconversion.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/color-conversion/hdr_sdr.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/color-conversion/monochrome.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/color-conversion/rgb2rgb.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/color-conversion/rgb2yuv.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/color-conversion/rgb2yuv_sharp.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/color-conversion/yuv2rgb.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/common_utils.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/compression.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/context.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/error.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/file.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/file_layout.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/avc.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/avif.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/grid.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/hevc.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/iden.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/image_item.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/jpeg.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/jpeg2000.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/mask_image.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/overlay.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/tiled.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/unc_image.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/image-items/vvc.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/init.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/logging.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/mdat_data.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/mini.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/nclx.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/pixelimage.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugin_registry.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/decoder_aom.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/decoder_dav1d.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/decoder_ffmpeg.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/decoder_jpeg.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/decoder_libde265.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/decoder_openh264.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/decoder_openjpeg.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/decoder_uncompressed.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/decoder_vvdec.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/decoder_webcodecs.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/encoder_aom.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/encoder_jpeg.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/encoder_kvazaar.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/encoder_mask.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/encoder_openjpeg.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/encoder_openjph.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/encoder_rav1e.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/encoder_svt.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/encoder_uncompressed.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/encoder_uvg266.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/encoder_vvenc.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/encoder_x264.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/encoder_x265.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins/nalu_utils.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins_unix.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/plugins_windows.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/region.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/security_limits.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/sequences/chunk.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/sequences/seq_boxes.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/sequences/track.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/sequences/track_metadata.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/sequences/track_visual.h \
    $${THIRDPARTY_LIBHEIF_PATH}/libheif/text.h \
    $${THIRDPARTY_LIBHEIF_INCLUDE_PATH}/libheif/heif_version.h

TR_EXCLUDE += $${THIRDPARTY_LIBHEIF_PATH}/*

