# URL: https://github.com/FLIF-hub/FLIF
# License: GNU LGPL v3+ (encoder) / Apache License 2.0 (decoder) - https://github.com/FLIF-hub/FLIF/blob/master/README.md#license

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_FLIF

QT -= core gui

CONFIG -= warn_on
CONFIG += warn_off

THIRDPARTY_FLIF_PATH = $${PWD}/FLIF-0.4

include(../CommonSettings.pri)

INCLUDEPATH = $${THIRDPARTY_FLIF_PATH}/src/library $${INCLUDEPATH}

DEFINES += DECODER_ONLY LODEPNG_NO_COMPILE_PNG LODEPNG_NO_COMPILE_DISK LODEPNG_NO_COMPILE_ERROR_TEXT LODEPNG_NO_COMPILE_CPP

SOURCES += \
    $${THIRDPARTY_FLIF_PATH}/src/image/color_range.cpp \
    $${THIRDPARTY_FLIF_PATH}/src/image/crc32k.cpp \
\#    $${THIRDPARTY_FLIF_PATH}/src/image/image-metadata.cpp \
\#    $${THIRDPARTY_FLIF_PATH}/src/image/image-pam.cpp \
\#    $${THIRDPARTY_FLIF_PATH}/src/image/image-png.cpp \
\#    $${THIRDPARTY_FLIF_PATH}/src/image/image-pnm.cpp \
\#    $${THIRDPARTY_FLIF_PATH}/src/image/image-rggb.cpp \
\#    $${THIRDPARTY_FLIF_PATH}/src/image/image.cpp \
    $${THIRDPARTY_FLIF_PATH}/src/maniac/bit.cpp \
    $${THIRDPARTY_FLIF_PATH}/src/maniac/chance.cpp \
    $${THIRDPARTY_FLIF_PATH}/src/maniac/symbol.cpp \
    $${THIRDPARTY_FLIF_PATH}/src/transform/factory.cpp \
    $${THIRDPARTY_FLIF_PATH}/src/io.cpp \
    $${THIRDPARTY_FLIF_PATH}/src/common.cpp \
    $${THIRDPARTY_FLIF_PATH}/src/flif-dec.cpp \
    $${THIRDPARTY_FLIF_PATH}/extern/lodepng.cpp \
    $${THIRDPARTY_FLIF_PATH}/src/library/flif-interface_dec.cpp \

HEADERS += \
    $${THIRDPARTY_FLIF_PATH}/src/common.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/compiler-specific.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/config.h \
    $${THIRDPARTY_FLIF_PATH}/src/fileio.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/flif-dec.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/flif-enc.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/flif_config.h \
    $${THIRDPARTY_FLIF_PATH}/src/image/color_range.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/image/crc32k.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/image/image-metadata.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/image/image-pam.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/image/image-png-metadata.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/image/image-png.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/image/image-pnm.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/image/image-rggb.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/image/image.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/io.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/library/flif-interface-private.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/library/flif-interface-private_common.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/library/flif-interface-private_dec.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/library/flif-interface-private_enc.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/library/flif.h \
    $${THIRDPARTY_FLIF_PATH}/src/library/flif_common.h \
    $${THIRDPARTY_FLIF_PATH}/src/library/flif_dec.h \
    $${THIRDPARTY_FLIF_PATH}/src/library/flif_enc.h \
    $${THIRDPARTY_FLIF_PATH}/src/maniac/bit.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/maniac/chance.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/maniac/compound.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/maniac/compound_enc.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/maniac/rac.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/maniac/rac_enc.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/maniac/symbol.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/maniac/symbol_enc.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/maniac/util.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/transform/bounds.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/transform/colorbuckets.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/transform/factory.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/transform/framecombine.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/transform/framedup.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/transform/frameshape.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/transform/palette.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/transform/palette_A.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/transform/palette_C.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/transform/permute.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/transform/transform.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/transform/yc1c2.hpp \
    $${THIRDPARTY_FLIF_PATH}/src/transform/ycocg.hpp \

TR_EXCLUDE += $${THIRDPARTY_FLIF_PATH}/*

