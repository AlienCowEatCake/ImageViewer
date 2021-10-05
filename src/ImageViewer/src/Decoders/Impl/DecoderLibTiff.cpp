/*
   Copyright (C) 2017-2021 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <tiffio.h>

#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QDebug>

#include "Utils/Global.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"
#include "Internal/PayloadWithMetaData.h"
#include "Internal/Utils/CmsUtils.h"

namespace
{

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

ICCProfile *readICCProfile(TIFF *tiff)
{
    unsigned iccProfileSize = 0;
    void *iccProfileData = Q_NULLPTR;
    if(TIFFGetField(tiff, TIFFTAG_ICCPROFILE, &iccProfileSize, &iccProfileData))
    {
        qDebug() << "Found ICCP metadata (TIFFTAG_ICCPROFILE)";
        return new ICCProfile(QByteArray(reinterpret_cast<const char*>(iccProfileData), static_cast<int>(iccProfileSize)));
    }

    float *whitePoint = Q_NULLPTR, *primaryChromaticities = Q_NULLPTR;
    unsigned short *transferFunctionRed = Q_NULLPTR, *transferFunctionGreen = Q_NULLPTR, *transferFunctionBlue = Q_NULLPTR;
    if(TIFFGetField(tiff, TIFFTAG_WHITEPOINT, &whitePoint) && TIFFGetField(tiff, TIFFTAG_PRIMARYCHROMATICITIES, &primaryChromaticities) &&
       TIFFGetFieldDefaulted(tiff, TIFFTAG_TRANSFERFUNCTION, &transferFunctionRed, &transferFunctionGreen, &transferFunctionBlue))
    {
        qDebug() << "Found ICCP metadata (TIFFTAG_WHITEPOINT + TIFFTAG_PRIMARYCHROMATICITIES + TIFFTAG_TRANSFERFUNCTION)";
        return new ICCProfile(whitePoint, primaryChromaticities, transferFunctionRed, transferFunctionGreen, transferFunctionBlue);
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
        qWarning() << "TIFFFieldDataType for tag (" << tagDescription << ") is not TIFF_IFD8!";
        return;
    }
    quint64 exifOffset = 0;
    if(!TIFFGetField(tiff, tag, &exifOffset))
        return;
    if(!TIFFReadEXIFDirectory(tiff, exifOffset))
        return;
    qDebug() << "Found EXIF metadata (" << tagDescription << ")";
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

IImageMetaData *readExifMetaData(TIFF *tiff)
{
    ImageMetaData *metaData = Q_NULLPTR;
    readTiffTagToMetaData(tiff, metaData, TIFFTAG_EXIFIFD, QString::fromLatin1("TIFFTAG_EXIFIFD"));
    readTiffTagToMetaData(tiff, metaData, TIFFTAG_GPSIFD, QString::fromLatin1("TIFFTAG_GPSIFD"));
    readTiffTagToMetaData(tiff, metaData, TIFFTAG_INTEROPERABILITYIFD, QString::fromLatin1("TIFFTAG_INTEROPERABILITYIFD"));
    return metaData;
}

PayloadWithMetaData<QImage> readTiffFile(const QString &filename)
{
    QFile inFile(filename);
    if(!inFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "Can't open" << filename;
        return QImage();
    }

    TIFF *tiff = TIFFClientOpen("foo", "r", &inFile, readProc, writeProc, seekProc, closeProc, sizeProc, mapProc, unmapProc);
    if(!tiff)
    {
        qWarning() << "Can't TIFFClientOpen for" << filename;
        return QImage();
    }

    if(!TIFFSetDirectory(tiff, 0))
    {
        qWarning() << "Can't TIFFSetDirectory for" << filename;
        TIFFClose(tiff);
        return QImage();
    }

    ICCProfile *iccProfile = readICCProfile(tiff);

    TIFFRGBAImage img;
    char emsg[1024];
    if(!TIFFRGBAImageBegin(&img, tiff, 0, emsg))
    {
        qWarning() << "Can't TIFFRGBAImageBegin for" << filename;
        qWarning() << "Reason:" << emsg;
        TIFFClose(tiff);
        if(iccProfile)
            delete iccProfile;
        return QImage();
    }

#define USE_RGBA_8888 (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0) && Q_BYTE_ORDER == Q_LITTLE_ENDIAN)

    QImage result(static_cast<int>(img.width), static_cast<int>(img.height),
#if (USE_RGBA_8888)
                  QImage::Format_RGBA8888);
#else
                  QImage::Format_ARGB32);
#endif
    if(result.isNull())
    {
        qWarning() << "Invalid image size";
        TIFFClose(tiff);
        if(iccProfile)
            delete iccProfile;
        return QImage();
    }

    img.req_orientation = ORIENTATION_TOPLEFT;

#if defined (TIFFLIB_VERSION) && (TIFFLIB_VERSION >= 20210416)
    typedef uint32_t TiffImageBitsType;
#else
    typedef uint32 TiffImageBitsType;
#endif

    if(!TIFFRGBAImageGet(&img, reinterpret_cast<TiffImageBitsType*>(result.bits()), img.width, img.height))
    {
        qWarning() << "Can't TIFFRGBAImageGet for" << filename;
        TIFFClose(tiff);
        if(iccProfile)
            delete iccProfile;
        return QImage();
    }

#if (!USE_RGBA_8888)
    result = result.rgbSwapped();
#endif

#undef USE_RGBA_8888

    if(iccProfile)
    {
        iccProfile->applyToImage(&result);
        delete iccProfile;
        iccProfile = Q_NULLPTR;
    }

    IImageMetaData *metaData = ImageMetaData::createMetaData(filename);
    if(!metaData)
        metaData = readExifMetaData(tiff);

    TIFFRGBAImageEnd(&img);
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
