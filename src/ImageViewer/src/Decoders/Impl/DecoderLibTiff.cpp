/*
   Copyright (C) 2017-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <limits>

#if !defined (TIFF_DISABLE_DEPRECATED)
#define TIFF_DISABLE_DEPRECATED
#endif
#include <tiffio.h>

#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QByteArray>

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

#define USE_RGBA_8888   (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0) && Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
#define USE_GRAYSCALE_8 (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
#define USE_CMYK_8888   (QT_VERSION >= QT_VERSION_CHECK(6, 8, 0))
// #define DEBUG_FORCE_BIT_ACCESS

#if defined (NDEBUG)
#define BUFFER_FILL_PATTERN 0
#else
#define BUFFER_FILL_PATTERN 0x7f
#endif
#define BUFFER_MAX_SIZE (static_cast<qint64>(std::numeric_limits<int>::max()))

namespace
{

// ====================================================================================================

struct Context
{
    TIFF *tiff;
    qint64 width;
    qint64 height;
    quint16 compression;
    quint16 photometric;
    qint64 samplesPerPixel;
    qint64 bitsPerSample;
    quint16 sampleFormat;
    quint16 inkSet;
    ICCProfile *iccProfile;
    const float *ycbcrcoeffs;
    qint64 subsamplinghor;
    qint64 subsamplingver;
    quint16 ycbcrpositioning;
    qint64 extrasamplesCount;
    qint64 primarysamplesCount;
    int alphaIndex;
    bool alphaPremultiplied;
    const quint16 *redTable;
    const quint16 *greenTable;
    const quint16 *blueTable;
    bool colorTablesIs16Bit;
};

// ====================================================================================================

QString photometricToString(quint16 photometric)
{
    switch(photometric)
    {
#define ADD_CASE(X) case X: return QString::fromLatin1(#X)
    ADD_CASE(PHOTOMETRIC_MINISWHITE);
    ADD_CASE(PHOTOMETRIC_MINISBLACK);
    ADD_CASE(PHOTOMETRIC_RGB);
    ADD_CASE(PHOTOMETRIC_PALETTE);
    ADD_CASE(PHOTOMETRIC_MASK);
    ADD_CASE(PHOTOMETRIC_SEPARATED);
    ADD_CASE(PHOTOMETRIC_YCBCR);
    ADD_CASE(PHOTOMETRIC_CIELAB);
    ADD_CASE(PHOTOMETRIC_ICCLAB);
    ADD_CASE(PHOTOMETRIC_ITULAB);
#if defined (PHOTOMETRIC_CFA)
    ADD_CASE(PHOTOMETRIC_CFA);
#endif
    ADD_CASE(PHOTOMETRIC_LOGL);
    ADD_CASE(PHOTOMETRIC_LOGLUV);
#undef ADD_CASE
    default:
        break;
    }
    return QString::fromLatin1("%1").arg(photometric);
}

QString inkSetToString(quint16 inkSet)
{
    switch(inkSet)
    {
#define ADD_CASE(X) case X: return QString::fromLatin1(#X)
    ADD_CASE(INKSET_CMYK);
    ADD_CASE(INKSET_MULTIINK);
#undef ADD_CASE
    default:
        break;
    }
    return QString::fromLatin1("%1").arg(inkSet);
}

QString orientationToString(quint16 orientation)
{
    switch(orientation)
    {
#define ADD_CASE(X) case X: return QString::fromLatin1(#X)
    ADD_CASE(ORIENTATION_TOPLEFT);
    ADD_CASE(ORIENTATION_TOPRIGHT);
    ADD_CASE(ORIENTATION_BOTRIGHT);
    ADD_CASE(ORIENTATION_BOTLEFT);
    ADD_CASE(ORIENTATION_LEFTTOP);
    ADD_CASE(ORIENTATION_RIGHTTOP);
    ADD_CASE(ORIENTATION_RIGHTBOT);
    ADD_CASE(ORIENTATION_LEFTBOT);
#undef ADD_CASE
    default:
        break;
    }
    return QString::fromLatin1("%1").arg(orientation);
}

QString sampleFormatToString(quint16 sampleFormat)
{
    switch(sampleFormat)
    {
#define ADD_CASE(X) case X: return QString::fromLatin1(#X)
    ADD_CASE(SAMPLEFORMAT_UINT);
    ADD_CASE(SAMPLEFORMAT_INT);
    ADD_CASE(SAMPLEFORMAT_IEEEFP);
    ADD_CASE(SAMPLEFORMAT_VOID);
    ADD_CASE(SAMPLEFORMAT_COMPLEXINT);
    ADD_CASE(SAMPLEFORMAT_COMPLEXIEEEFP);
#undef ADD_CASE
    default:
        break;
    }
    return QString::fromLatin1("%1").arg(sampleFormat);
}

QString extrasamplesToString(quint16 extrasamplesCount, const quint16 *extrasamples)
{
    QString result;
    for(quint16 i = 0; i < extrasamplesCount; ++i)
    {
        switch(extrasamples[i])
        {
#define APPEND_RESULT(S) result = result + (result.isEmpty() ? QString() : QString::fromLatin1(", ")) + (S)
#define ADD_CASE(X) case X: APPEND_RESULT(QString::fromLatin1(#X)); break
        ADD_CASE(EXTRASAMPLE_UNSPECIFIED);
        ADD_CASE(EXTRASAMPLE_ASSOCALPHA);
        ADD_CASE(EXTRASAMPLE_UNASSALPHA);
#undef ADD_CASE
        default:
            APPEND_RESULT(QString::fromLatin1("%1").arg(extrasamples[i]));
            break;
#undef APPEND_RESULT
        }
    }
    return QString::fromLatin1("(%1)").arg(result);
}

QString planarConfigToString(quint16 planarConfig)
{
    switch(planarConfig)
    {
#define ADD_CASE(X) case X: return QString::fromLatin1(#X)
    ADD_CASE(PLANARCONFIG_CONTIG);
    ADD_CASE(PLANARCONFIG_SEPARATE);
#undef ADD_CASE
    default:
        break;
    }
    return QString::fromLatin1("%1").arg(planarConfig);
}

QString compressionToString(quint16 compression)
{
    switch(compression)
    {
#define ADD_CASE(X) case X: return QString::fromLatin1(#X)
#if defined (COMPRESSION_NONE)
    ADD_CASE(COMPRESSION_NONE);
#endif
#if defined (COMPRESSION_CCITTRLE)
    ADD_CASE(COMPRESSION_CCITTRLE);
#endif
#if defined (COMPRESSION_CCITTFAX3)
    ADD_CASE(COMPRESSION_CCITTFAX3);
#elif defined (COMPRESSION_CCITT_T4)
    ADD_CASE(COMPRESSION_CCITT_T4);
#endif
#if defined (COMPRESSION_CCITTFAX4)
    ADD_CASE(COMPRESSION_CCITTFAX4);
#elif defined (COMPRESSION_CCITT_T6)
    ADD_CASE(COMPRESSION_CCITT_T6);
#endif
#if defined (COMPRESSION_LZW)
    ADD_CASE(COMPRESSION_LZW);
#endif
#if defined (COMPRESSION_OJPEG)
    ADD_CASE(COMPRESSION_OJPEG);
#endif
#if defined (COMPRESSION_JPEG)
    ADD_CASE(COMPRESSION_JPEG);
#endif
#if defined (COMPRESSION_T85)
    ADD_CASE(COMPRESSION_T85);
#endif
#if defined (COMPRESSION_T43)
    ADD_CASE(COMPRESSION_T43);
#endif
#if defined (COMPRESSION_NEXT)
    ADD_CASE(COMPRESSION_NEXT);
#endif
#if defined (COMPRESSION_CCITTRLEW)
    ADD_CASE(COMPRESSION_CCITTRLEW);
#endif
#if defined (COMPRESSION_PACKBITS)
    ADD_CASE(COMPRESSION_PACKBITS);
#endif
#if defined (COMPRESSION_THUNDERSCAN)
    ADD_CASE(COMPRESSION_THUNDERSCAN);
#endif
#if defined (COMPRESSION_IT8CTPAD)
    ADD_CASE(COMPRESSION_IT8CTPAD);
#endif
#if defined (COMPRESSION_IT8LW)
    ADD_CASE(COMPRESSION_IT8LW);
#endif
#if defined (COMPRESSION_IT8MP)
    ADD_CASE(COMPRESSION_IT8MP);
#endif
#if defined (COMPRESSION_IT8BL)
    ADD_CASE(COMPRESSION_IT8BL);
#endif
#if defined (COMPRESSION_PIXARFILM)
    ADD_CASE(COMPRESSION_PIXARFILM);
#endif
#if defined (COMPRESSION_PIXARLOG)
    ADD_CASE(COMPRESSION_PIXARLOG);
#endif
#if defined (COMPRESSION_DEFLATE)
    ADD_CASE(COMPRESSION_DEFLATE);
#endif
#if defined (COMPRESSION_ADOBE_DEFLATE)
    ADD_CASE(COMPRESSION_ADOBE_DEFLATE);
#endif
#if defined (COMPRESSION_DCS)
    ADD_CASE(COMPRESSION_DCS);
#endif
#if defined (COMPRESSION_JBIG)
    ADD_CASE(COMPRESSION_JBIG);
#endif
#if defined (COMPRESSION_SGILOG)
    ADD_CASE(COMPRESSION_SGILOG);
#endif
#if defined (COMPRESSION_SGILOG24)
    ADD_CASE(COMPRESSION_SGILOG24);
#endif
#if defined (COMPRESSION_JP2000)
    ADD_CASE(COMPRESSION_JP2000);
#endif
#if defined (COMPRESSION_LERC)
    ADD_CASE(COMPRESSION_LERC);
#endif
#if defined (COMPRESSION_LZMA)
    ADD_CASE(COMPRESSION_LZMA);
#endif
#if defined (COMPRESSION_ZSTD)
    ADD_CASE(COMPRESSION_ZSTD);
#endif
#if defined (COMPRESSION_WEBP)
    ADD_CASE(COMPRESSION_WEBP);
#endif
#if defined (COMPRESSION_JXL)
    ADD_CASE(COMPRESSION_JXL);
#endif
#undef ADD_CASE
    default:
        break;
    }
    return QString::fromLatin1("%1").arg(compression);
}

QString ycbcrpositioningToString(quint16 ycbcrpositioning)
{
    switch(ycbcrpositioning)
    {
#define ADD_CASE(X) case X: return QString::fromLatin1(#X)
        ADD_CASE(YCBCRPOSITION_CENTERED);
        ADD_CASE(YCBCRPOSITION_COSITED);
#undef ADD_CASE
    default:
        break;
    }
    return QString::fromLatin1("%1").arg(ycbcrpositioning);
}

// ====================================================================================================

tsize_t readProc(thandle_t fd, tdata_t buf, tsize_t size)
{
    QIODevice *device = static_cast<QIODevice*>(fd);
    return device->isReadable() ? device->read(static_cast<char*>(buf), size) : -1;
}

tsize_t writeProc(thandle_t fd, tdata_t buf, tsize_t size)
{
    QIODevice *device = static_cast<QIODevice*>(fd);
    return device->isWritable() ? device->write(static_cast<char*>(buf), size) : -1;
}

toff_t seekProc(thandle_t fd, toff_t off, int whence)
{
    QIODevice *device = static_cast<QIODevice*>(fd);
    const qint64 soff = static_cast<qint64>(off);
    switch(whence)
    {
    case SEEK_SET:
        device->seek(soff);
        break;
    case SEEK_CUR:
        device->seek(device->pos() + soff);
        break;
    case SEEK_END:
        device->seek(device->size() + soff);
        break;
    }
    return static_cast<toff_t>(device->pos());
}

int closeProc(thandle_t /*fd*/)
{
    return 0;
}

toff_t sizeProc(thandle_t fd)
{
    QIODevice *device = static_cast<QIODevice*>(fd);
    return static_cast<toff_t>(device->size());
}

int mapProc(thandle_t /*fd*/, tdata_t* /*pbase*/, toff_t* /*psize*/)
{
    return 0;
}

void unmapProc(thandle_t /*fd*/, tdata_t /*base*/, toff_t /*size*/)
{
}

#if defined (TIFFLIB_VERSION) && (TIFFLIB_VERSION >= 20221213) && (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
int errorHandlerProc(TIFF* /*tiff*/, void* /*user_data*/, const char *module, const char *fmt, va_list ap)
{
    LOG_WARNING() << LOGGING_CTXS(module) <<  QString::vasprintf(fmt, ap).toLocal8Bit().data();
    return 1;
}

int warningHandlerProc(TIFF* /*tiff*/, void* /*user_data*/, const char *module, const char *fmt, va_list ap)
{
    LOG_WARNING() << LOGGING_CTXS(module) <<  QString::vasprintf(fmt, ap).toLocal8Bit().data();
    return 1;
}
#endif

ICCProfile *readICCProfile(TIFF *tiff)
{
    unsigned iccProfileSize = 0;
    void *iccProfileData = Q_NULLPTR;
    if(TIFFGetField(tiff, TIFFTAG_ICCPROFILE, &iccProfileSize, &iccProfileData))
    {
        LOG_DEBUG() << LOGGING_CTX << "Found ICCP metadata (TIFFTAG_ICCPROFILE)";
        return new ICCProfile(QByteArray(reinterpret_cast<const char*>(iccProfileData), static_cast<int>(iccProfileSize)));
    }

    quint16 samplesPerPixel = 0;
    if(!TIFFGetFieldDefaulted(tiff, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel))
        return Q_NULLPTR;

    quint16 bitsPerSample = 0;
    if(!TIFFGetFieldDefaulted(tiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample))
        return Q_NULLPTR;

    float *whitePoint = Q_NULLPTR, *primaryChromaticities = Q_NULLPTR;
    unsigned short *transferFunctionRed = Q_NULLPTR, *transferFunctionGreen = Q_NULLPTR, *transferFunctionBlue = Q_NULLPTR;
    if(!TIFFGetField(tiff, TIFFTAG_WHITEPOINT, &whitePoint))
        whitePoint = Q_NULLPTR;
    if(!TIFFGetField(tiff, TIFFTAG_PRIMARYCHROMATICITIES, &primaryChromaticities))
        primaryChromaticities = Q_NULLPTR;
    if(samplesPerPixel == 1)
    {
        if(!TIFFGetField(tiff, TIFFTAG_TRANSFERFUNCTION, &transferFunctionRed, &transferFunctionGreen, &transferFunctionBlue))
            transferFunctionRed = transferFunctionGreen = transferFunctionBlue = Q_NULLPTR;
        else
            transferFunctionGreen = transferFunctionBlue = transferFunctionRed;
    }
    else
    {
        if(!TIFFGetField(tiff, TIFFTAG_TRANSFERFUNCTION, &transferFunctionRed, &transferFunctionGreen, &transferFunctionBlue))
            transferFunctionRed = transferFunctionGreen = transferFunctionBlue = Q_NULLPTR;
    }

    if(whitePoint || primaryChromaticities || transferFunctionRed || transferFunctionGreen || transferFunctionBlue)
    {
        LOG_DEBUG() << LOGGING_CTX << "Found ICCP metadata (TIFFTAG_WHITEPOINT + TIFFTAG_PRIMARYCHROMATICITIES + TIFFTAG_TRANSFERFUNCTION)";

        /// @note TIFF defaults (CIE D50) does not match sRGB defaults (CIE D65)
        if(!whitePoint && !TIFFGetFieldDefaulted(tiff, TIFFTAG_WHITEPOINT, &whitePoint))
            whitePoint = Q_NULLPTR;

        const size_t transferFunctionSize = (static_cast<std::size_t>(1) << bitsPerSample);
        return new ICCProfile(whitePoint, primaryChromaticities, transferFunctionRed, transferFunctionGreen, transferFunctionBlue, transferFunctionSize);
    }

    return Q_NULLPTR;
}

/// @note See https://learn.foundry.com/nuke/developers/63/ndkreference/examples/tiffReader.cpp
template<class T>
void addMetaData(TIFF *tiff, const TIFFField *field, ImageMetaData *metaData, const QString &group, const QString &tag)
{
    const int readCount = TIFFFieldReadCount(field);
    if(readCount == TIFF_VARIABLE2 || readCount == TIFF_VARIABLE || readCount > 1)
    {
        quint32 actualCount = 0;
        T *data;
        if(readCount == TIFF_VARIABLE)
        {
            quint16 gotCount = 0;
            if(!TIFFGetField(tiff, TIFFFieldTag(field), &gotCount, &data))
                return;
            actualCount = gotCount;
        }
        else if(readCount == TIFF_VARIABLE2)
        {
            quint32 gotCount = 0;
            if(!TIFFGetField(tiff, TIFFFieldTag(field), &gotCount, &data))
                return;
            actualCount = gotCount;
        }
        else
        {
            if(!TIFFGetField(tiff, TIFFFieldTag(field), &data))
                return;
            actualCount = readCount;
        }
        if(TIFFFieldDataType(field) == TIFF_UNDEFINED)
        {
            const char *charData = reinterpret_cast<const char*>(data);
            const int charSize = static_cast<int>(actualCount * sizeof(T) / sizeof(char));
            metaData->addExifEntry(group, TIFFFieldTag(field), tag, QString::fromLatin1(QByteArray(charData, charSize).toHex().prepend("0x")));
        }
        else
        {
            QStringList values;
            for(quint32 i = 0; i < actualCount; i++)
                values.append(QString::fromLatin1("%1").arg(data[i]));
            metaData->addExifEntry(group, TIFFFieldTag(field), tag, QString::fromLatin1("{ %1 }").arg(values.join(QString::fromLatin1(", "))));
        }
    }
    else if(readCount == 1)
    {
        T data;
        TIFFGetField(tiff, TIFFFieldTag(field), &data);
        metaData->addExifEntry(group, TIFFFieldTag(field), tag, QString::fromLatin1("%1").arg(data));
    }
}

