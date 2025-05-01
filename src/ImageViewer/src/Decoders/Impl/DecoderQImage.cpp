/*
   Copyright (C) 2017-2025 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <set>

#include <QImageReader>
#include <QFileInfo>
#include <QColor>
#include <QImage>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#include <QColorSpace>
#endif

#include "Utils/Global.h"
#include "Utils/IsOneOf.h"
#include "Utils/Logging.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"
#include "Internal/Utils/CmsUtils.h"

namespace {

class DecoderQImage : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderQImage");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        // https://doc.qt.io/archives/qtextended4.4/qimagereader.html#supportedImageFormats
        const QStringList defaultReaderFormats = QStringList()
                << QString::fromLatin1("bmp")
                << QString::fromLatin1("jpg")
                << QString::fromLatin1("jpeg")
//                << QString::fromLatin1("mng")
                << QString::fromLatin1("png")
                << QString::fromLatin1("pbm")
                << QString::fromLatin1("pgm")
                << QString::fromLatin1("ppm")
//                << QString::fromLatin1("tiff")
                << QString::fromLatin1("xbm")
                << QString::fromLatin1("xpm");
        const QList<QByteArray> readerFormats = QImageReader::supportedImageFormats();
        std::set<QString> allReaderFormats;
        for(QList<QByteArray>::ConstIterator it = readerFormats.constBegin(); it != readerFormats.constEnd(); ++it)
            allReaderFormats.insert(QString::fromLatin1(*it).toLower());
        for(QStringList::ConstIterator it = defaultReaderFormats.constBegin(); it != defaultReaderFormats.constEnd(); ++it)
            allReaderFormats.insert(it->toLower());
        QStringList result;
        for(std::set<QString>::const_iterator it = allReaderFormats.begin(); it != allReaderFormats.end(); ++it)
            result.append(*it);
        return result;
    }

    QStringList advancedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
                << QString::fromLatin1("heic"); // iPhone 7 Plus, iPhone 8 Plus, iPhone XS Max, etc.
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
        QImageReader imageReader(filePath);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 6, 0))
        imageReader.setDecideFormatFromContent(true);
        if(!imageReader.canRead())
            imageReader.setDecideFormatFromContent(false);
#endif
        imageReader.setBackgroundColor(Qt::transparent);
        imageReader.setQuality(100);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
        imageReader.setAutoTransform(true);
#endif
        QImage image = imageReader.read();
        if(image.isNull())
        {
            LOG_WARNING() << LOGGING_CTX << imageReader.errorString();
            return QSharedPointer<IImageData>();
        }

        ImageMetaData *metaData = ImageMetaData::createMetaData(filePath);
        if(!metaData)
            metaData = ImageMetaData::createQImageMetaData(image);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        /// @note Supress '@2x' logic: https://github.com/qt/qtbase/blob/v5.9.8/src/gui/image/qimagereader.cpp#L1364
        image.setDevicePixelRatio(1);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(6, 8, 0))
        if(image.colorSpace().isValid())
            image.convertToColorSpace(QColorSpace::SRgb, image.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32);
        if(image.format() == QImage::Format_CMYK8888)
            ICCProfile(ICCProfile::defaultCmykProfileData()).applyToImage(&image);
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        if(image.colorSpace().isValid())
            image.convertToColorSpace(QColorSpace::SRgb);
#endif

        // Some image formats can't be rendered successfully
        if(!IsOneOf(image.format(), QImage::Format_RGB32, QImage::Format_ARGB32))
            QImage_convertTo(image, image.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32);

#if (QT_VERSION < QT_VERSION_CHECK(5, 5, 0))
        if(metaData)
            metaData->applyExifOrientation(&image);
#endif

        QGraphicsItem *item = GraphicsItemsFactory::instance().createImageItem(image);
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), metaData));
    }
};

DecoderAutoRegistrator registrator(new DecoderQImage, true);

} // namespace
