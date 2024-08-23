/*
   Copyright (C) 2021-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <cmath>
#include <cstring>
#include <limits>

#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QVector>
#include <QList>
#include <QPair>

#include "Utils/Global.h"
#include "Utils/IsOneOf.h"
#include "Utils/Logging.h"
#include "Utils/ScopedPointer.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"
#include "Internal/PayloadWithMetaData.h"
#include "Internal/Utils/CmsUtils.h"
#include "Internal/Utils/DataProcessing.h"

#if !defined (__ANSI__)
#define __ANSI__
#endif
#include <JXRGlue.h>
#if defined (min)
#undef min
#endif
#if defined (max)
#undef max
#endif

#define USE_RGBX_8888       (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0) && Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
#define USE_RGB_30          (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0) && Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
#define USE_GRAYSCALE_8     (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
#define USE_RGBA_64         (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0) && Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
#define USE_GRAYSCALE_16    (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
#define USE_BGR_888         (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#define USE_RGBX_32FPx4     (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0) && Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
#define USE_CMYK_8888       (QT_VERSION >= QT_VERSION_CHECK(6, 8, 0))

namespace
{

template<typename T>
struct JxrReleaseDeleter
{
    static inline void cleanup(T *data)
    {
        if(data)
            data->Release(&data);
    }
};

const char *errorToString(ERR nErrorCode)
{
    switch(nErrorCode)
    {
    case WMP_errSuccess:
        return "Success";
    case WMP_errFail:
        return "Fail";
    case WMP_errNotYetImplemented:
        return "NotYetImplemented";
    case WMP_errAbstractMethod:
        return "AbstractMethod:";
    case WMP_errOutOfMemory:
        return "OutOfMemory";
    case WMP_errFileIO:
        return "FileIO";
    case WMP_errBufferOverflow:
        return "BufferOverflow";
    case WMP_errInvalidParameter:
        return "InvalidParameter";
    case WMP_errInvalidArgument:
        return "InvalidArgument";
    case WMP_errUnsupportedFormat:
        return "UnsupportedFormat";
    case WMP_errIncorrectCodecVersion:
        return "IncorrectCodecVersion";
    case WMP_errIndexNotFound:
        return "IndexNotFound";
    case WMP_errOutOfSequence:
        return "OutOfSequence:";
    case WMP_errNotInitialized:
        return "NotInitialized";
    case WMP_errMustBeMultipleOf16LinesUntilLastCall:
        return "MustBeMultipleOf16LinesUntilLastCall";
    case WMP_errPlanarAlphaBandedEncRequiresTempFile:
        return "PlanarAlphaBandedEncRequiresTempFile";
    case WMP_errAlphaModeCannotBeTranscoded:
        return "AlphaModeCannotBeTranscoded";
    case WMP_errIncorrectCodecSubVersion:
        return "IncorrectCodecSubVersion";
    default:
        break;
    }
    return "Unknown";
}

const char *pixelFormatToString(const PKPixelFormatGUID &pixelFormat)
{
#define ADD_CASE(X) \
    if(IsEqualGUID(pixelFormat, X)) \
        return #X
    ADD_CASE(GUID_PKPixelFormatUndefined);
    ADD_CASE(GUID_PKPixelFormatDontCare);
//    ADD_CASE(GUID_PKPixelFormat1bppIndexed);
//    ADD_CASE(GUID_PKPixelFormat2bppIndexed);
//    ADD_CASE(GUID_PKPixelFormat4bppIndexed);
//    ADD_CASE(GUID_PKPixelFormat8bppIndexed);
    ADD_CASE(GUID_PKPixelFormatBlackWhite);
//    ADD_CASE(GUID_PKPixelFormat2bppGray);
//    ADD_CASE(GUID_PKPixelFormat4bppGray);
    ADD_CASE(GUID_PKPixelFormat8bppGray);
    ADD_CASE(GUID_PKPixelFormat16bppRGB555);
    ADD_CASE(GUID_PKPixelFormat16bppRGB565);
    ADD_CASE(GUID_PKPixelFormat16bppGray);
    ADD_CASE(GUID_PKPixelFormat24bppBGR);
    ADD_CASE(GUID_PKPixelFormat24bppRGB);
    ADD_CASE(GUID_PKPixelFormat32bppBGR);
    ADD_CASE(GUID_PKPixelFormat32bppBGRA);
    ADD_CASE(GUID_PKPixelFormat32bppPBGRA);
    ADD_CASE(GUID_PKPixelFormat32bppGrayFloat);
    ADD_CASE(GUID_PKPixelFormat32bppRGB);
    ADD_CASE(GUID_PKPixelFormat32bppRGBA);
    ADD_CASE(GUID_PKPixelFormat32bppPRGBA);
    ADD_CASE(GUID_PKPixelFormat48bppRGBFixedPoint);
    ADD_CASE(GUID_PKPixelFormat16bppGrayFixedPoint);
    ADD_CASE(GUID_PKPixelFormat32bppRGB101010);
    ADD_CASE(GUID_PKPixelFormat48bppRGB);
    ADD_CASE(GUID_PKPixelFormat64bppRGBA);
    ADD_CASE(GUID_PKPixelFormat64bppPRGBA);
    ADD_CASE(GUID_PKPixelFormat96bppRGBFixedPoint);
    ADD_CASE(GUID_PKPixelFormat96bppRGBFloat);
    ADD_CASE(GUID_PKPixelFormat128bppRGBAFloat);
    ADD_CASE(GUID_PKPixelFormat128bppPRGBAFloat);
    ADD_CASE(GUID_PKPixelFormat128bppRGBFloat);
    ADD_CASE(GUID_PKPixelFormat32bppCMYK);
    ADD_CASE(GUID_PKPixelFormat64bppRGBAFixedPoint);
    ADD_CASE(GUID_PKPixelFormat64bppRGBFixedPoint);
    ADD_CASE(GUID_PKPixelFormat128bppRGBAFixedPoint);
    ADD_CASE(GUID_PKPixelFormat128bppRGBFixedPoint);
    ADD_CASE(GUID_PKPixelFormat64bppRGBAHalf);
    ADD_CASE(GUID_PKPixelFormat64bppRGBHalf);
    ADD_CASE(GUID_PKPixelFormat48bppRGBHalf);
    ADD_CASE(GUID_PKPixelFormat32bppRGBE);
    ADD_CASE(GUID_PKPixelFormat16bppGrayHalf);
    ADD_CASE(GUID_PKPixelFormat32bppGrayFixedPoint);
    ADD_CASE(GUID_PKPixelFormat64bppCMYK);
    ADD_CASE(GUID_PKPixelFormat24bpp3Channels);
    ADD_CASE(GUID_PKPixelFormat32bpp4Channels);
    ADD_CASE(GUID_PKPixelFormat40bpp5Channels);
    ADD_CASE(GUID_PKPixelFormat48bpp6Channels);
    ADD_CASE(GUID_PKPixelFormat56bpp7Channels);
    ADD_CASE(GUID_PKPixelFormat64bpp8Channels);
    ADD_CASE(GUID_PKPixelFormat48bpp3Channels);
    ADD_CASE(GUID_PKPixelFormat64bpp4Channels);
    ADD_CASE(GUID_PKPixelFormat80bpp5Channels);
    ADD_CASE(GUID_PKPixelFormat96bpp6Channels);
    ADD_CASE(GUID_PKPixelFormat112bpp7Channels);
    ADD_CASE(GUID_PKPixelFormat128bpp8Channels);
    ADD_CASE(GUID_PKPixelFormat40bppCMYKAlpha);
    ADD_CASE(GUID_PKPixelFormat80bppCMYKAlpha);
    ADD_CASE(GUID_PKPixelFormat32bpp3ChannelsAlpha);
    ADD_CASE(GUID_PKPixelFormat40bpp4ChannelsAlpha);
    ADD_CASE(GUID_PKPixelFormat48bpp5ChannelsAlpha);
    ADD_CASE(GUID_PKPixelFormat56bpp6ChannelsAlpha);
    ADD_CASE(GUID_PKPixelFormat64bpp7ChannelsAlpha);
    ADD_CASE(GUID_PKPixelFormat72bpp8ChannelsAlpha);
    ADD_CASE(GUID_PKPixelFormat64bpp3ChannelsAlpha);
    ADD_CASE(GUID_PKPixelFormat80bpp4ChannelsAlpha);
    ADD_CASE(GUID_PKPixelFormat96bpp5ChannelsAlpha);
    ADD_CASE(GUID_PKPixelFormat112bpp6ChannelsAlpha);
    ADD_CASE(GUID_PKPixelFormat128bpp7ChannelsAlpha);
    ADD_CASE(GUID_PKPixelFormat144bpp8ChannelsAlpha);
    ADD_CASE(GUID_PKPixelFormat12bppYCC420);
    ADD_CASE(GUID_PKPixelFormat16bppYCC422);
    ADD_CASE(GUID_PKPixelFormat20bppYCC422);
    ADD_CASE(GUID_PKPixelFormat32bppYCC422);
    ADD_CASE(GUID_PKPixelFormat24bppYCC444);
    ADD_CASE(GUID_PKPixelFormat30bppYCC444);
    ADD_CASE(GUID_PKPixelFormat48bppYCC444);
    ADD_CASE(GUID_PKPixelFormat16bpp48bppYCC444FixedPoint);
    ADD_CASE(GUID_PKPixelFormat20bppYCC420Alpha);
    ADD_CASE(GUID_PKPixelFormat24bppYCC422Alpha);
    ADD_CASE(GUID_PKPixelFormat30bppYCC422Alpha);
    ADD_CASE(GUID_PKPixelFormat48bppYCC422Alpha);
    ADD_CASE(GUID_PKPixelFormat32bppYCC444Alpha);
    ADD_CASE(GUID_PKPixelFormat40bppYCC444Alpha);
    ADD_CASE(GUID_PKPixelFormat64bppYCC444Alpha);
    ADD_CASE(GUID_PKPixelFormat64bppYCC444AlphaFixedPoint);
    ADD_CASE(GUID_PKPixelFormat12bppYUV420);
    ADD_CASE(GUID_PKPixelFormat16bppYUV422);
    ADD_CASE(GUID_PKPixelFormat24bppYUV444);
    ADD_CASE(GUID_PKPixelFormat32bppCMYKDIRECT);
    ADD_CASE(GUID_PKPixelFormat64bppCMYKDIRECT);
    ADD_CASE(GUID_PKPixelFormat40bppCMYKDIRECTAlpha);
    ADD_CASE(GUID_PKPixelFormat80bppCMYKDIRECTAlpha);
#undef ADD_CASE
    return Q_NULLPTR;
}

#if defined (Report)
#undef Report
#define Report(err, szExp, szFile, nLine) \
    LOG_WARNING() << LOGGING_CTX << "FAILED:" << errorToString(err) << "in" << szExp; \
    err = err
#endif

ERR getMetadata(PKImageDecode *decoder, QByteArray &rawMetaData, U32 offset, U32 byteCount)
{
    rawMetaData = QByteArray(static_cast<int>(byteCount), 0);
    ERR err = WMP_errSuccess;
    WMPStream* stream = decoder->pStream;
    size_t oldPos = 0;
    Call(stream->GetPos(stream, &oldPos));
    Call(stream->SetPos(stream, offset));
    Call(stream->Read(stream, reinterpret_cast<void*>(rawMetaData.data()), static_cast<size_t>(rawMetaData.size())));
    Call(stream->SetPos(stream, oldPos));
Cleanup:
    if(Failed(err))
        rawMetaData = QByteArray();
    return err;
}

ERR getXmpMetadata(PKImageDecode *decoder, ImageMetaData **metaData)
{
    QByteArray rawMetaData;
    ERR err = WMP_errSuccess;
    const U32 offset = decoder->WMP.wmiDEMisc.uXMPMetadataOffset;
    const U32 byteCount = decoder->WMP.wmiDEMisc.uXMPMetadataByteCount;
    if(offset && byteCount)
    {
        LOG_DEBUG() << LOGGING_CTX << "Found XMP metadata";
        Call(getMetadata(decoder, rawMetaData, offset, byteCount));
        *metaData = ImageMetaData::joinMetaData(*metaData, ImageMetaData::createXmpMetaData(rawMetaData));
    }
Cleanup:
    return err;
}

ERR getExifMetadata(PKImageDecode *decoder, ImageMetaData **metaData)
{
    QByteArray rawMetaData;
    ERR err = WMP_errSuccess;
    const U32 offset = decoder->WMP.wmiDEMisc.uEXIFMetadataOffset;
    const U32 byteCount = decoder->WMP.wmiDEMisc.uEXIFMetadataByteCount;
    if(offset && byteCount)
    {
        LOG_DEBUG() << LOGGING_CTX << "Found EXIF metadata";
        Call(getMetadata(decoder, rawMetaData, offset, byteCount));
        *metaData = ImageMetaData::joinMetaData(*metaData, ImageMetaData::createExifMetaData(rawMetaData));
    }
Cleanup:
    return err;
}

ERR getIccProfileData(PKImageDecode *decoder, QByteArray *profileData)
{
    *profileData = QByteArray();
    ERR err = WMP_errSuccess;
    U32 size = 0;
    Call(decoder->GetColorContext(decoder, Q_NULLPTR, &size));
    if(size > 0)
    {
        LOG_DEBUG() << LOGGING_CTX << "Found ICCP metadata";
        profileData->resize(size);
        Call(decoder->GetColorContext(decoder, reinterpret_cast<U8*>(profileData->data()), &size));
    }
Cleanup:
    if(Failed(err))
        *profileData = QByteArray();
    return err;
}

QString toString(const DPKPROPVARIANT& prop)
{
    switch(prop.vt)
    {
    case DPKVT_EMPTY:
        return QString();
    case DPKVT_UI1:
        return QString::number(prop.VT.bVal);
    case DPKVT_UI2:
        return QString::number(prop.VT.uiVal);
    case DPKVT_UI4:
        return QString::number(prop.VT.ulVal);
    case DPKVT_LPSTR:
        return QString::fromLocal8Bit(prop.VT.pszVal);
    case DPKVT_LPWSTR:
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        return QString::fromUtf16(reinterpret_cast<const char16_t*>(prop.VT.pwszVal));
#else
        return QString::fromUtf16(prop.VT.pwszVal);
#endif
//    case DPKVT_BYREF | DPKVT_UI1:
//        return QString::fromLatin1(QByteArray(reinterpret_cast<char*>(prop.VT.pbVal)).toHex());
    default:
        break;
    }
    return QString();
}

float scRgbFloatTosRgbFloat(float f)
{
    // convert from linear scRGB to non-linear sRGB
    if(f <= 0)
        return 0.0f;
    else if(f <= 0.0031308f)
        return qBound(0.0f, f * 12.92f, 1.0f);
    else if(f < 1.0f)
        return qBound(0.0f, ((1.055f * static_cast<float>(std::pow(static_cast<double>(f), 1.0 / 2.4))) - 0.055f), 1.0f);
    else
        return 1.0f;
}

float scRgbAlphaFloatTosRgbAlphaFloat(float f)
{
    // alpha is converted differently than RGB in scRGB
    return qBound(0.0f, f, 1.0f);
}

U8 floatToU8(float f)
{
    return static_cast<U8>(scRgbFloatTosRgbFloat(f) * 255.0f);
}

U8 alphaFloatToU8(float f)
{
    return static_cast<U8>(scRgbAlphaFloatTosRgbAlphaFloat(f) * 255.0f);
}

U8 halfToU8(U16 u16)
{
    return floatToU8(DataProcessing::float16ToFloat(&u16));
}

U8 alphaHalfToU8(U16 u16)
{
    return alphaFloatToU8(DataProcessing::float16ToFloat(&u16));
}

float fixedToFloat(I16 i16)
{
    const float fltCvtFactor = 1.0f / (1 << 13);
    return i16 * fltCvtFactor;
}

float fixedToFloat(I32 i32)
{
    const float fltCvtFactor = 1.0f / (1 << 24);
    return i32 * fltCvtFactor;
}

template<typename T>
U8 fixedToU8(T data)
{
    return floatToU8(fixedToFloat(data));
}

template<typename T>
U8 alphaFixedToU8(T data)
{
    return alphaFloatToU8(fixedToFloat(data));
}

template<typename T>
QRgb convertFromGray(const T *data)
{
    const double scale = static_cast<double>(std::numeric_limits<T>::max());
    const int c = qBound<int>(0, static_cast<int>(255.0 * data[0] / scale), 255);
    return qRgb(c, c, c);
}

template<typename T>
QRgb convertFromGrayFloat(const T *data)
{
    const int c = floatToU8(*reinterpret_cast<const float*>(data));
    return qRgb(c, c, c);
}

template<typename T>
QRgb convertFromGrayHalf(const T *data)
{
    const int c = halfToU8(*reinterpret_cast<const U16*>(data));
    return qRgb(c, c, c);
}

template<typename T>
QRgb convertFromGrayFixed(const T *data)
{
    const int c = qBound<int>(0, static_cast<int>(fixedToU8(data[0])), 255);
    return qRgb(c, c, c);
}

template<typename T>
QRgb convertFromRGB(const T *data)
{
    const double scale = static_cast<double>(std::numeric_limits<T>::max());
    const int r = qBound<int>(0, static_cast<int>(255.0 * data[0] / scale), 255);
    const int g = qBound<int>(0, static_cast<int>(255.0 * data[1] / scale), 255);
    const int b = qBound<int>(0, static_cast<int>(255.0 * data[2] / scale), 255);
    return qRgb(r, g, b);
}

template<typename T>
QRgb convertFromRGBFloat(const T *data)
{
    const float *pixelData = reinterpret_cast<const float*>(data);
    return qRgb(floatToU8(pixelData[0]), floatToU8(pixelData[1]), floatToU8(pixelData[2]));
}

template<typename T>
QRgb convertFromRGBHalf(const T *data)
{
    const U16 *pixelData = reinterpret_cast<const U16*>(data);
    return qRgb(halfToU8(pixelData[0]), halfToU8(pixelData[1]), halfToU8(pixelData[2]));
}

template<typename T>
QRgb convertFromRGBFixed(const T *data)
{
    const int r = qBound<int>(0, static_cast<int>(fixedToU8(data[0])), 255);
    const int g = qBound<int>(0, static_cast<int>(fixedToU8(data[1])), 255);
    const int b = qBound<int>(0, static_cast<int>(fixedToU8(data[2])), 255);
    return qRgb(r, g, b);
}

template<typename T>
QRgb convertFromRGB101010(const T *data)
{
    const int maxValue = (1 << 10) - 1;
    const double scale = static_cast<double>(maxValue);
    const U32 *pixelData = reinterpret_cast<const U32*>(data);
    const int r = qBound<int>(0, static_cast<int>(255.0 * ((pixelData[0] >> 20) & maxValue) / scale), 255);
    const int g = qBound<int>(0, static_cast<int>(255.0 * ((pixelData[0] >> 10) & maxValue) / scale), 255);
    const int b = qBound<int>(0, static_cast<int>(255.0 * ((pixelData[0] >>  0) & maxValue) / scale), 255);
    return qRgb(r, g, b);
}

template<typename T>
QRgb convertFromRGBE(const T *data)
{
    const U8 *pixelData = reinterpret_cast<const U8*>(data);

    // First read the exponent
    const U8 rawExp = pixelData[3];
    if(rawExp == 0)
        return qRgb(0, 0, 0);

    const I32 adjExp = static_cast<I32>(rawExp) - 128 - 8; // Can be negative
    float fltExp;
    if(adjExp > -32 && adjExp < 32)
    {
        fltExp = static_cast<float>(static_cast<U32>(1) << std::abs(adjExp));
        if(adjExp < 0)
            fltExp = 1.0f / fltExp;
    }
    else
    {
        fltExp = static_cast<float>(std::ldexp(1.0f, adjExp));
    }

    const int r = floatToU8(pixelData[0] * fltExp);
    const int g = floatToU8(pixelData[1] * fltExp);
    const int b = floatToU8(pixelData[2] * fltExp);
    return qRgb(r, g, b);
}

template<typename T>
QRgb convertFromRGBA(const T *data)
{
    const double scale = static_cast<double>(std::numeric_limits<T>::max());
    const int r = qBound<int>(0, static_cast<int>(255.0 * data[0] / scale), 255);
    const int g = qBound<int>(0, static_cast<int>(255.0 * data[1] / scale), 255);
    const int b = qBound<int>(0, static_cast<int>(255.0 * data[2] / scale), 255);
    const int a = qBound<int>(0, static_cast<int>(255.0 * data[3] / scale), 255);
    return qRgba(r, g, b, a);
}

template<typename T>
QRgb convertFromRGBAFloat(const T *data)
{
    const float *pixelData = reinterpret_cast<const float*>(data);
    return qRgba(floatToU8(pixelData[0]), floatToU8(pixelData[1]), floatToU8(pixelData[2]), alphaFloatToU8(pixelData[3]));
}

template<typename T>
QRgb convertFromRGBAHalf(const T *data)
{
    const U16 *pixelData = reinterpret_cast<const U16*>(data);
    return qRgba(halfToU8(pixelData[0]), halfToU8(pixelData[1]), halfToU8(pixelData[2]), alphaHalfToU8(pixelData[3]));
}

template<typename T>
QRgb convertFromRGBAFixed(const T *data)
{
    const int r = qBound<int>(0, static_cast<int>(fixedToU8(data[0])), 255);
    const int g = qBound<int>(0, static_cast<int>(fixedToU8(data[1])), 255);
    const int b = qBound<int>(0, static_cast<int>(fixedToU8(data[2])), 255);
    const int a = qBound<int>(0, static_cast<int>(alphaFixedToU8(data[3])), 255);
    return qRgba(r, g, b, a);
}

template<typename T>
QRgb convertFromPRGBA(const T *data)
{
    const double scale = static_cast<double>(std::numeric_limits<T>::max());
    const double alphaScale = data[3] > 0.0 ? (scale / static_cast<double>(data[3])) : 1.0;
    const int r = qBound<int>(0, static_cast<int>(255.0 * data[0] / scale * alphaScale), 255);
    const int g = qBound<int>(0, static_cast<int>(255.0 * data[1] / scale * alphaScale), 255);
    const int b = qBound<int>(0, static_cast<int>(255.0 * data[2] / scale * alphaScale), 255);
    const int a = qBound<int>(0, static_cast<int>(255.0 * data[3] / scale), 255);
    return qRgba(r, g, b, a);
}

template<typename T>
QRgb convertFromPRGBAFloat(const T *data)
{
    const float *pixelData = reinterpret_cast<const float*>(data);
    const float a = pixelData[3];
    const float r = a > 0.0f ? (pixelData[0] / a) : pixelData[0];
    const float g = a > 0.0f ? (pixelData[1] / a) : pixelData[1];
    const float b = a > 0.0f ? (pixelData[2] / a) : pixelData[2];
    return qRgba(floatToU8(r), floatToU8(g), floatToU8(b), alphaFloatToU8(a));
}

template<typename T>
QRgb convertFromBGR(const T *data)
{
    const double scale = static_cast<double>(std::numeric_limits<T>::max());
    const int b = qBound<int>(0, static_cast<int>(255.0 * data[0] / scale), 255);
    const int g = qBound<int>(0, static_cast<int>(255.0 * data[1] / scale), 255);
    const int r = qBound<int>(0, static_cast<int>(255.0 * data[2] / scale), 255);
    return qRgb(r, g, b);
}

template<typename T>
QRgb convertFromBGRA(const T *data)
{
    const double scale = static_cast<double>(std::numeric_limits<T>::max());
    const int b = qBound<int>(0, static_cast<int>(255.0 * data[0] / scale), 255);
    const int g = qBound<int>(0, static_cast<int>(255.0 * data[1] / scale), 255);
    const int r = qBound<int>(0, static_cast<int>(255.0 * data[2] / scale), 255);
    const int a = qBound<int>(0, static_cast<int>(255.0 * data[3] / scale), 255);
    return qRgba(r, g, b, a);
}

template<typename T>
QRgb convertFromPBGRA(const T *data)
{
    const double scale = static_cast<double>(std::numeric_limits<T>::max());
    const double alphaScale = data[3] > 0.0 ? (scale / static_cast<double>(data[3])) : 1.0;
    const int b = qBound<int>(0, static_cast<int>(255.0 * data[0] / scale * alphaScale), 255);
    const int g = qBound<int>(0, static_cast<int>(255.0 * data[1] / scale * alphaScale), 255);
    const int r = qBound<int>(0, static_cast<int>(255.0 * data[2] / scale * alphaScale), 255);
    const int a = qBound<int>(0, static_cast<int>(255.0 * data[3] / scale), 255);
    return qRgba(r, g, b, a);
}

template<typename T>
QRgb convertFromCMYK(const T *data)
{
    const double scale = static_cast<double>(std::numeric_limits<T>::max());
    const double c = data[0] / scale;
    const double m = data[1] / scale;
    const double y = data[2] / scale;
    const double k = data[3] / scale;
    const int r = qBound<int>(0, static_cast<int>(255.0 * (1.0 - c) * (1.0 - k)), 255);
    const int g = qBound<int>(0, static_cast<int>(255.0 * (1.0 - m) * (1.0 - k)), 255);
    const int b = qBound<int>(0, static_cast<int>(255.0 * (1.0 - y) * (1.0 - k)), 255);
    return qRgb(r, g, b);
}

template<typename T>
QRgb convertFromCMYKtoCMYK8888(const T *data)
{
    const double scale = static_cast<double>(std::numeric_limits<T>::max());
    const double c = data[0] / scale;
    const double m = data[1] / scale;
    const double y = data[2] / scale;
    const double k = data[3] / scale;
    QRgb result = 0;
    quint8 *resultData = reinterpret_cast<quint8*>(&result);
    resultData[0] = qBound<int>(0, static_cast<int>(255.0 * c), 255);
    resultData[1] = qBound<int>(0, static_cast<int>(255.0 * m), 255);
    resultData[2] = qBound<int>(0, static_cast<int>(255.0 * y), 255);
    resultData[3] = qBound<int>(0, static_cast<int>(255.0 * k), 255);
    return result;
}

template<typename T, size_t alphaIndex>
QRgb convertFromCMYKA(const T *data)
{
    const double scale = static_cast<double>(std::numeric_limits<T>::max());
    const double c = data[0] / scale;
    const double m = data[1] / scale;
    const double y = data[2] / scale;
    const double k = data[3] / scale;
    const int a = qBound<int>(0, static_cast<int>(255.0 * data[alphaIndex] / scale), 255);
    const int r = qBound<int>(0, static_cast<int>(255.0 * (1.0 - c) * (1.0 - k)), 255);
    const int g = qBound<int>(0, static_cast<int>(255.0 * (1.0 - m) * (1.0 - k)), 255);
    const int b = qBound<int>(0, static_cast<int>(255.0 * (1.0 - y) * (1.0 - k)), 255);
    return qRgba(r, g, b, a);
}

template<typename T>
QPair<QRgb, quint8> convertFromCMYKAtoCMYK8888PlusAlpha(const T *data, size_t alphaIndex)
{
    const double scale = static_cast<double>(std::numeric_limits<T>::max());
    const double c = data[0] / scale;
    const double m = data[1] / scale;
    const double y = data[2] / scale;
    const double k = data[3] / scale;
    const double a = data[alphaIndex] / scale;
    QRgb result = 0;
    quint8 *resultData = reinterpret_cast<quint8*>(&result);
    resultData[0] = qBound<int>(0, static_cast<int>(255.0 * c), 255);
    resultData[1] = qBound<int>(0, static_cast<int>(255.0 * m), 255);
    resultData[2] = qBound<int>(0, static_cast<int>(255.0 * y), 255);
    resultData[3] = qBound<int>(0, static_cast<int>(255.0 * k), 255);
    return qMakePair<QRgb, quint8>(static_cast<QRgb>(result), static_cast<quint8>(qBound<int>(0, static_cast<int>(255.0 * a), 255)));
}

void directCopy(PKImageDecode *decoder, const PKRect &rect, QImage &image, QImage::Format format)
{
    image = QImage(rect.Width, rect.Height, format);
    if(image.isNull())
    {
        LOG_WARNING() << LOGGING_CTX << "Invalid image size";
        return;
    }
    ERR err = WMP_errSuccess;
    Call(decoder->Copy(decoder, &rect, image.bits(), image.bytesPerLine()));
Cleanup:
    if(Failed(err))
        image = QImage();
}

template<typename T>
void copyViaBuffer(PKImageDecode *decoder, const PKRect &rect, QImage &image, QImage::Format format, size_t bytesPerPixel, QRgb(*func)(const T*))
{
    image = QImage(rect.Width, rect.Height, format);
    if(image.isNull())
    {
        LOG_WARNING() << LOGGING_CTX << "Invalid image size";
        return;
    }
    const U32 bytesPerLine = static_cast<U32>(image.width() * bytesPerPixel);
    QByteArray buffer;
    buffer.resize(image.height() * bytesPerLine);
    ERR err = WMP_errSuccess;
    Call(decoder->Copy(decoder, &rect, reinterpret_cast<U8*>(buffer.data()), bytesPerLine));
    for(int y = 0; y < image.height(); ++y)
    {
        QRgb *outScanLine = reinterpret_cast<QRgb*>(image.scanLine(y));
        const quint8 *inScanLine = reinterpret_cast<const quint8*>(buffer.data() + bytesPerLine * y);
        for(int x = 0; x < image.width(); ++x)
            outScanLine[x] = func(reinterpret_cast<const T*>(inScanLine + x * bytesPerPixel));
    }
Cleanup:
    if(Failed(err))
        image = QImage();
}

#if (USE_CMYK_8888)
template<typename T>
void copyViaBufferCMYKA(PKImageDecode *decoder, const PKRect &rect, QImage &image, size_t bytesPerPixel, QByteArray &iccProfileData)
{
    image = QImage(rect.Width, rect.Height, QImage::Format_CMYK8888);
    if(image.isNull())
    {
        LOG_WARNING() << LOGGING_CTX << "Invalid image size";
        return;
    }
    const size_t bytesPerLine = image.width() * bytesPerPixel;
    QByteArray buffer;
    buffer.resize(image.height() * bytesPerLine);
    QImage alphaChannel(image.width(), image.height(), QImage::Format_Alpha8);
    if(alphaChannel.isNull())
    {
        LOG_WARNING() << LOGGING_CTX << "Invalid image size";
        image = QImage();
        return;
    }
    ERR err = WMP_errSuccess;
    Call(decoder->Copy(decoder, &rect, reinterpret_cast<U8*>(buffer.data()), static_cast<U32>(bytesPerLine)));
    for(int y = 0; y < image.height(); ++y)
    {
        QRgb *outScanLine = reinterpret_cast<QRgb*>(image.scanLine(y));
        const quint8 *inScanLine = reinterpret_cast<const quint8*>(buffer.data() + bytesPerLine * y);
        quint8 *alphaScanLine = reinterpret_cast<quint8*>(alphaChannel.scanLine(y));
        for(int x = 0; x < image.width(); ++x)
        {
            QPair<QRgb, quint8> outValues = convertFromCMYKAtoCMYK8888PlusAlpha(reinterpret_cast<const T*>(inScanLine + x * bytesPerPixel), bytesPerPixel / sizeof(T) - 1);
            outScanLine[x] = outValues.first;
            alphaScanLine[x] = outValues.second;
        }
    }
    if(!iccProfileData.isEmpty())
        ICCProfile(iccProfileData).applyToImage(&image);
    iccProfileData = QByteArray();
    QImage_convertTo(image, QImage::Format_ARGB32);
    image.setAlphaChannel(alphaChannel);
Cleanup:
    if(Failed(err))
        image = QImage();
}
#endif

#if !(USE_RGBX_8888)
void postprocessARGB32(QImage &image, QRgb(*func)(const quint8*))
{
    for(int y = 0; y < image.height(); ++y)
    {
        QRgb *scanLine = reinterpret_cast<QRgb*>(image.scanLine(y));
        for(int x = 0; x < image.width(); ++x)
            scanLine[x] = func(reinterpret_cast<const quint8*>(scanLine + x));
    }
}
#endif

#if (USE_RGBX_32FPx4)
void postprocessscRgbFloat(QImage &image)
{
    for(int y = 0; y < image.height(); ++y)
    {
        float *scanLine = reinterpret_cast<float*>(image.scanLine(y));
        for(int x = 0; x < image.width(); ++x)
        {
            float *pixel = scanLine + 4 * x;
            pixel[0] = scRgbFloatTosRgbFloat(pixel[0]);
            pixel[1] = scRgbFloatTosRgbFloat(pixel[1]);
            pixel[2] = scRgbFloatTosRgbFloat(pixel[2]);
            pixel[3] = scRgbAlphaFloatTosRgbAlphaFloat(pixel[3]);
        }
    }
}
#endif

PayloadWithMetaData<QImage> readJxrFile(const QString &filePath)
{
    QScopedPointer<PKCodecFactory, JxrReleaseDeleter<PKCodecFactory> > codecFactory;
    {
        PKCodecFactory *codecFactoryData = Q_NULLPTR;
        const ERR err = PKCreateCodecFactory(&codecFactoryData, WMP_SDK_VERSION);
        if(Failed(err) || !codecFactoryData)
        {
            LOG_WARNING() << LOGGING_CTX << "PKCreateCodecFactory failed:" << errorToString(err);
            return QImage();
        }
        codecFactory.reset(codecFactoryData);
    }

    QScopedPointer<PKImageDecode, JxrReleaseDeleter<PKImageDecode> > decoder;
    {
        const QByteArray filename = filePath.toLocal8Bit();
        PKImageDecode *decoderData = Q_NULLPTR;
        const ERR err = codecFactory->CreateDecoderFromFile(filename.data(), &decoderData);
        if(Failed(err) || !decoderData)
        {
            LOG_WARNING() << LOGGING_CTX << "CreateDecoderFromFile failed:" << errorToString(err);
            return QImage();
        }
        decoder.reset(decoderData);
    }

    PKPixelFormatGUID pixelFormat = GUID_PKPixelFormatUndefined;
    decoder->GetPixelFormat(decoder.data(), &pixelFormat);

    if(const char *pixelFormatStr = pixelFormatToString(pixelFormat))
    {
        LOG_DEBUG("%s Color format: %s", LOGGING_CTX, pixelFormatStr);
    }
    else
    {
        LOG_DEBUG("%s Color format: %08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X",
                LOGGING_CTX,
                pixelFormat.Data1, pixelFormat.Data2, pixelFormat.Data3,
                pixelFormat.Data4[0], pixelFormat.Data4[1], pixelFormat.Data4[2],
                pixelFormat.Data4[3], pixelFormat.Data4[4], pixelFormat.Data4[5],
                pixelFormat.Data4[6], pixelFormat.Data4[7]);
    }

    PKRect rect;
    memset(&rect, 0, sizeof(rect));
    decoder->GetSize(decoder.data(), &rect.Width, &rect.Height);

    ImageMetaData *metaData = ImageMetaData::createMetaData(filePath);
    if(!metaData)
        metaData = new ImageMetaData;

#define ADD_ENTRY(VAR, STR) \
    do \
    { \
        const QString value = toString(decoder->WMP.sDescMetadata.VAR); \
        if(!value.isEmpty()) \
            metaData->addCustomEntry(QString::fromLatin1("WMP"), QString::fromLatin1(STR), value); \
    } \
    while(false)
    ADD_ENTRY(pvarImageDescription, "ImageDescription");
    ADD_ENTRY(pvarCameraMake,       "CameraMake");
    ADD_ENTRY(pvarCameraModel,      "CameraModel");
    ADD_ENTRY(pvarSoftware,         "Software");
    ADD_ENTRY(pvarDateTime,         "DateTime");
    ADD_ENTRY(pvarArtist,           "Artist");
    ADD_ENTRY(pvarCopyright,        "Copyright");
    ADD_ENTRY(pvarRatingStars,      "RatingStars");
    ADD_ENTRY(pvarRatingValue,      "RatingValue");
    ADD_ENTRY(pvarCaption,          "Caption");
    ADD_ENTRY(pvarDocumentName,     "DocumentName");
    ADD_ENTRY(pvarPageName,         "PageName");
    ADD_ENTRY(pvarPageNumber,       "PageNumber");
    ADD_ENTRY(pvarHostComputer,     "HostComputer");
#undef ADD_ENTRY

    QByteArray iccProfileData;
#define ADD_METADATA(FUNC, ARG) \
    do \
    { \
        const ERR err = FUNC(decoder.data(), &ARG); \
        if(Failed(err)) \
            LOG_WARNING() << LOGGING_CTX << #FUNC " failed:" << errorToString(err); \
    } \
    while(false)
    ADD_METADATA(getXmpMetadata, metaData);
    ADD_METADATA(getExifMetadata, metaData);
    ADD_METADATA(getIccProfileData, iccProfileData);
#undef ADD_METADATA

    {
        Float resolutionX = 0.0f, resolutionY = 0.0f;
        const ERR err = decoder->GetResolution(decoder.data(), &resolutionX, &resolutionY);
        if(Failed(err))
            LOG_WARNING() << LOGGING_CTX << "GetResolution failed:" << errorToString(err);
        else
            metaData->addCustomDpi(static_cast<qreal>(resolutionX), static_cast<qreal>(resolutionY));

        switch(decoder->WMP.oOrientationFromContainer)
        {
        case O_NONE:
            metaData->addCustomOrientation(1);
            break;
        case O_FLIPV:
            metaData->addCustomOrientation(4);
            break;
        case O_FLIPH:
            metaData->addCustomOrientation(2);
            break;
        case O_FLIPVH:
            metaData->addCustomOrientation(3);
            break;
        case O_RCW:
            metaData->addCustomOrientation(6);
            break;
        case O_RCW_FLIPV:
            metaData->addCustomOrientation(5);
            break;
        case O_RCW_FLIPH:
            metaData->addCustomOrientation(7);
            break;
        case O_RCW_FLIPVH:
            metaData->addCustomOrientation(8);
            break;
        default:
            break;
        }
    }

    QImage result;
    if(IsEqualGUID(pixelFormat, GUID_PKPixelFormatBlackWhite))
    {
        directCopy(decoder.data(), rect, result, QImage::Format_Mono);
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat8bppGray))
    {
#if (USE_GRAYSCALE_8)
        directCopy(decoder.data(), rect, result, QImage::Format_Grayscale8);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 8 / 8, convertFromGray<quint8>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat16bppRGB555))
    {
        directCopy(decoder.data(), rect, result, QImage::Format_RGB555);
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat16bppRGB565))
    {
        directCopy(decoder.data(), rect, result, QImage::Format_RGB16);
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat16bppGray))
    {
#if (USE_GRAYSCALE_16)
        directCopy(decoder.data(), rect, result, QImage::Format_Grayscale16);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 16 / 8, convertFromGray<quint16>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat24bppBGR))
    {
#if (USE_BGR_888)
        directCopy(decoder.data(), rect, result, QImage::Format_BGR888);
#else
        directCopy(decoder.data(), rect, result, QImage::Format_RGB888);
        QImage_rgbSwap(result);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat24bppRGB) || IsEqualGUID(pixelFormat, GUID_PKPixelFormat24bpp3Channels))
    {
        directCopy(decoder.data(), rect, result, QImage::Format_RGB888);
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat32bppBGR))
    {
#if (USE_RGBX_8888)
        directCopy(decoder.data(), rect, result, QImage::Format_RGBX8888);
        QImage_rgbSwap(result);
#else
        directCopy(decoder.data(), rect, result, QImage::Format_RGB32);
        postprocessARGB32(result, convertFromBGR<quint8>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat32bppBGRA))
    {
        decoder->WMP.wmiSCP.uAlphaMode = 2;
#if (USE_RGBX_8888)
        directCopy(decoder.data(), rect, result, QImage::Format_RGBA8888);
        QImage_rgbSwap(result);
#else
        directCopy(decoder.data(), rect, result, QImage::Format_ARGB32);
        postprocessARGB32(result, convertFromBGRA<quint8>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat32bppPBGRA))
    {
        decoder->WMP.wmiSCP.uAlphaMode = 2;
#if (USE_RGBX_8888)
        directCopy(decoder.data(), rect, result, QImage::Format_RGBA8888_Premultiplied);
        QImage_convertTo(result, QImage::Format_RGBA8888);
        QImage_rgbSwap(result);
#else
        directCopy(decoder.data(), rect, result, QImage::Format_ARGB32);
        postprocessARGB32(result, convertFromPBGRA<quint8>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat32bppGrayFloat))
    {
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 32 / 8, convertFromGrayFloat<float>);
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat32bppRGB))
    {
#if (USE_RGBX_8888)
        directCopy(decoder.data(), rect, result, QImage::Format_RGBX8888);
#else
        directCopy(decoder.data(), rect, result, QImage::Format_RGB32);
        postprocessARGB32(result, convertFromRGB<quint8>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat32bppRGBA) || IsEqualGUID(pixelFormat, GUID_PKPixelFormat32bpp3ChannelsAlpha))
    {
        decoder->WMP.wmiSCP.uAlphaMode = 2;
#if (USE_RGBX_8888)
        directCopy(decoder.data(), rect, result, QImage::Format_RGBA8888);
#else
        directCopy(decoder.data(), rect, result, QImage::Format_ARGB32);
        postprocessARGB32(result, convertFromRGBA<quint8>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat32bppPRGBA))
    {
        decoder->WMP.wmiSCP.uAlphaMode = 2;
#if (USE_RGBX_8888)
        directCopy(decoder.data(), rect, result, QImage::Format_RGBA8888_Premultiplied);
        QImage_convertTo(result, QImage::Format_RGBA8888);
#else
        directCopy(decoder.data(), rect, result, QImage::Format_ARGB32);
        postprocessARGB32(result, convertFromPRGBA<quint8>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat48bppRGBFixedPoint))
    {
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 48 / 8, convertFromRGBFixed<I16>);
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat16bppGrayFixedPoint))
    {
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 16 / 8, convertFromGrayFixed<I16>);
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat32bppRGB101010))
    {
#if (USE_RGB_30)
        directCopy(decoder.data(), rect, result, QImage::Format_RGB30);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 32 / 8, convertFromRGB101010<U32>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat48bppRGB) || IsEqualGUID(pixelFormat, GUID_PKPixelFormat48bpp3Channels))
    {
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 48 / 8, convertFromRGB<quint16>);
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat64bppRGBA) || IsEqualGUID(pixelFormat, GUID_PKPixelFormat64bpp3ChannelsAlpha))
    {
        decoder->WMP.wmiSCP.uAlphaMode = 2;
#if (USE_RGBA_64)
        directCopy(decoder.data(), rect, result, QImage::Format_RGBA64);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_ARGB32, 64 / 8, convertFromRGBA<quint16>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat64bppPRGBA))
    {
        decoder->WMP.wmiSCP.uAlphaMode = 2;
#if (USE_RGBA_64)
        directCopy(decoder.data(), rect, result, QImage::Format_RGBA64_Premultiplied);
        QImage_convertTo(result, QImage::Format_RGBA64);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_ARGB32, 64 / 8, convertFromPRGBA<quint16>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat96bppRGBFixedPoint))
    {
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 96 / 8, convertFromRGBFixed<I32>);
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat96bppRGBFloat))
    {
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 96 / 8, convertFromRGBFloat<float>);
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat128bppRGBAFloat))
    {
        decoder->WMP.wmiSCP.uAlphaMode = 2;
#if (USE_RGBX_32FPx4)
        directCopy(decoder.data(), rect, result, QImage::Format_RGBA32FPx4);
        postprocessscRgbFloat(result);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_ARGB32, 128 / 8, convertFromRGBAFloat<float>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat128bppPRGBAFloat))
    {
        decoder->WMP.wmiSCP.uAlphaMode = 2;
#if (USE_RGBX_32FPx4)
        directCopy(decoder.data(), rect, result, QImage::Format_RGBA32FPx4_Premultiplied);
        QImage_convertTo(result, QImage::Format_RGBA32FPx4);
        postprocessscRgbFloat(result);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_ARGB32, 128 / 8, convertFromPRGBAFloat<float>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat128bppRGBFloat))
    {
#if (USE_RGBX_32FPx4)
        directCopy(decoder.data(), rect, result, QImage::Format_RGBX32FPx4);
        postprocessscRgbFloat(result);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 128 / 8, convertFromRGBFloat<float>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat32bppCMYK) || IsEqualGUID(pixelFormat, GUID_PKPixelFormat32bpp4Channels))
    {
        if(iccProfileData.isEmpty())
            iccProfileData = ICCProfile::defaultCmykProfileData();
#if (USE_CMYK_8888)
        directCopy(decoder.data(), rect, result, QImage::Format_CMYK8888);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 32 / 8, convertFromCMYK<quint8>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat64bppRGBAFixedPoint))
    {
        decoder->WMP.wmiSCP.uAlphaMode = 2;
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_ARGB32, 64 / 8, convertFromRGBAFixed<I16>);
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat64bppRGBFixedPoint))
    {
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 64 / 8, convertFromRGBFixed<I16>);
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat128bppRGBAFixedPoint))
    {
        decoder->WMP.wmiSCP.uAlphaMode = 2;
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_ARGB32, 128 / 8, convertFromRGBAFixed<I32>);
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat128bppRGBFixedPoint))
    {
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 128 / 8, convertFromRGBFixed<I32>);
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat64bppRGBAHalf))
    {
        decoder->WMP.wmiSCP.uAlphaMode = 2;
#if (USE_RGBX_32FPx4)
        directCopy(decoder.data(), rect, result, QImage::Format_RGBA16FPx4);
        QImage_convertTo(result, QImage::Format_RGBA32FPx4);
        postprocessscRgbFloat(result);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_ARGB32, 64 / 8, convertFromRGBAHalf<U16>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat64bppRGBHalf))
    {
#if (USE_RGBX_32FPx4)
        directCopy(decoder.data(), rect, result, QImage::Format_RGBA16FPx4); /// @todo Format_RGBX16FPx4 completely black with Qt 6.8.0-beta2
        QImage_convertTo(result, QImage::Format_RGBX32FPx4);
        postprocessscRgbFloat(result);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 64 / 8, convertFromRGBHalf<U16>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat48bppRGBHalf))
    {
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 48 / 8, convertFromRGBHalf<U16>);
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat32bppRGBE))
    {
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 32 / 8, convertFromRGBE<U8>);
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat16bppGrayHalf))
    {
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 16 / 8, convertFromGrayHalf<U16>);
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat32bppGrayFixedPoint))
    {
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 32 / 8, convertFromGrayFixed<I32>);
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat64bppCMYK) || IsEqualGUID(pixelFormat, GUID_PKPixelFormat64bpp4Channels))
    {
        if(iccProfileData.isEmpty())
            iccProfileData = ICCProfile::defaultCmykProfileData();
#if (USE_CMYK_8888)
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_CMYK8888, 64 / 8, convertFromCMYKtoCMYK8888<quint16>);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 64 / 8, convertFromCMYK<quint16>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat40bppCMYKAlpha) || IsEqualGUID(pixelFormat, GUID_PKPixelFormat40bpp4ChannelsAlpha))
    {
        if(iccProfileData.isEmpty())
            iccProfileData = ICCProfile::defaultCmykProfileData();
        decoder->WMP.wmiSCP.uAlphaMode = 2;
#if (USE_CMYK_8888)
        copyViaBufferCMYKA<quint8>(decoder.data(), rect, result, 40 / 8, iccProfileData);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_ARGB32, 40 / 8, convertFromCMYKA<quint8, 4>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat80bppCMYKAlpha) || IsEqualGUID(pixelFormat, GUID_PKPixelFormat80bpp4ChannelsAlpha))
    {
        if(iccProfileData.isEmpty())
            iccProfileData = ICCProfile::defaultCmykProfileData();
        decoder->WMP.wmiSCP.uAlphaMode = 2;
#if (USE_CMYK_8888)
        copyViaBufferCMYKA<quint16>(decoder.data(), rect, result, 80 / 8, iccProfileData);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_ARGB32, 80 / 8, convertFromCMYKA<quint16, 4>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat40bpp5Channels))
    {
        if(iccProfileData.isEmpty())
            iccProfileData = ICCProfile::defaultCmykProfileData();
#if (USE_CMYK_8888)
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_CMYK8888, 40 / 8, convertFromCMYKtoCMYK8888<quint8>);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 40 / 8, convertFromCMYK<quint8>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat48bpp6Channels))
    {
        if(iccProfileData.isEmpty())
            iccProfileData = ICCProfile::defaultCmykProfileData();
#if (USE_CMYK_8888)
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_CMYK8888, 48 / 8, convertFromCMYKtoCMYK8888<quint8>);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 48 / 8, convertFromCMYK<quint8>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat56bpp7Channels))
    {
        if(iccProfileData.isEmpty())
            iccProfileData = ICCProfile::defaultCmykProfileData();
#if (USE_CMYK_8888)
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_CMYK8888, 56 / 8, convertFromCMYKtoCMYK8888<quint8>);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 56 / 8, convertFromCMYK<quint8>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat64bpp8Channels))
    {
        if(iccProfileData.isEmpty())
            iccProfileData = ICCProfile::defaultCmykProfileData();
#if (USE_CMYK_8888)
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_CMYK8888, 64 / 8, convertFromCMYKtoCMYK8888<quint8>);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 64 / 8, convertFromCMYK<quint8>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat80bpp5Channels))
    {
        if(iccProfileData.isEmpty())
            iccProfileData = ICCProfile::defaultCmykProfileData();
#if (USE_CMYK_8888)
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_CMYK8888, 80 / 8, convertFromCMYKtoCMYK8888<quint16>);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 80 / 8, convertFromCMYK<quint16>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat96bpp6Channels))
    {
        if(iccProfileData.isEmpty())
            iccProfileData = ICCProfile::defaultCmykProfileData();
#if (USE_CMYK_8888)
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_CMYK8888, 96 / 8, convertFromCMYKtoCMYK8888<quint16>);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 96 / 8, convertFromCMYK<quint16>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat112bpp7Channels))
    {
        if(iccProfileData.isEmpty())
            iccProfileData = ICCProfile::defaultCmykProfileData();
#if (USE_CMYK_8888)
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_CMYK8888, 112 / 8, convertFromCMYKtoCMYK8888<quint16>);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 112 / 8, convertFromCMYK<quint16>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat128bpp8Channels))
    {
        if(iccProfileData.isEmpty())
            iccProfileData = ICCProfile::defaultCmykProfileData();
#if (USE_CMYK_8888)
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_CMYK8888, 128 / 8, convertFromCMYKtoCMYK8888<quint16>);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_RGB32, 128 / 8, convertFromCMYK<quint16>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat48bpp5ChannelsAlpha))
    {
        if(iccProfileData.isEmpty())
            iccProfileData = ICCProfile::defaultCmykProfileData();
        decoder->WMP.wmiSCP.uAlphaMode = 2;
#if (USE_CMYK_8888)
        copyViaBufferCMYKA<quint8>(decoder.data(), rect, result, 48 / 8, iccProfileData);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_ARGB32, 48 / 8, convertFromCMYKA<quint8, 5>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat56bpp6ChannelsAlpha))
    {
        if(iccProfileData.isEmpty())
            iccProfileData = ICCProfile::defaultCmykProfileData();
        decoder->WMP.wmiSCP.uAlphaMode = 2;
#if (USE_CMYK_8888)
        copyViaBufferCMYKA<quint8>(decoder.data(), rect, result, 56 / 8, iccProfileData);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_ARGB32, 56 / 8, convertFromCMYKA<quint8, 6>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat64bpp7ChannelsAlpha))
    {
        if(iccProfileData.isEmpty())
            iccProfileData = ICCProfile::defaultCmykProfileData();
        decoder->WMP.wmiSCP.uAlphaMode = 2;
#if (USE_CMYK_8888)
        copyViaBufferCMYKA<quint8>(decoder.data(), rect, result, 64 / 8, iccProfileData);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_ARGB32, 64 / 8, convertFromCMYKA<quint8, 7>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat72bpp8ChannelsAlpha))
    {
        if(iccProfileData.isEmpty())
            iccProfileData = ICCProfile::defaultCmykProfileData();
        decoder->WMP.wmiSCP.uAlphaMode = 2;
#if (USE_CMYK_8888)
        copyViaBufferCMYKA<quint8>(decoder.data(), rect, result, 72 / 8, iccProfileData);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_ARGB32, 72 / 8, convertFromCMYKA<quint8, 8>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat96bpp5ChannelsAlpha))
    {
        if(iccProfileData.isEmpty())
            iccProfileData = ICCProfile::defaultCmykProfileData();
        decoder->WMP.wmiSCP.uAlphaMode = 2;
#if (USE_CMYK_8888)
        copyViaBufferCMYKA<quint16>(decoder.data(), rect, result, 96 / 8, iccProfileData);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_ARGB32, 96 / 8, convertFromCMYKA<quint16, 5>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat112bpp6ChannelsAlpha))
    {
        if(iccProfileData.isEmpty())
            iccProfileData = ICCProfile::defaultCmykProfileData();
        decoder->WMP.wmiSCP.uAlphaMode = 2;
#if (USE_CMYK_8888)
        copyViaBufferCMYKA<quint16>(decoder.data(), rect, result, 112 / 8, iccProfileData);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_ARGB32, 112 / 8, convertFromCMYKA<quint16, 6>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat128bpp7ChannelsAlpha))
    {
        if(iccProfileData.isEmpty())
            iccProfileData = ICCProfile::defaultCmykProfileData();
        decoder->WMP.wmiSCP.uAlphaMode = 2;
#if (USE_CMYK_8888)
        copyViaBufferCMYKA<quint16>(decoder.data(), rect, result, 128 / 8, iccProfileData);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_ARGB32, 128 / 8, convertFromCMYKA<quint16, 7>);
#endif
    }
    else if(IsEqualGUID(pixelFormat, GUID_PKPixelFormat144bpp8ChannelsAlpha))
    {
        if(iccProfileData.isEmpty())
            iccProfileData = ICCProfile::defaultCmykProfileData();
        decoder->WMP.wmiSCP.uAlphaMode = 2;
#if (USE_CMYK_8888)
        copyViaBufferCMYKA<quint16>(decoder.data(), rect, result, 144 / 8, iccProfileData);
#else
        copyViaBuffer(decoder.data(), rect, result, QImage::Format_ARGB32, 144 / 8, convertFromCMYKA<quint16, 8>);
#endif
    }

    if(!iccProfileData.isEmpty())
        ICCProfile(iccProfileData).applyToImage(&result);

    // Some image formats can't be rendered successfully
    if(!IsOneOf(result.format(), QImage::Format_RGB32, QImage::Format_ARGB32))
        QImage_convertTo(result, result.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32);

    return PayloadWithMetaData<QImage>(result, metaData);
}

class DecoderJxrLib : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderJxrLib");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
                << QString::fromLatin1("jxr")
                << QString::fromLatin1("hdp")
                << QString::fromLatin1("wdp");
    }

    QStringList advancedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList();
    }

    bool isAvailable() const Q_DECL_OVERRIDE
    {
        return true;
    }

    QSharedPointer<IImageData> loadImage(const QString &filePath) Q_DECL_OVERRIDE
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return QSharedPointer<IImageData>();
        const PayloadWithMetaData<QImage> readData = readJxrFile(filePath);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createImageItem(readData);
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), readData.metaData()));
    }
};

DecoderAutoRegistrator registrator(new DecoderJxrLib);

} // namespace