template<>
void addMetaData<QString>(TIFF *tiff, const TIFFField *field, ImageMetaData *metaData, const QString &group, const QString &tag)
{
    if(TIFFFieldReadCount(field) <= 1)
        return;
    char *data = Q_NULLPTR;
    if(!TIFFGetField(tiff, TIFFFieldTag(field), &data) || !data)
        return;
    metaData->addExifEntry(group, TIFFFieldTag(field), tag, QString::fromUtf8(data));
}

void readTiffTagToMetaData(TIFF *tiff, ImageMetaData *&metaData, quint32 tag, const QString &tagDescription)
{
    const TIFFField *tagField = TIFFFindField(tiff, tag, TIFF_ANY);
    if(!tagField)
        return;
    if(TIFFFieldDataType(tagField) != TIFF_IFD8)
    {
        LOG_WARNING() << LOGGING_CTX << "TIFFFieldDataType for tag (" << tagDescription << ") is not TIFF_IFD8!";
        return;
    }
    quint64 exifOffset = 0;
    if(!TIFFGetField(tiff, tag, &exifOffset))
        return;
    if(!TIFFReadEXIFDirectory(tiff, exifOffset))
        return;
    LOG_DEBUG() << LOGGING_CTX << "Found EXIF metadata (" << tagDescription << ")";
    if(!metaData)
        metaData = new ImageMetaData;
    for(int i = 0, tagListCount = TIFFGetTagListCount(tiff); i < tagListCount; i++)
    {
        ttag_t tag = TIFFGetTagListEntry(tiff, i);
        const TIFFField *field = TIFFFieldWithTag(tiff, tag);
        const QString exifName = QString::fromUtf8(TIFFFieldName(field));
        /// @note See _TIFFVGetField in tif_dir.c
        switch(TIFFFieldDataType(field))
        {
        case TIFF_BYTE:
        case TIFF_UNDEFINED:
            addMetaData<quint8>(tiff, field, metaData, tagDescription, exifName);
            break;
        case TIFF_ASCII:
            addMetaData<QString>(tiff, field, metaData, tagDescription, exifName);
            break;
        case TIFF_SHORT:
            addMetaData<quint16>(tiff, field, metaData, tagDescription, exifName);
            break;
        case TIFF_LONG:
            addMetaData<quint32>(tiff, field, metaData, tagDescription, exifName);
            break;
        case TIFF_SBYTE:
            addMetaData<qint8>(tiff, field, metaData, tagDescription, exifName);
            break;
        case TIFF_SSHORT:
            addMetaData<qint16>(tiff, field, metaData, tagDescription, exifName);
            break;
        case TIFF_SLONG:
            addMetaData<qint32>(tiff, field, metaData, tagDescription, exifName);
            break;
        case TIFF_SRATIONAL:
        case TIFF_RATIONAL:
        case TIFF_FLOAT:
            addMetaData<float>(tiff, field, metaData, tagDescription, exifName);
            break;
        case TIFF_DOUBLE:
            addMetaData<double>(tiff, field, metaData, tagDescription, exifName);
            break;
        case TIFF_LONG8:
            addMetaData<quint64>(tiff, field, metaData, tagDescription, exifName);
            break;
        case TIFF_SLONG8:
            addMetaData<qint64>(tiff, field, metaData, tagDescription, exifName);
            break;
        default:
            break;
        }
    }
}

ImageMetaData *readExifMetaData(TIFF *tiff)
{
    ImageMetaData *metaData = Q_NULLPTR;
    readTiffTagToMetaData(tiff, metaData, TIFFTAG_EXIFIFD, QString::fromLatin1("TIFFTAG_EXIFIFD"));
    readTiffTagToMetaData(tiff, metaData, TIFFTAG_GPSIFD, QString::fromLatin1("TIFFTAG_GPSIFD"));
    readTiffTagToMetaData(tiff, metaData, TIFFTAG_INTEROPERABILITYIFD, QString::fromLatin1("TIFFTAG_INTEROPERABILITYIFD"));
    return metaData;
}

// ====================================================================================================

quint64 getMaxValueUInt(quint64 bitsPerSample)
{
    assert(bitsPerSample <= 64);
    switch(bitsPerSample)
    {
    case 8:
        return static_cast<quint64>(std::numeric_limits<quint8>::max());
    case 16:
        return static_cast<quint64>(std::numeric_limits<quint16>::max());
    case 32:
        return static_cast<quint64>(std::numeric_limits<quint32>::max());
    case 64:
        return static_cast<quint64>(std::numeric_limits<quint64>::max());
    default:
        break;
    }
    return static_cast<quint64>(0xffffffffffffffffull >> (64 - bitsPerSample));
}

quint64 getMinValueUInt(quint64 bitsPerSample)
{
    assert(bitsPerSample <= 64);
    return 0;
}

quint64 getValueUInt(const quint8 *buffer, quint64 bitsOffset, quint64 bitsPerSample)
{
    assert(buffer);
    assert(bitsPerSample <= 64);
    switch(bitsPerSample)
    {
    case 8:
        assert(bitsOffset % 8 == 0);
        return static_cast<quint64>(*reinterpret_cast<const quint8*>(buffer + bitsOffset / 8));
    case 16:
        assert(bitsOffset % 8 == 0);
        return static_cast<quint64>(DataProcessing::extractFromUnalignedPtr<quint16>(buffer + bitsOffset / 8));
    case 32:
        assert(bitsOffset % 8 == 0);
        return static_cast<quint64>(DataProcessing::extractFromUnalignedPtr<quint32>(buffer + bitsOffset / 8));
    case 64:
        assert(bitsOffset % 8 == 0);
        return static_cast<quint64>(DataProcessing::extractFromUnalignedPtr<quint64>(buffer + bitsOffset / 8));
    default:
        break;
    }
    return static_cast<quint64>(DataProcessing::getBits(buffer, bitsOffset, bitsPerSample));
}

qint64 getMaxValueSInt(quint64 bitsPerSample)
{
    assert(bitsPerSample <= 64);
    switch(bitsPerSample)
    {
    case 8:
        return static_cast<qint64>(std::numeric_limits<qint8>::max());
    case 16:
        return static_cast<qint64>(std::numeric_limits<qint16>::max());
    case 32:
        return static_cast<qint64>(std::numeric_limits<qint32>::max());
    case 64:
        return static_cast<qint64>(std::numeric_limits<qint64>::max());
    default:
        break;
    }
    return static_cast<qint64>(0x7fffffffffffffffull >> (64 - bitsPerSample));
}

qint64 getMinValueSInt(quint64 bitsPerSample)
{
    assert(bitsPerSample <= 64);
    switch(bitsPerSample)
    {
    case 8:
        return static_cast<qint64>(std::numeric_limits<qint8>::min());
    case 16:
        return static_cast<qint64>(std::numeric_limits<qint16>::min());
    case 32:
        return static_cast<qint64>(std::numeric_limits<qint32>::min());
    case 64:
        return static_cast<qint64>(std::numeric_limits<qint64>::min());
    default:
        break;
    }
    return static_cast<qint64>(~0ull << (bitsPerSample - 1));
}

qint64 getValueSInt(const quint8 *buffer, quint64 bitsOffset, quint64 bitsPerSample)
{
    assert(buffer);
    assert(bitsPerSample <= 64);
    switch(bitsPerSample)
    {
    case 8:
        assert(bitsOffset % 8 == 0);
        return static_cast<qint64>(*reinterpret_cast<const qint8*>(buffer + bitsOffset / 8));
    case 16:
        assert(bitsOffset % 8 == 0);
        return static_cast<qint64>(DataProcessing::extractFromUnalignedPtr<qint16>(buffer + bitsOffset / 8));
    case 32:
        assert(bitsOffset % 8 == 0);
        return static_cast<qint64>(DataProcessing::extractFromUnalignedPtr<qint32>(buffer + bitsOffset / 8));
    case 64:
        assert(bitsOffset % 8 == 0);
        return static_cast<qint64>(DataProcessing::extractFromUnalignedPtr<qint64>(buffer + bitsOffset / 8));
    default:
        break;
    }
    quint64 rawValue = static_cast<quint64>(DataProcessing::getBits(buffer, bitsOffset, bitsPerSample));
    const quint64 signBit = (rawValue >> (bitsPerSample - 1)) & 0x1;
    if(signBit)
        rawValue |= static_cast<quint64>(~0ull << (bitsPerSample - 1));
    return static_cast<qint64>(rawValue);
}

float getValueFP(const quint8 *buffer, quint64 bitsOffset, quint64 bitsPerSample)
{
    assert(buffer);
    assert(bitsPerSample <= 64);
    switch(bitsPerSample)
    {
    case 16:
        return DataProcessing::float16ToFloat(buffer + bitsOffset / 8);
    case 24:
        return DataProcessing::float24ToFloat(buffer + bitsOffset / 8);
    case 32:
        return DataProcessing::extractFromUnalignedPtr<float>(buffer + bitsOffset / 8);
    case 64:
        return static_cast<float>(DataProcessing::extractFromUnalignedPtr<double>(buffer + bitsOffset / 8));
    }
    LOG_WARNING() << LOGGING_CTX << "Unsupported floating bits per sample =" << bitsPerSample;
    assert(false);
    return static_cast<float>(nan(""));
}

float convertValueToFP(const quint8 *buffer, quint64 bitsOffset, quint16 sampleFormat, quint64 bitsPerSample)
{
    assert(buffer);
    assert(bitsPerSample <= 64);
    switch(sampleFormat)
    {
    case SAMPLEFORMAT_UINT:
    case SAMPLEFORMAT_VOID:
    {
        bitsPerSample = std::min<quint64>(bitsPerSample, static_cast<quint64>(64));
        const double minValue = static_cast<double>(getMinValueUInt(bitsPerSample));
        const double maxValue = static_cast<double>(getMaxValueUInt(bitsPerSample));
        const double value = (static_cast<double>(getValueUInt(buffer, bitsOffset, bitsPerSample)) - minValue) / (maxValue - minValue);
        return static_cast<float>(value);
    }
    case SAMPLEFORMAT_INT:
    {
        bitsPerSample = std::min<quint64>(bitsPerSample, static_cast<quint64>(64));
        const double minValue = static_cast<double>(getMinValueSInt(bitsPerSample));
        const double maxValue = static_cast<double>(getMaxValueSInt(bitsPerSample));
        const double value = (static_cast<double>(getValueSInt(buffer, bitsOffset, bitsPerSample)) - minValue) / (maxValue - minValue);
        return static_cast<float>(value);
    }
    case SAMPLEFORMAT_IEEEFP:
    {
        const float value = getValueFP(buffer, bitsOffset, bitsPerSample);
        return value;
    }
    case SAMPLEFORMAT_COMPLEXINT:
    case SAMPLEFORMAT_COMPLEXIEEEFP:
        LOG_WARNING() << LOGGING_CTX << "SAMPLEFORMAT_COMPLEXINT and SAMPLEFORMAT_COMPLEXIEEEFP are not supported";
        assert(false);
        break;
    default:
        break;
    }
    return static_cast<float>(nan(""));
}

quint8 convertValueToU8(const quint8 *buffer, quint64 bitsOffset, quint16 sampleFormat, quint64 bitsPerSample)
{
    assert(buffer);
    assert(bitsPerSample <= 64);
    switch(sampleFormat)
    {
    case SAMPLEFORMAT_UINT:
    case SAMPLEFORMAT_VOID:
    {
        bitsPerSample = std::min<quint64>(bitsPerSample, static_cast<quint64>(64));
        if(bitsPerSample == 8)
        {
            const quint64 value = getValueUInt(buffer, bitsOffset, bitsPerSample);
            return DataProcessing::clampByte(value);
        }
        if(bitsPerSample < 8 || bitsPerSample > 62)
        {
            const float value = convertValueToFP(buffer, bitsOffset, sampleFormat, bitsPerSample);
            return DataProcessing::clampByte(value * 255.0f);
        }
        const quint64 shift = bitsPerSample - 8;
        const quint64 value = getValueUInt(buffer, bitsOffset, bitsPerSample);
        return DataProcessing::clampByte(value >> shift);
    }
    case SAMPLEFORMAT_INT:
    {
        bitsPerSample = std::min<quint64>(bitsPerSample, static_cast<quint64>(64));
        if(bitsPerSample == 8)
        {
            const qint64 minValue = getMinValueSInt(bitsPerSample);
            const qint64 value = getValueSInt(buffer, bitsOffset, bitsPerSample);
            return DataProcessing::clampByte(value - minValue);
        }
        if(bitsPerSample < 8 || bitsPerSample > 62)
        {
            const float value = convertValueToFP(buffer, bitsOffset, sampleFormat, bitsPerSample);
            return DataProcessing::clampByte(value * 255.0f);
        }
        const quint64 shift = bitsPerSample - 8;
        const qint64 minValue = getMinValueSInt(bitsPerSample);
        const qint64 value = getValueSInt(buffer, bitsOffset, bitsPerSample);
        return DataProcessing::clampByte((value - minValue) >> shift);
    }
    case SAMPLEFORMAT_IEEEFP:
    {
        const float value = getValueFP(buffer, bitsOffset, bitsPerSample);
        return DataProcessing::clampByte(value * 255.0f);
    }
    case SAMPLEFORMAT_COMPLEXINT:
    case SAMPLEFORMAT_COMPLEXIEEEFP:
        LOG_WARNING() << LOGGING_CTX << "SAMPLEFORMAT_COMPLEXINT and SAMPLEFORMAT_COMPLEXIEEEFP are not supported";
        assert(false);
        break;
    default:
        break;
    }
    return 0;
}

// ====================================================================================================

