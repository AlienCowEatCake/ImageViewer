/*
 * Copyright (c) 2016, Alliance for Open Media. All rights reserved
 *
 * This source code is subject to the terms of the BSD 2 Clause License and
 * the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
 * was not distributed with this source code in the LICENSE file, you can
 * obtain it at www.aomedia.org/license/software. If the Alliance for Open
 * Media Patent License 1.0 was not distributed with this source code in the
 * PATENTS file, you can obtain it at www.aomedia.org/license/patent.
 */
#include "aom/aom_codec.h"
static const char* const cfg = "cmake ../ -G \"Unix Makefiles\" -DAOM_TARGET_CPU=generic -DCONFIG_GCOV=0 -DCONFIG_GPROF=0 -DCONFIG_LIBYUV=0 -DCONFIG_MULTITHREAD=0 -DCONFIG_OS_SUPPORT=0 -DCONFIG_RUNTIME_CPU_DETECT=0 -DCONFIG_WEBM_IO=0 -DCONFIG_TUNE_VMAF=0 -DCONFIG_TUNE_BUTTERAUGLI=0 -DENABLE_DOCS=0 -DENABLE_EXAMPLES=0 -DENABLE_TESTDATA=0 -DENABLE_TESTS=0 -DENABLE_TOOLS=0 -DENABLE_NEON=0 -DENABLE_VSX=0 -DENABLE_MMX=0 -DENABLE_SSE=0 -DENABLE_SSE2=0 -DENABLE_SSE3=0 -DENABLE_SSSE3=0 -DENABLE_SSE4_1=0 -DENABLE_SSE4_2=0 -DENABLE_AVX=0 -DENABLE_AVX2=0";
const char *aom_codec_build_config(void) {return cfg;}
