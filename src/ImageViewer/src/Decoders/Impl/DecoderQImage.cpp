/*
   Copyright (C) 2017-2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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
#include <QDebug>

#include "Utils/Global.h"
#include "Utils/ScopedPointer.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"

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
#endif
        imageReader.setBackgroundColor(Qt::transparent);
        imageReader.setQuality(100);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
        imageReader.setAutoTransform(true);
#endif
        QImage image = imageReader.read();
        if(image.isNull())
        {
            qDebug() << imageReader.errorString();
            return QSharedPointer<IImageData>();
        }

        ImageMetaData *metaData = ImageMetaData::createMetaData(filePath);
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
