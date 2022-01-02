# URL: https://github.com/facebook/zstd
# License: 3-clause BSD License - https://github.com/facebook/zstd/blob/dev/LICENSE

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_zstd

QT -= core gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_ZSTD_PATH = $${PWD}/zstd-1.5.1
THIRDPARTY_ZSTD_WORKAROUND_PATH = $${PWD}/workaround

include(../CommonSettings.pri)

INCLUDEPATH = $${THIRDPARTY_ZSTD_PATH}/lib $${THIRDPARTY_ZSTD_PATH}/lib/common $${THIRDPARTY_ZSTD_PATH}/lib/legacy $${INCLUDEPATH}

DEFINES += XXH_NAMESPACE=tp_ZSTD_ ZSTD_LEGACY_SUPPORT=1 DYNAMIC_BMI2=0 ZSTD_HAVE_WEAK_SYMBOLS=0 ZSTD_DISABLE_ASM=1

*msvc* {
    DEFINES += ZSTD_HEAPMODE=0
}

SOURCES += \
    $${THIRDPARTY_ZSTD_PATH}/lib/common/debug.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/common/entropy_common.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/common/error_private.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/common/fse_decompress.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/common/pool.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/common/threading.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/common/xxhash.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/common/zstd_common.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/fse_compress.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/hist.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/huf_compress.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/zstd_compress.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/zstd_compress_literals.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/zstd_compress_sequences.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/zstd_compress_superblock.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/zstd_double_fast.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/zstd_fast.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/zstd_lazy.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/zstd_ldm.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/zstd_opt.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/zstdmt_compress.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/decompress/huf_decompress.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/decompress/zstd_ddict.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/decompress/zstd_decompress.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/decompress/zstd_decompress_block.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/deprecated/zbuff_common.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/deprecated/zbuff_compress.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/deprecated/zbuff_decompress.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/dictBuilder/cover.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/dictBuilder/divsufsort.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/dictBuilder/fastcover.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/dictBuilder/zdict.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/legacy/zstd_v01.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/legacy/zstd_v02.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/legacy/zstd_v03.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/legacy/zstd_v04.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/legacy/zstd_v05.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/legacy/zstd_v06.c \
    $${THIRDPARTY_ZSTD_PATH}/lib/legacy/zstd_v07.c \
    $${THIRDPARTY_ZSTD_WORKAROUND_PATH}/wa_xxhash.c

HEADERS += \
    $${THIRDPARTY_ZSTD_PATH}/lib/common/bitstream.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/common/compiler.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/common/cpu.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/common/debug.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/common/error_private.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/common/fse.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/common/huf.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/common/mem.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/common/pool.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/common/threading.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/common/xxhash.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/common/zstd_deps.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/common/zstd_internal.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/common/zstd_trace.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/hist.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/zstd_compress_internal.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/zstd_compress_literals.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/zstd_compress_sequences.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/zstd_compress_superblock.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/zstd_cwksp.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/zstd_double_fast.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/zstd_fast.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/zstd_lazy.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/zstd_ldm.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/zstd_ldm_geartab.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/zstd_opt.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/compress/zstdmt_compress.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/decompress/zstd_ddict.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/decompress/zstd_decompress_block.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/decompress/zstd_decompress_internal.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/deprecated/zbuff.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/dictBuilder/cover.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/dictBuilder/divsufsort.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/legacy/zstd_legacy.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/legacy/zstd_v01.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/legacy/zstd_v02.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/legacy/zstd_v03.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/legacy/zstd_v04.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/legacy/zstd_v05.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/legacy/zstd_v06.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/legacy/zstd_v07.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/zdict.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/zstd.h \
    $${THIRDPARTY_ZSTD_PATH}/lib/zstd_errors.h \

TR_EXCLUDE += $${THIRDPARTY_ZSTD_PATH}/* $${THIRDPARTY_ZSTD_WORKAROUND_PATH}/*

