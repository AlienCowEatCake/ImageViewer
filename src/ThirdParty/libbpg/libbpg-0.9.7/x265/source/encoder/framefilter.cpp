/*****************************************************************************
 * Copyright (C) 2013 x265 project
 *
 * Authors: Chung Shin Yee <shinyee@multicorewareinc.com>
 *          Min Chen <chenm003@163.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 * For more information, contact us at license @ x265.com.
 *****************************************************************************/

#include "common.h"
#include "frame.h"
#include "framedata.h"
#include "encoder.h"
#include "framefilter.h"
#include "frameencoder.h"
#include "wavefront.h"

using namespace X265_NS;

static uint64_t computeSSD(pixel *fenc, pixel *rec, intptr_t stride, uint32_t width, uint32_t height);
static float calculateSSIM(pixel *pix1, intptr_t stride1, pixel *pix2, intptr_t stride2, uint32_t width, uint32_t height, void *buf, uint32_t& cnt);

FrameFilter::FrameFilter()
    : m_param(NULL)
    , m_frame(NULL)
    , m_frameEncoder(NULL)
    , m_ssimBuf(NULL)
{
}

void FrameFilter::destroy()
{
    if (m_param->bEnableSAO)
        m_sao.destroy();

    X265_FREE(m_ssimBuf);
}

void FrameFilter::init(Encoder *top, FrameEncoder *frame, int numRows)
{
    m_param = top->m_param;
    m_frameEncoder = frame;
    m_numRows = numRows;
    m_hChromaShift = CHROMA_H_SHIFT(m_param->internalCsp);
    m_vChromaShift = CHROMA_V_SHIFT(m_param->internalCsp);
    m_pad[0] = top->m_sps.conformanceWindow.rightOffset;
    m_pad[1] = top->m_sps.conformanceWindow.bottomOffset;
    m_saoRowDelay = m_param->bEnableLoopFilter ? 1 : 0;
    m_lastHeight = m_param->sourceHeight % g_maxCUSize ? m_param->sourceHeight % g_maxCUSize : g_maxCUSize;

    if (m_param->bEnableSAO)
        if (!m_sao.create(m_param))
            m_param->bEnableSAO = 0;

    if (m_param->bEnableSsim)
        m_ssimBuf = X265_MALLOC(int, 8 * (m_param->sourceWidth / 4 + 3));
}

void FrameFilter::start(Frame *frame, Entropy& initState, int qp)
{
    m_frame = frame;

    if (m_param->bEnableSAO)
        m_sao.startSlice(frame, initState, qp);
}

void FrameFilter::processRow(int row)
{
    ProfileScopeEvent(filterCTURow);

#if DETAILED_CU_STATS
    ScopedElapsedTime filterPerfScope(m_frameEncoder->m_cuStats.loopFilterElapsedTime);
    m_frameEncoder->m_cuStats.countLoopFilter++;
#endif

    if (!m_param->bEnableLoopFilter && !m_param->bEnableSAO)
    {
        processRowPost(row);
        return;
    }
    FrameData& encData = *m_frame->m_encData;
    const uint32_t numCols = encData.m_slice->m_sps->numCuInWidth;
    const uint32_t lineStartCUAddr = row * numCols;

    if (m_param->bEnableLoopFilter)
    {
        const CUGeom* cuGeoms = m_frameEncoder->m_cuGeoms;
        const uint32_t* ctuGeomMap = m_frameEncoder->m_ctuGeomMap;

        for (uint32_t col = 0; col < numCols; col++)
        {
            uint32_t cuAddr = lineStartCUAddr + col;
            const CUData* ctu = encData.getPicCTU(cuAddr);
            deblockCTU(ctu, cuGeoms[ctuGeomMap[cuAddr]], Deblock::EDGE_VER);

            if (col > 0)
            {
                const CUData* ctuPrev = encData.getPicCTU(cuAddr - 1);
                deblockCTU(ctuPrev, cuGeoms[ctuGeomMap[cuAddr - 1]], Deblock::EDGE_HOR);
            }
        }

        const CUData* ctuPrev = encData.getPicCTU(lineStartCUAddr + numCols - 1);
        deblockCTU(ctuPrev, cuGeoms[ctuGeomMap[lineStartCUAddr + numCols - 1]], Deblock::EDGE_HOR);
    }

    // SAO
    SAOParam* saoParam = encData.m_saoParam;
    if (m_param->bEnableSAO)
    {
        m_sao.m_entropyCoder.load(m_frameEncoder->m_initSliceContext);
        m_sao.m_rdContexts.next.load(m_frameEncoder->m_initSliceContext);
        m_sao.m_rdContexts.cur.load(m_frameEncoder->m_initSliceContext);

        m_sao.rdoSaoUnitRow(saoParam, row);

        // NOTE: Delay a row because SAO decide need top row pixels at next row, is it HM's bug?
        if (row >= m_saoRowDelay)
            processSao(row - m_saoRowDelay);
    }

    // this row of CTUs has been encoded

    if (row > 0)
        processRowPost(row - 1);

    if (row == m_numRows - 1)
    {
        if (m_param->bEnableSAO)
        {
            m_sao.rdoSaoUnitRowEnd(saoParam, encData.m_slice->m_sps->numCUsInFrame);

            for (int i = m_numRows - m_saoRowDelay; i < m_numRows; i++)
                processSao(i);
        }

        processRowPost(row);
    }
}

