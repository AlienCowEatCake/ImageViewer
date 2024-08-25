# URL: https://github.com/google/brotli
# License: MIT License - https://github.com/google/brotli/blob/master/LICENSE

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_brotli

QT -= core gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_BROTLI_PATH = $${PWD}/brotli-1.1.0

include(../../Features.pri)
include(../CommonSettings.pri)

*g++*|*clang*|*llvm*|*xcode* {
    QMAKE_CFLAGS += -std=gnu99
}

INCLUDEPATH = \
    $${THIRDPARTY_BROTLI_PATH}/c/include \
    $${THIRDPARTY_BROTLI_PATH}/c/common \
    $${THIRDPARTY_BROTLI_PATH}/c/dec \
    $${THIRDPARTY_BROTLI_PATH}/c/enc \
    $${THIRDPARTY_BROTLI_PATH}/c \
    $${THIRDPARTY_BROTLI_PATH} \
    $${INCLUDEPATH}

# find ./c -name '*.c' | grep -v './c/tools/' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_BROTLI_PATH}| ; s|$| \\|'
SOURCES += \
    $${THIRDPARTY_BROTLI_PATH}/c/common/constants.c \
    $${THIRDPARTY_BROTLI_PATH}/c/common/context.c \
    $${THIRDPARTY_BROTLI_PATH}/c/common/dictionary.c \
    $${THIRDPARTY_BROTLI_PATH}/c/common/platform.c \
    $${THIRDPARTY_BROTLI_PATH}/c/common/shared_dictionary.c \
    $${THIRDPARTY_BROTLI_PATH}/c/common/transform.c \
    $${THIRDPARTY_BROTLI_PATH}/c/dec/bit_reader.c \
    $${THIRDPARTY_BROTLI_PATH}/c/dec/decode.c \
    $${THIRDPARTY_BROTLI_PATH}/c/dec/huffman.c \
    $${THIRDPARTY_BROTLI_PATH}/c/dec/state.c \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/backward_references.c \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/backward_references_hq.c \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/bit_cost.c \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/block_splitter.c \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/brotli_bit_stream.c \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/cluster.c \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/command.c \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/compound_dictionary.c \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/compress_fragment.c \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/compress_fragment_two_pass.c \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/dictionary_hash.c \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/encode.c \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/encoder_dict.c \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/entropy_encode.c \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/fast_log.c \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/histogram.c \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/literal_cost.c \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/memory.c \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/metablock.c \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/static_dict.c \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/utf8_util.c \

# find ./c -name '*.h' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_BROTLI_PATH}| ; s|$| \\|'
HEADERS += \
    $${THIRDPARTY_BROTLI_PATH}/c/common/constants.h \
    $${THIRDPARTY_BROTLI_PATH}/c/common/context.h \
    $${THIRDPARTY_BROTLI_PATH}/c/common/dictionary.h \
    $${THIRDPARTY_BROTLI_PATH}/c/common/platform.h \
    $${THIRDPARTY_BROTLI_PATH}/c/common/shared_dictionary_internal.h \
    $${THIRDPARTY_BROTLI_PATH}/c/common/transform.h \
    $${THIRDPARTY_BROTLI_PATH}/c/common/version.h \
    $${THIRDPARTY_BROTLI_PATH}/c/dec/bit_reader.h \
    $${THIRDPARTY_BROTLI_PATH}/c/dec/huffman.h \
    $${THIRDPARTY_BROTLI_PATH}/c/dec/prefix.h \
    $${THIRDPARTY_BROTLI_PATH}/c/dec/state.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/backward_references.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/backward_references_hq.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/backward_references_inc.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/bit_cost.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/bit_cost_inc.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/block_encoder_inc.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/block_splitter.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/block_splitter_inc.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/brotli_bit_stream.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/cluster.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/cluster_inc.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/command.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/compound_dictionary.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/compress_fragment.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/compress_fragment_two_pass.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/dictionary_hash.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/encoder_dict.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/entropy_encode.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/entropy_encode_static.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/fast_log.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/find_match_length.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/hash.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/hash_composite_inc.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/hash_forgetful_chain_inc.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/hash_longest_match64_inc.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/hash_longest_match_inc.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/hash_longest_match_quickly_inc.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/hash_rolling_inc.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/hash_to_binary_tree_inc.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/histogram.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/histogram_inc.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/literal_cost.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/memory.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/metablock.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/metablock_inc.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/params.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/prefix.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/quality.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/ringbuffer.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/state.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/static_dict.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/static_dict_lut.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/utf8_util.h \
    $${THIRDPARTY_BROTLI_PATH}/c/enc/write_bits.h \
    $${THIRDPARTY_BROTLI_PATH}/c/include/brotli/decode.h \
    $${THIRDPARTY_BROTLI_PATH}/c/include/brotli/encode.h \
    $${THIRDPARTY_BROTLI_PATH}/c/include/brotli/port.h \
    $${THIRDPARTY_BROTLI_PATH}/c/include/brotli/shared_dictionary.h \
    $${THIRDPARTY_BROTLI_PATH}/c/include/brotli/types.h \

TR_EXCLUDE += $${THIRDPARTY_BROTLI_PATH}/*