QImage readFromRawBuffer(const quint8 *buffer, qint64 width, qint64 height, qint64 bufferLineSize, const Context *ctx)
{
    assert(buffer);
    assert(ctx);

    switch(ctx->photometric)
    {
    case PHOTOMETRIC_MINISWHITE:
    case PHOTOMETRIC_MINISBLACK:
    case PHOTOMETRIC_LOGL: // TIFFTAG_SGILOGDATAFMT == SGILOGDATAFMT_8BIT is required
    {
        assert(ctx->primarysamplesCount >= 1);
        const QImage::Format resultFormat = ctx->alphaIndex >= 0 ? QImage::Format_ARGB32 : QImage::Format_RGB32;
#if (USE_GRAYSCALE_8)
        QImage result(width, height, QImage::Format_Grayscale8);
        QImage alphaChannel;
        if(ctx->alphaIndex >= 0)
            alphaChannel = QImage(width, height, QImage::Format_Alpha8);
#else
        QImage result(width, height, resultFormat);
#endif
        if(result.isNull())
        {
            LOG_WARNING() << LOGGING_CTX << "Invalid image size";
            return result;
        }
        for(qint64 y = 0; y < height; ++y)
        {
            QRgb *outPtr = reinterpret_cast<QRgb*>(result.scanLine(y));
            const quint8 *inPtr = reinterpret_cast<const quint8*>(buffer + bufferLineSize * y);
            for(qint64 x = 0; x < width; ++x)
            {
                const qint64 bitOffset = x * ctx->samplesPerPixel * ctx->bitsPerSample;
                quint8 gray = convertValueToU8(inPtr, static_cast<quint64>(bitOffset), ctx->sampleFormat, static_cast<quint64>(ctx->bitsPerSample));
                if(ctx->photometric == PHOTOMETRIC_MINISWHITE)
                    gray = 255 - gray;
                quint8 alpha = 255;
                if(ctx->alphaIndex >= 0)
                {
                    alpha = convertValueToU8(inPtr, static_cast<quint64>(bitOffset + ctx->bitsPerSample * (ctx->primarysamplesCount + ctx->alphaIndex)), ctx->sampleFormat, static_cast<quint64>(ctx->bitsPerSample));
                    if(ctx->photometric == PHOTOMETRIC_MINISWHITE)
                        alpha = 255 - alpha;
                }

#if (USE_GRAYSCALE_8)
                if(ctx->alphaPremultiplied)
                    reinterpret_cast<quint8*>(outPtr)[x] = qGray(DataProcessing::unpremultiply(qRgba(gray, gray, gray, alpha)));
                else
                    reinterpret_cast<quint8*>(outPtr)[x] = gray;
                if(!alphaChannel.isNull())
                    *reinterpret_cast<quint8*>(alphaChannel.scanLine(y) + x) = alpha;
#else
                if(ctx->alphaPremultiplied)
                    outPtr[x] = DataProcessing::unpremultiply(qRgba(gray, gray, gray, alpha));
                else
                    outPtr[x] = qRgba(gray, gray, gray, alpha);
#endif
            }
        }
        if(ctx->iccProfile)
            ctx->iccProfile->applyToImage(&result);
#if (USE_GRAYSCALE_8)
        if(!alphaChannel.isNull())
        {
            if(result.hasAlphaChannel())
                QImage_convertTo(result, QImage::Format_RGB32);
            QImage_convertTo(result, QImage::Format_ARGB32);
            result.setAlphaChannel(alphaChannel);
        }
#endif
        if(!IsOneOf(result.format(), resultFormat, QImage::Format_RGB32, QImage::Format_ARGB32))
            QImage_convertTo(result, resultFormat);
        return result;
    }
    case PHOTOMETRIC_RGB:
    case PHOTOMETRIC_LOGLUV: // TIFFTAG_SGILOGDATAFMT == SGILOGDATAFMT_8BIT is required
    {
        assert(ctx->primarysamplesCount >= 3);
        const QImage::Format resultFormat = ctx->alphaIndex >= 0 ? QImage::Format_ARGB32 : QImage::Format_RGB32;
        QImage result(width, height, resultFormat);
        if(result.isNull())
        {
            LOG_WARNING() << LOGGING_CTX << "Invalid image size";
            return result;
        }
        for(qint64 y = 0; y < height; ++y)
        {
            QRgb *outPtr = reinterpret_cast<QRgb*>(result.scanLine(y));
            const quint8 *inPtr = reinterpret_cast<const quint8*>(buffer + bufferLineSize * y);
            for(qint64 x = 0; x < width; ++x)
            {
                const qint64 bitOffset = x * ctx->samplesPerPixel * ctx->bitsPerSample;
                const quint8 red = convertValueToU8(inPtr, static_cast<quint64>(bitOffset + ctx->bitsPerSample * 0), ctx->sampleFormat, static_cast<quint64>(ctx->bitsPerSample));
                const quint8 green = convertValueToU8(inPtr, static_cast<quint64>(bitOffset + ctx->bitsPerSample * 1), ctx->sampleFormat, static_cast<quint64>(ctx->bitsPerSample));
                const quint8 blue = convertValueToU8(inPtr, static_cast<quint64>(bitOffset + ctx->bitsPerSample * 2), ctx->sampleFormat, static_cast<quint64>(ctx->bitsPerSample));
                const quint8 alpha = ctx->alphaIndex >= 0
                        ? convertValueToU8(inPtr, static_cast<quint64>(bitOffset + ctx->bitsPerSample * (ctx->primarysamplesCount + ctx->alphaIndex)), ctx->sampleFormat, static_cast<quint64>(ctx->bitsPerSample))
                        : 255;
                if(ctx->alphaPremultiplied)
                    outPtr[x] = DataProcessing::unpremultiply(qRgba(red, green, blue, alpha));
                else
                    outPtr[x] = qRgba(red, green, blue, alpha);
            }
        }
        if(ctx->iccProfile)
            ctx->iccProfile->applyToImage(&result);
        if(!IsOneOf(result.format(), resultFormat, QImage::Format_RGB32, QImage::Format_ARGB32))
            QImage_convertTo(result, resultFormat);
        return result;
    }
    case PHOTOMETRIC_SEPARATED:
    {
        assert(ctx->primarysamplesCount >= 4);
        const QImage::Format resultFormat = ctx->alphaIndex >= 0 ? QImage::Format_ARGB32 : QImage::Format_RGB32;
#if (USE_CMYK_8888)
        QImage result(width, height, QImage::Format_CMYK8888);
        QImage alphaChannel;
        if(ctx->alphaIndex >= 0)
            alphaChannel = QImage(width, height, QImage::Format_Alpha8);
#else
        QImage result(width, height, resultFormat);
#endif
        if(result.isNull())
        {
            LOG_WARNING() << LOGGING_CTX << "Invalid image size";
            return result;
        }
        for(qint64 y = 0; y < height; ++y)
        {
            QRgb *outPtr = reinterpret_cast<QRgb*>(result.scanLine(y));
            const quint8 *inPtr = reinterpret_cast<const quint8*>(buffer + bufferLineSize * y);
            for(qint64 x = 0; x < width; ++x)
            {
                const qint64 bitOffset = x * ctx->samplesPerPixel * ctx->bitsPerSample;
                const quint8 cyan = convertValueToU8(inPtr, static_cast<quint64>(bitOffset + ctx->bitsPerSample * 0), ctx->sampleFormat, static_cast<quint64>(ctx->bitsPerSample));
                const quint8 magenta = convertValueToU8(inPtr, static_cast<quint64>(bitOffset + ctx->bitsPerSample * 1), ctx->sampleFormat, static_cast<quint64>(ctx->bitsPerSample));
                const quint8 yellow = convertValueToU8(inPtr, static_cast<quint64>(bitOffset + ctx->bitsPerSample * 2), ctx->sampleFormat, static_cast<quint64>(ctx->bitsPerSample));
                const quint8 key = convertValueToU8(inPtr, static_cast<quint64>(bitOffset + ctx->bitsPerSample * 3), ctx->sampleFormat, static_cast<quint64>(ctx->bitsPerSample));
                const quint8 alpha = ctx->alphaIndex >= 0
                        ? convertValueToU8(inPtr, static_cast<quint64>(bitOffset + ctx->bitsPerSample * (ctx->primarysamplesCount + ctx->alphaIndex)), ctx->sampleFormat, static_cast<quint64>(ctx->bitsPerSample))
                        : 255;
                /// @todo Why alpha is not premultiplied in CMYK images from Adobe Photoshop even for EXTRASAMPLE_ASSOCALPHA?
#if (USE_CMYK_8888)
                quint8 *outColor = reinterpret_cast<quint8*>(outPtr + x);
                outColor[0] = cyan;
                outColor[1] = magenta;
                outColor[2] = yellow;
                outColor[3] = key;
                if(!alphaChannel.isNull())
                    *reinterpret_cast<quint8*>(alphaChannel.scanLine(y) + x) = alpha;
#else
                outPtr[x] = DataProcessing::CMYK8ToRgba(cyan, magenta, yellow, key, alpha);
#endif
            }
        }
        if(ctx->iccProfile)
            ctx->iccProfile->applyToImage(&result);
#if (USE_CMYK_8888)
        if(!alphaChannel.isNull())
        {
            if(result.hasAlphaChannel())
                QImage_convertTo(result, QImage::Format_RGB32);
            QImage_convertTo(result, QImage::Format_ARGB32);
            result.setAlphaChannel(alphaChannel);
        }
#endif
        if(!IsOneOf(result.format(), resultFormat, QImage::Format_RGB32, QImage::Format_ARGB32))
            QImage_convertTo(result, resultFormat);
        return result;
    }
    case PHOTOMETRIC_YCBCR:
    {
        assert(ctx->primarysamplesCount == 3);
        const QImage::Format resultFormat = ctx->alphaIndex >= 0 ? QImage::Format_ARGB32 : QImage::Format_RGB32;
        QImage result(width, height, resultFormat);
        if(result.isNull())
        {
            LOG_WARNING() << LOGGING_CTX << "Invalid image size";
            return result;
        }
        if(ctx->subsamplinghor > 1 || ctx->subsamplingver > 1)
        {
            QImage upsampledCbCr(result.width(), result.height(), QImage::Format_ARGB32);
            if(upsampledCbCr.isNull())
            {
                LOG_WARNING() << LOGGING_CTX << "Invalid upsampled image size";
                goto NoUpsampling;
            }
            for(qint64 y = 0; y < height; ++y)
            {
                quint8 *outPtr = reinterpret_cast<quint8*>(upsampledCbCr.scanLine(y));
                const quint8 *inPtr = reinterpret_cast<const quint8*>(buffer + bufferLineSize * y);
                for(qint64 x = 0; x < width; ++x)
                {
                    const qint64 bitOffset = x * ctx->samplesPerPixel * ctx->bitsPerSample;
                    const quint8 Cb = convertValueToU8(inPtr, static_cast<quint64>(bitOffset + ctx->bitsPerSample * 1), ctx->sampleFormat, static_cast<quint64>(ctx->bitsPerSample));
                    const quint8 Cr = convertValueToU8(inPtr, static_cast<quint64>(bitOffset + ctx->bitsPerSample * 2), ctx->sampleFormat, static_cast<quint64>(ctx->bitsPerSample));
                    outPtr[x * 4 + 0] = outPtr[x * 4 + 2] = Cb;
                    outPtr[x * 4 + 1] = outPtr[x * 4 + 3] = Cr;
                }
            }
            if(ctx->subsamplinghor > 1)
            {
                for(qint64 y = 0; y < height; y += ctx->subsamplingver)
                {
                    quint8 *outPtr = reinterpret_cast<quint8*>(upsampledCbCr.scanLine(y));
                    const quint8 *inPtr = reinterpret_cast<const quint8*>(upsampledCbCr.scanLine(y));
                    for(qint64 x = 0; x < width; ++x)
                    {
                        qint64 leftIndex;
                        float rightWeight;
                        if(ctx->ycbcrpositioning == YCBCRPOSITION_COSITED)
                        {
                            leftIndex = std::max<qint64>(0, x - (x % ctx->subsamplinghor));
                            rightWeight = static_cast<float>(x - leftIndex) / static_cast<float>(ctx->subsamplinghor);
                        }
                        else
                        {
                            const float leftPos = static_cast<float>(x - (x % ctx->subsamplinghor) - (((x % ctx->subsamplinghor) >= ctx->subsamplinghor / 2) ? 0 : ctx->subsamplinghor))
                                    + static_cast<float>(ctx->subsamplinghor - 1) / 2.0f;
                            leftIndex = std::max<qint64>(0, static_cast<int>(leftPos));
                            rightWeight = (static_cast<float>(x) - leftPos) / static_cast<float>(ctx->subsamplinghor);
                        }
                        const qint64 rightIndex = std::min<qint64>(width - 1, leftIndex + ctx->subsamplinghor);
                        const float leftWeight = 1.0f - rightWeight;
                        for(qint64 c = 0; c < 2; ++c)
                        {
                            const quint8 leftValue = inPtr[leftIndex * 4 + c];
                            const quint8 rightValue = inPtr[rightIndex * 4 + c];
                            const float newValue = static_cast<float>(leftValue) * leftWeight + static_cast<float>(rightValue) * rightWeight;
                            outPtr[x * 4 + c + 2] = DataProcessing::clampByte(newValue);
                        }
                    }
                    for(qint64 x = 0; x < width; ++x)
                    {
                        for(qint64 c = 0; c < 2; ++c)
                            outPtr[x * 4 + c] = outPtr[x * 4 + c + 2];
                    }
                    for(qint64 i = 0; i < ctx->subsamplingver && y + i < height; ++i)
                        memcpy(upsampledCbCr.scanLine(y + i), outPtr, width * 4);
                }
            }
            if(ctx->subsamplingver > 1)
            {
                for(qint64 y = 0; y < height; ++y)
                {
                    qint64 upIndex;
                    float downWeight;
                    if(ctx->ycbcrpositioning == YCBCRPOSITION_COSITED)
                    {
                        upIndex = std::max<qint64>(0, y - (y % ctx->subsamplingver));
                        downWeight = static_cast<float>(y - upIndex) / static_cast<float>(ctx->subsamplingver);
                    }
                    else
                    {
                        const float upPos = static_cast<float>(y - (y % ctx->subsamplingver) - (((y % ctx->subsamplingver) >= ctx->subsamplingver / 2) ? 0 : ctx->subsamplingver))
                                + static_cast<float>(ctx->subsamplingver - 1) / 2.0f;
                        upIndex = std::max<qint64>(0, static_cast<int>(upPos));
                        downWeight = (static_cast<float>(y) - upPos) / static_cast<float>(ctx->subsamplingver);
                    }
                    const qint64 downIndex = std::min<qint64>(height - 1, upIndex + ctx->subsamplingver);
                    const float upWeight = 1.0f - downWeight;
                    const quint8 *upPtr = reinterpret_cast<const quint8*>(upsampledCbCr.scanLine(upIndex));
                    const quint8 *downPtr = reinterpret_cast<const quint8*>(upsampledCbCr.scanLine(downIndex));
                    quint8 *outPtr = reinterpret_cast<quint8*>(upsampledCbCr.scanLine(y));
                    for(qint64 x = 0; x < width; ++x)
                    {
                        for(qint64 c = 0; c < 2; ++c)
                        {
                            const quint8 upValue = upPtr[x * 4 + c];
                            const quint8 downValue = downPtr[x * 4 + c];
                            const float newValue = static_cast<float>(upValue) * upWeight + static_cast<float>(downValue) * downWeight;
                            outPtr[x * 4 + c + 2] = DataProcessing::clampByte(newValue);
                        }
                    }
                }
                for(qint64 y = 0; y < height; ++y)
                {
                    quint8 *outPtr = reinterpret_cast<quint8*>(upsampledCbCr.scanLine(y));
                    for(qint64 x = 0; x < width; ++x)
                    {
                        for(qint64 c = 0; c < 2; ++c)
                            outPtr[x * 4 + c] = outPtr[x * 4 + c + 2];
                    }
                }
            }
            for(qint64 y = 0; y < height; ++y)
            {
                QRgb *outPtr = reinterpret_cast<QRgb*>(result.scanLine(y));
                const quint8 *inPtr = reinterpret_cast<const quint8*>(buffer + bufferLineSize * y);
                const quint8 *inCbCrPtr = reinterpret_cast<const quint8*>(upsampledCbCr.scanLine(y));
                for(qint64 x = 0; x < width; ++x)
                {
                    const qint64 bitOffset = x * ctx->samplesPerPixel * ctx->bitsPerSample;
                    float Y  = convertValueToFP(inPtr, static_cast<quint64>(bitOffset + ctx->bitsPerSample * 0), ctx->sampleFormat, static_cast<quint64>(ctx->bitsPerSample));
                    float Cb = inCbCrPtr[x * 4 + 0] / 255.0f;
                    float Cr = inCbCrPtr[x * 4 + 1] / 255.0f;
                    const float alpha = ctx->alphaIndex >= 0
                            ? convertValueToFP(inPtr, static_cast<quint64>(bitOffset + ctx->bitsPerSample * (ctx->primarysamplesCount + ctx->alphaIndex)), ctx->sampleFormat, static_cast<quint64>(ctx->bitsPerSample))
                            : 1.0f;
                    if(ctx->alphaPremultiplied)
                    {
                        const float alphaScale = alpha > 0.0f ? (1.0f / alpha) : 1.0f;
                        Y *= alphaScale;
                        Cb *= alphaScale;
                        Cr *= alphaScale;
                    }
                    Cb -= 0.5f;
                    Cr -= 0.5f;
                    outPtr[x] = DataProcessing::YCbCrToRgba(Y, Cb, Cr, alpha, ctx->ycbcrcoeffs);
                }
            }
        }
        else
        {
NoUpsampling:
            for(qint64 y = 0; y < height; ++y)
            {
                QRgb *outPtr = reinterpret_cast<QRgb*>(result.scanLine(y));
                const quint8 *inPtr = reinterpret_cast<const quint8*>(buffer + bufferLineSize * y);
                for(qint64 x = 0; x < width; ++x)
                {
                    const qint64 bitOffset = x * ctx->samplesPerPixel * ctx->bitsPerSample;
                    float Y  = convertValueToFP(inPtr, static_cast<quint64>(bitOffset + ctx->bitsPerSample * 0), ctx->sampleFormat, static_cast<quint64>(ctx->bitsPerSample));
                    float Cb = convertValueToFP(inPtr, static_cast<quint64>(bitOffset + ctx->bitsPerSample * 1), ctx->sampleFormat, static_cast<quint64>(ctx->bitsPerSample));
                    float Cr = convertValueToFP(inPtr, static_cast<quint64>(bitOffset + ctx->bitsPerSample * 2), ctx->sampleFormat, static_cast<quint64>(ctx->bitsPerSample));
                    const float alpha = ctx->alphaIndex >= 0
                            ? convertValueToFP(inPtr, static_cast<quint64>(bitOffset + ctx->bitsPerSample * (ctx->primarysamplesCount + ctx->alphaIndex)), ctx->sampleFormat, static_cast<quint64>(ctx->bitsPerSample))
                            : 1.0f;
                    if(ctx->alphaPremultiplied)
                    {
                        const float alphaScale = alpha > 0.0f ? (1.0f / alpha) : 1.0f;
                        Y *= alphaScale;
                        Cb *= alphaScale;
                        Cr *= alphaScale;
                    }
                    Cb -= 0.5f;
                    Cr -= 0.5f;
                    outPtr[x] = DataProcessing::YCbCrToRgba(Y, Cb, Cr, alpha, ctx->ycbcrcoeffs);
                }
            }
        }
        if(ctx->iccProfile)
            ctx->iccProfile->applyToImage(&result);
        if(!IsOneOf(result.format(), resultFormat, QImage::Format_RGB32, QImage::Format_ARGB32))
            QImage_convertTo(result, resultFormat);
        return result;
    }
    case PHOTOMETRIC_CIELAB:
    case PHOTOMETRIC_ICCLAB:
    case PHOTOMETRIC_ITULAB:
    {
        assert(ctx->primarysamplesCount >= 1);
        const QImage::Format resultFormat = ctx->alphaIndex >= 0 ? QImage::Format_ARGB32 : QImage::Format_RGB32;
        QImage result(width, height, resultFormat);
        if(result.isNull())
        {
            LOG_WARNING() << LOGGING_CTX << "Invalid image size";
            return result;
        }
        for(qint64 y = 0; y < height; ++y)
        {
            QRgb *outPtr = reinterpret_cast<QRgb*>(result.scanLine(y));
            const quint8 *inPtr = reinterpret_cast<const quint8*>(buffer + bufferLineSize * y);
            for(qint64 x = 0; x < width; ++x)
            {
                quint16 abSampleFormat = ctx->sampleFormat;
                /// @note CIELAB is meaningless for unsigned input, as for ITULAB is meaningless for signed input.
                /// But we will try to fix some ill-formed files with wrong sample format because it should be safe
                if(ctx->photometric == PHOTOMETRIC_CIELAB && IsOneOf(ctx->sampleFormat, SAMPLEFORMAT_VOID, SAMPLEFORMAT_UINT))
                    abSampleFormat = SAMPLEFORMAT_INT;
                else if(IsOneOf(ctx->photometric, PHOTOMETRIC_ITULAB, PHOTOMETRIC_ITULAB) && IsOneOf(ctx->sampleFormat, SAMPLEFORMAT_VOID, SAMPLEFORMAT_INT))
                    abSampleFormat = SAMPLEFORMAT_UINT;
                const qint64 bitOffset = x * ctx->samplesPerPixel * ctx->bitsPerSample;
                float L = convertValueToFP(inPtr, static_cast<quint64>(bitOffset + ctx->bitsPerSample * 0), ctx->sampleFormat, static_cast<quint64>(ctx->bitsPerSample));
                float a = ctx->primarysamplesCount >= 3
                        ? convertValueToFP(inPtr, static_cast<quint64>(bitOffset + ctx->bitsPerSample * 1), abSampleFormat, static_cast<quint64>(ctx->bitsPerSample))
                        : 0.0f;
                float b = ctx->primarysamplesCount >= 3
                        ? convertValueToFP(inPtr, static_cast<quint64>(bitOffset + ctx->bitsPerSample * 2), abSampleFormat, static_cast<quint64>(ctx->bitsPerSample))
                        : 0.0f;
                const float alpha = ctx->alphaIndex >= 0
                        ? convertValueToFP(inPtr, static_cast<quint64>(bitOffset + ctx->bitsPerSample * (ctx->primarysamplesCount + ctx->alphaIndex)), ctx->sampleFormat, static_cast<quint64>(ctx->bitsPerSample))
                        : 1.0f;
                if(ctx->alphaPremultiplied)
                {
                    const float alphaScale = alpha > 0.0f ? (1.0f / alpha) : 1.0f;
                    L *= alphaScale;
                    a *= alphaScale;
                    b *= alphaScale;
                }
                L *= 100.0f;
                if(ctx->primarysamplesCount >= 3)
                {
                    if(ctx->photometric == PHOTOMETRIC_ITULAB)
                    {
                        a = (-21760.0f / 255.0f) + a * ((21590.0f / 255.0f) - (-21760.0f / 255.0f));
                        b = (-19200.0f / 255.0f) + b * ((31800.0f / 255.0f) - (-19200.0f / 255.0f));
                    }
                    else
                    {
                        a = a * 256.0f - 128.0f;
                        b = b * 256.0f - 128.0f;
                    }
                }
                outPtr[x] = DataProcessing::LabToRgba(L, a, b, alpha);
            }
        }
        if(ctx->iccProfile)
            ctx->iccProfile->applyToImage(&result);
        if(!IsOneOf(result.format(), resultFormat, QImage::Format_RGB32, QImage::Format_ARGB32))
            QImage_convertTo(result, resultFormat);
        return result;
    }
    case PHOTOMETRIC_PALETTE:
    {
        assert(ctx->redTable);
        assert(ctx->greenTable);
        assert(ctx->blueTable);
        assert(ctx->primarysamplesCount >= 1);
        const QImage::Format resultFormat = ctx->alphaIndex >= 0 ? QImage::Format_ARGB32 : QImage::Format_RGB32;
        QImage result(width, height, resultFormat);
        if(result.isNull())
        {
            LOG_WARNING() << LOGGING_CTX << "Invalid image size";
            return result;
        }
        const quint16 shiftFor8BitColors = ctx->colorTablesIs16Bit ? 8 : 0;
        for(qint64 y = 0; y < height; ++y)
        {
            QRgb *outPtr = reinterpret_cast<QRgb*>(result.scanLine(y));
            const quint8 *inPtr = reinterpret_cast<const quint8*>(buffer + bufferLineSize * y);
            for(qint64 x = 0; x < width; ++x)
            {
                const qint64 bitOffset = x * ctx->samplesPerPixel * ctx->bitsPerSample;
                const quint64 colorIndex = getValueUInt(inPtr, static_cast<quint64>(bitOffset), static_cast<quint64>(ctx->bitsPerSample));
                assert(colorIndex <= getMaxValueUInt(static_cast<quint64>(ctx->bitsPerSample)));
                const quint8 red = ctx->redTable[colorIndex] >> shiftFor8BitColors;
                const quint8 green = ctx->greenTable[colorIndex] >> shiftFor8BitColors;
                const quint8 blue = ctx->blueTable[colorIndex] >> shiftFor8BitColors;
                const quint8 alpha = ctx->alphaIndex >= 0
                        ? convertValueToU8(inPtr, static_cast<quint64>(bitOffset + ctx->bitsPerSample * (ctx->primarysamplesCount + ctx->alphaIndex)), ctx->sampleFormat, static_cast<quint64>(ctx->bitsPerSample))
                        : 255;
                if(ctx->alphaPremultiplied)
                    outPtr[x] = DataProcessing::unpremultiply(qRgba(red, green, blue, alpha));
                else
                    outPtr[x] = qRgba(red, green, blue, alpha);
            }
        }
        if(ctx->iccProfile)
            ctx->iccProfile->applyToImage(&result);
        if(!IsOneOf(result.format(), resultFormat, QImage::Format_RGB32, QImage::Format_ARGB32))
            QImage_convertTo(result, resultFormat);
        return result;
    }
    default:
        break;
    }
    return QImage();
}