uint32_t FrameFilter::getCUHeight(int rowNum) const
{
    return rowNum == m_numRows - 1 ? m_lastHeight : g_maxCUSize;
}

void FrameFilter::processRowPost(int row)
{
    PicYuv *reconPic = m_frame->m_reconPic;
    const uint32_t numCols = m_frame->m_encData->m_slice->m_sps->numCuInWidth;
    const uint32_t lineStartCUAddr = row * numCols;
    const int realH = getCUHeight(row);

    // Border extend Left and Right
    primitives.extendRowBorder(reconPic->getLumaAddr(lineStartCUAddr), reconPic->m_stride, reconPic->m_picWidth, realH, reconPic->m_lumaMarginX);
    if (reconPic->m_picCsp != X265_CSP_I400) {
        primitives.extendRowBorder(reconPic->getCbAddr(lineStartCUAddr), reconPic->m_strideC, reconPic->m_picWidth >> m_hChromaShift, realH >> m_vChromaShift, reconPic->m_chromaMarginX);
        primitives.extendRowBorder(reconPic->getCrAddr(lineStartCUAddr), reconPic->m_strideC, reconPic->m_picWidth >> m_hChromaShift, realH >> m_vChromaShift, reconPic->m_chromaMarginX);
    }

    // Border extend Top
    if (!row)
    {
        const intptr_t stride = reconPic->m_stride;
        pixel *pixY = reconPic->getLumaAddr(lineStartCUAddr) - reconPic->m_lumaMarginX;

        for (uint32_t y = 0; y < reconPic->m_lumaMarginY; y++)
            memcpy(pixY - (y + 1) * stride, pixY, stride * sizeof(pixel));

        if (reconPic->m_picCsp != X265_CSP_I400) {
            const intptr_t strideC = reconPic->m_strideC;
            pixel *pixU = reconPic->getCbAddr(lineStartCUAddr) - reconPic->m_chromaMarginX;
            pixel *pixV = reconPic->getCrAddr(lineStartCUAddr) - reconPic->m_chromaMarginX;
            for (uint32_t y = 0; y < reconPic->m_chromaMarginY; y++)
            {
                memcpy(pixU - (y + 1) * strideC, pixU, strideC * sizeof(pixel));
                memcpy(pixV - (y + 1) * strideC, pixV, strideC * sizeof(pixel));
            }
        }
    }

    // Border extend Bottom
    if (row == m_numRows - 1)
    {
        const intptr_t stride = reconPic->m_stride;
        pixel *pixY = reconPic->getLumaAddr(lineStartCUAddr) - reconPic->m_lumaMarginX + (realH - 1) * stride;
        for (uint32_t y = 0; y < reconPic->m_lumaMarginY; y++)
            memcpy(pixY + (y + 1) * stride, pixY, stride * sizeof(pixel));

        if (reconPic->m_picCsp != X265_CSP_I400) {
            const intptr_t strideC = reconPic->m_strideC;
            pixel *pixU = reconPic->getCbAddr(lineStartCUAddr) - reconPic->m_chromaMarginX + ((realH >> m_vChromaShift) - 1) * strideC;
            pixel *pixV = reconPic->getCrAddr(lineStartCUAddr) - reconPic->m_chromaMarginX + ((realH >> m_vChromaShift) - 1) * strideC;
            for (uint32_t y = 0; y < reconPic->m_chromaMarginY; y++)
            {
                memcpy(pixU + (y + 1) * strideC, pixU, strideC * sizeof(pixel));
                memcpy(pixV + (y + 1) * strideC, pixV, strideC * sizeof(pixel));
            }
        }
    }

    // Notify other FrameEncoders that this row of reconstructed pixels is available
    m_frame->m_reconRowCount.incr();

    uint32_t cuAddr = lineStartCUAddr;
    if (m_param->bEnablePsnr)
    {
        PicYuv* fencPic = m_frame->m_fencPic;

        intptr_t stride = reconPic->m_stride;
        uint32_t width  = reconPic->m_picWidth - m_pad[0];
        uint32_t height = getCUHeight(row);

        uint64_t ssdY = computeSSD(fencPic->getLumaAddr(cuAddr), reconPic->getLumaAddr(cuAddr), stride, width, height);
        m_frameEncoder->m_SSDY += ssdY;
        if (reconPic->m_picCsp != X265_CSP_I400) {
            height >>= m_vChromaShift;
            width  >>= m_hChromaShift;
            stride = reconPic->m_strideC;
            
            uint64_t ssdU = computeSSD(fencPic->getCbAddr(cuAddr), reconPic->getCbAddr(cuAddr), stride, width, height);
            uint64_t ssdV = computeSSD(fencPic->getCrAddr(cuAddr), reconPic->getCrAddr(cuAddr), stride, width, height);

            m_frameEncoder->m_SSDU += ssdU;
            m_frameEncoder->m_SSDV += ssdV;
        }
    }
    if (m_param->bEnableSsim && m_ssimBuf)
    {
        pixel *rec = reconPic->m_picOrg[0];
        pixel *fenc = m_frame->m_fencPic->m_picOrg[0];
        intptr_t stride1 = reconPic->m_stride;
        intptr_t stride2 = m_frame->m_fencPic->m_stride;
        uint32_t bEnd = ((row + 1) == (this->m_numRows - 1));
        uint32_t bStart = (row == 0);
        uint32_t minPixY = row * g_maxCUSize - 4 * !bStart;
        uint32_t maxPixY = (row + 1) * g_maxCUSize - 4 * !bEnd;
        uint32_t ssim_cnt;
        x265_emms();

        /* SSIM is done for each row in blocks of 4x4 . The First blocks are offset by 2 pixels to the right
        * to avoid alignment of ssim blocks with DCT blocks. */
        minPixY += bStart ? 2 : -6;
        m_frameEncoder->m_ssim += calculateSSIM(rec + 2 + minPixY * stride1, stride1, fenc + 2 + minPixY * stride2, stride2,
                                                m_param->sourceWidth - 2, maxPixY - minPixY, m_ssimBuf, ssim_cnt);
        m_frameEncoder->m_ssimCnt += ssim_cnt;
    }
    if (m_param->decodedPictureHashSEI == 1)
    {
        uint32_t height = getCUHeight(row);
        uint32_t width = reconPic->m_picWidth;
        intptr_t stride = reconPic->m_stride;

        if (!row)
        {
            for (int i = 0; i < 3; i++)
                MD5Init(&m_frameEncoder->m_state[i]);
        }

        updateMD5Plane(m_frameEncoder->m_state[0], reconPic->getLumaAddr(cuAddr), width, height, stride);
        if (reconPic->m_picCsp != X265_CSP_I400) {
            width  >>= m_hChromaShift;
            height >>= m_vChromaShift;
            stride = reconPic->m_strideC;
            
            updateMD5Plane(m_frameEncoder->m_state[1], reconPic->getCbAddr(cuAddr), width, height, stride);
            updateMD5Plane(m_frameEncoder->m_state[2], reconPic->getCrAddr(cuAddr), width, height, stride);
        }
    }
    else if (m_param->decodedPictureHashSEI == 2)
    {
        uint32_t height = getCUHeight(row);
        uint32_t width = reconPic->m_picWidth;
        intptr_t stride = reconPic->m_stride;
        if (!row)
            m_frameEncoder->m_crc[0] = m_frameEncoder->m_crc[1] = m_frameEncoder->m_crc[2] = 0xffff;
        updateCRC(reconPic->getLumaAddr(cuAddr), m_frameEncoder->m_crc[0], height, width, stride);
        if (reconPic->m_picCsp != X265_CSP_I400) {
            width  >>= m_hChromaShift;
            height >>= m_vChromaShift;
            stride = reconPic->m_strideC;
            
            updateCRC(reconPic->getCbAddr(cuAddr), m_frameEncoder->m_crc[1], height, width, stride);
            updateCRC(reconPic->getCrAddr(cuAddr), m_frameEncoder->m_crc[2], height, width, stride);
        }
    }
    else if (m_param->decodedPictureHashSEI == 3)
    {
        uint32_t width = reconPic->m_picWidth;
        uint32_t height = getCUHeight(row);
        intptr_t stride = reconPic->m_stride;
        uint32_t cuHeight = g_maxCUSize;
        if (!row)
            m_frameEncoder->m_checksum[0] = m_frameEncoder->m_checksum[1] = m_frameEncoder->m_checksum[2] = 0;
        updateChecksum(reconPic->m_picOrg[0], m_frameEncoder->m_checksum[0], height, width, stride, row, cuHeight);
        if (reconPic->m_picCsp != X265_CSP_I400) {
            width  >>= m_hChromaShift;
            height >>= m_vChromaShift;
            stride = reconPic->m_strideC;
            cuHeight >>= m_vChromaShift;
            
            updateChecksum(reconPic->m_picOrg[1], m_frameEncoder->m_checksum[1], height, width, stride, row, cuHeight);
            updateChecksum(reconPic->m_picOrg[2], m_frameEncoder->m_checksum[2], height, width, stride, row, cuHeight);
        }
    }

    if (ATOMIC_INC(&m_frameEncoder->m_completionCount) == 2 * (int)m_frameEncoder->m_numRows)
        m_frameEncoder->m_completionEvent.trigger();
}

