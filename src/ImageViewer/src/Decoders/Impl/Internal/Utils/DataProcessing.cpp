/*
   Copyright (C) 2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

   This file is part of the `ImageViewer' program.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "DataProcessing.h"

#include <cassert>
#include <cmath>

namespace DataProcessing {

quint8 bitReverse(quint8 x)
{
    // https://graphics.stanford.edu/~seander/bithacks.html#BitReverseTable
    static const quint8 table[256] = {
#define R2(n)     n,     n + 2*64,     n + 1*64,     n + 3*64
#define R4(n) R2(n), R2(n + 2*16), R2(n + 1*16), R2(n + 3*16)
#define R6(n) R4(n), R4(n + 2*4 ), R4(n + 1*4 ), R4(n + 3*4 )
        R6(0), R6(2), R6(1), R6(3)
#undef R2
#undef R4
#undef R6
    };
    return table[x];
}

quint16 swapBytes16(quint16 x)
{
    x = (x >> 8) | ((x & 0xff) << 8);
    return x;
}

quint32 swapBytes32(quint32 x)
{
    x = (x >> 24) | ((x >> 8) & 0xff00) | ((x << 8) & 0xff0000) | (x << 24);
    return x;
}

quint64 swapBytes64(quint64 x)
{
    x = ((x & 0xffffffff00000000ull) >> 32) | ((x & 0x00000000ffffffffull) << 32);
    x = ((x & 0xffff0000ffff0000ull) >> 16) | ((x & 0x0000ffff0000ffffull) << 16);
    x = ((x & 0xff00ff00ff00ff00ull) >>  8) | ((x & 0x00ff00ff00ff00ffull) <<  8);
    return x;
}

bool getBit(const void *buffer, quint64 bitOffset)
{
    assert(buffer);
    const quint8 *bufferU8 = reinterpret_cast<const quint8*>(buffer);
    const quint8 *currByte = bufferU8 + bitOffset / 8;
    const quint64 bitPos = 7 - (bitOffset % 8);
    const quint8 bitMask = static_cast<quint8>(1) << bitPos;
    const quint8 result = (*currByte & bitMask) >> bitPos;
    return result != 0;
}

void setBit(void *buffer, quint64 bitOffset, bool value)
{
    assert(buffer);
    quint8 *bufferU8 = reinterpret_cast<quint8*>(buffer);
    quint8 *currByte = bufferU8 + bitOffset / 8;
    const quint64 bitPos = 7 - (bitOffset % 8);
    const quint8 bitMask = static_cast<quint8>(1) << bitPos;
    if(value)
        *currByte |= (static_cast<quint8>(0xff) & bitMask);
    else
        *currByte &= ~(static_cast<quint8>(0xff) & bitMask);
}

quint64 getBits(const void *buffer, quint64 bitsOffset, quint64 bitsCount)
{
    assert(buffer);
    assert(bitsCount <= 64);
    const quint8 *bufferU8 = reinterpret_cast<const quint8*>(buffer);
    quint64 result = 0;
    for(quint64 currBitsOffset = bitsOffset, endBitsOffset = bitsOffset + bitsCount; currBitsOffset < endBitsOffset; ++currBitsOffset)
    {
        if(currBitsOffset % 8 == 0 && currBitsOffset + 8 < endBitsOffset)
        {
            result = (result << 8) | *(bufferU8 + currBitsOffset / 8);
            currBitsOffset += 7;
        }
        else
        {
            result = (result << 1) | (getBit(bufferU8, currBitsOffset) ? 1 : 0);
        }
    }
    return result;
}

void memcpyBits(void *dst, quint64 dstBitsOffset, const void *src, quint64 srcBitsOffset, quint64 bitsCount)
{
    for(quint64 b = 0; b < bitsCount; ++b)
    {
        const quint64 srcCurrBitsOffset = srcBitsOffset + b;
        const quint64 dstCurrBitsOffset = dstBitsOffset + b;
        if(srcCurrBitsOffset % 8 == 0 && dstCurrBitsOffset % 8 == 0 && b + 8 < bitsCount)
        {
            *(reinterpret_cast<quint8*>(dst) + dstCurrBitsOffset / 8) = *(reinterpret_cast<const quint8*>(src) + srcCurrBitsOffset / 8);
            b += 7;
        }
        else
        {
            const bool bit = getBit(src, srcCurrBitsOffset);
            DataProcessing::setBit(dst, dstCurrBitsOffset, bit);
        }
    }
}

float float16ToFloat(const void *buffer)
{
    assert(buffer);
    const quint16 u16 = *reinterpret_cast<const quint16*>(buffer);

    // 1s5e10m -> 1s8e23m
    const quint32 s = (u16 >> 15) & 0x0001;
    const quint32 e = (u16 >> 10) & 0x001f;
    const quint32 m = (u16 >>  0) & 0x03ff;

    quint32 f = 0;
    if(e == 0) // 0, denorm
        f = s << 31;
    else if(e == ~(/*~0*/0xffffffff << 5)) // inf, snan, qnan
        f = (s << 31) | ~(/*~0*/0xffffffff << 8) << 23 | (m << 13);
    else
        f = (s << 31) | ((e - 15 + 127) << 23) | (m << 13); // norm

    return extractFromUnalignedPtr<float>(&f);
}

