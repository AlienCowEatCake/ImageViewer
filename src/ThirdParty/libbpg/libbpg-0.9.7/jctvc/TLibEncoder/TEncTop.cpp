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

/** \file     TEncTop.cpp
    \brief    encoder class
*/

#include "TLibCommon/CommonDef.h"
#include "TEncTop.h"
#include "TEncPic.h"
#include "TLibCommon/TComChromaFormat.h"
#if FAST_BIT_EST
#include "TLibCommon/ContextModel.h"
#endif

//! \ingroup TLibEncoder
//! \{

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TEncTop::TEncTop()
{
  m_iPOCLast          = -1;
  m_iNumPicRcvd       =  0;
  m_uiNumAllPicCoded  =  0;
  m_pppcRDSbacCoder   =  NULL;
  m_pppcBinCoderCABAC =  NULL;
  m_cRDGoOnSbacCoder.init( &m_cRDGoOnBinCoderCABAC );
#if ENC_DEC_TRACE
  if (g_hTrace == NULL)
  {
    g_hTrace = fopen( "TraceEnc.txt", "wb" );
  }
  g_bJustDoIt = g_bEncDecTraceDisable;
  g_nSymbolCounter = 0;
#endif

  m_iMaxRefPicNum     = 0;

#if FAST_BIT_EST
  ContextModel::buildNextStateTable();
#endif
}

TEncTop::~TEncTop()
{
#if ENC_DEC_TRACE
  if (g_hTrace != stdout)
  {
    fclose( g_hTrace );
  }
#endif
}

