/*
   Copyright (C) 2021 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <cstring>
#include <limits>

#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QDebug>
#include <QVector>
#include <QList>
#include <QPair>

#include "Utils/Global.h"
#include "Utils/ScopedPointer.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"
#include "Internal/PayloadWithMetaData.h"

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

namespace
{

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

#define Report_WARN(err, szExp, szFile, nLine) \
    qWarning() << "FAILED:" << errorToString(err) << "in" << szExp; \
    qWarning() << "       " << szFile << ":" << nLine; \
    err = err
#define Report_DEBUG(err, szExp, szFile, nLine) \
    qWarning() << "FAILED:" << errorToString(err) << "in" << szExp; \
    qWarning() << "       " << szFile << ":" << nLine; \
    err = err
#if defined (Report)
#undef Report
#define Report Report_WARN
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
        qDebug() << "Found XMP metadata";
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
        qDebug() << "Found EXIF metadata";
        Call(getMetadata(decoder, rawMetaData, offset, byteCount));
        *metaData = ImageMetaData::joinMetaData(*metaData, ImageMetaData::createExifMetaData(rawMetaData));
    }
Cleanup:
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

template<typename T>
QRgb convertFromGray(const T *data)
{
    const double scale = static_cast<double>(std::numeric_limits<T>::max());
    const int c = qBound<int>(0, static_cast<int>(255.0 * data[0] / scale), 255);
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
QRgb convertFromRGBA(const T *data)
{
    const double scale = static_cast<double>(std::numeric_limits<T>::max());
    const int a = qBound<int>(0, static_cast<int>(255.0 * data[3] / scale), 255);
    const int r = qBound<int>(0, static_cast<int>(255.0 * data[0] / scale), 255);
    const int g = qBound<int>(0, static_cast<int>(255.0 * data[1] / scale), 255);
    const int b = qBound<int>(0, static_cast<int>(255.0 * data[2] / scale), 255);
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
QRgb convertFromCMYKA(const T *data)
{
    const double scale = static_cast<double>(std::numeric_limits<T>::max());
    const double c = data[0] / scale;
    const double m = data[1] / scale;
    const double y = data[2] / scale;
    const double k = data[3] / scale;
    const int a = qBound<int>(0, static_cast<int>(255.0 * data[4] / scale), 255);
    const int r = qBound<int>(0, static_cast<int>(255.0 * (1.0 - c) * (1.0 - k)), 255);
    const int g = qBound<int>(0, static_cast<int>(255.0 * (1.0 - m) * (1.0 - k)), 255);
    const int b = qBound<int>(0, static_cast<int>(255.0 * (1.0 - y) * (1.0 - k)), 255);
    return qRgba(r, g, b, a);
}

QRgb convertFrom8bppGray(const uchar *data)
{
    return convertFromGray(data);
}

QRgb convertFrom24bppRGB(const uchar *data)
{
    return convertFromRGB(data);
}

QRgb convertFrom32bppRGBA(const uchar *data)
{
    return convertFromRGBA(data);
}

QRgb convertFrom32bppCMYK(const uchar *data)
{
    return convertFromCMYK(data);
}

QRgb convertFrom64bppCMYK(const uchar *data)
{
    return convertFromCMYK(reinterpret_cast<const quint16*>(data));
}

QRgb convertFrom40bppCMYKAlpha(const uchar *data)
{
    return convertFromCMYKA(data);
}

QRgb convertFrom80bppCMYKAlpha(const uchar *data)
{
    return convertFromCMYKA(reinterpret_cast<const quint16*>(data));
}

PayloadWithMetaData<QImage> readJxrFile(const QString &filePath)
{
    ERR err = WMP_errSuccess;
    PKCodecFactory *codec_factory = Q_NULLPTR;
    PKImageDecode *decoder = Q_NULLPTR;
    PKFormatConverter *converter = Q_NULLPTR;
    PKRect rect;
    PKPixelInfo PI;
    static const QList<QPair<const PKPixelFormatGUID*, QRgb(*)(const uchar*)> > convertableFormats =
            QList<QPair<const PKPixelFormatGUID*, QRgb(*)(const uchar*)> >()
            << qMakePair<const PKPixelFormatGUID*, QRgb(*)(const uchar*)>(&GUID_PKPixelFormat32bppRGBA, &convertFrom32bppRGBA)
            << qMakePair<const PKPixelFormatGUID*, QRgb(*)(const uchar*)>(&GUID_PKPixelFormat24bppRGB, &convertFrom24bppRGB)
            << qMakePair<const PKPixelFormatGUID*, QRgb(*)(const uchar*)>(&GUID_PKPixelFormat8bppGray, &convertFrom8bppGray)
            << qMakePair<const PKPixelFormatGUID*, QRgb(*)(const uchar*)>(&GUID_PKPixelFormat32bppCMYK, &convertFrom32bppCMYK)
            << qMakePair<const PKPixelFormatGUID*, QRgb(*)(const uchar*)>(&GUID_PKPixelFormat64bppCMYK, &convertFrom64bppCMYK)
            << qMakePair<const PKPixelFormatGUID*, QRgb(*)(const uchar*)>(&GUID_PKPixelFormat40bppCMYKAlpha, &convertFrom40bppCMYKAlpha)
            << qMakePair<const PKPixelFormatGUID*, QRgb(*)(const uchar*)>(&GUID_PKPixelFormat80bppCMYKAlpha, &convertFrom80bppCMYKAlpha)
            ;
    static const QList<QPair<const PKPixelFormatGUID*, const PKPixelFormatGUID*> > premultipliedFormats =
            QList<QPair<const PKPixelFormatGUID*, const PKPixelFormatGUID*> >()
            << qMakePair<const PKPixelFormatGUID*, const PKPixelFormatGUID*>(&GUID_PKPixelFormat32bppPBGRA, &GUID_PKPixelFormat32bppBGRA)
            << qMakePair<const PKPixelFormatGUID*, const PKPixelFormatGUID*>(&GUID_PKPixelFormat32bppPRGBA, &GUID_PKPixelFormat32bppRGBA)
            << qMakePair<const PKPixelFormatGUID*, const PKPixelFormatGUID*>(&GUID_PKPixelFormat64bppPRGBA, &GUID_PKPixelFormat64bppRGBA)
            << qMakePair<const PKPixelFormatGUID*, const PKPixelFormatGUID*>(&GUID_PKPixelFormat128bppPRGBAFloat, &GUID_PKPixelFormat128bppRGBAFloat)
            ;

    QImage result;
    ImageMetaData *metaData = ImageMetaData::createMetaData(filePath);
    if(!metaData)
        metaData = new ImageMetaData;

    bool hasAlpha = false, hasPremultipliedAlpha = false;
    int bpp, bpl, bpi;
    void *dataBuffer = Q_NULLPTR;

    const QByteArray filename = filePath.toLocal8Bit();
    Call(PKCreateCodecFactory(&codec_factory, WMP_SDK_VERSION));
    Call(codec_factory->CreateDecoderFromFile(filename.data(), &decoder));
    Call(codec_factory->CreateFormatConverter(&converter));

    memset(&PI, 0, sizeof(PI));
    PI.pGUIDPixFmt = &decoder->guidPixFormat;
    Call(PixelFormatLookup(&PI, LOOKUP_FORWARD));
    Call(PixelFormatLookup(&PI, LOOKUP_BACKWARD_TIF));
    hasAlpha = !!(PI.grBit & PK_pixfmtHasAlpha);
    if(const char *pixelFormatStr = pixelFormatToString(*PI.pGUIDPixFmt))
    {
        qDebug("Color format: %s", pixelFormatStr);
    }
    else
    {
        qDebug("Color format: %08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X",
               PI.pGUIDPixFmt->Data1, PI.pGUIDPixFmt->Data2, PI.pGUIDPixFmt->Data3,
               PI.pGUIDPixFmt->Data4[0], PI.pGUIDPixFmt->Data4[1], PI.pGUIDPixFmt->Data4[2],
               PI.pGUIDPixFmt->Data4[3], PI.pGUIDPixFmt->Data4[4], PI.pGUIDPixFmt->Data4[5],
               PI.pGUIDPixFmt->Data4[6], PI.pGUIDPixFmt->Data4[7]);
    }

    for(QList<QPair<const PKPixelFormatGUID*, const PKPixelFormatGUID*> >::ConstIterator it = premultipliedFormats.constBegin(); it != premultipliedFormats.constEnd(); ++it)
    {
        if(!IsEqualGUID(*it->first, *PI.pGUIDPixFmt))
            continue;
        PI.pGUIDPixFmt = it->second;
        hasPremultipliedAlpha = true;
        break;
    }

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

    CallIgnoreError(err, getXmpMetadata(decoder, &metaData));
    CallIgnoreError(err, getExifMetadata(decoder, &metaData));
    err = WMP_errSuccess;

    decoder->guidPixFormat = *PI.pGUIDPixFmt;
    decoder->WMP.wmiI.bRGB = !(PI.grBit & PK_pixfmtBGR);
    decoder->WMP.wmiSCP.bfBitstreamFormat = SPATIAL;
    decoder->WMP.wmiSCP.uAlphaMode = hasAlpha ? 2 : 0;
    decoder->WMP.wmiSCP.sbSubband = SB_ALL;
    decoder->WMP.bIgnoreOverlap = FALSE;
    decoder->WMP.wmiI.cfColorFormat = PI.cfColorFormat;
    decoder->WMP.wmiI.bdBitDepth = PI.bdBitDepth;
    decoder->WMP.wmiI.cBitsPerUnit = PI.cbitUnit;
    decoder->WMP.wmiI.cThumbnailWidth = decoder->WMP.wmiI.cWidth;
    decoder->WMP.wmiI.cThumbnailHeight = decoder->WMP.wmiI.cHeight;
    decoder->WMP.wmiI.bSkipFlexbits = FALSE;
    decoder->WMP.wmiI.cROILeftX = 0;
    decoder->WMP.wmiI.cROITopY = 0;
    decoder->WMP.wmiI.cROIWidth = decoder->WMP.wmiI.cThumbnailWidth;
    decoder->WMP.wmiI.cROIHeight = decoder->WMP.wmiI.cThumbnailHeight;
    decoder->WMP.wmiI.oOrientation = O_NONE;
    decoder->WMP.wmiI.cPostProcStrength = 4;
    decoder->WMP.wmiSCP.bVerbose = FALSE;

    rect.X = static_cast<I32>(decoder->WMP.wmiI.cROILeftX);
    rect.Y = static_cast<I32>(decoder->WMP.wmiI.cROITopY);
    rect.Width = static_cast<I32>(decoder->WMP.wmiI.cROIWidth);
    rect.Height = static_cast<I32>(decoder->WMP.wmiI.cROIHeight);

    result = QImage(rect.Width, rect.Height, hasAlpha ? (hasPremultipliedAlpha ? QImage::Format_ARGB32_Premultiplied : QImage::Format_ARGB32) : QImage::Format_RGB32);

    bpp = 144 / 8;
    bpl = bpp * static_cast<int>(rect.Width);
    bpi = bpl * static_cast<int>(rect.Height);
    Call(PKAllocAligned(&dataBuffer, bpi, 128));

    for(QList<QPair<const PKPixelFormatGUID*, QRgb(*)(const uchar*)> >::ConstIterator it = convertableFormats.constBegin(); it != convertableFormats.constEnd(); ++it)
    {
        PKPixelInfo targetPI;
        memset(&targetPI, 0, sizeof(targetPI));
        targetPI.pGUIDPixFmt = it->first;

        if(Failed(PixelFormatLookup(&targetPI, LOOKUP_FORWARD)))
            continue;
        if(Failed(converter->Initialize(converter, decoder, Q_NULLPTR, *it->first)))
            continue;
        if(Failed(converter->Copy(converter, &rect, reinterpret_cast<U8*>(dataBuffer), static_cast<U32>(bpl))))
            continue;

        for(int i = 0; i < result.height(); ++i)
        {
            QRgb *out = reinterpret_cast<QRgb*>(result.scanLine(i));
            uchar *in = reinterpret_cast<uchar*>(dataBuffer) + i * bpl;
            for(int j = 0; j < result.width(); ++j)
                out[j] = it->second(in + j * targetPI.uSamplePerPixel * targetPI.uBitsPerSample / 8);
        }

        qDebug("Decoded with convertioin to format: %s", pixelFormatToString(*it->first));
        goto Cleanup;
    }

    memset(&PI, 0, sizeof(PI));
    PI.pGUIDPixFmt = &GUID_PKPixelFormat32bppRGBA;
    Call(PixelFormatLookup(&PI, LOOKUP_FORWARD));
    decoder->guidPixFormat = *PI.pGUIDPixFmt;
    decoder->WMP.wmiI.bRGB = 0;
    decoder->WMP.wmiI.cfColorFormat = PI.cfColorFormat;
    decoder->WMP.wmiI.bdBitDepth = PI.bdBitDepth;
    decoder->WMP.wmiI.cBitsPerUnit = PI.cbitUnit;
    Call(converter->Initialize(converter, decoder, Q_NULLPTR, *PI.pGUIDPixFmt));
    Call(converter->Copy(converter, &rect, reinterpret_cast<U8*>(dataBuffer), static_cast<U32>(bpl)));

    for(int i = 0; i < result.height(); ++i)
    {
        uchar *out = result.scanLine(i);
        uchar *in = reinterpret_cast<uchar*>(dataBuffer) + i * bpl;
        memcpy(out, in, result.bytesPerLine());
    }

    qDebug("Decoded with fallback convertioin to format: %s", pixelFormatToString(*PI.pGUIDPixFmt));

Cleanup:
    if(Failed(err))
        result = QImage();
    if(dataBuffer)
        PKFreeAligned(&dataBuffer);
    if(converter)
        converter->Release(&converter);
    if(decoder)
        decoder->Release(&decoder);
    if(codec_factory)
        codec_factory->Release(&codec_factory);

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