float float24ToFloat(const void *buffer)
{
    assert(buffer);
    const quint8 *fp24 = reinterpret_cast<const quint8*>(buffer);
    quint8 fp32[4];

    // https://github.com/ImageMagick/ImageMagick/issues/1842
#if (Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
    const bool lsb = true;
#else
    const bool lsb = false;
#endif
    quint8 e, m24[2], m32[3], s;
    if((fp24[0] | fp24[1] | fp24[2]) == 0u)
    {
        fp32[0] = fp32[1] = fp32[2] = fp32[3] = 0;
        return extractFromUnalignedPtr<float>(fp32);
    }
    if(lsb)
    {
        s = fp24[2] & 0x80;
        e = fp24[2] & 0x7F;
        m24[0] = fp24[0];
        m24[1] = fp24[1];
    }
    else
    {
        s = fp24[0] & 0x80;
        e = fp24[0] & 0x7F;
        m24[0] = fp24[2];
        m24[1] = fp24[1];
    }
    if (e != 0)
        e = e - 63 + 127;
    m32[0] = (m24[0] & 0x01) << 7;
    m32[1] = ((m24[1] & 0x01) << 7) | ((m24[0] & 0xFE) >> 1);
    m32[2] = (m24[1] & 0xFE) >> 1;
    if(lsb)
    {
        fp32[0] = m32[0];
        fp32[1] = m32[1];
        fp32[2] = ((e & 1) << 7) | m32[2];
        fp32[3] = s | (e >> 1);
    }
    else
    {
        fp32[0] = s | (e >> 1);
        fp32[1] = ((e & 0x01) << 7) | m32[2];
        fp32[2] = m32[1];
        fp32[3] = m32[0];
    }
    return extractFromUnalignedPtr<float>(fp32);
}

QRgb premultiply(QRgb rgb)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    return qPremultiply(rgb);
#else
    const int a = qAlpha(rgb);
    const int r = (qRed(rgb)   * a) / 255;
    const int g = (qGreen(rgb) * a) / 255;
    const int b = (qBlue(rgb)  * a) / 255;
    return qRgba(qBound<int>(0, r, 255),
                 qBound<int>(0, g, 255),
                 qBound<int>(0, b, 255),
                 qBound<int>(0, a, 255));
#endif
}

