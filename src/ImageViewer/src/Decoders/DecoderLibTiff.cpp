/*
   Copyright (C) 2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QFile>
#include <QByteArray>
#include <QDebug>

#include "IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/CmsUtils.h"

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
    void *iccProfileData = NULL;
    if(TIFFGetField(tiff, TIFFTAG_ICCPROFILE, &iccProfileSize, &iccProfileData))
    {
        qDebug() << "Found ICCP metadata (TIFFTAG_ICCPROFILE)";
        return new ICCProfile(QByteArray(reinterpret_cast<const char*>(iccProfileData), static_cast<int>(iccProfileSize)));
    }

    float *whitePoint = NULL, *primaryChromaticities = NULL;
    unsigned short *transferFunctionRed = NULL, *transferFunctionGreen = NULL, *transferFunctionBlue = NULL;
    if(TIFFGetField(tiff, TIFFTAG_WHITEPOINT, &whitePoint) && TIFFGetField(tiff, TIFFTAG_PRIMARYCHROMATICITIES, &primaryChromaticities) &&
       TIFFGetFieldDefaulted(tiff, TIFFTAG_TRANSFERFUNCTION, &transferFunctionRed, &transferFunctionGreen, &transferFunctionBlue))
    {
        qDebug() << "Found ICCP metadata (TIFFTAG_WHITEPOINT + TIFFTAG_PRIMARYCHROMATICITIES + TIFFTAG_TRANSFERFUNCTION)";
        return new ICCProfile(whitePoint, primaryChromaticities, transferFunctionRed, transferFunctionGreen, transferFunctionBlue);
    }

    return NULL;
}

QImage readTiffFile(const QString &filename)
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

    QImage result(static_cast<int>(img.width), static_cast<int>(img.height),
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
                  QImage::Format_RGBA8888);
#else
                  QImage::Format_ARGB32);
#endif
    img.req_orientation = ORIENTATION_TOPLEFT;

    if(!TIFFRGBAImageGet(&img, reinterpret_cast<uint32*>(result.bits()), img.width, img.height))
    {
        qWarning() << "Can't TIFFRGBAImageGet for" << filename;
        TIFFClose(tiff);
        if(iccProfile)
            delete iccProfile;
        return QImage();
    }

#if (QT_VERSION < QT_VERSION_CHECK(5, 2, 0))
    QRgb *imageData = reinterpret_cast<QRgb*>(result.bits());
    for(int i = 0; i < result.width() * result.height(); i++)
    {
        uchar *rawPixelData = reinterpret_cast<uchar*>(imageData);
        *(imageData++) = qRgba(rawPixelData[0], rawPixelData[1], rawPixelData[2], rawPixelData[3]);
    }
#endif

    if(iccProfile)
    {
        iccProfile->applyToImage(&result);
        delete iccProfile;
        iccProfile = NULL;
    }

    TIFFRGBAImageEnd(&img);
    TIFFClose(tiff);

    return result;
}

// ====================================================================================================

class DecoderLibTiff : public IDecoder
{
public:
    QString name() const
    {
        return QString::fromLatin1("DecoderLibTiff");
    }

    QStringList supportedFormats() const
    {
        return QStringList()
                << QString::fromLatin1("tif")
                << QString::fromLatin1("tiff");
    }

    QGraphicsItem *loadImage(const QString &filePath)
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return NULL;

        const QImage image = readTiffFile(filePath);
        if(image.isNull())
            return NULL;

        return new QGraphicsPixmapItem(QPixmap::fromImage(image));
    }
};

DecoderAutoRegistrator registrator(new DecoderLibTiff);

// ====================================================================================================

} // namespace