Void TEncTop::create ()
{
  // initialize global variables
  initROM();

  // create processing unit classes
  m_cGOPEncoder.        create( );
  m_cSliceEncoder.      create( getSourceWidth(), getSourceHeight(), m_chromaFormatIDC, g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
  m_cCuEncoder.         create( g_uiMaxCUDepth, g_uiMaxCUWidth, g_uiMaxCUHeight, m_chromaFormatIDC );
  if (m_bUseSAO)
  {
    m_cEncSAO.create( getSourceWidth(), getSourceHeight(), m_chromaFormatIDC, g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth, m_saoOffsetBitShift[CHANNEL_TYPE_LUMA], m_saoOffsetBitShift[CHANNEL_TYPE_CHROMA] );
#if SAO_ENCODE_ALLOW_USE_PREDEBLOCK
    m_cEncSAO.createEncData(getSaoCtuBoundary());
#else
    m_cEncSAO.createEncData();
#endif
  }
#if ADAPTIVE_QP_SELECTION
  if (m_bUseAdaptQpSelect)
  {
    m_cTrQuant.initSliceQpDelta();
  }
#endif

  m_cLoopFilter.create( g_uiMaxCUDepth );

  if ( m_RCEnableRateControl )
  {
    m_cRateCtrl.init( m_framesToBeEncoded, m_RCTargetBitrate, m_iFrameRate, m_iGOPSize, m_iSourceWidth, m_iSourceHeight,
                      g_uiMaxCUWidth, g_uiMaxCUHeight, m_RCKeepHierarchicalBit, m_RCUseLCUSeparateModel, m_GOPList );
  }

  m_pppcRDSbacCoder = new TEncSbac** [g_uiMaxCUDepth+1];
#if FAST_BIT_EST
  m_pppcBinCoderCABAC = new TEncBinCABACCounter** [g_uiMaxCUDepth+1];
#else
  m_pppcBinCoderCABAC = new TEncBinCABAC** [g_uiMaxCUDepth+1];
#endif

  for ( Int iDepth = 0; iDepth < g_uiMaxCUDepth+1; iDepth++ )
  {
    m_pppcRDSbacCoder[iDepth] = new TEncSbac* [CI_NUM];
#if FAST_BIT_EST
    m_pppcBinCoderCABAC[iDepth] = new TEncBinCABACCounter* [CI_NUM];
#else
    m_pppcBinCoderCABAC[iDepth] = new TEncBinCABAC* [CI_NUM];
#endif

    for (Int iCIIdx = 0; iCIIdx < CI_NUM; iCIIdx ++ )
    {
      m_pppcRDSbacCoder[iDepth][iCIIdx] = new TEncSbac;
#if FAST_BIT_EST
      m_pppcBinCoderCABAC [iDepth][iCIIdx] = new TEncBinCABACCounter;
#else
      m_pppcBinCoderCABAC [iDepth][iCIIdx] = new TEncBinCABAC;
#endif
      m_pppcRDSbacCoder   [iDepth][iCIIdx]->init( m_pppcBinCoderCABAC [iDepth][iCIIdx] );
    }
  }
}

Void TEncTop::destroy ()
{
  // destroy processing unit classes
  m_cGOPEncoder.        destroy();
  m_cSliceEncoder.      destroy();
  m_cCuEncoder.         destroy();
  if (m_cSPS.getUseSAO())
  {
    m_cEncSAO.destroyEncData();
    m_cEncSAO.destroy();
  }
  m_cLoopFilter.        destroy();
  m_cRateCtrl.          destroy();
  Int iDepth;
  for ( iDepth = 0; iDepth < g_uiMaxCUDepth+1; iDepth++ )
  {
    for (Int iCIIdx = 0; iCIIdx < CI_NUM; iCIIdx ++ )
    {
      delete m_pppcRDSbacCoder[iDepth][iCIIdx];
      delete m_pppcBinCoderCABAC[iDepth][iCIIdx];
    }
  }

  for ( iDepth = 0; iDepth < g_uiMaxCUDepth+1; iDepth++ )
  {
    delete [] m_pppcRDSbacCoder[iDepth];
    delete [] m_pppcBinCoderCABAC[iDepth];
  }

  delete [] m_pppcRDSbacCoder;
  delete [] m_pppcBinCoderCABAC;

  // destroy ROM
  destroyROM();

  return;
}

Void TEncTop::init(Bool isFieldCoding)
{
  // initialize SPS
  xInitSPS();

  // set the VPS profile information
  *m_cVPS.getPTL() = *m_cSPS.getPTL();
  m_cVPS.getTimingInfo()->setTimingInfoPresentFlag       ( false );

  m_cRdCost.setCostMode(m_costMode);

  // initialize PPS
  m_cPPS.setSPS(&m_cSPS);
  xInitPPS();
  xInitRPS(isFieldCoding);

  xInitPPSforTiles();

  // initialize processing unit classes
  m_cGOPEncoder.  init( this );
  m_cSliceEncoder.init( this );
  m_cCuEncoder.   init( this );

  // initialize transform & quantization class
  m_pcCavlcCoder = getCavlcCoder();

  m_cTrQuant.init( 1 << m_uiQuadtreeTULog2MaxSize,
                   m_useRDOQ,
                   m_useRDOQTS,
                   true
                  ,m_useTransformSkipFast
#if ADAPTIVE_QP_SELECTION
                  ,m_bUseAdaptQpSelect
#endif
                  );

  // initialize encoder search class
  m_cSearch.init( this, &m_cTrQuant, m_iSearchRange, m_bipredSearchRange, m_iFastSearch, 0, &m_cEntropyCoder, &m_cRdCost, getRDSbacCoder(), getRDGoOnSbacCoder() );

  m_iMaxRefPicNum = 0;
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TEncTop::deletePicBuffer()
{
  TComList<TComPic*>::iterator iterPic = m_cListPic.begin();
  Int iSize = Int( m_cListPic.size() );

  for ( Int i = 0; i < iSize; i++ )
  {
    TComPic* pcPic = *(iterPic++);

    pcPic->destroy();
    delete pcPic;
    pcPic = NULL;
  }
}

/**
 - Application has picture buffer list with size of GOP + 1
 - Picture buffer list acts like as ring buffer
 - End of the list has the latest picture
 .
 \param   flush               cause encoder to encode a partial GOP
 \param   pcPicYuvOrg         original YUV picture
 \retval  rcListPicYuvRecOut  list of reconstruction YUV pictures
 \retval  rcListBitstreamOut  list of output bitstreams
 \retval  iNumEncoded         number of encoded pictures
 */
Void TEncTop::encode( Bool flush, TComPicYuv* pcPicYuvOrg, TComPicYuv* pcPicYuvTrueOrg, const InputColourSpaceConversion snrCSC, TComList<TComPicYuv*>& rcListPicYuvRecOut, std::list<AccessUnit>& accessUnitsOut, Int& iNumEncoded )
{
  if (pcPicYuvOrg != NULL)
  {
    // get original YUV
    TComPic* pcPicCurr = NULL;

    xGetNewPicBuffer( pcPicCurr );
    pcPicYuvOrg->copyToPic( pcPicCurr->getPicYuvOrg() );
    pcPicYuvTrueOrg->copyToPic( pcPicCurr->getPicYuvTrueOrg() );

    // compute image characteristics
    if ( getUseAdaptiveQP() )
    {
      m_cPreanalyzer.xPreanalyze( dynamic_cast<TEncPic*>( pcPicCurr ) );
    }
  }

  if ((m_iNumPicRcvd == 0) || (!flush && (m_iPOCLast != 0) && (m_iNumPicRcvd != m_iGOPSize) && (m_iGOPSize != 0)))
  {
    iNumEncoded = 0;
    return;
  }

  if ( m_RCEnableRateControl )
  {
    m_cRateCtrl.initRCGOP( m_iNumPicRcvd );
  }

  // compress GOP
  m_cGOPEncoder.compressGOP(m_iPOCLast, m_iNumPicRcvd, m_cListPic, rcListPicYuvRecOut, accessUnitsOut, false, false, snrCSC, m_printFrameMSE);

  if ( m_RCEnableRateControl )
  {
    m_cRateCtrl.destroyRCGOP();
  }

  iNumEncoded         = m_iNumPicRcvd;
  m_iNumPicRcvd       = 0;
  m_uiNumAllPicCoded += iNumEncoded;
}

/**------------------------------------------------
 Separate interlaced frame into two fields
 -------------------------------------------------**/
Void separateFields(Pel* org, Pel* dstField, UInt stride, UInt width, UInt height, Bool isTop)
{
  if (!isTop)
  {
    org += stride;
  }
  for (Int y = 0; y < height>>1; y++)
  {
    for (Int x = 0; x < width; x++)
    {
      dstField[x] = org[x];
    }

    dstField += stride;
    org += stride*2;
  }

}

Void TEncTop::encode(Bool flush, TComPicYuv* pcPicYuvOrg, TComPicYuv* pcPicYuvTrueOrg, const InputColourSpaceConversion snrCSC, TComList<TComPicYuv*>& rcListPicYuvRecOut, std::list<AccessUnit>& accessUnitsOut, Int& iNumEncoded, Bool isTff)
{
  iNumEncoded = 0;

  for (Int fieldNum=0; fieldNum<2; fieldNum++)
  {
    if (pcPicYuvOrg)
    {

      /* -- field initialization -- */
      const Bool isTopField=isTff==(fieldNum==0);

      TComPic *pcField;
      xGetNewPicBuffer( pcField );
      pcField->setReconMark (false);                     // where is this normally?

      if (fieldNum==1)                                   // where is this normally?
      {
        TComPicYuv* rpcPicYuvRec;

        // org. buffer
        if ( rcListPicYuvRecOut.size() >= (UInt)m_iGOPSize+1 ) // need to maintain field 0 in list of RecOuts while processing field 1. Hence +1 on m_iGOPSize.
        {
          rpcPicYuvRec = rcListPicYuvRecOut.popFront();
        }
        else
        {
          rpcPicYuvRec = new TComPicYuv;
          rpcPicYuvRec->create( m_iSourceWidth, m_iSourceHeight, m_chromaFormatIDC, g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth);
        }
        rcListPicYuvRecOut.pushBack( rpcPicYuvRec );
      }

      pcField->getSlice(0)->setPOC( m_iPOCLast );        // superfluous?
      pcField->getPicYuvRec()->setBorderExtension(false);// where is this normally?

      pcField->setTopField(isTopField);                  // interlaced requirement

      for (UInt componentIndex = 0; componentIndex < pcPicYuvOrg->getNumberValidComponents(); componentIndex++)
      {
        const ComponentID component = ComponentID(componentIndex);
        const UInt stride = pcPicYuvOrg->getStride(component);

        separateFields((pcPicYuvOrg->getBuf(component) + pcPicYuvOrg->getMarginX(component) + (pcPicYuvOrg->getMarginY(component) * stride)),
                       pcField->getPicYuvOrg()->getAddr(component),
                       pcPicYuvOrg->getStride(component),
                       pcPicYuvOrg->getWidth(component),
                       pcPicYuvOrg->getHeight(component),
                       isTopField);

        separateFields((pcPicYuvTrueOrg->getBuf(component) + pcPicYuvTrueOrg->getMarginX(component) + (pcPicYuvTrueOrg->getMarginY(component) * stride)),
                       pcField->getPicYuvTrueOrg()->getAddr(component),
                       pcPicYuvTrueOrg->getStride(component),
                       pcPicYuvTrueOrg->getWidth(component),
                       pcPicYuvTrueOrg->getHeight(component),
                       isTopField);
      }

      // compute image characteristics
      if ( getUseAdaptiveQP() )
      {
        m_cPreanalyzer.xPreanalyze( dynamic_cast<TEncPic*>( pcField ) );
      }
    }

    if ( m_iNumPicRcvd && ((flush&&fieldNum==1) || (m_iPOCLast/2)==0 || m_iNumPicRcvd==m_iGOPSize ) )
    {
      // compress GOP
      m_cGOPEncoder.compressGOP(m_iPOCLast, m_iNumPicRcvd, m_cListPic, rcListPicYuvRecOut, accessUnitsOut, true, isTff, snrCSC, m_printFrameMSE);

      iNumEncoded += m_iNumPicRcvd;
      m_uiNumAllPicCoded += m_iNumPicRcvd;
      m_iNumPicRcvd = 0;
    }
  }
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

/**
 - Application has picture buffer list with size of GOP + 1
 - Picture buffer list acts like as ring buffer
 - End of the list has the latest picture
 .
 \retval rpcPic obtained picture buffer
 */
Void TEncTop::xGetNewPicBuffer ( TComPic*& rpcPic )
{
  TComSlice::sortPicList(m_cListPic);

  if (m_cListPic.size() >= (UInt)(m_iGOPSize + getMaxDecPicBuffering(MAX_TLAYER-1) + 2) )
  {
    TComList<TComPic*>::iterator iterPic  = m_cListPic.begin();
    Int iSize = Int( m_cListPic.size() );
    for ( Int i = 0; i < iSize; i++ )
    {
      rpcPic = *(iterPic++);
      if(rpcPic->getSlice(0)->isReferenced() == false)
      {
        break;
      }
    }
  }
  else
  {
    if ( getUseAdaptiveQP() )
    {
      TEncPic* pcEPic = new TEncPic;
      pcEPic->create( m_iSourceWidth, m_iSourceHeight, m_chromaFormatIDC, g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth, m_cPPS.getMaxCuDQPDepth()+1, m_conformanceWindow, m_defaultDisplayWindow, m_numReorderPics);
      rpcPic = pcEPic;
    }
    else
    {
      rpcPic = new TComPic;
      rpcPic->create( m_iSourceWidth, m_iSourceHeight, m_chromaFormatIDC, g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth, m_conformanceWindow, m_defaultDisplayWindow, m_numReorderPics, false );
    }

    m_cListPic.pushBack( rpcPic );
  }
  rpcPic->setReconMark (false);

  m_iPOCLast++;
  m_iNumPicRcvd++;

  rpcPic->getSlice(0)->setPOC( m_iPOCLast );
  // mark it should be extended
  rpcPic->getPicYuvRec()->setBorderExtension(false);
}

Void TEncTop::xInitSPS()
{
  ProfileTierLevel& profileTierLevel = *m_cSPS.getPTL()->getGeneralPTL();
  profileTierLevel.setLevelIdc(m_level);
  profileTierLevel.setTierFlag(m_levelTier);
  profileTierLevel.setProfileIdc(m_profile);
  profileTierLevel.setProfileCompatibilityFlag(m_profile, 1);
  profileTierLevel.setProgressiveSourceFlag(m_progressiveSourceFlag);
  profileTierLevel.setInterlacedSourceFlag(m_interlacedSourceFlag);
  profileTierLevel.setNonPackedConstraintFlag(m_nonPackedConstraintFlag);
  profileTierLevel.setFrameOnlyConstraintFlag(m_frameOnlyConstraintFlag);
  profileTierLevel.setBitDepthConstraint(m_bitDepthConstraintValue);
  profileTierLevel.setChromaFormatConstraint(m_chromaFormatConstraintValue);
  profileTierLevel.setIntraConstraintFlag(m_intraConstraintFlag);
  profileTierLevel.setLowerBitRateConstraintFlag(m_lowerBitRateConstraintFlag);

  if ((m_profile == Profile::MAIN10) && (g_bitDepth[CHANNEL_TYPE_LUMA] == 8) && (g_bitDepth[CHANNEL_TYPE_CHROMA] == 8))
  {
    /* The above constraint is equal to Profile::MAIN */
    profileTierLevel.setProfileCompatibilityFlag(Profile::MAIN, 1);
  }
  if (m_profile == Profile::MAIN)
  {
    /* A Profile::MAIN10 decoder can always decode Profile::MAIN */
    profileTierLevel.setProfileCompatibilityFlag(Profile::MAIN10, 1);
  }
  /* XXX: should Main be marked as compatible with still picture? */
  /* XXX: may be a good idea to refactor the above into a function
   * that chooses the actual compatibility based upon options */

  m_cSPS.setPicWidthInLumaSamples         ( m_iSourceWidth      );
  m_cSPS.setPicHeightInLumaSamples        ( m_iSourceHeight     );
  m_cSPS.setConformanceWindow             ( m_conformanceWindow );
  m_cSPS.setMaxCUWidth    ( g_uiMaxCUWidth      );
  m_cSPS.setMaxCUHeight   ( g_uiMaxCUHeight     );
  m_cSPS.setMaxCUDepth    ( g_uiMaxCUDepth      );
  m_cSPS.setChromaFormatIdc( m_chromaFormatIDC);

  Int minCUSize = m_cSPS.getMaxCUWidth() >> ( m_cSPS.getMaxCUDepth()-g_uiAddCUDepth );
  Int log2MinCUSize = 0;
  while(minCUSize > 1)
  {
    minCUSize >>= 1;
    log2MinCUSize++;
  }

  m_cSPS.setLog2MinCodingBlockSize(log2MinCUSize);
  m_cSPS.setLog2DiffMaxMinCodingBlockSize(m_cSPS.getMaxCUDepth()-g_uiAddCUDepth-getMaxCUDepthOffset(m_cSPS.getChromaFormatIdc(), m_cSPS.getQuadtreeTULog2MinSize()));

  m_cSPS.setPCMLog2MinSize (m_uiPCMLog2MinSize);
  m_cSPS.setUsePCM        ( m_usePCM           );
  m_cSPS.setPCMLog2MaxSize( m_pcmLog2MaxSize  );

  m_cSPS.setQuadtreeTULog2MaxSize( m_uiQuadtreeTULog2MaxSize );
  m_cSPS.setQuadtreeTULog2MinSize( m_uiQuadtreeTULog2MinSize );
  m_cSPS.setQuadtreeTUMaxDepthInter( m_uiQuadtreeTUMaxDepthInter    );
  m_cSPS.setQuadtreeTUMaxDepthIntra( m_uiQuadtreeTUMaxDepthIntra    );

  m_cSPS.setTMVPFlagsPresent(false);

  m_cSPS.setMaxTrSize   ( 1 << m_uiQuadtreeTULog2MaxSize );

  Int i;

  for (i = 0; i < g_uiMaxCUDepth-g_uiAddCUDepth; i++ )
  {
    m_cSPS.setAMPAcc( i, m_useAMP );
    //m_cSPS.setAMPAcc( i, 1 );
  }

  m_cSPS.setUseAMP ( m_useAMP );

  for (i = g_uiMaxCUDepth-g_uiAddCUDepth; i < g_uiMaxCUDepth; i++ )
  {
    m_cSPS.setAMPAcc(i, 0);
  }


  for (UInt channelType = 0; channelType < MAX_NUM_CHANNEL_TYPE; channelType++)
  {
    m_cSPS.setBitDepth    (ChannelType(channelType), g_bitDepth[channelType]            );
    m_cSPS.setQpBDOffset  (ChannelType(channelType), (6 * (g_bitDepth[channelType] - 8)));
    m_cSPS.setPCMBitDepth (ChannelType(channelType), g_PCMBitDepth[channelType]         );
  }

  m_cSPS.setUseExtendedPrecision(m_useExtendedPrecision);
  m_cSPS.setUseHighPrecisionPredictionWeighting(m_useHighPrecisionPredictionWeighting);

  m_cSPS.setUseSAO( m_bUseSAO );
  m_cSPS.setUseResidualRotation(m_useResidualRotation);
  m_cSPS.setUseSingleSignificanceMapContext(m_useSingleSignificanceMapContext);
  m_cSPS.setUseGolombRiceParameterAdaptation(m_useGolombRiceParameterAdaptation);
  m_cSPS.setAlignCABACBeforeBypass(m_alignCABACBeforeBypass);

  for (UInt signallingModeIndex = 0; signallingModeIndex < NUMBER_OF_RDPCM_SIGNALLING_MODES; signallingModeIndex++)
  {
    m_cSPS.setUseResidualDPCM(RDPCMSignallingMode(signallingModeIndex), m_useResidualDPCM[signallingModeIndex]);
  }

  m_cSPS.setMaxTLayers( m_maxTempLayer );
  m_cSPS.setTemporalIdNestingFlag( ( m_maxTempLayer == 1 ) ? true : false );

  for ( i = 0; i < min(m_cSPS.getMaxTLayers(),(UInt) MAX_TLAYER); i++ )
  {
    m_cSPS.setMaxDecPicBuffering(m_maxDecPicBuffering[i], i);
    m_cSPS.setNumReorderPics(m_numReorderPics[i], i);
  }

  m_cSPS.setPCMFilterDisableFlag  ( m_bPCMFilterDisableFlag );
  m_cSPS.setDisableIntraReferenceSmoothing( m_disableIntraReferenceSmoothing );
  m_cSPS.setScalingListFlag ( (m_useScalingListId == 0) ? 0 : 1 );
  m_cSPS.setUseStrongIntraSmoothing( m_useStrongIntraSmoothing );
  m_cSPS.setVuiParametersPresentFlag(getVuiParametersPresentFlag());

  if (m_cSPS.getVuiParametersPresentFlag())
  {
    TComVUI* pcVUI = m_cSPS.getVuiParameters();
    pcVUI->setAspectRatioInfoPresentFlag(getAspectRatioInfoPresentFlag());
    pcVUI->setAspectRatioIdc(getAspectRatioIdc());
    pcVUI->setSarWidth(getSarWidth());
    pcVUI->setSarHeight(getSarHeight());
    pcVUI->setOverscanInfoPresentFlag(getOverscanInfoPresentFlag());
    pcVUI->setOverscanAppropriateFlag(getOverscanAppropriateFlag());
    pcVUI->setVideoSignalTypePresentFlag(getVideoSignalTypePresentFlag());
    pcVUI->setVideoFormat(getVideoFormat());
    pcVUI->setVideoFullRangeFlag(getVideoFullRangeFlag());
    pcVUI->setColourDescriptionPresentFlag(getColourDescriptionPresentFlag());
    pcVUI->setColourPrimaries(getColourPrimaries());
    pcVUI->setTransferCharacteristics(getTransferCharacteristics());
    pcVUI->setMatrixCoefficients(getMatrixCoefficients());
    pcVUI->setChromaLocInfoPresentFlag(getChromaLocInfoPresentFlag());
    pcVUI->setChromaSampleLocTypeTopField(getChromaSampleLocTypeTopField());
    pcVUI->setChromaSampleLocTypeBottomField(getChromaSampleLocTypeBottomField());
    pcVUI->setNeutralChromaIndicationFlag(getNeutralChromaIndicationFlag());
    pcVUI->setDefaultDisplayWindow(getDefaultDisplayWindow());
    pcVUI->setFrameFieldInfoPresentFlag(getFrameFieldInfoPresentFlag());
    pcVUI->setFieldSeqFlag(false);
    pcVUI->setHrdParametersPresentFlag(false);
    pcVUI->getTimingInfo()->setPocProportionalToTimingFlag(getPocProportionalToTimingFlag());
    pcVUI->getTimingInfo()->setNumTicksPocDiffOneMinus1   (getNumTicksPocDiffOneMinus1()   );
    pcVUI->setBitstreamRestrictionFlag(getBitstreamRestrictionFlag());
    pcVUI->setTilesFixedStructureFlag(getTilesFixedStructureFlag());
    pcVUI->setMotionVectorsOverPicBoundariesFlag(getMotionVectorsOverPicBoundariesFlag());
    pcVUI->setMinSpatialSegmentationIdc(getMinSpatialSegmentationIdc());
    pcVUI->setMaxBytesPerPicDenom(getMaxBytesPerPicDenom());
    pcVUI->setMaxBitsPerMinCuDenom(getMaxBitsPerMinCuDenom());
    pcVUI->setLog2MaxMvLengthHorizontal(getLog2MaxMvLengthHorizontal());
    pcVUI->setLog2MaxMvLengthVertical(getLog2MaxMvLengthVertical());
  }
}

Void TEncTop::xInitPPS()
{
  m_cPPS.setConstrainedIntraPred( m_bUseConstrainedIntraPred );
  Bool bUseDQP = (getMaxCuDQPDepth() > 0)? true : false;

  if((getMaxDeltaQP() != 0 )|| getUseAdaptiveQP())
  {
    bUseDQP = true;
  }

  if (m_costMode==COST_SEQUENCE_LEVEL_LOSSLESS || m_costMode==COST_LOSSLESS_CODING) bUseDQP=false;

  if(bUseDQP)
  {
    m_cPPS.setUseDQP(true);
    m_cPPS.setMaxCuDQPDepth( m_iMaxCuDQPDepth );
    m_cPPS.setMinCuDQPSize( m_cPPS.getSPS()->getMaxCUWidth() >> ( m_cPPS.getMaxCuDQPDepth()) );
  }
  else
  {
    m_cPPS.setUseDQP(false);
    m_cPPS.setMaxCuDQPDepth( 0 );
    m_cPPS.setMinCuDQPSize( m_cPPS.getSPS()->getMaxCUWidth() >> ( m_cPPS.getMaxCuDQPDepth()) );
  }

  if ( m_maxCUChromaQpAdjustmentDepth >= 0 )
  {
    m_cPPS.setMaxCuChromaQpAdjDepth(m_maxCUChromaQpAdjustmentDepth);
    m_cPPS.setChromaQpAdjTableAt(1, 6, 6);
    /* todo, insert table entries from command line (NB, 0 should not be touched) */
  }
  else
  {
    m_cPPS.setMaxCuChromaQpAdjDepth(0);
    m_cPPS.clearChromaQpAdjTable();
  }

  if ( m_RCEnableRateControl )
  {
    m_cPPS.setUseDQP(true);
    m_cPPS.setMaxCuDQPDepth( 0 );
    m_cPPS.setMinCuDQPSize( m_cPPS.getSPS()->getMaxCUWidth() >> ( m_cPPS.getMaxCuDQPDepth()) );
  }

  m_cPPS.setMinCuChromaQpAdjSize( m_cPPS.getSPS()->getMaxCUWidth() >> ( m_cPPS.getMaxCuChromaQpAdjDepth()) );

  m_cPPS.setQpOffset(COMPONENT_Cb, m_chromaCbQpOffset );
  m_cPPS.setQpOffset(COMPONENT_Cr, m_chromaCrQpOffset );

  m_cPPS.setNumSubstreams(m_iWaveFrontSubstreams);
  m_cPPS.setEntropyCodingSyncEnabledFlag( m_iWaveFrontSynchro > 0 );
  m_cPPS.setTilesEnabledFlag( (m_iNumColumnsMinus1 > 0 || m_iNumRowsMinus1 > 0) );
  m_cPPS.setUseWP( m_useWeightedPred );
  m_cPPS.setWPBiPred( m_useWeightedBiPred );
  m_cPPS.setUseCrossComponentPrediction(m_useCrossComponentPrediction);
  m_cPPS.setSaoOffsetBitShift(CHANNEL_TYPE_LUMA,   m_saoOffsetBitShift[CHANNEL_TYPE_LUMA  ]);
  m_cPPS.setSaoOffsetBitShift(CHANNEL_TYPE_CHROMA, m_saoOffsetBitShift[CHANNEL_TYPE_CHROMA]);
  m_cPPS.setOutputFlagPresentFlag( false );
  m_cPPS.setSignHideFlag(getSignHideFlag());
  if ( getDeblockingFilterMetric() )
  {
    m_cPPS.setDeblockingFilterControlPresentFlag (true);
    m_cPPS.setDeblockingFilterOverrideEnabledFlag(true);
    m_cPPS.setPicDisableDeblockingFilterFlag(false);
    m_cPPS.setDeblockingFilterBetaOffsetDiv2(0);
    m_cPPS.setDeblockingFilterTcOffsetDiv2(0);
  }
  else
  {
    m_cPPS.setDeblockingFilterControlPresentFlag (m_DeblockingFilterControlPresent );
  }
  m_cPPS.setLog2ParallelMergeLevelMinus2   (m_log2ParallelMergeLevelMinus2 );
  m_cPPS.setCabacInitPresentFlag(CABAC_INIT_PRESENT_FLAG);
  m_cPPS.setLoopFilterAcrossSlicesEnabledFlag( m_bLFCrossSliceBoundaryFlag );

  Int histogram[MAX_NUM_REF + 1];
  for( Int i = 0; i <= MAX_NUM_REF; i++ )
  {
    histogram[i]=0;
  }
  for( Int i = 0; i < getGOPSize(); i++)
  {
    assert(getGOPEntry(i).m_numRefPicsActive >= 0 && getGOPEntry(i).m_numRefPicsActive <= MAX_NUM_REF);
    histogram[getGOPEntry(i).m_numRefPicsActive]++;
  }

  Int maxHist=-1;
  Int bestPos=0;
  for( Int i = 0; i <= MAX_NUM_REF; i++ )
  {
    if(histogram[i]>maxHist)
    {
      maxHist=histogram[i];
      bestPos=i;
    }
  }
  assert(bestPos <= 15);
  m_cPPS.setNumRefIdxL0DefaultActive(bestPos);
  m_cPPS.setNumRefIdxL1DefaultActive(bestPos);
  m_cPPS.setTransquantBypassEnableFlag(getTransquantBypassEnableFlag());
  m_cPPS.setUseTransformSkip( m_useTransformSkip );
  m_cPPS.setTransformSkipLog2MaxSize( m_transformSkipLog2MaxSize  );

  if (m_sliceSegmentMode != NO_SLICES)
  {
    m_cPPS.setDependentSliceSegmentsEnabledFlag( true );
  }
}

//Function for initializing m_RPSList, a list of TComReferencePictureSet, based on the GOPEntry objects read from the config file.
Void TEncTop::xInitRPS(Bool isFieldCoding)
{
  TComReferencePictureSet*      rps;

  m_cSPS.createRPSList(getGOPSize() + m_extraRPSs + 1);
  TComRPSList* rpsList = m_cSPS.getRPSList();

  for( Int i = 0; i < getGOPSize()+m_extraRPSs; i++)
  {
    GOPEntry ge = getGOPEntry(i);
    rps = rpsList->getReferencePictureSet(i);
    rps->setNumberOfPictures(ge.m_numRefPics);
    rps->setNumRefIdc(ge.m_numRefIdc);
    Int numNeg = 0;
    Int numPos = 0;
    for( Int j = 0; j < ge.m_numRefPics; j++)
    {
      rps->setDeltaPOC(j,ge.m_referencePics[j]);
      rps->setUsed(j,ge.m_usedByCurrPic[j]);
      if(ge.m_referencePics[j]>0)
      {
        numPos++;
      }
      else
      {
        numNeg++;
      }
    }
    rps->setNumberOfNegativePictures(numNeg);
    rps->setNumberOfPositivePictures(numPos);

    // handle inter RPS intialization from the config file.
#if AUTO_INTER_RPS
    rps->setInterRPSPrediction(ge.m_interRPSPrediction > 0);  // not very clean, converting anything > 0 to true.
    rps->setDeltaRIdxMinus1(0);                               // index to the Reference RPS is always the previous one.
    TComReferencePictureSet*     RPSRef = rpsList->getReferencePictureSet(i-1);  // get the reference RPS

    if (ge.m_interRPSPrediction == 2)  // Automatic generation of the inter RPS idc based on the RIdx provided.
    {
      Int deltaRPS = getGOPEntry(i-1).m_POC - ge.m_POC;  // the ref POC - current POC
      Int numRefDeltaPOC = RPSRef->getNumberOfPictures();

      rps->setDeltaRPS(deltaRPS);           // set delta RPS
      rps->setNumRefIdc(numRefDeltaPOC+1);  // set the numRefIdc to the number of pictures in the reference RPS + 1.
      Int count=0;
      for (Int j = 0; j <= numRefDeltaPOC; j++ ) // cycle through pics in reference RPS.
      {
        Int RefDeltaPOC = (j<numRefDeltaPOC)? RPSRef->getDeltaPOC(j): 0;  // if it is the last decoded picture, set RefDeltaPOC = 0
        rps->setRefIdc(j, 0);
        for (Int k = 0; k < rps->getNumberOfPictures(); k++ )  // cycle through pics in current RPS.
        {
          if (rps->getDeltaPOC(k) == ( RefDeltaPOC + deltaRPS))  // if the current RPS has a same picture as the reference RPS.
          {
              rps->setRefIdc(j, (rps->getUsed(k)?1:2));
              count++;
              break;
          }
        }
      }
      if (count != rps->getNumberOfPictures())
      {
        printf("Warning: Unable fully predict all delta POCs using the reference RPS index given in the config file.  Setting Inter RPS to false for this RPS.\n");
        rps->setInterRPSPrediction(0);
      }
    }
    else if (ge.m_interRPSPrediction == 1)  // inter RPS idc based on the RefIdc values provided in config file.
    {
      rps->setDeltaRPS(ge.m_deltaRPS);
      rps->setNumRefIdc(ge.m_numRefIdc);
      for (Int j = 0; j < ge.m_numRefIdc; j++ )
      {
        rps->setRefIdc(j, ge.m_refIdc[j]);
      }
#if WRITE_BACK
      // the folowing code overwrite the deltaPOC and Used by current values read from the config file with the ones
      // computed from the RefIdc.  A warning is printed if they are not identical.
      numNeg = 0;
      numPos = 0;
      TComReferencePictureSet      RPSTemp;  // temporary variable

      for (Int j = 0; j < ge.m_numRefIdc; j++ )
      {
        if (ge.m_refIdc[j])
        {
          Int deltaPOC = ge.m_deltaRPS + ((j < RPSRef->getNumberOfPictures())? RPSRef->getDeltaPOC(j) : 0);
          RPSTemp.setDeltaPOC((numNeg+numPos),deltaPOC);
          RPSTemp.setUsed((numNeg+numPos),ge.m_refIdc[j]==1?1:0);
          if (deltaPOC<0)
          {
            numNeg++;
          }
          else
          {
            numPos++;
          }
        }
      }
      if (numNeg != rps->getNumberOfNegativePictures())
      {
        printf("Warning: number of negative pictures in RPS is different between intra and inter RPS specified in the config file.\n");
        rps->setNumberOfNegativePictures(numNeg);
        rps->setNumberOfPictures(numNeg+numPos);
      }
      if (numPos != rps->getNumberOfPositivePictures())
      {
        printf("Warning: number of positive pictures in RPS is different between intra and inter RPS specified in the config file.\n");
        rps->setNumberOfPositivePictures(numPos);
        rps->setNumberOfPictures(numNeg+numPos);
      }
      RPSTemp.setNumberOfPictures(numNeg+numPos);
      RPSTemp.setNumberOfNegativePictures(numNeg);
      RPSTemp.sortDeltaPOC();     // sort the created delta POC before comparing
      // check if Delta POC and Used are the same
      // print warning if they are not.
      for (Int j = 0; j < ge.m_numRefIdc; j++ )
      {
        if (RPSTemp.getDeltaPOC(j) != rps->getDeltaPOC(j))
        {
          printf("Warning: delta POC is different between intra RPS and inter RPS specified in the config file.\n");
          rps->setDeltaPOC(j,RPSTemp.getDeltaPOC(j));
        }
        if (RPSTemp.getUsed(j) != rps->getUsed(j))
        {
          printf("Warning: Used by Current in RPS is different between intra and inter RPS specified in the config file.\n");
          rps->setUsed(j,RPSTemp.getUsed(j));
        }
      }
#endif
    }
#else
    rps->setInterRPSPrediction(ge.m_interRPSPrediction);
    if (ge.m_interRPSPrediction)
    {
      rps->setDeltaRIdxMinus1(0);
      rps->setDeltaRPS(ge.m_deltaRPS);
      rps->setNumRefIdc(ge.m_numRefIdc);
      for (Int j = 0; j < ge.m_numRefIdc; j++ )
      {
        rps->setRefIdc(j, ge.m_refIdc[j]);
      }
#if WRITE_BACK
      // the folowing code overwrite the deltaPOC and Used by current values read from the config file with the ones
      // computed from the RefIdc.  This is not necessary if both are identical. Currently there is no check to see if they are identical.
      numNeg = 0;
      numPos = 0;
      TComReferencePictureSet*     RPSRef = m_RPSList.getReferencePictureSet(i-1);

      for (Int j = 0; j < ge.m_numRefIdc; j++ )
      {
        if (ge.m_refIdc[j])
        {
          Int deltaPOC = ge.m_deltaRPS + ((j < RPSRef->getNumberOfPictures())? RPSRef->getDeltaPOC(j) : 0);
          rps->setDeltaPOC((numNeg+numPos),deltaPOC);
          rps->setUsed((numNeg+numPos),ge.m_refIdc[j]==1?1:0);
          if (deltaPOC<0)
          {
            numNeg++;
          }
          else
          {
            numPos++;
          }
        }
      }
      rps->setNumberOfNegativePictures(numNeg);
      rps->setNumberOfPositivePictures(numPos);
      rps->sortDeltaPOC();
#endif
    }
#endif //INTER_RPS_AUTO
  }
  //In case of field coding, we need to set special parameters for the first bottom field of the sequence, since it is not specified in the cfg file.
  //The position = GOPSize + extraRPSs which is (a priori) unused is reserved for this field in the RPS.
  if (isFieldCoding)
  {
    rps = rpsList->getReferencePictureSet(getGOPSize()+m_extraRPSs);
    rps->setNumberOfPictures(1);
    rps->setNumberOfNegativePictures(1);
    rps->setNumberOfPositivePictures(0);
    rps->setNumberOfLongtermPictures(0);
    rps->setDeltaPOC(0,-1);
    rps->setPOC(0,0);
    rps->setUsed(0,true);
    rps->setInterRPSPrediction(false);
    rps->setDeltaRIdxMinus1(0);
    rps->setDeltaRPS(0);
    rps->setNumRefIdc(0);
  }
}

   // This is a function that
   // determines what Reference Picture Set to use
   // for a specific slice (with POC = POCCurr)
Void TEncTop::selectReferencePictureSet(TComSlice* slice, Int POCCurr, Int GOPid )
{
  slice->setRPSidx(GOPid);

  for(Int extraNum=m_iGOPSize; extraNum<m_extraRPSs+m_iGOPSize; extraNum++)
  {
    if(m_uiIntraPeriod > 0 && getDecodingRefreshType() > 0)
    {
      Int POCIndex = POCCurr%m_uiIntraPeriod;
      if(POCIndex == 0)
      {
        POCIndex = m_uiIntraPeriod;
      }
      if(POCIndex == m_GOPList[extraNum].m_POC)
      {
        slice->setRPSidx(extraNum);
      }
    }
    else
    {
      if(POCCurr==m_GOPList[extraNum].m_POC)
      {
        slice->setRPSidx(extraNum);
      }
    }
  }

  if(POCCurr == 1 && slice->getPic()->isField())
  {
    slice->setRPSidx(m_iGOPSize+m_extraRPSs);
  }

  slice->setRPS(getSPS()->getRPSList()->getReferencePictureSet(slice->getRPSidx()));
  slice->getRPS()->setNumberOfPictures(slice->getRPS()->getNumberOfNegativePictures()+slice->getRPS()->getNumberOfPositivePictures());
}

Int TEncTop::getReferencePictureSetIdxForSOP(TComSlice* slice, Int POCCurr, Int GOPid )
{
  Int rpsIdx = GOPid;

  for(Int extraNum=m_iGOPSize; extraNum<m_extraRPSs+m_iGOPSize; extraNum++)
  {
    if(m_uiIntraPeriod > 0 && getDecodingRefreshType() > 0)
    {
      Int POCIndex = POCCurr%m_uiIntraPeriod;
      if(POCIndex == 0)
      {
        POCIndex = m_uiIntraPeriod;
      }
      if(POCIndex == m_GOPList[extraNum].m_POC)
      {
        rpsIdx = extraNum;
      }
    }
    else
    {
      if(POCCurr==m_GOPList[extraNum].m_POC)
      {
        rpsIdx = extraNum;
      }
    }
  }

  return rpsIdx;
}

Void  TEncTop::xInitPPSforTiles()
{
  m_cPPS.setTileUniformSpacingFlag( m_tileUniformSpacingFlag );
  m_cPPS.setNumTileColumnsMinus1( m_iNumColumnsMinus1 );
  m_cPPS.setNumTileRowsMinus1( m_iNumRowsMinus1 );
  if( !m_tileUniformSpacingFlag )
  {
    m_cPPS.setTileColumnWidth( m_tileColumnWidth );
    m_cPPS.setTileRowHeight( m_tileRowHeight );
  }
  m_cPPS.setLoopFilterAcrossTilesEnabledFlag( m_loopFilterAcrossTilesEnabledFlag );

  // # substreams is "per tile" when tiles are independent.
  if (m_iWaveFrontSynchro )
  {
    m_cPPS.setNumSubstreams(m_iWaveFrontSubstreams * (m_iNumColumnsMinus1+1));
  }
  else
  {
    m_cPPS.setNumSubstreams((m_iNumRowsMinus1+1) * (m_iNumColumnsMinus1+1));
  }
}

Void  TEncCfg::xCheckGSParameters()
{
  Int   iWidthInCU = ( m_iSourceWidth%g_uiMaxCUWidth ) ? m_iSourceWidth/g_uiMaxCUWidth + 1 : m_iSourceWidth/g_uiMaxCUWidth;
  Int   iHeightInCU = ( m_iSourceHeight%g_uiMaxCUHeight ) ? m_iSourceHeight/g_uiMaxCUHeight + 1 : m_iSourceHeight/g_uiMaxCUHeight;
  UInt  uiCummulativeColumnWidth = 0;
  UInt  uiCummulativeRowHeight = 0;

  //check the column relative parameters
  if( m_iNumColumnsMinus1 >= (1<<(LOG2_MAX_NUM_COLUMNS_MINUS1+1)) )
  {
    printf( "The number of columns is larger than the maximum allowed number of columns.\n" );
    exit( EXIT_FAILURE );
  }

  if( m_iNumColumnsMinus1 >= iWidthInCU )
  {
    printf( "The current picture can not have so many columns.\n" );
    exit( EXIT_FAILURE );
  }

  if( m_iNumColumnsMinus1 && !m_tileUniformSpacingFlag )
  {
    for(Int i=0; i<m_iNumColumnsMinus1; i++)
    {
      uiCummulativeColumnWidth += m_tileColumnWidth[i];
    }

    if( uiCummulativeColumnWidth >= iWidthInCU )
    {
      printf( "The width of the column is too large.\n" );
      exit( EXIT_FAILURE );
    }
  }

  //check the row relative parameters
  if( m_iNumRowsMinus1 >= (1<<(LOG2_MAX_NUM_ROWS_MINUS1+1)) )
  {
    printf( "The number of rows is larger than the maximum allowed number of rows.\n" );
    exit( EXIT_FAILURE );
  }

  if( m_iNumRowsMinus1 >= iHeightInCU )
  {
    printf( "The current picture can not have so many rows.\n" );
    exit( EXIT_FAILURE );
  }

  if( m_iNumRowsMinus1 && !m_tileUniformSpacingFlag )
  {
    for(Int i=0; i<m_iNumRowsMinus1; i++)
      uiCummulativeRowHeight += m_tileRowHeight[i];

    if( uiCummulativeRowHeight >= iHeightInCU )
    {
      printf( "The height of the row is too large.\n" );
      exit( EXIT_FAILURE );
    }
  }
}
//! \}