// ====================================================================================================

QImage readTiffFileTiledContig(const Context *ctx, qint64 nTiles, qint64 tileWidth, qint64 tileHeight)
{
    assert(ctx);
    assert(ctx->tiff);
    assert(nTiles > 0);
    assert(tileWidth > 0);
    assert(tileHeight > 0);
    LOG_DEBUG() << LOGGING_CTX << "Tiled data with single image plane of data detected";

    QImage result(ctx->width, ctx->height, ctx->alphaIndex >= 0 ? QImage::Format_ARGB32 : QImage::Format_RGB32);
    if(result.isNull())
    {
        LOG_WARNING() << LOGGING_CTX << "Invalid image size";
        return QImage();
    }
    result.fill(ctx->alphaIndex >= 0 ? Qt::transparent : Qt::white);

    const qint64 bufferAllocSize = tileWidth * tileHeight * ctx->samplesPerPixel * ctx->bitsPerSample / 8;
    if(bufferAllocSize > BUFFER_MAX_SIZE)
    {
        LOG_WARNING() << LOGGING_CTX << "Can't allocate buffer with size =" << bufferAllocSize << "due to allocation limit =" << BUFFER_MAX_SIZE;
        return QImage();
    }

    QByteArray buffer;
    buffer.resize(bufferAllocSize);

    const qint64 leftX = ctx->width % tileWidth;
    const qint64 leftY = ctx->height % tileHeight;
    const qint64 nTilesX = (ctx->width / tileWidth)   + (leftX == 0 ? 0 : 1);
    const qint64 nTilesY = (ctx->height / tileHeight) + (leftY == 0 ? 0 : 1);
    const qint64 tileLineSize = tileWidth * ctx->samplesPerPixel * ctx->bitsPerSample / 8;

    for(qint64 tile = 0; tile < nTiles; ++tile)
    {
        memset(buffer.data(), BUFFER_FILL_PATTERN, static_cast<size_t>(buffer.size()));
        const qint64 bufferSize = TIFFReadEncodedTile(ctx->tiff, static_cast<quint32>(tile), buffer.data(), buffer.size());
        if(bufferSize < 0)
        {
            LOG_WARNING() << LOGGING_CTX << "TIFFReadEncodedTile failed for tile" << tile;
            if(tile == 0)
                return QImage();
            // Try to handle if we can read some tiles
            break;
        }
        const qint64 expectedTileBufferSize = tileLineSize * tileHeight;
        if(bufferSize < expectedTileBufferSize)
        {
            LOG_WARNING() << LOGGING_CTX << "TIFFReadEncodedTile returns not enough data to decode tile" << tile << "got:" << bufferSize << "expected:" << expectedTileBufferSize;
            if(tile == 0)
                return QImage();
            // Try to handle if we can read some tiles
            break;
        }

        const qint64 currTileX = tile % nTilesX;
        const qint64 currTileY = tile / nTilesX;
        const qint64 currTileWidth  = ((currTileX == nTilesX - 1) && leftX) ? leftX : tileWidth;
        const qint64 currTileHeight = ((currTileY == nTilesY - 1) && leftY) ? leftY : tileHeight;

        assert(tileLineSize * tileHeight <= bufferSize);
        const QImage tileRgb = readFromRawBuffer(reinterpret_cast<const quint8*>(buffer.data()), tileWidth, tileHeight, tileLineSize, ctx);
        if(tileRgb.isNull())
            continue;

        assert(IsOneOf(tileRgb.format(), QImage::Format_ARGB32, QImage::Format_RGB32));
        for(qint64 y = 0; y < currTileHeight; ++y)
        {
            const QRgb *inScanLine = reinterpret_cast<const QRgb*>(tileRgb.scanLine(y));
            QRgb *outScanLine = reinterpret_cast<QRgb*>(result.scanLine(currTileY * tileHeight + y)) + currTileX * tileWidth;
            memcpy(outScanLine, inScanLine, static_cast<size_t>(currTileWidth * 4));
        }
    }

    return result;
}

QImage readTiffFileTiledContigSubsampled(const Context *ctx, qint64 nTiles, qint64 tileWidth, qint64 tileHeight)
{
    assert(ctx);
    assert(ctx->tiff);
    assert(nTiles > 0);
    assert(tileWidth > 0);
    assert(tileHeight > 0);
    LOG_DEBUG() << LOGGING_CTX << "Tiled data with single image plane of subsampled data detected";

    QImage result(ctx->width, ctx->height, ctx->alphaIndex >= 0 ? QImage::Format_ARGB32 : QImage::Format_RGB32);
    if(result.isNull())
    {
        LOG_WARNING() << LOGGING_CTX << "Invalid image size";
        return QImage();
    }
    result.fill(ctx->alphaIndex >= 0 ? Qt::transparent : Qt::white);

    const qint64 bufferAllocSize = tileWidth * tileHeight * ctx->samplesPerPixel * ctx->bitsPerSample / 8;
    if(bufferAllocSize > BUFFER_MAX_SIZE)
    {
        LOG_WARNING() << LOGGING_CTX << "Can't allocate buffer with size =" << bufferAllocSize << "due to allocation limit =" << BUFFER_MAX_SIZE;
        return QImage();
    }

    QByteArray buffer;
    buffer.resize(bufferAllocSize);

    QByteArray tileBuffer;
    tileBuffer.resize(bufferAllocSize);

    const qint64 leftX = ctx->width % tileWidth;
    const qint64 leftY = ctx->height % tileHeight;
    const qint64 nTilesX = (ctx->width / tileWidth)   + (leftX == 0 ? 0 : 1);
    const qint64 nTilesY = (ctx->height / tileHeight) + (leftY == 0 ? 0 : 1);
    const qint64 tileLineBitSize = tileWidth * ctx->samplesPerPixel * ctx->bitsPerSample;
    const qint64 tileLineSize = tileLineBitSize / 8;

    for(qint64 tile = 0; tile < nTiles; ++tile)
    {
        memset(buffer.data(), BUFFER_FILL_PATTERN, static_cast<size_t>(buffer.size()));
        memset(tileBuffer.data(), BUFFER_FILL_PATTERN, static_cast<size_t>(tileBuffer.size()));
        const qint64 tileBufferSize = TIFFReadEncodedTile(ctx->tiff, static_cast<quint32>(tile), tileBuffer.data(), tileBuffer.size());
        if(tileBufferSize < 0)
        {
            LOG_WARNING() << LOGGING_CTX << "TIFFReadEncodedTile failed for tile" << tile;
            if(tile == 0)
                return QImage();
            // Try to handle if we can read some tiles
            break;
        }
        const qint64 expectedTileBufferSize = tileHeight * tileWidth * (ctx->samplesPerPixel - 2) * ctx->bitsPerSample / 8 + tileHeight / ctx->subsamplingver * tileWidth / ctx->subsamplinghor * 2 * ctx->bitsPerSample / 8;
        if(tileBufferSize < expectedTileBufferSize)
        {
            LOG_WARNING() << LOGGING_CTX << "TIFFReadEncodedTile returns not enough data to decode tile" << tile << "got:" << tileBufferSize << "expected:" << expectedTileBufferSize;
            if(tile == 0)
                return QImage();
            // Try to handle if we can read some tiles
            break;
        }

        const qint64 currTileX = tile % nTilesX;
        const qint64 currTileY = tile / nTilesX;
        const qint64 currTileWidth  = ((currTileX == nTilesX - 1) && leftX) ? leftX : tileWidth;
        const qint64 currTileHeight = ((currTileY == nTilesY - 1) && leftY) ? leftY : tileHeight;
        const qint64 tileScanlineSize = tileBufferSize / ctx->subsamplinghor / tileHeight * ctx->subsamplingver;

#if !defined (DEBUG_FORCE_BIT_ACCESS)
        if(ctx->bitsPerSample % 8 == 0)
#else
        if(false)
#endif
        {
            qint64 bufferLinePos = 0;
            qint64 bufferOffset = bufferLinePos;
            qint64 tileBufferLinePos = 0;
            qint64 tileBufferOffset = tileBufferLinePos;

            while(tileBufferOffset < tileBufferSize && bufferOffset < buffer.size())
            {
                for(qint64 c = 0; c < ctx->samplesPerPixel; ++c)
                {
                    const bool isChroma = IsOneOf(c, 1, 2);
                    if(!isChroma)
                    {
                        for(qint64 y = 0; y < ctx->subsamplingver; ++y)
                        {
                            for(qint64 x = 0; x < ctx->subsamplinghor; ++x)
                            {
                                const qint64 bufferCurrOffset = bufferOffset + y * tileLineSize + x * ctx->bitsPerSample / 8 * ctx->samplesPerPixel + c * ctx->bitsPerSample / 8;
                                qint64 tileBufferCurrOffset = tileBufferOffset;
                                assert(bufferCurrOffset + ctx->bitsPerSample / 8 <= buffer.size());
                                assert(tileBufferCurrOffset + ctx->bitsPerSample / 8 <= tileBufferSize);
                                memcpy(buffer.data() + bufferCurrOffset, tileBuffer.data() + tileBufferCurrOffset, static_cast<size_t>(ctx->bitsPerSample / 8));
                                tileBufferOffset += ctx->bitsPerSample / 8;
                            }
                        }
                    }
                    else
                    {
                        for(qint64 y = 0; y < ctx->subsamplingver; ++y)
                        {
                            for(qint64 x = 0; x < ctx->subsamplinghor; ++x)
                            {
                                const qint64 bufferCurrOffset = bufferOffset + y * tileLineSize + x * ctx->bitsPerSample / 8 * ctx->samplesPerPixel + c * ctx->bitsPerSample / 8;
                                assert(bufferCurrOffset + ctx->bitsPerSample / 8 <= buffer.size());
                                assert(tileBufferOffset + ctx->bitsPerSample / 8 <= tileBufferSize);
                                memcpy(buffer.data() + bufferCurrOffset, tileBuffer.data() + tileBufferOffset, static_cast<size_t>(ctx->bitsPerSample / 8));
                            }
                        }
                        tileBufferOffset += ctx->bitsPerSample / 8;
                    }
                }
                bufferOffset += ctx->bitsPerSample / 8 * ctx->samplesPerPixel * ctx->subsamplinghor;
                if(bufferLinePos + tileWidth * ctx->samplesPerPixel * ctx->bitsPerSample / 8 <= bufferOffset)
                {
                    bufferLinePos += tileLineSize * ctx->subsamplingver;
                    bufferOffset = bufferLinePos;
                    tileBufferLinePos += tileScanlineSize * ctx->subsamplingver;
                    tileBufferOffset = tileBufferLinePos;
                }
                if(bufferLinePos + tileLineSize > tileLineSize * tileHeight)
                    break;
            }
        }
        else
        {
            qint64 bufferLineBitPos = 0;
            qint64 bufferBitOffset = bufferLineBitPos;
            qint64 tileBufferLineBitPos = 0;
            qint64 tileBufferBitOffset = tileBufferLineBitPos;

            while(tileBufferBitOffset < tileBufferSize * 8 && bufferBitOffset < buffer.size() * 8)
            {
                for(qint64 c = 0; c < ctx->samplesPerPixel; ++c)
                {
                    const bool isChroma = IsOneOf(c, 1, 2);
                    if(!isChroma)
                    {
                        for(qint64 y = 0; y < ctx->subsamplingver; ++y)
                        {
                            for(qint64 x = 0; x < ctx->subsamplinghor; ++x)
                            {
                                const qint64 bufferCurrBitOffset = bufferBitOffset + y * tileLineSize * 8 + x * ctx->bitsPerSample * ctx->samplesPerPixel + c * ctx->bitsPerSample;
                                qint64 tileBufferCurrBitOffset = tileBufferBitOffset;
                                assert(bufferCurrBitOffset + ctx->bitsPerSample <= buffer.size() * 8);
                                assert(tileBufferCurrBitOffset + ctx->bitsPerSample <= tileBufferSize * 8);
                                DataProcessing::memcpyBits(buffer.data(), static_cast<quint64>(bufferCurrBitOffset),
                                                           tileBuffer.data(), static_cast<quint64>(tileBufferCurrBitOffset),
                                                           static_cast<quint64>(ctx->bitsPerSample));
                                tileBufferBitOffset += ctx->bitsPerSample;
                            }
                        }
                    }
                    else
                    {
                        for(qint64 y = 0; y < ctx->subsamplingver; ++y)
                        {
                            for(qint64 x = 0; x < ctx->subsamplinghor; ++x)
                            {
                                const qint64 bufferCurrBitOffset = bufferBitOffset + y * tileLineSize * 8 + x * ctx->bitsPerSample * ctx->samplesPerPixel + c * ctx->bitsPerSample;
                                assert(bufferCurrBitOffset + ctx->bitsPerSample <= buffer.size() * 8);
                                assert(tileBufferBitOffset + ctx->bitsPerSample <= tileBufferSize * 8);
                                DataProcessing::memcpyBits(buffer.data(), static_cast<quint64>(bufferCurrBitOffset),
                                                           tileBuffer.data(), static_cast<quint64>(tileBufferBitOffset),
                                                           static_cast<quint64>(ctx->bitsPerSample));
                            }
                        }
                        tileBufferBitOffset += ctx->bitsPerSample;
                    }
                }
                bufferBitOffset += ctx->bitsPerSample * ctx->samplesPerPixel * ctx->subsamplinghor;
                if(bufferLineBitPos + currTileWidth * ctx->samplesPerPixel * ctx->bitsPerSample <= bufferBitOffset)
                {
                    bufferLineBitPos += tileLineSize * ctx->subsamplingver * 8;
                    bufferBitOffset = bufferLineBitPos;
                    tileBufferLineBitPos += tileScanlineSize * ctx->subsamplingver * 8;
                    tileBufferBitOffset = tileBufferLineBitPos;
                }
                if(bufferLineBitPos + tileLineSize * 8 > tileLineSize * 8 * tileHeight)
                    break;
            }
        }

        assert(tileLineSize * tileHeight <= buffer.size());
        const QImage tileRgb = readFromRawBuffer(reinterpret_cast<const quint8*>(buffer.data()), tileWidth, tileHeight, tileLineSize, ctx);
        if(tileRgb.isNull())
            continue;

        assert(IsOneOf(tileRgb.format(), QImage::Format_ARGB32, QImage::Format_RGB32));
        for(qint64 y = 0; y < currTileHeight; ++y)
        {
            const QRgb *inScanLine = reinterpret_cast<const QRgb*>(tileRgb.scanLine(y));
            QRgb *outScanLine = reinterpret_cast<QRgb*>(result.scanLine(currTileY * tileHeight + y)) + currTileX * tileWidth;
            memcpy(outScanLine, inScanLine, static_cast<size_t>(currTileWidth * 4));
        }
    }

    return result;
}