static uint64_t computeSSD(pixel *fenc, pixel *rec, intptr_t stride, uint32_t width, uint32_t height)
{
    uint64_t ssd = 0;

    if ((width | height) & 3)
    {
        /* Slow Path */
        for (uint32_t y = 0; y < height; y++)
        {
            for (uint32_t x = 0; x < width; x++)
            {
                int diff = (int)(fenc[x] - rec[x]);
                ssd += diff * diff;
            }

            fenc += stride;
            rec += stride;
        }

        return ssd;
    }

    uint32_t y = 0;

    /* Consume rows in ever narrower chunks of height */
    for (int size = BLOCK_64x64; size >= BLOCK_4x4 && y < height; size--)
    {
        uint32_t rowHeight = 1 << (size + 2);

        for (; y + rowHeight <= height; y += rowHeight)
        {
            uint32_t y1, x = 0;

            /* Consume each row using the largest square blocks possible */
            if (size == BLOCK_64x64 && !(stride & 31))
                for (; x + 64 <= width; x += 64)
                    ssd += primitives.cu[BLOCK_64x64].sse_pp(fenc + x, stride, rec + x, stride);

            if (size >= BLOCK_32x32 && !(stride & 15))
                for (; x + 32 <= width; x += 32)
                    for (y1 = 0; y1 + 32 <= rowHeight; y1 += 32)
                        ssd += primitives.cu[BLOCK_32x32].sse_pp(fenc + y1 * stride + x, stride, rec + y1 * stride + x, stride);

            if (size >= BLOCK_16x16)
                for (; x + 16 <= width; x += 16)
                    for (y1 = 0; y1 + 16 <= rowHeight; y1 += 16)
                        ssd += primitives.cu[BLOCK_16x16].sse_pp(fenc + y1 * stride + x, stride, rec + y1 * stride + x, stride);

            if (size >= BLOCK_8x8)
                for (; x + 8 <= width; x += 8)
                    for (y1 = 0; y1 + 8 <= rowHeight; y1 += 8)
                        ssd += primitives.cu[BLOCK_8x8].sse_pp(fenc + y1 * stride + x, stride, rec + y1 * stride + x, stride);

            for (; x + 4 <= width; x += 4)
                for (y1 = 0; y1 + 4 <= rowHeight; y1 += 4)
                    ssd += primitives.cu[BLOCK_4x4].sse_pp(fenc + y1 * stride + x, stride, rec + y1 * stride + x, stride);

            fenc += stride * rowHeight;
            rec += stride * rowHeight;
        }
    }

    return ssd;
}

