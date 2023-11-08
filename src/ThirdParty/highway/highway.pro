# URL: https://github.com/google/highway
# License: Apache-2.0 License - https://github.com/google/highway/blob/master/LICENSE

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_highway

QT -= core gui

CONFIG -= warn_on
CONFIG += warn_off

THIRDPARTY_HIGHWAY_PATH = $${PWD}/highway-1.0.7

include(../CommonSettings.pri)

INCLUDEPATH = \
    $${THIRDPARTY_HIGHWAY_PATH} \
    $${INCLUDEPATH}

# find ./hwy -name '*.cc' | egrep -v '(/tests/|/contrib/|/examples/|_test)' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_HIGHWAY_PATH}| ; s|$| \\|'
SOURCES += \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/aligned_allocator.cc \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/nanobenchmark.cc \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/per_target.cc \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/print.cc \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/targets.cc \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/timer.cc \

# find ./hwy -name '*.h' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_HIGHWAY_PATH}| ; s|$| \\|'
HEADERS += \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/aligned_allocator.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/base.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/cache_control.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/contrib/algo/copy-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/contrib/algo/find-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/contrib/algo/transform-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/contrib/bit_pack/bit_pack-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/contrib/dot/dot-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/contrib/image/image.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/contrib/math/math-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/contrib/sort/algo-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/contrib/sort/order.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/contrib/sort/result-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/contrib/sort/shared-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/contrib/sort/sorting_networks-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/contrib/sort/traits-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/contrib/sort/traits128-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/contrib/sort/vqsort-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/contrib/sort/vqsort.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/contrib/unroller/unroller-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/detect_compiler_arch.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/detect_targets.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/examples/skeleton-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/examples/skeleton.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/foreach_target.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/highway.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/highway_export.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/nanobenchmark.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/arm_neon-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/arm_sve-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/emu128-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/generic_ops-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/ppc_vsx-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/rvv-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/scalar-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/set_macros-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/shared-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/tuple-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/wasm_128-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/wasm_256-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/x86_128-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/x86_256-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/ops/x86_512-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/per_target.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/print-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/print.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/robust_statistics.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/targets.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/tests/hwy_gtest.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/tests/test_util-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/tests/test_util.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/timer-inl.h \
    $${THIRDPARTY_HIGHWAY_PATH}/hwy/timer.h \

TR_EXCLUDE += $${THIRDPARTY_HIGHWAY_PATH}/*