QImage readTiffFileTiledSeparate(const Context *ctx, qint64 nTiles, qint64 tileWidth, qint64 tileHeight)
{
    assert(ctx);
    assert(ctx->tiff);
    assert(nTiles > 0);
    assert(tileWidth > 0);
    assert(tileHeight > 0);
    LOG_DEBUG() << LOGGING_CTX << "Tiled data with separate planes of subsampled data detected";

    if(nTiles % ctx->samplesPerPixel != 0)
    {
        LOG_WARNING() << LOGGING_CTX << "Invalid tiles count," << nTiles << "should be multiple of" << ctx->samplesPerPixel;
        return QImage();
    }

    QImage result(ctx->width, ctx->height, ctx->alphaIndex >= 0 ? QImage::Format_ARGB32 : QImage::Format_RGB32);
    if(result.isNull())
    {
        LOG_WARNING() << LOGGING_CTX << "Invalid image size";
        return QImage();
    }
    result.fill(ctx->alphaIndex >= 0 ? Qt::transparent : Qt::white);

    const qint64 bufferAllocSize = tileWidth * tileHeight * ctx->samplesPerPixel * ctx->bitsPerSample / 8;
    if(bufferAllocSize > BUFFER_MAX_SIZE)
    {
        LOG_WARNING() << LOGGING_CTX << "Can't allocate buffer with size =" << bufferAllocSize << "due to allocation limit =" << BUFFER_MAX_SIZE;
        return QImage();
    }

    QByteArray buffer;
    buffer.resize(bufferAllocSize);

    QByteArray tileBuffer;
    tileBuffer.resize(tileWidth * tileHeight * ctx->bitsPerSample / 8);

    const qint64 leftX = ctx->width % tileWidth;
    const qint64 leftY = ctx->height % tileHeight;
    const qint64 nTilesX = (ctx->width / tileWidth)   + (leftX == 0 ? 0 : 1);
    const qint64 nTilesY = (ctx->height / tileHeight) + (leftY == 0 ? 0 : 1);
    const qint64 tileLineBitSize = tileWidth * ctx->samplesPerPixel * ctx->bitsPerSample;
    const qint64 tileLineSize = tileLineBitSize / 8;
    const qint64 tilesPerSample = nTiles / ctx->samplesPerPixel;

    for(qint64 tile = 0; tile < tilesPerSample; ++tile)
    {
        memset(buffer.data(), BUFFER_FILL_PATTERN, static_cast<size_t>(buffer.size()));
        const qint64 currTileX = tile % nTilesX;
        const qint64 currTileY = tile / nTilesX;
        const qint64 currTileWidth  = ((currTileX == nTilesX - 1) && leftX) ? leftX : tileWidth;
        const qint64 currTileHeight = ((currTileY == nTilesY - 1) && leftY) ? leftY : tileHeight;

        for(qint64 sample = 0; sample < ctx->samplesPerPixel; ++sample)
        {
            memset(tileBuffer.data(), BUFFER_FILL_PATTERN, static_cast<size_t>(tileBuffer.size()));
            const qint64 tileBufferSize = TIFFReadEncodedTile(ctx->tiff, static_cast<quint32>(sample * tilesPerSample + tile), tileBuffer.data(), tileBuffer.size());
            if(tileBufferSize < 0)
            {
                LOG_WARNING() << LOGGING_CTX << "TIFFReadEncodedTile failed for tile" << tile;
                if(tile == 0)
                    return QImage();
                // Try to handle if we can read some tiles
                break;
            }
            const qint64 expectedTileBufferSize = tileHeight * tileWidth * ctx->bitsPerSample / 8;
            if(tileBufferSize < expectedTileBufferSize)
            {
                LOG_WARNING() << LOGGING_CTX << "TIFFReadEncodedTile returns not enough data to decode tile" << tile << "got:" << tileBufferSize << "expected:" << expectedTileBufferSize;
                if(tile == 0)
                    return QImage();
                // Try to handle if we can read some tiles
                break;
            }

#if !defined (DEBUG_FORCE_BIT_ACCESS)
            if(ctx->bitsPerSample % 8 == 0)
#else
            if(false)
#endif
            {
                for(qint64 y = 0; y < tileHeight; ++y)
                {
                    for(qint64 x = 0; x < tileWidth; ++x)
                    {
                        const qint64 bufferOffset = ((y * tileWidth + x) * ctx->samplesPerPixel + sample) * ctx->bitsPerSample / 8;
                        const qint64 tileBufferOffset = (y * tileWidth + x) * ctx->bitsPerSample / 8;
                        assert(bufferOffset + ctx->bitsPerSample / 8 <= buffer.size());
                        assert(tileBufferOffset + ctx->bitsPerSample / 8 <= tileBufferSize);
                        memcpy(buffer.data() + bufferOffset, tileBuffer.data() + tileBufferOffset, static_cast<size_t>(ctx->bitsPerSample / 8));
                    }
                }
            }
            else
            {
                for(qint64 y = 0; y < tileHeight; ++y)
                {
                    for(qint64 x = 0; x < tileWidth; ++x)
                    {
                        const qint64 bufferBitOffset = ((y * tileWidth + x) * ctx->samplesPerPixel + sample) * ctx->bitsPerSample;
                        const qint64 tileBufferBitOffset = (y * tileWidth + x) * ctx->bitsPerSample;
                        assert(bufferBitOffset + ctx->bitsPerSample <= buffer.size() * 8);
                        assert(tileBufferBitOffset + ctx->bitsPerSample <= tileBufferSize * 8);
                        DataProcessing::memcpyBits(buffer.data(), static_cast<quint64>(bufferBitOffset),
                                                   tileBuffer.data(), static_cast<quint64>(tileBufferBitOffset),
                                                   static_cast<quint64>(ctx->bitsPerSample));
                    }
                }
            }
        }

        assert(tileLineSize * tileHeight <= buffer.size());
        const QImage tileRgb = readFromRawBuffer(reinterpret_cast<const quint8*>(buffer.data()), tileWidth, tileHeight, tileLineSize, ctx);
        if(tileRgb.isNull())
            continue;

        assert(IsOneOf(tileRgb.format(), QImage::Format_ARGB32, QImage::Format_RGB32));
        for(qint64 y = 0; y < currTileHeight; ++y)
        {
            const QRgb *inScanLine = reinterpret_cast<const QRgb*>(tileRgb.scanLine(y));
            QRgb *outScanLine = reinterpret_cast<QRgb*>(result.scanLine(currTileY * tileHeight + y)) + currTileX * tileWidth;
            memcpy(outScanLine, inScanLine, static_cast<size_t>(currTileWidth * 4));
        }
    }

    return result;
}

QImage readTiffFileTiledSeparateSubsampled(const Context *ctx, qint64 nTiles, qint64 tileWidth, qint64 tileHeight)
{
    /// @todo No known test samples for this layout
    assert(ctx);
    assert(ctx->tiff);
    assert(nTiles > 0);
    assert(tileWidth > 0);
    assert(tileHeight > 0);
    LOG_DEBUG() << LOGGING_CTX << "Tiled data with separate planes of subsampled data detected";

    if(nTiles % ctx->samplesPerPixel != 0)
    {
        LOG_WARNING() << LOGGING_CTX << "Invalid tiles count," << nTiles << "should be multiple of" << ctx->samplesPerPixel;
        return QImage();
    }

    QImage result(ctx->width, ctx->height, ctx->alphaIndex >= 0 ? QImage::Format_ARGB32 : QImage::Format_RGB32);
    if(result.isNull())
    {
        LOG_WARNING() << LOGGING_CTX << "Invalid image size";
        return QImage();
    }
    result.fill(ctx->alphaIndex >= 0 ? Qt::transparent : Qt::white);

    const qint64 bufferAllocSize = tileWidth * tileHeight * ctx->samplesPerPixel * ctx->bitsPerSample / 8;
    if(bufferAllocSize > BUFFER_MAX_SIZE)
    {
        LOG_WARNING() << LOGGING_CTX << "Can't allocate buffer with size =" << bufferAllocSize << "due to allocation limit =" << BUFFER_MAX_SIZE;
        return QImage();
    }

    QByteArray buffer;
    buffer.resize(bufferAllocSize);

    QByteArray tileBuffer;
    tileBuffer.resize(tileWidth * tileHeight * ctx->bitsPerSample / 8);

    const qint64 leftX = ctx->width % tileWidth;
    const qint64 leftY = ctx->height % tileHeight;
    const qint64 nTilesX = (ctx->width / tileWidth)   + (leftX == 0 ? 0 : 1);
    const qint64 nTilesY = (ctx->height / tileHeight) + (leftY == 0 ? 0 : 1);
    const qint64 tileLineBitSize = tileWidth * ctx->samplesPerPixel * ctx->bitsPerSample;
    const qint64 tileLineSize = tileLineBitSize / 8;
    const qint64 tilesPerSample = nTiles / ctx->samplesPerPixel;

    for(qint64 tile = 0; tile < tilesPerSample; ++tile)
    {
        memset(buffer.data(), BUFFER_FILL_PATTERN, static_cast<size_t>(buffer.size()));
        const qint64 currTileX = tile % nTilesX;
        const qint64 currTileY = tile / nTilesX;
        const qint64 currTileWidth  = ((currTileX == nTilesX - 1) && leftX) ? leftX : tileWidth;
        const qint64 currTileHeight = ((currTileY == nTilesY - 1) && leftY) ? leftY : tileHeight;

        for(qint64 sample = 0; sample < ctx->samplesPerPixel; ++sample)
        {
            memset(tileBuffer.data(), BUFFER_FILL_PATTERN, static_cast<size_t>(tileBuffer.size()));
            const qint64 tileBufferSize = TIFFReadEncodedTile(ctx->tiff, static_cast<quint32>(sample * tilesPerSample + tile), tileBuffer.data(), tileBuffer.size());
            if(tileBufferSize < 0)
            {
                LOG_WARNING() << LOGGING_CTX << "TIFFReadEncodedTile failed for tile" << tile;
                if(tile == 0)
                    return QImage();
                // Try to handle if we can read some tiles
                break;
            }

            const bool isSubsampledChroma = IsOneOf(sample, 1, 2);
            const qint64 expectedTileBufferSize = tileHeight * tileWidth * ctx->bitsPerSample / 8 / (isSubsampledChroma ? (ctx->subsamplingver * ctx->subsamplinghor) : 1);
            if(tileBufferSize < expectedTileBufferSize)
            {
                LOG_WARNING() << LOGGING_CTX << "TIFFReadEncodedTile returns not enough data to decode tile" << tile << "got:" << tileBufferSize << "expected:" << expectedTileBufferSize;
                if(tile == 0)
                    return QImage();
                // Try to handle if we can read some tiles
                break;
            }

            if(isSubsampledChroma)
            {
#if !defined (DEBUG_FORCE_BIT_ACCESS)
                if(ctx->bitsPerSample % 8 == 0)
#else
                if(false)
#endif
                {
                    for(qint64 y = 0; y < tileHeight / ctx->subsamplingver; ++y)
                    {
                        for(qint64 x = 0; x < tileWidth / ctx->subsamplinghor; ++x)
                        {
                            const qint64 tileBufferOffset = (y * tileWidth / ctx->subsamplinghor + x) * ctx->bitsPerSample / 8;
                            assert(tileBufferOffset + ctx->bitsPerSample / 8 <= tileBufferSize);
                            for(qint64 yu = y * ctx->subsamplingver; yu < (y + 1) * ctx->subsamplingver; ++yu)
                            {
                                for(qint64 xu = x * ctx->subsamplinghor; xu < (x + 1) * ctx->subsamplinghor; ++xu)
                                {
                                    const qint64 bufferOffset = ((yu * tileWidth + xu) * ctx->samplesPerPixel + sample) * ctx->bitsPerSample / 8;
                                    assert(bufferOffset + ctx->bitsPerSample / 8 <= buffer.size());
                                    memcpy(buffer.data() + bufferOffset, tileBuffer.data() + tileBufferOffset, static_cast<size_t>(ctx->bitsPerSample / 8));
                                }
                            }
                        }
                    }
                }
                else
                {
                    for(qint64 y = 0; y < tileHeight / ctx->subsamplingver; ++y)
                    {
                        for(qint64 x = 0; x < tileWidth / ctx->subsamplinghor; ++x)
                        {
                            const qint64 tileBufferBitOffset = (y * tileWidth / ctx->subsamplinghor + x) * ctx->bitsPerSample;
                            assert(tileBufferBitOffset + ctx->bitsPerSample <= tileBufferSize * 8);
                            for(qint64 yu = y * ctx->subsamplingver; yu < (y + 1) * ctx->subsamplingver; ++yu)
                            {
                                for(qint64 xu = x * ctx->subsamplinghor; xu < (x + 1) * ctx->subsamplinghor; ++xu)
                                {
                                    const qint64 bufferBitOffset = ((yu * tileWidth + xu) * ctx->samplesPerPixel + sample) * ctx->bitsPerSample;
                                    assert(bufferBitOffset + ctx->bitsPerSample <= buffer.size() * 8);
                                    DataProcessing::memcpyBits(buffer.data(), static_cast<quint64>(bufferBitOffset),
                                                               tileBuffer.data(), static_cast<quint64>(tileBufferBitOffset),
                                                               static_cast<quint64>(ctx->bitsPerSample));
                                }
                            }
                        }
                    }
                }
            }
            else
            {
#if !defined (DEBUG_FORCE_BIT_ACCESS)
                if(ctx->bitsPerSample % 8 == 0)
#else
                if(false)
#endif
                {
                    for(qint64 y = 0; y < tileHeight; ++y)
                    {
                        for(qint64 x = 0; x < tileWidth; ++x)
                        {
                            const qint64 bufferOffset = ((y * tileWidth + x) * ctx->samplesPerPixel + sample) * ctx->bitsPerSample / 8;
                            const qint64 tileBufferOffset = (y * tileWidth + x) * ctx->bitsPerSample / 8;
                            assert(bufferOffset + ctx->bitsPerSample / 8 <= buffer.size());
                            assert(tileBufferOffset + ctx->bitsPerSample / 8 <= tileBufferSize);
                            memcpy(buffer.data() + bufferOffset, tileBuffer.data() + tileBufferOffset, static_cast<size_t>(ctx->bitsPerSample / 8));
                        }
                    }
                }
                else
                {
                    for(qint64 y = 0; y < tileHeight; ++y)
                    {
                        for(qint64 x = 0; x < tileWidth; ++x)
                        {
                            const qint64 bufferBitOffset = ((y * tileWidth + x) * ctx->samplesPerPixel + sample) * ctx->bitsPerSample;
                            const qint64 tileBufferBitOffset = (y * tileWidth + x) * ctx->bitsPerSample;
                            assert(bufferBitOffset + ctx->bitsPerSample <= buffer.size() * 8);
                            assert(tileBufferBitOffset + ctx->bitsPerSample <= tileBufferSize * 8);
                            DataProcessing::memcpyBits(buffer.data(), static_cast<quint64>(bufferBitOffset),
                                                       tileBuffer.data(), static_cast<quint64>(tileBufferBitOffset),
                                                       static_cast<quint64>(ctx->bitsPerSample));
                        }
                    }
                }
            }
        }

        assert(tileLineSize * tileHeight <= buffer.size());
        const QImage tileRgb = readFromRawBuffer(reinterpret_cast<const quint8*>(buffer.data()), tileWidth, tileHeight, tileLineSize, ctx);
        if(tileRgb.isNull())
            continue;

        assert(IsOneOf(tileRgb.format(), QImage::Format_ARGB32, QImage::Format_RGB32));
        for(qint64 y = 0; y < currTileHeight; ++y)
        {
            const QRgb *inScanLine = reinterpret_cast<const QRgb*>(tileRgb.scanLine(y));
            QRgb *outScanLine = reinterpret_cast<QRgb*>(result.scanLine(currTileY * tileHeight + y)) + currTileX * tileWidth;
            memcpy(outScanLine, inScanLine, static_cast<size_t>(currTileWidth * 4));
        }
    }

    return result;
}

