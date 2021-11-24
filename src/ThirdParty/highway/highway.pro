# URL: https://github.com/google/highway
# License: Apache-2.0 License - https://github.com/google/highway/blob/master/LICENSE

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_highway

QT -= core gui

CONFIG -= warn_on
CONFIG += warn_off

THIRDPARTY_HIGHWAY_PATH = $${PWD}/highway-0.15.0

include(../CommonSettings.pri)

INCLUDEPATH = \
    $${THIRDPARTY_HIGHWAY_PATH} \
    $${INCLUDEPATH}

SOURCES += \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/aligned_allocator.cc \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/nanobenchmark.cc \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/targets.cc \

HEADERS += \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/aligned_allocator.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/base.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/cache_control.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/contrib/dot/dot-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/contrib/image/image.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/contrib/math/math-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/contrib/sort/sort-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/detect_compiler_arch.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/detect_targets.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/examples/skeleton-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/examples/skeleton.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/foreach_target.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/highway.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/nanobenchmark.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/arm_neon-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/arm_sve-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/generic_ops-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/rvv-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/scalar-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/set_macros-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/shared-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/wasm_128-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/wasm_256-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/x86_128-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/x86_256-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/x86_512-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/targets.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/tests/hwy_gtest.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/tests/test_util-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/tests/test_util.h \

TR_EXCLUDE += $${THIRDPARTY_HIGHWAY_PATH}/*

