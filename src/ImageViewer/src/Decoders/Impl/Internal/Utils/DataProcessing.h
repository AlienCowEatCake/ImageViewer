/*
   Copyright (C) 2024-2025 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined(DATA_PROCESSING_H_INCLUDED)
#define DATA_PROCESSING_H_INCLUDED

#include <cassert>
#include <cstring>

#include <QRgb>

#include "Utils/Global.h"

namespace DataProcessing {

quint8 bitReverse(quint8 x);
quint16 swapBytes16(quint16 x);
quint32 swapBytes32(quint32 x);
quint64 swapBytes64(quint64 x);
bool getBit(const void *buffer, quint64 bitOffset);
void setBit(void *buffer, quint64 bitOffset, bool value);
quint64 getBits(const void *buffer, quint64 bitsOffset, quint64 bitsCount);
void memcpyBits(void *dst, quint64 dstBitsOffset, const void *src, quint64 srcBitsOffset, quint64 bitsCount);

template<typename T>
inline T extractFromAlignedPtr(const void *ptr)
{
    assert(ptr);
    return *reinterpret_cast<const T*>(ptr);
}

template<typename T>
inline T extractFromUnalignedPtr(const void *ptr)
{
    assert(ptr);
    T value;
    memcpy(&value, ptr, sizeof(T));
    return value;
}

#define ADD_ALIGN_INDIFFERENT_TYPE(TYPE) \
template<> \
inline TYPE extractFromUnalignedPtr<TYPE>(const void *ptr) \
{ \
    return extractFromAlignedPtr<TYPE>(ptr); \
}
ADD_ALIGN_INDIFFERENT_TYPE(char)
ADD_ALIGN_INDIFFERENT_TYPE(unsigned char)
ADD_ALIGN_INDIFFERENT_TYPE(signed char)
#undef ADD_ALIGN_INDIFFERENT_TYPE

template<typename T>
inline quint8 clampByte(T value)
{
    return static_cast<quint8>(qBound<int>(0, static_cast<int>(value), 255));
}

template<>
inline quint8 clampByte(quint8 value)
{
    return value;
}

float float16ToFloat(const void *buffer);
float float24ToFloat(const void *buffer);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
static inline QRgb premultiply(QRgb rgb) { return qPremultiply(rgb); }
static inline QRgb unpremultiply(QRgb rgb) { return qUnpremultiply(rgb); }
#else
QRgb premultiply(QRgb rgb);
QRgb unpremultiply(QRgb rgb);
#endif

// Y is [0..1]
// Cb, Cr is [-0.5..0.5]
QRgb YCbCrToRgb(float Y, float Cb, float Cr, const float *ycbcrcoeffs = Q_NULLPTR);
QRgb YCbCrToRgba(float Y, float Cb, float Cr, float alpha, const float *ycbcrcoeffs = Q_NULLPTR);

QRgb XYZToRgb(float X, float Y, float Z);
QRgb XYZToRgba(float X, float Y, float Z, float alpha);

// L is [0..100]
// a, b is [-128..128]
QRgb LabToRgb(float L, float a, float b);
QRgb LabToRgba(float L, float a, float b, float alpha);

QRgb LuvToRgb(float L, float u, float v);
QRgb LuvToRgba(float L, float u, float v, float alpha);

QRgb CMYKToRgb(float C, float M, float Y, float K);
QRgb CMYKToRgba(float C, float M, float Y, float K, float alpha);

QRgb CMYK8ToRgb(int C, int M, int Y, int K);
QRgb CMYK8ToRgba(int C, int M, int Y, int K, int alpha);

} // namespace DataProcessing

#endif