QImage readTiffFileTiled(const Context *ctx)
{
    assert(ctx);
    assert(ctx->tiff);

    quint16 planarConfig = 0;
    if(!TIFFGetFieldDefaulted(ctx->tiff, TIFFTAG_PLANARCONFIG, &planarConfig) || !IsOneOf(planarConfig, PLANARCONFIG_CONTIG, PLANARCONFIG_SEPARATE))
        planarConfig = PLANARCONFIG_CONTIG;

    const qint64 nTiles = static_cast<qint64>(TIFFNumberOfTiles(ctx->tiff));
    if(nTiles <= 0)
    {
        LOG_WARNING() << LOGGING_CTX << "Invalid TIFFNumberOfTiles =" << nTiles;
        return QImage();
    }
    LOG_DEBUG() << LOGGING_CTX << "TIFFNumberOfTiles =" << nTiles;

    quint32 tileWidth = 0;
    if(!TIFFGetFieldDefaulted(ctx->tiff, TIFFTAG_TILEWIDTH, &tileWidth) || tileWidth == 0)
    {
        LOG_WARNING() << LOGGING_CTX << "Can't get TIFFTAG_TILEWIDTH";
        return QImage();
    }
    const qint64 tileWidth64 = static_cast<qint64>(tileWidth);

    quint32 tileHeight = 0;
    if(!TIFFGetFieldDefaulted(ctx->tiff, TIFFTAG_TILELENGTH, &tileHeight) || tileHeight == 0)
    {
        LOG_WARNING() << LOGGING_CTX << "Can't get TIFFTAG_TILELENGTH";
        return QImage();
    }
    const qint64 tileHeight64 = static_cast<qint64>(tileHeight);

    const bool isValidSubsamplingH = IsOneOf(ctx->subsamplinghor, 1, 2, 4);
    const bool isValidSubsamplingV = IsOneOf(ctx->subsamplingver, 1, 2, 4);
    const bool isValidSubsampledYCbCr = ctx->photometric == PHOTOMETRIC_YCBCR && ctx->primarysamplesCount == 3 && isValidSubsamplingH && isValidSubsamplingV && (ctx->subsamplinghor > 1 || ctx->subsamplingver > 1);

    if(planarConfig == PLANARCONFIG_CONTIG)
    {
        if(isValidSubsampledYCbCr)
            return readTiffFileTiledContigSubsampled(ctx, nTiles, tileWidth64, tileHeight64);
        else
            return readTiffFileTiledContig(ctx, nTiles, tileWidth64, tileHeight64);
    }
    else if(planarConfig == PLANARCONFIG_SEPARATE)
    {
        if(isValidSubsampledYCbCr)
            return readTiffFileTiledSeparateSubsampled(ctx, nTiles, tileWidth64, tileHeight64);
        else
            return readTiffFileTiledSeparate(ctx, nTiles, tileWidth64, tileHeight64);
    }

    assert(false);
    return QImage();
}

// ====================================================================================================

QImage readTiffFileStripedContig(const Context *ctx, qint64 numberOfStrips, qint64 stripSize, qint64 rowsperstrip, qint64 scanlineSize, qint64 rasterScanlineSize)
{
    assert(ctx);
    assert(ctx->tiff);
    assert(numberOfStrips > 0);
    assert(stripSize > 0);
    assert(rowsperstrip > 0);
    LOG_DEBUG() << LOGGING_CTX << "Striped data with single image plane of data detected";

    if(scanlineSize != rasterScanlineSize)
    {
        LOG_WARNING() << LOGGING_CTX << "Invalid scanline sizes: scanlineSize =" << scanlineSize << "rasterScanlineSize =" << rasterScanlineSize;
        return QImage();
    }

    if(scanlineSize < ctx->width * ctx->samplesPerPixel * ctx->bitsPerSample / 8)
    {
        LOG_WARNING() << LOGGING_CTX << "Invalid scanline size," << scanlineSize << "should be great or equal" << ctx->width * ctx->samplesPerPixel * ctx->bitsPerSample / 8;
        return QImage();
    }

    QImage result(ctx->width, ctx->height, ctx->alphaIndex >= 0 ? QImage::Format_ARGB32 : QImage::Format_RGB32);
    if(result.isNull())
    {
        LOG_WARNING() << LOGGING_CTX << "Invalid image size";
        return QImage();
    }
    result.fill(ctx->alphaIndex >= 0 ? Qt::transparent : Qt::white);

    const qint64 bufferAllocSize = stripSize;
    if(bufferAllocSize > BUFFER_MAX_SIZE)
    {
        LOG_WARNING() << LOGGING_CTX << "Can't allocate buffer with size =" << bufferAllocSize << "due to allocation limit =" << BUFFER_MAX_SIZE;
        return QImage();
    }

    QByteArray buffer;
    buffer.resize(bufferAllocSize);

    const qint64 leftRows = ctx->height % rowsperstrip;

    for(qint64 strip = 0; strip < numberOfStrips; ++strip)
    {
        memset(buffer.data(), BUFFER_FILL_PATTERN, static_cast<size_t>(buffer.size()));
        const qint64 stripBufferSize = TIFFReadEncodedStrip(ctx->tiff, static_cast<quint32>(strip), buffer.data(), buffer.size());
        if(stripBufferSize < 0)
        {
            LOG_WARNING() << LOGGING_CTX << "TIFFReadEncodedStrip failed for strip" << strip;
            if(strip == 0)
                return QImage();
            // Try to handle if we can read some strips
            break;
        }
        const qint64 currStripHeight = ((strip == numberOfStrips - 1) && leftRows) ? leftRows : rowsperstrip;
        const qint64 expectedStripBufferSize = currStripHeight * rasterScanlineSize;
        if(stripBufferSize < expectedStripBufferSize)
        {
            LOG_WARNING() << LOGGING_CTX << "TIFFReadEncodedStrip returns not enough data to decode strip" << strip << "got:" << stripBufferSize << "expected:" << expectedStripBufferSize;
            if(strip == 0)
                return QImage();
            // Try to handle if we can read some strips
            break;
        }

        assert(currStripHeight * rasterScanlineSize <= stripBufferSize);
        const QImage stripRgb = readFromRawBuffer(reinterpret_cast<const quint8*>(buffer.data()), ctx->width, currStripHeight, rasterScanlineSize, ctx);
        if(stripRgb.isNull())
            continue;

        assert(IsOneOf(stripRgb.format(), QImage::Format_ARGB32, QImage::Format_RGB32));
        for(qint64 y = 0; y < currStripHeight && strip * rowsperstrip + y < ctx->height; ++y)
        {
            const QRgb *inScanLine = reinterpret_cast<const QRgb*>(stripRgb.scanLine(y));
            QRgb *outScanLine = reinterpret_cast<QRgb*>(result.scanLine(strip * rowsperstrip + y));
            memcpy(outScanLine, inScanLine, static_cast<size_t>(ctx->width * 4));
        }
    }

    return result;
}

QImage readTiffFileStripedContigSubsampled(const Context *ctx, qint64 numberOfStrips, qint64 stripSize, qint64 rowsperstrip, qint64 scanlineSize, qint64 rasterScanlineSize)
{
    assert(ctx);
    assert(ctx->tiff);
    assert(numberOfStrips > 0);
    assert(stripSize > 0);
    assert(rowsperstrip > 0);
    LOG_DEBUG() << LOGGING_CTX << "Striped data with single image plane of subsampled data detected";

    QImage result(ctx->width, ctx->height, ctx->alphaIndex >= 0 ? QImage::Format_ARGB32 : QImage::Format_RGB32);
    if(result.isNull())
    {
        LOG_WARNING() << LOGGING_CTX << "Invalid image size";
        return QImage();
    }
    result.fill(ctx->alphaIndex >= 0 ? Qt::transparent : Qt::white);

    const qint64 bufferAllocSize = rasterScanlineSize * (ctx->height + 4);
    if(bufferAllocSize > BUFFER_MAX_SIZE)
    {
        LOG_WARNING() << LOGGING_CTX << "Can't allocate buffer with size =" << bufferAllocSize << "due to allocation limit =" << BUFFER_MAX_SIZE;
        return QImage();
    }

    QByteArray buffer;
    buffer.resize(bufferAllocSize);

    QByteArray stripBuffer;
    stripBuffer.resize(stripSize);

    const qint64 leftRows = ctx->height % rowsperstrip;
    const qint64 minLineBitSize = ctx->width * ctx->samplesPerPixel * ctx->bitsPerSample;
    const qint64 minLineSize = minLineBitSize / 8;

    for(qint64 strip = 0; strip < numberOfStrips; ++strip)
    {
        memset(buffer.data(), BUFFER_FILL_PATTERN, static_cast<size_t>(buffer.size()));
        memset(stripBuffer.data(), BUFFER_FILL_PATTERN, static_cast<size_t>(stripBuffer.size()));

        const qint64 stripBufferSize = TIFFReadEncodedStrip(ctx->tiff, static_cast<quint32>(strip), stripBuffer.data(), stripBuffer.size());
        if(stripBufferSize < 0)
        {
            LOG_WARNING() << LOGGING_CTX << "TIFFReadEncodedStrip failed for strip" << strip;
            if(strip == 0)
                return QImage();
            // Try to handle if we can read some strips
            break;
        }
        const qint64 currStripHeight = ((strip == numberOfStrips - 1) && leftRows) ? leftRows : rowsperstrip;
        const qint64 expectedStripBufferSize = currStripHeight * ctx->width * (ctx->samplesPerPixel - 2) * ctx->bitsPerSample / 8 + currStripHeight / ctx->subsamplingver * ctx->width / ctx->subsamplinghor * 2 * ctx->bitsPerSample / 8;
        if(stripBufferSize < expectedStripBufferSize)
        {
            LOG_WARNING() << LOGGING_CTX << "TIFFReadEncodedStrip returns not enough data to decode strip" << strip << "got:" << stripBufferSize << "expected:" << expectedStripBufferSize;
            if(strip == 0)
                return QImage();
            // Try to handle if we can read some strips
            break;
        }

#if !defined (DEBUG_FORCE_BIT_ACCESS)
        if(ctx->bitsPerSample % 8 == 0)
#else
        if(false)
#endif
        {
            qint64 bufferLinePos = 0;
            qint64 bufferOffset = bufferLinePos;
            qint64 stripBufferLinePos = 0;
            qint64 stripBufferOffset = stripBufferLinePos;

            while(stripBufferOffset < stripBufferSize && bufferOffset < buffer.size())
            {
                for(qint64 c = 0; c < ctx->samplesPerPixel; ++c)
                {
                    const bool isChroma = IsOneOf(c, 1, 2);
                    if(!isChroma)
                    {
                        for(qint64 y = 0; y < ctx->subsamplingver; ++y)
                        {
                            for(qint64 x = 0; x < ctx->subsamplinghor; ++x)
                            {
                                const qint64 bufferCurrOffset = bufferOffset + y * rasterScanlineSize + x * ctx->bitsPerSample / 8 * ctx->samplesPerPixel + c * ctx->bitsPerSample / 8;
                                qint64 stripBufferCurrOffset = stripBufferOffset;
                                assert(bufferCurrOffset + ctx->bitsPerSample / 8 <= buffer.size());
                                assert(stripBufferCurrOffset + ctx->bitsPerSample / 8 <= stripBufferSize);
                                memcpy(buffer.data() + bufferCurrOffset, stripBuffer.data() + stripBufferCurrOffset, static_cast<size_t>(ctx->bitsPerSample / 8));
                                stripBufferOffset += ctx->bitsPerSample / 8;
                            }
                        }
                    }
                    else
                    {
                        for(qint64 y = 0; y < ctx->subsamplingver; ++y)
                        {
                            for(qint64 x = 0; x < ctx->subsamplinghor; ++x)
                            {
                                const qint64 bufferCurrOffset = bufferOffset + y * rasterScanlineSize + x * ctx->bitsPerSample / 8 * ctx->samplesPerPixel + c * ctx->bitsPerSample / 8;
                                assert(bufferCurrOffset + ctx->bitsPerSample / 8 <= buffer.size());
                                assert(stripBufferOffset + ctx->bitsPerSample / 8 <= stripBufferSize);
                                memcpy(buffer.data() + bufferCurrOffset, stripBuffer.data() + stripBufferOffset, static_cast<size_t>(ctx->bitsPerSample / 8));
                            }
                        }
                        stripBufferOffset += ctx->bitsPerSample / 8;
                    }
                }
                bufferOffset += ctx->bitsPerSample / 8 * ctx->samplesPerPixel * ctx->subsamplinghor;
                if(bufferLinePos + minLineSize <= bufferOffset)
                {
                    bufferLinePos += rasterScanlineSize * ctx->subsamplingver;
                    bufferOffset = bufferLinePos;
                    stripBufferLinePos += scanlineSize * ctx->subsamplingver;
                    stripBufferOffset = stripBufferLinePos;
                }
                if(bufferLinePos + rasterScanlineSize > rasterScanlineSize * ctx->height)
                    break;
            }
        }
        else
        {
            qint64 bufferLineBitPos = 0;
            qint64 bufferBitOffset = bufferLineBitPos;
            qint64 stripBufferLineBitPos = 0;
            qint64 stripBufferBitOffset = stripBufferLineBitPos;

            while(stripBufferBitOffset < stripBufferSize * 8 && bufferBitOffset < buffer.size() * 8)
            {
                for(qint64 c = 0; c < ctx->samplesPerPixel; ++c)
                {
                    const bool isChroma = IsOneOf(c, 1, 2);
                    if(!isChroma)
                    {
                        for(qint64 y = 0; y < ctx->subsamplingver; ++y)
                        {
                            for(qint64 x = 0; x < ctx->subsamplinghor; ++x)
                            {
                                const qint64 bufferCurrBitOffset = bufferBitOffset + y * rasterScanlineSize * 8 + x * ctx->bitsPerSample * ctx->samplesPerPixel + c * ctx->bitsPerSample;
                                qint64 stripBufferCurrBitOffset = stripBufferBitOffset;
                                assert(bufferCurrBitOffset + ctx->bitsPerSample <= buffer.size() * 8);
                                assert(stripBufferCurrBitOffset + ctx->bitsPerSample <= stripBufferSize * 8);
                                DataProcessing::memcpyBits(buffer.data(), static_cast<quint64>(bufferCurrBitOffset),
                                                           stripBuffer.data(), static_cast<quint64>(stripBufferCurrBitOffset),
                                                           static_cast<quint64>(ctx->bitsPerSample));
                                stripBufferBitOffset += ctx->bitsPerSample;
                            }
                        }
                    }
                    else
                    {
                        for(qint64 y = 0; y < ctx->subsamplingver; ++y)
                        {
                            for(qint64 x = 0; x < ctx->subsamplinghor; ++x)
                            {
                                const qint64 bufferCurrBitOffset = bufferBitOffset + y * rasterScanlineSize * 8 + x * ctx->bitsPerSample * ctx->samplesPerPixel + c * ctx->bitsPerSample;
                                assert(bufferCurrBitOffset + ctx->bitsPerSample <= buffer.size() * 8);
                                assert(stripBufferBitOffset + ctx->bitsPerSample <= stripBufferSize * 8);
                                DataProcessing::memcpyBits(buffer.data(), static_cast<quint64>(bufferCurrBitOffset),
                                                           stripBuffer.data(), static_cast<quint64>(stripBufferBitOffset),
                                                           static_cast<quint64>(ctx->bitsPerSample));
                            }
                        }
                        stripBufferBitOffset += ctx->bitsPerSample;
                    }
                }
                bufferBitOffset += ctx->bitsPerSample * ctx->samplesPerPixel * ctx->subsamplinghor;
                if(bufferLineBitPos + minLineBitSize <= bufferBitOffset)
                {
                    bufferLineBitPos += rasterScanlineSize * 8 * ctx->subsamplingver;
                    bufferBitOffset = bufferLineBitPos;
                    stripBufferLineBitPos += scanlineSize * 8 * ctx->subsamplingver;
                    stripBufferBitOffset = stripBufferLineBitPos;
                }
                if(bufferLineBitPos + rasterScanlineSize * 8 > rasterScanlineSize * 8 * ctx->height)
                    break;
            }
        }

        assert(currStripHeight * rasterScanlineSize <= buffer.size());
        const QImage stripRgb = readFromRawBuffer(reinterpret_cast<const quint8*>(buffer.data()), ctx->width, currStripHeight, rasterScanlineSize, ctx);
        if(stripRgb.isNull())
            continue;

        assert(IsOneOf(stripRgb.format(), QImage::Format_ARGB32, QImage::Format_RGB32));
        for(qint64 y = 0; y < currStripHeight && strip * rowsperstrip + y < ctx->height; ++y)
        {
            const QRgb *inScanLine = reinterpret_cast<const QRgb*>(stripRgb.scanLine(y));
            QRgb *outScanLine = reinterpret_cast<QRgb*>(result.scanLine(strip * rowsperstrip + y));
            memcpy(outScanLine, inScanLine, static_cast<size_t>(ctx->width * 4));
        }
    }

    return result;
}

