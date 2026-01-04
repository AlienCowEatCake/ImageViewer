# URL: https://github.com/aous72/OpenJPH
# License: 2-Clause BSD License - https://github.com/aous72/OpenJPH/blob/master/LICENSE

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_openjph

QT -= core gui

CONFIG -= warn_on
CONFIG += warn_off

THIRDPARTY_OPENJPH_PATH = $${PWD}/OpenJPH-0.26.0
THIRDPARTY_OPENJPH_INCLUDE_PATH = $${PWD}/include
THIRDPARTY_OPENJPH_CONFIG_PATH = $${PWD}/config

include(../../Features.pri)
include(../CommonSettings.pri)

INCLUDEPATH = $${THIRDPARTY_OPENJPH_PATH}/src/core/openjph $${INCLUDEPATH}

DEFINES += OJPH_DISABLE_SIMD

SOURCES += \
    $${THIRDPARTY_OPENJPH_CONFIG_PATH}/ojph_mem_c.c

# find ./src/core -name '*.cpp' | LANG=C sort | egrep -v '(_sse|_avx|_ssse3|_wasm)' | sed 's|^\.|    $${THIRDPARTY_OPENJPH_PATH}| ; s|$| \\|'
SOURCES += \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/codestream/ojph_codeblock.cpp \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/codestream/ojph_codeblock_fun.cpp \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/codestream/ojph_codestream.cpp \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/codestream/ojph_codestream_gen.cpp \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/codestream/ojph_codestream_local.cpp \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/codestream/ojph_params.cpp \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/codestream/ojph_precinct.cpp \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/codestream/ojph_resolution.cpp \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/codestream/ojph_subband.cpp \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/codestream/ojph_tile.cpp \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/codestream/ojph_tile_comp.cpp \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/coding/ojph_block_common.cpp \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/coding/ojph_block_decoder32.cpp \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/coding/ojph_block_decoder64.cpp \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/coding/ojph_block_encoder.cpp \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/others/ojph_arch.cpp \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/others/ojph_file.cpp \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/others/ojph_mem.cpp \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/others/ojph_message.cpp \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/transform/ojph_colour.cpp \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/transform/ojph_transform.cpp \

# find ./src/core -name '*.h' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_OPENJPH_PATH}| ; s|$| \\|'
HEADERS += \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/codestream/ojph_bitbuffer_read.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/codestream/ojph_bitbuffer_write.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/codestream/ojph_codeblock.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/codestream/ojph_codeblock_fun.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/codestream/ojph_codestream_local.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/codestream/ojph_params_local.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/codestream/ojph_precinct.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/codestream/ojph_resolution.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/codestream/ojph_subband.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/codestream/ojph_tile.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/codestream/ojph_tile_comp.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/coding/ojph_block_common.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/coding/ojph_block_decoder.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/coding/ojph_block_encoder.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/coding/table0.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/coding/table1.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/openjph/ojph_arch.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/openjph/ojph_arg.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/openjph/ojph_base.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/openjph/ojph_codestream.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/openjph/ojph_defs.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/openjph/ojph_file.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/openjph/ojph_mem.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/openjph/ojph_message.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/openjph/ojph_params.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/openjph/ojph_version.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/transform/ojph_colour.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/transform/ojph_colour_local.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/transform/ojph_transform.h \
    $${THIRDPARTY_OPENJPH_PATH}/src/core/transform/ojph_transform_local.h \

# find ./ -name '*.h' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_OPENJPH_INCLUDE_PATH}| ; s|$| \\|'
HEADERS += \
    $${THIRDPARTY_OPENJPH_INCLUDE_PATH}/openjph/ojph_arch.h \
    $${THIRDPARTY_OPENJPH_INCLUDE_PATH}/openjph/ojph_arg.h \
    $${THIRDPARTY_OPENJPH_INCLUDE_PATH}/openjph/ojph_base.h \
    $${THIRDPARTY_OPENJPH_INCLUDE_PATH}/openjph/ojph_codestream.h \
    $${THIRDPARTY_OPENJPH_INCLUDE_PATH}/openjph/ojph_defs.h \
    $${THIRDPARTY_OPENJPH_INCLUDE_PATH}/openjph/ojph_file.h \
    $${THIRDPARTY_OPENJPH_INCLUDE_PATH}/openjph/ojph_mem.h \
    $${THIRDPARTY_OPENJPH_INCLUDE_PATH}/openjph/ojph_message.h \
    $${THIRDPARTY_OPENJPH_INCLUDE_PATH}/openjph/ojph_params.h \
    $${THIRDPARTY_OPENJPH_INCLUDE_PATH}/openjph/ojph_version.h \

TR_EXCLUDE += $${THIRDPARTY_OPENJPH_PATH}/* $${THIRDPARTY_OPENJPH_INCLUDE_PATH}/*

