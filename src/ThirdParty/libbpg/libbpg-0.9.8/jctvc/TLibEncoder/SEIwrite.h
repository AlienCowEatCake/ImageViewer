/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2014, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#ifndef __SEIWRITE__
#define __SEIWRITE__

#include "SyntaxElementWriter.h"
#include "TLibCommon/SEI.h"

class TComBitIf;

//! \ingroup TLibEncoder
//! \{
class SEIWriter:public SyntaxElementWriter
{
public:
  SEIWriter() {};
  virtual ~SEIWriter() {};

  Void writeSEImessage(TComBitIf& bs, const SEI& sei, TComSPS *sps);

protected:
  Void xWriteSEIpayloadData(TComBitIf& bs, const SEI& sei, TComSPS *sps);
  Void xWriteSEIuserDataUnregistered(const SEIuserDataUnregistered &sei);
  Void xWriteSEIActiveParameterSets(const SEIActiveParameterSets& sei);
  Void xWriteSEIDecodingUnitInfo(const SEIDecodingUnitInfo& sei, TComSPS *sps);
  Void xWriteSEIDecodedPictureHash(const SEIDecodedPictureHash& sei);
  Void xWriteSEIBufferingPeriod(const SEIBufferingPeriod& sei, TComSPS *sps);
  Void xWriteSEIPictureTiming(const SEIPictureTiming& sei, TComSPS *sps);
  TComSPS *m_pSPS;
  Void xWriteSEIRecoveryPoint(const SEIRecoveryPoint& sei);
  Void xWriteSEIFramePacking(const SEIFramePacking& sei);
  Void xWriteSEISegmentedRectFramePacking(const SEISegmentedRectFramePacking& sei);
  Void xWriteSEIDisplayOrientation(const SEIDisplayOrientation &sei);
  Void xWriteSEITemporalLevel0Index(const SEITemporalLevel0Index &sei);
  Void xWriteSEIGradualDecodingRefreshInfo(const SEIGradualDecodingRefreshInfo &sei);
  Void xWriteSEINoDisplay(const SEINoDisplay &sei);
  Void xWriteSEIToneMappingInfo(const SEIToneMappingInfo& sei);
  Void xWriteSEISOPDescription(const SEISOPDescription& sei);
  Void xWriteSEIScalableNesting(TComBitIf& bs, const SEIScalableNesting& sei, TComSPS *sps);
  Void xWriteSEITempMotionConstrainedTileSets(TComBitIf& bs, const SEITempMotionConstrainedTileSets& sei);
  Void xWriteSEITimeCode(const SEITimeCode& sei);
  Void xWriteSEIChromaSamplingFilterHint(const SEIChromaSamplingFilterHint& sei/*, TComSPS *sps*/);
  Void writeUserDefinedCoefficients(const SEIChromaSamplingFilterHint& sei);
  Void xWriteSEIKneeFunctionInfo(const SEIKneeFunctionInfo &sei);
  Void xWriteSEIMasteringDisplayColourVolume( const SEIMasteringDisplayColourVolume& sei);
  Void xWriteByteAlign();
};

//! \}

#endif