QImage readTiffFileStripedSeparate(const Context *ctx, qint64 numberOfStrips, qint64 stripSize, qint64 rowsperstrip, qint64 scanlineSize, qint64 rasterScanlineSize)
{
    assert(ctx);
    assert(ctx->tiff);
    assert(numberOfStrips > 0);
    assert(stripSize > 0);
    assert(rowsperstrip > 0);
    LOG_DEBUG() << LOGGING_CTX << "Striped data with separate planes of data detected";

    if(numberOfStrips % ctx->samplesPerPixel != 0)
    {
        LOG_WARNING() << LOGGING_CTX << "Invalid strips count," << numberOfStrips << "should be multiple of" << ctx->samplesPerPixel;
        return QImage();
    }

    if(scanlineSize < ctx->width * ctx->bitsPerSample / 8)
    {
        LOG_WARNING() << LOGGING_CTX << "Invalid scanline size," << scanlineSize << "should be great or equal" << ctx->width * ctx->bitsPerSample / 8;
        return QImage();
    }

    QImage result(ctx->width, ctx->height, ctx->alphaIndex >= 0 ? QImage::Format_ARGB32 : QImage::Format_RGB32);
    if(result.isNull())
    {
        LOG_WARNING() << LOGGING_CTX << "Invalid image size";
        return QImage();
    }
    result.fill(ctx->alphaIndex >= 0 ? Qt::transparent : Qt::white);

    const qint64 bufferAllocSize = rasterScanlineSize * (ctx->height + 4);
    if(bufferAllocSize > BUFFER_MAX_SIZE)
    {
        LOG_WARNING() << LOGGING_CTX << "Can't allocate buffer with size =" << bufferAllocSize << "due to allocation limit =" << BUFFER_MAX_SIZE;
        return QImage();
    }

    QByteArray buffer;
    buffer.resize(bufferAllocSize);

    QByteArray stripBuffer;
    stripBuffer.resize(stripSize);

    const qint64 leftRows = ctx->height % rowsperstrip;
    const qint64 stripsPerSample = numberOfStrips / ctx->samplesPerPixel;

    for(qint64 strip = 0; strip < stripsPerSample; ++strip)
    {
        memset(buffer.data(), BUFFER_FILL_PATTERN, static_cast<size_t>(buffer.size()));
        const qint64 currStripHeight = ((strip == stripsPerSample - 1) && leftRows) ? leftRows : rowsperstrip;

        for(qint64 sample = 0; sample < ctx->samplesPerPixel; ++sample)
        {
            memset(stripBuffer.data(), BUFFER_FILL_PATTERN, static_cast<size_t>(stripBuffer.size()));
            const qint64 stripBufferSize = TIFFReadEncodedStrip(ctx->tiff, static_cast<quint32>(sample * stripsPerSample + strip), stripBuffer.data(), stripBuffer.size());
            if(stripBufferSize < 0)
            {
                LOG_WARNING() << LOGGING_CTX << "TIFFReadEncodedStrip failed for strip" << strip;
                if(strip == 0)
                    return QImage();
                // Try to handle if we can read some strips
                break;
            }
            const qint64 expectedStripBufferSize = currStripHeight * ctx->width * ctx->bitsPerSample / 8;
            if(stripBufferSize < expectedStripBufferSize)
            {
                LOG_WARNING() << LOGGING_CTX << "TIFFReadEncodedStrip returns not enough data to decode strip" << strip << "got:" << stripBufferSize << "expected:" << expectedStripBufferSize;
                if(strip == 0)
                    return QImage();
                // Try to handle if we can read some strips
                break;
            }

#if !defined (DEBUG_FORCE_BIT_ACCESS)
            if(ctx->bitsPerSample % 8 == 0)
#else
            if(false)
#endif
            {
                for(qint64 y = 0; y < currStripHeight; ++y)
                {
                    for(qint64 x = 0; x < ctx->width; ++x)
                    {
                        const qint64 bufferOffset = y * rasterScanlineSize + (x * ctx->samplesPerPixel + sample) * ctx->bitsPerSample / 8;
                        const qint64 stripBufferOffset = y * scanlineSize + x * ctx->bitsPerSample / 8;
                        assert(bufferOffset + ctx->bitsPerSample / 8 <= buffer.size());
                        assert(stripBufferOffset + ctx->bitsPerSample / 8 <= stripBufferSize);
                        memcpy(buffer.data() + bufferOffset, stripBuffer.data() + stripBufferOffset, static_cast<size_t>(ctx->bitsPerSample / 8));
                    }
                }
            }
            else
            {
                for(qint64 y = 0; y < currStripHeight; ++y)
                {
                    for(qint64 x = 0; x < ctx->width; ++x)
                    {
                        const qint64 bufferBitOffset = y * rasterScanlineSize * 8 + (x * ctx->samplesPerPixel + sample) * ctx->bitsPerSample;
                        const qint64 stripBufferBitOffset = y * scanlineSize * 8 + x * ctx->bitsPerSample;
                        assert(bufferBitOffset + ctx->bitsPerSample <= buffer.size() * 8);
                        assert(stripBufferBitOffset + ctx->bitsPerSample <= stripBufferSize * 8);
                        DataProcessing::memcpyBits(buffer.data(), static_cast<quint64>(bufferBitOffset),
                                                   stripBuffer.data(), static_cast<quint64>(stripBufferBitOffset),
                                                   static_cast<quint64>(ctx->bitsPerSample));
                    }
                }
            }
        }

        assert(currStripHeight * rasterScanlineSize <= buffer.size());
        const QImage stripRgb = readFromRawBuffer(reinterpret_cast<const quint8*>(buffer.data()), ctx->width, currStripHeight, rasterScanlineSize, ctx);
        if(stripRgb.isNull())
            continue;

        assert(IsOneOf(stripRgb.format(), QImage::Format_ARGB32, QImage::Format_RGB32));
        for(qint64 y = 0; y < currStripHeight && strip * rowsperstrip + y < ctx->height; ++y)
        {
            const QRgb *inScanLine = reinterpret_cast<const QRgb*>(stripRgb.scanLine(y));
            QRgb *outScanLine = reinterpret_cast<QRgb*>(result.scanLine(strip * rowsperstrip + y));
            memcpy(outScanLine, inScanLine, static_cast<size_t>(ctx->width * 4));
        }
    }

    return result;
}

QImage readTiffFileStripedSeparateSubsampled(const Context *ctx, qint64 numberOfStrips, qint64 stripSize, qint64 rowsperstrip, qint64 scanlineSize, qint64 rasterScanlineSize)
{
    assert(ctx);
    assert(ctx->tiff);
    assert(numberOfStrips > 0);
    assert(stripSize > 0);
    assert(rowsperstrip > 0);
    LOG_DEBUG() << LOGGING_CTX << "Striped data with separate planes of subsampled data detected";

    if(numberOfStrips % ctx->samplesPerPixel != 0)
    {
        LOG_WARNING() << LOGGING_CTX << "Invalid strips count," << numberOfStrips << "should be multiple of" << ctx->samplesPerPixel;
        return QImage();
    }

    QImage result(ctx->width, ctx->height, ctx->alphaIndex >= 0 ? QImage::Format_ARGB32 : QImage::Format_RGB32);
    if(result.isNull())
    {
        LOG_WARNING() << LOGGING_CTX << "Invalid image size";
        return QImage();
    }
    result.fill(ctx->alphaIndex >= 0 ? Qt::transparent : Qt::white);

    const qint64 bufferAllocSize = rasterScanlineSize * (ctx->height + 4);
    if(bufferAllocSize > BUFFER_MAX_SIZE)
    {
        LOG_WARNING() << LOGGING_CTX << "Can't allocate buffer with size =" << bufferAllocSize << "due to allocation limit =" << BUFFER_MAX_SIZE;
        return QImage();
    }

    QByteArray buffer;
    buffer.resize(bufferAllocSize);

    QByteArray stripBuffer;
    stripBuffer.resize(stripSize);

    const qint64 leftRows = ctx->height % rowsperstrip;
    const qint64 stripsPerSample = numberOfStrips / ctx->samplesPerPixel;

    for(qint64 strip = 0; strip < stripsPerSample; ++strip)
    {
        memset(buffer.data(), BUFFER_FILL_PATTERN, static_cast<size_t>(buffer.size()));
        const qint64 currStripHeight = ((strip == stripsPerSample - 1) && leftRows) ? leftRows : rowsperstrip;

        for(qint64 sample = 0; sample < ctx->samplesPerPixel; ++sample)
        {
            memset(stripBuffer.data(), BUFFER_FILL_PATTERN, static_cast<size_t>(stripBuffer.size()));
            /// @todo LibTIFF-4.6.0: TIFFReadEncodedStrip can't handle subsampled YCbCr strips
            /// directly for PLANARCONFIG_SEPARATE. Error example: "TIFFReadEncodedStrip: Read
            /// error at scanline 4294967295; got 8540 bytes, expected 16384.". So we will try
            /// to read strip with reduced buffer size
            const bool isSubsampledChroma = IsOneOf(sample, 1, 2);
            const qint64 expectedStripBufferSize = currStripHeight * ctx->width * ctx->bitsPerSample / 8 / (isSubsampledChroma ? (ctx->subsamplingver * ctx->subsamplinghor) : 1);
            const qint64 stripBufferSize = TIFFReadEncodedStrip(ctx->tiff, static_cast<quint32>(sample * stripsPerSample + strip), stripBuffer.data(), expectedStripBufferSize);
            if(stripBufferSize < 0)
            {
                LOG_WARNING() << LOGGING_CTX << "TIFFReadEncodedStrip failed for strip" << strip;
                if(strip == 0)
                    return QImage();
                // Try to handle if we can read some strips
                break;
            }

            if(stripBufferSize < expectedStripBufferSize)
            {
                LOG_WARNING() << LOGGING_CTX << "TIFFReadEncodedStrip returns not enough data to decode strip" << strip << "got:" << stripBufferSize << "expected:" << expectedStripBufferSize;
                if(strip == 0)
                    return QImage();
                // Try to handle if we can read some strips
                break;
            }

            if(isSubsampledChroma)
            {
#if !defined (DEBUG_FORCE_BIT_ACCESS)
                if(ctx->bitsPerSample % 8 == 0)
#else
                if(false)
#endif
                {
                    for(qint64 y = 0; y < currStripHeight / ctx->subsamplingver; ++y)
                    {
                        for(qint64 x = 0; x < ctx->width / ctx->subsamplinghor; ++x)
                        {
                            const qint64 stripBufferOffset = y * scanlineSize / ctx->subsamplinghor + x * ctx->bitsPerSample / 8;
                            // const qint64 stripBufferOffset = (y * ctx->width / ctx->subsamplinghor + x) * ctx->bitsPerSample / 8;
                            assert(stripBufferOffset + ctx->bitsPerSample / 8 <= stripBufferSize);
                            for(qint64 yu = y * ctx->subsamplingver; yu < (y + 1) * ctx->subsamplingver; ++yu)
                            {
                                for(qint64 xu = x * ctx->subsamplinghor; xu < (x + 1) * ctx->subsamplinghor; ++xu)
                                {
                                    const qint64 bufferOffset = yu * rasterScanlineSize + (xu * ctx->samplesPerPixel + sample) * ctx->bitsPerSample / 8;
                                    // const qint64 bufferOffset = ((yu * ctx->width + xu) * ctx->samplesPerPixel + sample) * ctx->bitsPerSample / 8;
                                    assert(bufferOffset + ctx->bitsPerSample / 8 <= buffer.size());
                                    memcpy(buffer.data() + bufferOffset, stripBuffer.data() + stripBufferOffset, static_cast<size_t>(ctx->bitsPerSample / 8));
                                }
                            }
                        }
                    }
                }
                else
                {
                    for(qint64 y = 0; y < currStripHeight / ctx->subsamplingver; ++y)
                    {
                        for(qint64 x = 0; x < ctx->width / ctx->subsamplinghor; ++x)
                        {
                            const qint64 stripBufferBitOffset = y * scanlineSize / ctx->subsamplinghor * 8 + x * ctx->bitsPerSample;
                            // const qint64 stripBufferBitOffset = (y * ctx->width / ctx->subsamplinghor + x) * ctx->bitsPerSample;
                            assert(stripBufferBitOffset + ctx->bitsPerSample <= stripBufferSize * 8);
                            for(qint64 yu = y * ctx->subsamplingver; yu < (y + 1) * ctx->subsamplingver; ++yu)
                            {
                                for(qint64 xu = x * ctx->subsamplinghor; xu < (x + 1) * ctx->subsamplinghor; ++xu)
                                {
                                    const qint64 bufferBitOffset = yu * rasterScanlineSize * 8 + (xu * ctx->samplesPerPixel + sample) * ctx->bitsPerSample;
                                    // const qint64 bufferBitOffset = ((yu * ctx->width + xu) * ctx->samplesPerPixel + sample) * ctx->bitsPerSample;
                                    assert(bufferBitOffset + ctx->bitsPerSample <= buffer.size() * 8);
                                    DataProcessing::memcpyBits(buffer.data(), static_cast<quint64>(bufferBitOffset),
                                                               stripBuffer.data(), static_cast<quint64>(stripBufferBitOffset),
                                                               static_cast<quint64>(ctx->bitsPerSample));
                                }
                            }
                        }
                    }
                }
            }
            else
            {
#if !defined (DEBUG_FORCE_BIT_ACCESS)
                if(ctx->bitsPerSample % 8 == 0)
#else
                if(false)
#endif
                {
                    for(qint64 y = 0; y < currStripHeight; ++y)
                    {
                        for(qint64 x = 0; x < ctx->width; ++x)
                        {
                            const qint64 bufferOffset = y * rasterScanlineSize + (x * ctx->samplesPerPixel + sample) * ctx->bitsPerSample / 8;
                            const qint64 stripBufferOffset = y * scanlineSize + x * ctx->bitsPerSample / 8;
                            // const qint64 bufferOffset = ((y * ctx->width + x) * ctx->samplesPerPixel + sample) * ctx->bitsPerSample / 8;
                            // const qint64 stripBufferOffset = (y * ctx->width + x) * ctx->bitsPerSample / 8;
                            assert(bufferOffset + ctx->bitsPerSample / 8 <= buffer.size());
                            assert(stripBufferOffset + ctx->bitsPerSample / 8 <= stripBufferSize);
                            memcpy(buffer.data() + bufferOffset, stripBuffer.data() + stripBufferOffset, static_cast<size_t>(ctx->bitsPerSample / 8));
                        }
                    }
                }
                else
                {
                    for(qint64 y = 0; y < currStripHeight; ++y)
                    {
                        for(qint64 x = 0; x < ctx->width; ++x)
                        {
                            const qint64 bufferBitOffset = y * rasterScanlineSize * 8 + (x * ctx->samplesPerPixel + sample) * ctx->bitsPerSample;
                            const qint64 stripBufferBitOffset = y * scanlineSize * 8 + x * ctx->bitsPerSample;
                            // const qint64 bufferBitOffset = ((y * ctx->width + x) * ctx->samplesPerPixel + sample) * ctx->bitsPerSample;
                            // const qint64 stripBufferBitOffset = (y * ctx->width + x) * ctx->bitsPerSample;
                            assert(bufferBitOffset + ctx->bitsPerSample <= buffer.size() * 8);
                            assert(stripBufferBitOffset + ctx->bitsPerSample <= stripBufferSize * 8);
                            DataProcessing::memcpyBits(buffer.data(), static_cast<quint64>(bufferBitOffset),
                                                       stripBuffer.data(), static_cast<quint64>(stripBufferBitOffset),
                                                       static_cast<quint64>(ctx->bitsPerSample));
                        }
                    }
                }
            }
        }

        assert(currStripHeight * rasterScanlineSize <= buffer.size());
        const QImage stripRgb = readFromRawBuffer(reinterpret_cast<const quint8*>(buffer.data()), ctx->width, currStripHeight, rasterScanlineSize, ctx);
        if(stripRgb.isNull())
            continue;

        assert(IsOneOf(stripRgb.format(), QImage::Format_ARGB32, QImage::Format_RGB32));
        for(qint64 y = 0; y < currStripHeight && strip * rowsperstrip + y < ctx->height; ++y)
        {
            const QRgb *inScanLine = reinterpret_cast<const QRgb*>(stripRgb.scanLine(y));
            QRgb *outScanLine = reinterpret_cast<QRgb*>(result.scanLine(strip * rowsperstrip + y));
            memcpy(outScanLine, inScanLine, static_cast<size_t>(ctx->width * 4));
        }
    }

    return result;
}

QImage readTiffFileStriped(const Context *ctx)
{
    assert(ctx);
    assert(ctx->tiff);

    quint16 planarConfig = 0;
    if(!TIFFGetFieldDefaulted(ctx->tiff, TIFFTAG_PLANARCONFIG, &planarConfig) || !IsOneOf(planarConfig, PLANARCONFIG_CONTIG, PLANARCONFIG_SEPARATE))
        planarConfig = PLANARCONFIG_CONTIG;

    quint32 rowsperstrip = 0;
    if(!TIFFGetFieldDefaulted(ctx->tiff, TIFFTAG_ROWSPERSTRIP, &rowsperstrip) || rowsperstrip <= 0)
    {
        LOG_WARNING() << LOGGING_CTX << "Can't get TIFFTAG_ROWSPERSTRIP";
        return QImage();
    }
    LOG_DEBUG() << LOGGING_CTX << "TIFFTAG_ROWSPERSTRIP =" << rowsperstrip;

    const qint64 scanlineSize = static_cast<qint64>(TIFFScanlineSize(ctx->tiff));
    const qint64 rasterScanlineSize = static_cast<qint64>(TIFFRasterScanlineSize(ctx->tiff));
    const qint64 rowsperstrip64 = static_cast<qint64>(rowsperstrip);
    const qint64 numberOfStrips = static_cast<qint64>(TIFFNumberOfStrips(ctx->tiff));
    const qint64 stripSize = static_cast<qint64>(TIFFStripSize(ctx->tiff));

    const bool isValidSubsamplingH = IsOneOf(ctx->subsamplinghor, 1, 2, 4);
    const bool isValidSubsamplingV = IsOneOf(ctx->subsamplingver, 1, 2, 4);
    const bool isValidSubsampledYCbCr = ctx->photometric == PHOTOMETRIC_YCBCR && ctx->primarysamplesCount == 3 && isValidSubsamplingH && isValidSubsamplingV && (ctx->subsamplinghor > 1 || ctx->subsamplingver > 1);

    if(planarConfig == PLANARCONFIG_CONTIG)
    {
        if(isValidSubsampledYCbCr)
            return readTiffFileStripedContigSubsampled(ctx, numberOfStrips, stripSize, rowsperstrip64, scanlineSize, rasterScanlineSize);
        else
            return readTiffFileStripedContig(ctx, numberOfStrips, stripSize, rowsperstrip64, scanlineSize, rasterScanlineSize);
    }
    else if(planarConfig == PLANARCONFIG_SEPARATE)
    {
        if(isValidSubsampledYCbCr)
            return readTiffFileStripedSeparateSubsampled(ctx, numberOfStrips, stripSize, rowsperstrip64, scanlineSize, rasterScanlineSize);
        else
            return readTiffFileStripedSeparate(ctx, numberOfStrips, stripSize, rowsperstrip64, scanlineSize, rasterScanlineSize);
    }

    assert(false);
    return QImage();
}

