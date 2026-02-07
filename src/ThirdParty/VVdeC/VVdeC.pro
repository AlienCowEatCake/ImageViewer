# URL: https://github.com/fraunhoferhhi/vvdec
# License: 3-Clause Clear BSD License - https://github.com/fraunhoferhhi/vvdec/blob/master/LICENSE.txt

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_vvdec

QT -= core gui

CONFIG -= warn_on
CONFIG += warn_off

THIRDPARTY_VVDEC_PATH = $${PWD}/vvdec-3.1.0
THIRDPARTY_VVDEC_CONFIG_PATH = $${PWD}/config

include(../../Features.pri)
include(../CommonSettings.pri)

INCLUDEPATH = \
    $${THIRDPARTY_VVDEC_CONFIG_PATH} \
    $${THIRDPARTY_VVDEC_PATH}/include \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/FilmGrain \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/Utilities \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/vvdec \
    $${THIRDPARTY_VVDEC_PATH}/thirdparty \
    $${INCLUDEPATH}

# cmake DCMAKE_BUILD_TYPE=Release -DVVDEC_ENABLE_X86_SIMD=OFF -DVVDEC_ENABLE_ARM_SIMD=OFF -DVVDEC_ENABLE_LOONGARCH64_LSX_SIMD=OFF ..

# find ./source/Lib/ -name '*.cpp' | grep -E -v '/(x86|arm)/' | grep -E -v '_(sse|avx)' | grep -v '/wasm_' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_VVDEC_PATH}| ; s|$| \\|'
SOURCES += \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/AdaptiveLoopFilter.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/BitStream.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/Buffer.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/ChromaFormat.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/CodingStructure.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/ContextModelling.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/Contexts.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/InterPrediction.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/InterpolationFilter.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/IntraPrediction.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/LoopFilter.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/MatrixIntraPrediction.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/Mv.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/ParameterSetManager.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/PicListManager.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/PicYuvMD5.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/Picture.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/Quant.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/RdCost.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/Reshape.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/Rom.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/RomLFNST.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/RomTr.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/SEI_internal.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/SampleAdaptiveOffset.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/Slice.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/StatCounter.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/TrQuant.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/TrQuant_EMT.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/Unit.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/UnitPartitioner.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/UnitTools.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/WeightPrediction.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/dtrace.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/AnnexBread.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/BinDecoder.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/CABACReader.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/DecCu.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/DecLib.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/DecLibParser.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/DecLibRecon.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/DecSlice.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/HLSyntaxReader.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/NALread.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/SEIread.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/VLCReader.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/FilmGrain/FilmGrain.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/FilmGrain/FilmGrainImpl.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/Utilities/ThreadPool.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/vvdec/vvdec.cpp \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/vvdec/vvdecimpl.cpp \

# (find ./include -name '*.h' && find ./source/Lib/ -name '*.h') | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_VVDEC_PATH}| ; s|$| \\|'
HEADERS += \
    $${THIRDPARTY_VVDEC_PATH}/include/vvdec/sei.h \
    $${THIRDPARTY_VVDEC_PATH}/include/vvdec/vvdecDecl.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/AdaptiveLoopFilter.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/BitStream.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/Buffer.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/ChromaFormat.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/CodingStructure.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/Common.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/CommonDef.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/ContextModelling.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/Contexts.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/InterPrediction.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/InterpolationFilter.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/IntraPrediction.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/LoopFilter.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/MatrixIntraPrediction.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/MipData.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/MotionInfo.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/Mv.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/NAL.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/ParameterSetManager.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/PicListManager.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/Picture.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/Quant.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/RdCost.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/Reshape.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/Rom.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/SEI_internal.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/SampleAdaptiveOffset.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/Slice.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/StatCounter.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/TimeProfiler.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/TrQuant.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/TrQuant_EMT.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/TypeDef.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/Unit.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/UnitPartitioner.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/UnitTools.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/WeightPrediction.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/arm/BufferARM.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/arm/CommonDefARM.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/arm/RdCostARM.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/arm/neon/sum_neon.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/dtrace.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/dtrace_buffer.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/dtrace_codingstruct.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/dtrace_next.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/x86/AdaptiveLoopFilterX86.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/x86/BufferX86.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/x86/CommonDefX86.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/x86/FixMissingIntrin.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/x86/InterPredX86.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/x86/InterpolationFilterX86.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/x86/IntraPredX86.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/x86/LoopFilterX86.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/x86/PictureX86.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/x86/QuantX86.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/x86/RdCostX86.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/x86/SampleAdaptiveOffsetX86.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/CommonLib/x86/TrafoX86.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/AnnexBread.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/BinDecoder.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/CABACReader.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/DecCu.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/DecLib.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/DecLibParser.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/DecLibRecon.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/DecSlice.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/HLSyntaxReader.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/NALread.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/SEIread.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/DecoderLib/VLCReader.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/FilmGrain/FilmGrain.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/FilmGrain/FilmGrainImpl.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/FilmGrain/FilmGrainImplX86.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/FilmGrain/FilmGrainImpl_X86_SIMD.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/Utilities/ThreadPool.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/libmd5/MD5.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/vvdec/resource.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/vvdec/resource_version.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/vvdec/vvdecimpl.h \
    $${THIRDPARTY_VVDEC_PATH}/source/Lib/vvdec/wasm_bindings.h \
    $${THIRDPARTY_VVDEC_CONFIG_PATH}/vvdec/version.h \
    $${THIRDPARTY_VVDEC_CONFIG_PATH}/vvdec/vvdec.h

TR_EXCLUDE += $${THIRDPARTY_VVDEC_CONFIG_PATH}/* $${THIRDPARTY_VVDEC_PATH}/*

