# URL: https://www.libraw.org/download
# License: GNU LGPL v2.1 or CDDL v1.0 - https://github.com/LibRaw/LibRaw/blob/master/COPYRIGHT

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_LibRaw

#QT -= core gui

CONFIG -= warn_on
CONFIG += warn_off

THIRDPARTY_LIBRAW_PATH = $${PWD}/LibRaw-0.22.0

include(../../Features.pri)
include(../CommonSettings.pri)
include(../libjpeg/libjpeg.pri)
include(../LittleCMS2/LittleCMS2.pri)
include(../zlib/zlib.pri)

INCLUDEPATH = $${THIRDPARTY_LIBRAW_PATH} $${INCLUDEPATH}

DEFINES += LIBRAW_NOTHREADS USE_X3FTOOLS USE_6BY9RPI

!disable_libjpeg {
    DEFINES += USE_JPEG USE_JPEG8
} else {
    DEFINES += NO_JPEG
}

!disable_liblcms2 {
    DEFINES += USE_LCMS2
} else {
    DEFINES += NO_LCMS
}

!disable_zlib {
    DEFINES += USE_ZLIB
} else {
    DEFINES += NO_ZLIB
}

win32 {
    DEFINES += LIBRAW_NODLL
}

*clang* {
    DEFINES *= _LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR _LIBCPP_DISABLE_DEPRECATION_WARNINGS
}

# find ./src -name '*.cpp' | egrep -v '(_ph)' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_LIBRAW_PATH}| ; s|$| \\|'
SOURCES += \
    $${THIRDPARTY_LIBRAW_PATH}/src/decoders/canon_600.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/decoders/crx.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/decoders/decoders_dcraw.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/decoders/decoders_libraw.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/decoders/decoders_libraw_dcrdefs.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/decoders/dng.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/decoders/fp_dng.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/decoders/fuji_compressed.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/decoders/generic.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/decoders/kodak_decoders.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/decoders/load_mfbacks.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/decoders/olympus14.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/decoders/pana8.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/decoders/smal.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/decoders/sonycc.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/decoders/unpack.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/decoders/unpack_thumb.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/decompressors/losslessjpeg.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/demosaic/aahd_demosaic.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/demosaic/ahd_demosaic.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/demosaic/dcb_demosaic.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/demosaic/dht_demosaic.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/demosaic/misc_demosaic.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/demosaic/xtrans_demosaic.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/integration/dngsdk_glue.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/integration/rawspeed_glue.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/libraw_c_api.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/libraw_datastream.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/adobepano.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/canon.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/ciff.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/cr3_parser.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/epson.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/exif_gps.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/fuji.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/hasselblad_model.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/identify.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/identify_tools.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/kodak.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/leica.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/makernotes.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/mediumformat.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/minolta.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/misc_parsers.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/nikon.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/normalize_model.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/olympus.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/p1.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/pentax.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/samsung.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/sony.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/metadata/tiff.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/postprocessing/aspect_ratio.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/postprocessing/dcraw_process.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/postprocessing/mem_image.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/postprocessing/postprocessing_aux.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/postprocessing/postprocessing_utils.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/postprocessing/postprocessing_utils_dcrdefs.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/preprocessing/ext_preprocess.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/preprocessing/raw2image.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/preprocessing/subtract_black.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/tables/cameralist.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/tables/colorconst.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/tables/colordata.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/tables/wblists.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/utils/curves.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/utils/decoder_info.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/utils/init_close_utils.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/utils/open.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/utils/phaseone_processing.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/utils/read_utils.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/utils/thumb_utils.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/utils/utils_dcraw.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/utils/utils_libraw.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/write/apply_profile.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/write/file_write.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/write/tiff_writer.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/x3f/x3f_parse_process.cpp \
    $${THIRDPARTY_LIBRAW_PATH}/src/x3f/x3f_utils_patched.cpp \

# find . -name '*.h' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_LIBRAW_PATH}| ; s|$| \\|'
HEADERS += \
    $${THIRDPARTY_LIBRAW_PATH}/RawSpeed3/rawspeed3_c_api/rawspeed3_capi.h \
    $${THIRDPARTY_LIBRAW_PATH}/internal/dcraw_defs.h \
    $${THIRDPARTY_LIBRAW_PATH}/internal/dcraw_fileio_defs.h \
    $${THIRDPARTY_LIBRAW_PATH}/internal/defines.h \
    $${THIRDPARTY_LIBRAW_PATH}/internal/dmp_include.h \
    $${THIRDPARTY_LIBRAW_PATH}/internal/libraw_cameraids.h \
    $${THIRDPARTY_LIBRAW_PATH}/internal/libraw_checked_buffer.h \
    $${THIRDPARTY_LIBRAW_PATH}/internal/libraw_cxx_defs.h \
    $${THIRDPARTY_LIBRAW_PATH}/internal/libraw_internal_funcs.h \
    $${THIRDPARTY_LIBRAW_PATH}/internal/losslessjpeg.h \
    $${THIRDPARTY_LIBRAW_PATH}/internal/var_defines.h \
    $${THIRDPARTY_LIBRAW_PATH}/internal/x3f_tools.h \
    $${THIRDPARTY_LIBRAW_PATH}/libraw/libraw.h \
    $${THIRDPARTY_LIBRAW_PATH}/libraw/libraw_alloc.h \
    $${THIRDPARTY_LIBRAW_PATH}/libraw/libraw_const.h \
    $${THIRDPARTY_LIBRAW_PATH}/libraw/libraw_datastream.h \
    $${THIRDPARTY_LIBRAW_PATH}/libraw/libraw_internal.h \
    $${THIRDPARTY_LIBRAW_PATH}/libraw/libraw_types.h \
    $${THIRDPARTY_LIBRAW_PATH}/libraw/libraw_version.h \

TR_EXCLUDE += $${THIRDPARTY_LIBRAW_PATH}/*