QRgb unpremultiply(QRgb rgb)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    return qUnpremultiply(rgb);
#else
    const int a = qAlpha(rgb);
    if(a == 255)
        return rgb;
    if(a == 0)
        return 0;
    const int r = (qRed(rgb)   * 255) / a;
    const int g = (qGreen(rgb) * 255) / a;
    const int b = (qBlue(rgb)  * 255) / a;
    return qRgba(qBound<int>(0, r, 255),
                 qBound<int>(0, g, 255),
                 qBound<int>(0, b, 255),
                 qBound<int>(0, a, 255));
#endif
}

QRgb YCbCrToRgb(float Y, float Cb, float Cr, const float *ycbcrcoeffs)
{
    return YCbCrToRgba(Y, Cb, Cr, 1.0f, ycbcrcoeffs);
}

QRgb YCbCrToRgba(float Y, float Cb, float Cr, float alpha, const float *ycbcrcoeffs)
{
    if(!ycbcrcoeffs)
    {
        // ITU-R BT.601
        static const float ycbcrcoeffs601[] = {0.299f, 0.587f, 0.114f};
        ycbcrcoeffs = ycbcrcoeffs601;
    }
    float R = Cr * (2.0f - 2.0f * ycbcrcoeffs[0]) + Y;
    float B = Cb * (2.0f - 2.0f * ycbcrcoeffs[2]) + Y;
    float G = (Y - ycbcrcoeffs[2] * B - ycbcrcoeffs[0] * R) / ycbcrcoeffs[1];
    return qRgba(qBound<int>(0, static_cast<int>(R * 255.0f), 255),
                 qBound<int>(0, static_cast<int>(G * 255.0f), 255),
                 qBound<int>(0, static_cast<int>(B * 255.0f), 255),
                 qBound<int>(0, static_cast<int>(alpha * 255.0f), 255));
}

QRgb XYZToRgb(float X, float Y, float Z)
{
    return XYZToRgba(X, Y, Z, 1.0f);
}

QRgb XYZToRgba(float X, float Y, float Z, float alpha)
{
    // http://www.easyrgb.com/en/math.php
    // XYZ → Standard-RGB

    // X, Y and Z input refer to a D65/2° standard illuminant.
    // sR, sG and sB (standard RGB) output range = 0 ÷ 255

    const float var_X = X / 100.0f;
    const float var_Y = Y / 100.0f;
    const float var_Z = Z / 100.0f;

    float var_R = var_X *  3.2406f + var_Y * -1.5372f + var_Z * -0.4986f;
    float var_G = var_X * -0.9689f + var_Y *  1.8758f + var_Z *  0.0415f;
    float var_B = var_X *  0.0557f + var_Y * -0.2040f + var_Z *  1.0570f;

    if(var_R > 0.0031308f)
        var_R = 1.055f * std::pow(var_R, 1.0f / 2.4f) - 0.055f;
    else
        var_R = 12.92f * var_R;
    if(var_G > 0.0031308f)
        var_G = 1.055f * std::pow(var_G, 1.0f / 2.4f) - 0.055f;
    else
        var_G = 12.92f * var_G;
    if(var_B > 0.0031308f)
        var_B = 1.055f * std::pow(var_B, 1.0f / 2.4f) - 0.055f;
    else
        var_B = 12.92f * var_B;

    const float sR = var_R * 255.0f;
    const float sG = var_G * 255.0f;
    const float sB = var_B * 255.0f;

    return qRgba(qBound<int>(0, static_cast<int>(sR), 255),
                 qBound<int>(0, static_cast<int>(sG), 255),
                 qBound<int>(0, static_cast<int>(sB), 255),
                 qBound<int>(0, static_cast<int>(alpha * 255.0f), 255));
}

QRgb LabToRgb(float L, float a, float b)
{
    return LabToRgba(L, a, b, 1.0f);
}

