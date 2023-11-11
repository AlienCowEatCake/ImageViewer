# URL: https://tukaani.org/xz/
# License: Public Domain - https://git.tukaani.org/?p=xz.git;a=blob;f=COPYING

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_XZUtils

#QT -= core gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_XZUTILS_PATH = $${PWD}/xz-5.4.5
THIRDPARTY_XZUTILS_CONFIG_PATH = $${PWD}/config

include(../CommonSettings.pri)

*g++*|*clang* {
    greaterThan(QT_MAJOR_VERSION, 6) {
        CONFIG += xzutils_c11
    }
    equals(QT_MAJOR_VERSION, 6) : greaterThan(QT_MINOR_VERSION, 5) {
        CONFIG += xzutils_c11
    }
    xzutils_c11 {
        QMAKE_CFLAGS += -std=gnu11
    } else {
        QMAKE_CFLAGS += -std=gnu99
    }
}

INCLUDEPATH = \
    $${THIRDPARTY_XZUTILS_CONFIG_PATH} \
    $${THIRDPARTY_XZUTILS_PATH} \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/api \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/check \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/lz \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/rangecoder \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/lzma \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/delta \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/simple \
    $${THIRDPARTY_XZUTILS_PATH}/src/common \
    $${INCLUDEPATH}

DEFINES += HAVE_CONFIG_H
DEFINES += LZMA_API_STATIC
DEFINES += TUKLIB_SYMBOL_PREFIX=tp_lzma_

# find ./src/liblzma -name '*.c' | egrep -v '(_mt|_tablegen)' | egrep -v '(crc32|crc64|fastpos)(_fast|_table)' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_XZUTILS_PATH}| ; s|$| \\|'
SOURCES += \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/check/check.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/check/crc32_small.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/check/crc64_small.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/check/sha256.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/alone_decoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/alone_encoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/auto_decoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/block_buffer_decoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/block_buffer_encoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/block_decoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/block_encoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/block_header_decoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/block_header_encoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/block_util.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/common.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/easy_buffer_encoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/easy_decoder_memusage.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/easy_encoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/easy_encoder_memusage.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/easy_preset.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/file_info.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/filter_buffer_decoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/filter_buffer_encoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/filter_common.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/filter_decoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/filter_encoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/filter_flags_decoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/filter_flags_encoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/hardware_cputhreads.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/hardware_physmem.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/index.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/index_decoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/index_encoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/index_hash.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/lzip_decoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/microlzma_decoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/microlzma_encoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/outqueue.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/stream_buffer_decoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/stream_buffer_encoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/stream_decoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/stream_encoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/stream_flags_common.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/stream_flags_decoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/stream_flags_encoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/string_conversion.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/vli_decoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/vli_encoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/vli_size.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/delta/delta_common.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/delta/delta_decoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/delta/delta_encoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/lz/lz_decoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/lz/lz_encoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/lz/lz_encoder_mf.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/lzma/lzma2_decoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/lzma/lzma2_encoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/lzma/lzma_decoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/lzma/lzma_encoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/lzma/lzma_encoder_optimum_fast.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/lzma/lzma_encoder_optimum_normal.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/lzma/lzma_encoder_presets.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/rangecoder/price_table.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/simple/arm.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/simple/arm64.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/simple/armthumb.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/simple/ia64.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/simple/powerpc.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/simple/simple_coder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/simple/simple_decoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/simple/simple_encoder.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/simple/sparc.c \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/simple/x86.c \

# find ./src/liblzma -name '*.h' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_XZUTILS_PATH}| ; s|$| \\|'
HEADERS += \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/api/lzma.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/api/lzma/base.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/api/lzma/bcj.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/api/lzma/block.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/api/lzma/check.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/api/lzma/container.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/api/lzma/delta.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/api/lzma/filter.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/api/lzma/hardware.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/api/lzma/index.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/api/lzma/index_hash.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/api/lzma/lzma12.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/api/lzma/stream_flags.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/api/lzma/version.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/api/lzma/vli.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/check/check.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/check/crc32_table_be.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/check/crc32_table_le.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/check/crc64_table_be.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/check/crc64_table_le.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/check/crc_macros.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/alone_decoder.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/block_buffer_encoder.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/block_decoder.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/block_encoder.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/common.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/easy_preset.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/filter_common.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/filter_decoder.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/filter_encoder.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/index.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/index_decoder.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/index_encoder.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/lzip_decoder.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/memcmplen.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/outqueue.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/stream_decoder.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/common/stream_flags_common.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/delta/delta_common.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/delta/delta_decoder.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/delta/delta_encoder.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/delta/delta_private.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/lz/lz_decoder.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/lz/lz_encoder.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/lz/lz_encoder_hash.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/lz/lz_encoder_hash_table.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/lzma/fastpos.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/lzma/lzma2_decoder.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/lzma/lzma2_encoder.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/lzma/lzma_common.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/lzma/lzma_decoder.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/lzma/lzma_encoder.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/lzma/lzma_encoder_private.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/rangecoder/price.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/rangecoder/range_common.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/rangecoder/range_decoder.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/rangecoder/range_encoder.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/simple/simple_coder.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/simple/simple_decoder.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/simple/simple_encoder.h \
    $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/simple/simple_private.h \

HEADERS += \
    $${THIRDPARTY_XZUTILS_CONFIG_PATH}/config.h

TR_EXCLUDE += $${THIRDPARTY_XZUTILS_PATH}/* $${THIRDPARTY_XZUTILS_CONFIG_PATH}/*