/* Function to calculate SSIM for each row */
static float calculateSSIM(pixel *pix1, intptr_t stride1, pixel *pix2, intptr_t stride2, uint32_t width, uint32_t height, void *buf, uint32_t& cnt)
{
    uint32_t z = 0;
    float ssim = 0.0;

    int(*sum0)[4] = (int(*)[4])buf;
    int(*sum1)[4] = sum0 + (width >> 2) + 3;
    width >>= 2;
    height >>= 2;

    for (uint32_t y = 1; y < height; y++)
    {
        for (; z <= y; z++)
        {
            std::swap(sum0, sum1);
            for (uint32_t x = 0; x < width; x += 2)
                primitives.ssim_4x4x2_core(&pix1[(4 * x + (z * stride1))], stride1, &pix2[(4 * x + (z * stride2))], stride2, &sum0[x]);
        }

        for (uint32_t x = 0; x < width - 1; x += 4)
            ssim += primitives.ssim_end_4(sum0 + x, sum1 + x, X265_MIN(4, width - x - 1));
    }

    cnt = (height - 1) * (width - 1);
    return ssim;
}

/* restore original YUV samples to recon after SAO (if lossless) */
static void restoreOrigLosslessYuv(const CUData* cu, Frame& frame, uint32_t absPartIdx)
{
    int size = cu->m_log2CUSize[absPartIdx] - 2;
    uint32_t cuAddr = cu->m_cuAddr;

    PicYuv* reconPic = frame.m_reconPic;
    PicYuv* fencPic  = frame.m_fencPic;

    pixel* dst = reconPic->getLumaAddr(cuAddr, absPartIdx);
    pixel* src = fencPic->getLumaAddr(cuAddr, absPartIdx);

    primitives.cu[size].copy_pp(dst, reconPic->m_stride, src, fencPic->m_stride);
   
    int csp = fencPic->m_picCsp;
    if (csp != X265_CSP_I400) {
        pixel* dstCb = reconPic->getCbAddr(cuAddr, absPartIdx);
        pixel* srcCb = fencPic->getCbAddr(cuAddr, absPartIdx);

        pixel* dstCr = reconPic->getCrAddr(cuAddr, absPartIdx);
        pixel* srcCr = fencPic->getCrAddr(cuAddr, absPartIdx);

        primitives.chroma[csp].cu[size].copy_pp(dstCb, reconPic->m_strideC, srcCb, fencPic->m_strideC);
        primitives.chroma[csp].cu[size].copy_pp(dstCr, reconPic->m_strideC, srcCr, fencPic->m_strideC);
    }
}