QRgb LabToRgba(float L, float a, float b, float alpha)
{
    // http://www.easyrgb.com/en/math.php
    // CIE-L*ab → XYZ

    // X, Y and Z input refer to a D65/2° standard illuminant.

    float var_Y = (L + 16.0f) / 116.0f;
    float var_X = a / 500.0f + var_Y;
    float var_Z = var_Y - b / 200.0f;

    const float var_Y3 = var_Y * var_Y * var_Y;
    const float var_X3 = var_X * var_X * var_X;
    const float var_Z3 = var_Z * var_Z * var_Z;

    if(var_Y3 > 0.008856f)
        var_Y = var_Y3;
    else
        var_Y = (var_Y - 16.0f / 116.0f) / 7.787f;
    if(var_X3 > 0.008856f)
        var_X = var_X3;
    else
        var_X = (var_X - 16.0f / 116.0f) / 7.787f;
    if(var_Z3 > 0.008856f)
        var_Z = var_Z3;
    else
        var_Z = (var_Z - 16.0f / 116.0f) / 7.787f;

    const float X = var_X * 95.047f;
    const float Y = var_Y * 100.000f;
    const float Z = var_Z * 108.883f;

    return XYZToRgba(X, Y, Z, alpha);
}

QRgb LuvToRgb(float L, float u, float v)
{
    return LuvToRgba(L, u, v, 1.0f);
}

QRgb LuvToRgba(float L, float u, float v, float alpha)
{
    // https://www.easyrgb.com/en/math.php
    // CIE-L*uv → XYZ

    // X, Y and Z input refer to a D65/2° standard illuminant.

    float var_Y = (L + 16.0f) / 116.0f;
    const float var_Y3 = var_Y * var_Y * var_Y;

    if(var_Y3 > 0.008856f)
        var_Y = var_Y3;
    else
        var_Y = (var_Y - 16.0f / 116.0f) / 7.787f;

    const float ref_X = 95.047f;
    const float ref_Y = 100.000f;
    const float ref_Z = 108.883f;

    const float ref_U = (4.0f * ref_X) / (ref_X + (15.0f * ref_Y) + (3.0f * ref_Z));
    const float ref_V = (9.0f * ref_Y) / (ref_X + (15.0f * ref_Y) + (3.0f * ref_Z));

    const float var_U = u / (13.0f * L) + ref_U;
    const float var_V = v / (13.0f * L) + ref_V;

    const float Y = var_Y * 100.0f;
    const float X = -(9.0f * Y * var_U ) / ((var_U - 4.0f) * var_V - var_U * var_V);
    const float Z = (9.0f * Y - (15.0f * var_V * Y) - (var_V * X)) / (3.0f * var_V);

    return XYZToRgba(X, Y, Z, alpha);
}

QRgb CMYKToRgb(float C, float M, float Y, float K)
{
    return CMYKToRgba(C, M, Y, K, 1.0f);
}

QRgb CMYKToRgba(float C, float M, float Y, float K, float alpha)
{
    const float invC = 1.0f - C;
    const float invM = 1.0f - M;
    const float invY = 1.0f - Y;
    const float invK = 1.0f - K;
    return qRgba(qBound<int>(0, static_cast<int>(invC * invK * 255.0f), 255),
                 qBound<int>(0, static_cast<int>(invM * invK * 255.0f), 255),
                 qBound<int>(0, static_cast<int>(invY * invK * 255.0f), 255),
                 qBound<int>(0, static_cast<int>(alpha * 255.0f), 255));
}

QRgb CMYK8ToRgb(int C, int M, int Y, int K)
{
    return CMYK8ToRgba(C, M, Y, K, 255);
}

QRgb CMYK8ToRgba(int C, int M, int Y, int K, int alpha)
{
    C = 255 - C;
    M = 255 - M;
    Y = 255 - Y;
    K = 255 - K;
    return qRgba(qBound<int>(0, static_cast<int>(C * K / 255), 255),
                 qBound<int>(0, static_cast<int>(M * K / 255), 255),
                 qBound<int>(0, static_cast<int>(Y * K / 255), 255),
                 alpha);
}

} // namespace DataProcessing