// ====================================================================================================

PayloadWithMetaData<QImage> readTiffFile(const QString &filename)
{
    QFile inFile(filename);
    if(!inFile.open(QIODevice::ReadOnly))
    {
        LOG_WARNING() << LOGGING_CTX << "Can't open" << filename;
        return QImage();
    }

#if defined (TIFFLIB_VERSION) && (TIFFLIB_VERSION >= 20221213) && (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
    TIFFOpenOptions *opts = TIFFOpenOptionsAlloc();
    TIFFOpenOptionsSetErrorHandlerExtR(opts, errorHandlerProc, Q_NULLPTR);
    TIFFOpenOptionsSetWarningHandlerExtR(opts, &warningHandlerProc, Q_NULLPTR);
    TIFF *tiff = TIFFClientOpenExt("DecoderLibTiff", "r", &inFile, readProc, writeProc, seekProc, closeProc, sizeProc, mapProc, unmapProc, opts);
    TIFFOpenOptionsFree(opts);
#else
    TIFF *tiff = TIFFClientOpen("DecoderLibTiff", "r", &inFile, readProc, writeProc, seekProc, closeProc, sizeProc, mapProc, unmapProc);
#endif
    if(!tiff)
    {
        LOG_WARNING() << LOGGING_CTX << "Can't TIFFClientOpen for" << filename;
        return QImage();
    }

    if(!TIFFSetDirectory(tiff, 0))
    {
        LOG_WARNING() << LOGGING_CTX << "Can't TIFFSetDirectory for" << filename;
        TIFFClose(tiff);
        return QImage();
    }

    quint32 width = 0;
    if(!TIFFGetFieldDefaulted(tiff, TIFFTAG_IMAGEWIDTH, &width) || width <= 0)
    {
        LOG_WARNING() << LOGGING_CTX << "Can't get TIFFTAG_IMAGEWIDTH for" << filename;
        TIFFClose(tiff);
        return QImage();
    }

    quint32 height = 0;
    if(!TIFFGetFieldDefaulted(tiff, TIFFTAG_IMAGELENGTH, &height) || height <= 0)
    {
        LOG_WARNING() << LOGGING_CTX << "Can't get TIFFTAG_IMAGELENGTH for" << filename;
        TIFFClose(tiff);
        return QImage();
    }

    quint16 compression = 0;
    TIFFGetFieldDefaulted(tiff, TIFFTAG_COMPRESSION, &compression);

    /// @todo LibTIFF-4.6.0: Looks like samples layout for subsampled YCbCr data
    /// for JPEG compression is differ from samples layout for other compression
    /// types. So we will enforce built-in YCbCr to RGB converter. Despite tag
    /// name, non-YCbCr colorspaces like CMYK or RGB are not affected
    if(compression == COMPRESSION_JPEG)
        TIFFSetField(tiff, TIFFTAG_JPEGCOLORMODE, JPEGCOLORMODE_RGB);

    quint16 samplesPerPixel = 0;
    if(!TIFFGetFieldDefaulted(tiff, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel) || samplesPerPixel <= 0)
        samplesPerPixel = 1;

    int alphaIndex = -1;
    quint16 extrasamplesCount = 0;
    const quint16 *extrasamples = Q_NULLPTR;
    if(TIFFGetFieldDefaulted(tiff, TIFFTAG_EXTRASAMPLES, &extrasamplesCount, &extrasamples))
    {
        for(quint16 i = 0; i < extrasamplesCount && alphaIndex < 0; ++i)
        {
            if(extrasamples[i] == EXTRASAMPLE_ASSOCALPHA)
                alphaIndex = static_cast<int>(i);
        }
        for(quint16 i = 0; i < extrasamplesCount && alphaIndex < 0; ++i)
        {
            if(extrasamples[i] == EXTRASAMPLE_UNASSALPHA)
                alphaIndex = static_cast<int>(i);
        }
    }
    const quint16 primarysamplesCount = samplesPerPixel > extrasamplesCount ? samplesPerPixel - extrasamplesCount : 0;

    quint16 photometric = 0;
    if(!TIFFGetFieldDefaulted(tiff, TIFFTAG_PHOTOMETRIC, &photometric))
    {
        LOG_WARNING() << LOGGING_CTX << "Can't get TIFFTAG_PHOTOMETRIC for" << filename;
        if(primarysamplesCount == 1)
        {
            photometric = PHOTOMETRIC_MINISWHITE;
            LOG_WARNING() << LOGGING_CTX << "Assuming PHOTOMETRIC_MINISWHITE";
        }
        else if(primarysamplesCount == 3)
        {
            photometric = PHOTOMETRIC_RGB;
            LOG_WARNING() << LOGGING_CTX << "Assuming PHOTOMETRIC_RGB";
        }
        else
        {
            TIFFClose(tiff);
            return QImage();
        }
    }

    /// @note Enable converting PHOTOMETRIC_LOGL and PHOTOMETRIC_LOGLUV to 8 bit
    /// RGB and grayscale values respective
    if(IsOneOf(photometric, PHOTOMETRIC_LOGL, PHOTOMETRIC_LOGLUV))
        TIFFSetField(tiff, TIFFTAG_SGILOGDATAFMT, SGILOGDATAFMT_8BIT);

    quint16 orientation = 0;
    if(!TIFFGetFieldDefaulted(tiff, TIFFTAG_ORIENTATION, &orientation) || orientation <= 0)
        orientation = ORIENTATION_TOPLEFT;

    quint16 bitsPerSample = 0;
    if(!TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample) || bitsPerSample <= 0)
        bitsPerSample = 1;

    quint16 sampleFormat = 0;
    if(!TIFFGetFieldDefaulted(tiff, TIFFTAG_SAMPLEFORMAT, &sampleFormat) || sampleFormat <= 0)
        sampleFormat = SAMPLEFORMAT_VOID;

    quint16 inkSet = 0;
    if(photometric == PHOTOMETRIC_SEPARATED)
    {
        if(!TIFFGetFieldDefaulted(tiff, TIFFTAG_INKSET, &inkSet) || inkSet <= 0)
            inkSet = INKSET_CMYK;
    }

    quint16 planarConfig = 0;
    TIFFGetField(tiff, TIFFTAG_PLANARCONFIG, &planarConfig);

    const float *ycbcrcoeffs = Q_NULLPTR;
    TIFFGetFieldDefaulted(tiff, TIFFTAG_YCBCRCOEFFICIENTS, &ycbcrcoeffs);

    quint16 subsamplinghor = 0, subsamplingver = 0;
    TIFFGetFieldDefaulted(tiff, TIFFTAG_YCBCRSUBSAMPLING, &subsamplinghor, &subsamplingver);

    quint16 ycbcrpositioning = 0;
    TIFFGetFieldDefaulted(tiff, TIFFTAG_YCBCRPOSITIONING, &ycbcrpositioning);

    const quint16 *redTable = Q_NULLPTR;
    const quint16 *greenTable = Q_NULLPTR;
    const quint16 *blueTable = Q_NULLPTR;
    TIFFGetFieldDefaulted(tiff, TIFFTAG_COLORMAP, &redTable, &greenTable, &blueTable);

    QScopedPointer<ICCProfile> iccProfile(readICCProfile(tiff));
    if((!iccProfile || !iccProfile->isValid()) && photometric == PHOTOMETRIC_SEPARATED && inkSet == INKSET_CMYK)
        iccProfile.reset(new ICCProfile(ICCProfile::defaultCmykProfileData()));

    LOG_DEBUG() << LOGGING_CTX << "TIFFTAG_IMAGEWIDTH =" << width;
    LOG_DEBUG() << LOGGING_CTX << "TIFFTAG_IMAGELENGTH =" << height;
    LOG_DEBUG() << LOGGING_CTX << "TIFFTAG_PHOTOMETRIC =" << photometricToString(photometric).toLatin1().data();
    LOG_DEBUG() << LOGGING_CTX << "TIFFTAG_INKSET =" << inkSetToString(inkSet).toLatin1().data();
    LOG_DEBUG() << LOGGING_CTX << "TIFFTAG_ORIENTATION =" << orientationToString(orientation).toLatin1().data();
    LOG_DEBUG() << LOGGING_CTX << "TIFFTAG_BITSPERSAMPLE =" << bitsPerSample;
    LOG_DEBUG() << LOGGING_CTX << "TIFFTAG_SAMPLESPERPIXEL =" << samplesPerPixel;
    LOG_DEBUG() << LOGGING_CTX << "TIFFTAG_SAMPLEFORMAT =" << sampleFormatToString(sampleFormat).toLatin1().data();
    LOG_DEBUG() << LOGGING_CTX << "TIFFTAG_EXTRASAMPLES =" << extrasamplesToString(extrasamplesCount, extrasamples).toLatin1().data();
    LOG_DEBUG() << LOGGING_CTX << "TIFFTAG_PLANARCONFIG =" << planarConfigToString(planarConfig).toLatin1().data();
    LOG_DEBUG() << LOGGING_CTX << "TIFFTAG_COMPRESSION =" << compressionToString(compression).toLatin1().data();
    LOG_DEBUG() << LOGGING_CTX << "TIFFTAG_YCBCRSUBSAMPLING =" << subsamplinghor << subsamplingver;
    LOG_DEBUG() << LOGGING_CTX << "TIFFTAG_YCBCRPOSITIONING =" << ycbcrpositioningToString(ycbcrpositioning).toLatin1().data();

    const bool isSupportedInt = IsOneOf(sampleFormat, SAMPLEFORMAT_UINT, SAMPLEFORMAT_VOID, SAMPLEFORMAT_INT) && bitsPerSample <= 64;
    const bool isSupportedFp = sampleFormat == SAMPLEFORMAT_IEEEFP && IsOneOf(bitsPerSample, 16, 24, 32, 64);
    bool isReadingSupported = false;
    switch(photometric)
    {
    case PHOTOMETRIC_MINISWHITE:
    case PHOTOMETRIC_MINISBLACK:
    case PHOTOMETRIC_LOGL:
        isReadingSupported = (isSupportedInt || isSupportedFp) && primarysamplesCount >= 1;
        break;
    case PHOTOMETRIC_RGB:
    case PHOTOMETRIC_YCBCR:
    case PHOTOMETRIC_LOGLUV:
        isReadingSupported = (isSupportedInt || isSupportedFp) && primarysamplesCount >= 3;
        break;
    case PHOTOMETRIC_SEPARATED:
        isReadingSupported = (isSupportedInt || isSupportedFp) && primarysamplesCount >= 4;
        break;
    case PHOTOMETRIC_CIELAB:
    case PHOTOMETRIC_ICCLAB:
    case PHOTOMETRIC_ITULAB:
        isReadingSupported = (isSupportedInt || isSupportedFp) && primarysamplesCount >= 1;
        break;
    case PHOTOMETRIC_PALETTE:
        isReadingSupported = redTable && greenTable && blueTable && bitsPerSample <= 16 && primarysamplesCount >= 1;
        break;
    // case PHOTOMETRIC_MASK:
    // case PHOTOMETRIC_CFA:
    default:
        break;
    }

    QImage result;
    if(isReadingSupported)
    {
        Context ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.tiff = tiff;
        ctx.width = static_cast<qint64>(width);
        ctx.height = static_cast<qint64>(height);
        ctx.compression = compression;
        ctx.photometric = compression == COMPRESSION_JPEG && photometric == PHOTOMETRIC_YCBCR ? PHOTOMETRIC_RGB : photometric;
        ctx.samplesPerPixel = static_cast<qint64>(samplesPerPixel);
        ctx.bitsPerSample = static_cast<qint64>(bitsPerSample);
        ctx.sampleFormat = sampleFormat;
        ctx.inkSet = inkSet;
        ctx.iccProfile = iccProfile.data();
        ctx.ycbcrcoeffs = ycbcrcoeffs;
        ctx.subsamplinghor = static_cast<qint64>(subsamplinghor);
        ctx.subsamplingver = static_cast<qint64>(subsamplingver);
        ctx.ycbcrpositioning = ycbcrpositioning;
        ctx.extrasamplesCount = static_cast<qint64>(extrasamplesCount);
        ctx.primarysamplesCount = static_cast<qint64>(primarysamplesCount);
        ctx.alphaIndex = alphaIndex;
        ctx.alphaPremultiplied = alphaIndex >= 0 ? extrasamples[alphaIndex] == EXTRASAMPLE_ASSOCALPHA : false;
        ctx.redTable = redTable;
        ctx.greenTable = greenTable;
        ctx.blueTable = blueTable;
        ctx.colorTablesIs16Bit = false;

        if(photometric == PHOTOMETRIC_PALETTE && redTable && greenTable && blueTable)
        {
            /// @note See buildMap and checkcmap in tif_getimage.c
            for(quint64 i = 0, count = (1ull << bitsPerSample); i < count && !ctx.colorTablesIs16Bit; ++i)
                if(redTable[i] >= 256 || greenTable[i] >= 256 || blueTable[i] >= 256)
                    ctx.colorTablesIs16Bit = true;
        }

        if(TIFFIsTiled(tiff))
            result = readTiffFileTiled(&ctx);
        else
            result = readTiffFileStriped(&ctx);
    }

    /// @todo LibTIFF-4.6.0: TIFFRGBA* has bad errors handling and reports OK
    /// sometimes even if no data was decoded. So we will disable it for all
    /// supported formats
    if(result.isNull() && !isReadingSupported)
    {
        TIFFRGBAImage img;
        char emsg[1024];
        if(!TIFFRGBAImageBegin(&img, tiff, 0, emsg))
        {
            LOG_WARNING() << LOGGING_CTX << "Can't TIFFRGBAImageBegin for" << filename;
            LOG_WARNING() << LOGGING_CTX << "Reason:" << emsg;
            TIFFClose(tiff);
            return QImage();
        }

        result = QImage(static_cast<int>(img.width), static_cast<int>(img.height),
#if (USE_RGBA_8888)
                      QImage::Format_RGBA8888);
#else
                      QImage::Format_ARGB32);
#endif
        if(result.isNull())
        {
            LOG_WARNING() << LOGGING_CTX << "Invalid image size";
            TIFFClose(tiff);
            return QImage();
        }

        img.req_orientation = img.orientation;

#if defined (TIFFLIB_VERSION) && (TIFFLIB_VERSION >= 20210416)
        typedef uint32_t TiffImageBitsType;
#else
        typedef uint32 TiffImageBitsType;
#endif

        if(!TIFFRGBAImageGet(&img, reinterpret_cast<TiffImageBitsType*>(result.bits()), img.width, img.height))
        {
            LOG_WARNING() << LOGGING_CTX << "Can't TIFFRGBAImageGet for" << filename;
            TIFFClose(tiff);
            return QImage();
        }

#if (!USE_RGBA_8888)
        QImage_rgbSwap(result);
#endif

#undef USE_RGBA_8888

        TIFFRGBAImageEnd(&img);

        if(iccProfile)
            iccProfile->applyToImage(&result);
    }

    // Some image formats can't be rendered successfully
    if(!IsOneOf(result.format(), QImage::Format_RGB32, QImage::Format_ARGB32))
        QImage_convertTo(result, result.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32);

    ImageMetaData *metaData = ImageMetaData::createMetaData(filename);
    if(!metaData)
        metaData = readExifMetaData(tiff);
    if(!metaData)
        metaData = new ImageMetaData;

    metaData->addCustomOrientation(orientation);
    metaData->applyExifOrientation(&result);

    quint16 resUnit = 0;
    if(!TIFFGetField(tiff, TIFFTAG_RESOLUTIONUNIT, &resUnit))
        resUnit = RESUNIT_INCH;
    float resX = 0.0f, resY = 0.0f;
    if(TIFFGetField(tiff, TIFFTAG_XRESOLUTION, &resX) && TIFFGetField(tiff, TIFFTAG_YRESOLUTION, &resY))
    {
        if(resUnit == RESUNIT_CENTIMETER)
            metaData->addCustomDpi(static_cast<qreal>(resX * 2.54f), static_cast<qreal>(resY * 2.54f));
        else if(resUnit == RESUNIT_INCH)
            metaData->addCustomDpi(static_cast<qreal>(resX), static_cast<qreal>(resY));
    }

    TIFFClose(tiff);

    return PayloadWithMetaData<QImage>(result, metaData);
}

// ====================================================================================================

class DecoderLibTiff : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderLibTiff");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
                << QString::fromLatin1("tif")
                << QString::fromLatin1("tiff");
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
        const PayloadWithMetaData<QImage> readData = readTiffFile(filePath);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createImageItem(readData);
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), readData.metaData()));
    }
};

DecoderAutoRegistrator registrator(new DecoderLibTiff);

// ====================================================================================================

} // namespace