/* Original YUV restoration for CU in lossless coding */
static void origCUSampleRestoration(const CUData* cu, const CUGeom& cuGeom, Frame& frame)
{
    uint32_t absPartIdx = cuGeom.absPartIdx;
    if (cu->m_cuDepth[absPartIdx] > cuGeom.depth)
    {
        for (int subPartIdx = 0; subPartIdx < 4; subPartIdx++)
        {
            const CUGeom& childGeom = *(&cuGeom + cuGeom.childOffset + subPartIdx);
            if (childGeom.flags & CUGeom::PRESENT)
                origCUSampleRestoration(cu, childGeom, frame);
        }
        return;
    }

    // restore original YUV samples
    if (cu->m_tqBypass[absPartIdx])
        restoreOrigLosslessYuv(cu, frame, absPartIdx);
}

void FrameFilter::processSao(int row)
{
    FrameData& encData = *m_frame->m_encData;
    SAOParam* saoParam = encData.m_saoParam;

    if (saoParam->bSaoFlag[0])
        m_sao.processSaoUnitRow(saoParam->ctuParam[0], row, 0);

    if (saoParam->bSaoFlag[1])
    {
        m_sao.processSaoUnitRow(saoParam->ctuParam[1], row, 1);
        m_sao.processSaoUnitRow(saoParam->ctuParam[2], row, 2);
    }

    if (encData.m_slice->m_pps->bTransquantBypassEnabled)
    {
        uint32_t numCols = encData.m_slice->m_sps->numCuInWidth;
        uint32_t lineStartCUAddr = row * numCols;

        const CUGeom* cuGeoms = m_frameEncoder->m_cuGeoms;
        const uint32_t* ctuGeomMap = m_frameEncoder->m_ctuGeomMap;

        for (uint32_t col = 0; col < numCols; col++)
        {
            uint32_t cuAddr = lineStartCUAddr + col;
            const CUData* ctu = encData.getPicCTU(cuAddr);
            origCUSampleRestoration(ctu, cuGeoms[ctuGeomMap[cuAddr]], *m_frame);
        }
    }
}
