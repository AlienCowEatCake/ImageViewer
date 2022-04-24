/*
   Copyright (C) 2017-2022 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <QFileInfo>
#include <QColor>
#include <QImage>
#include <QDebug>

#include "KImageFormatsImageReader.h"

#include "Utils/Global.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"

namespace {

class DecoderKImageFormatsImage : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderKImageFormatsImage");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        const QList<QByteArray> readerFormats = KImageFormatsImageReader::supportedImageFormats();
        QStringList result;
        for(QList<QByteArray>::ConstIterator it = readerFormats.constBegin(); it != readerFormats.constEnd(); ++it)
            result.append(QString::fromLatin1(*it).toLower());
        return result;
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
        KImageFormatsImageReader imageReader(filePath);
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

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        /// @note Supress '@2x' logic: https://github.com/qt/qtbase/blob/v5.9.8/src/gui/image/qimagereader.cpp#L1364
        image.setDevicePixelRatio(1);
#endif

        ImageMetaData *metaData = ImageMetaData::createMetaData(filePath);
#if (QT_VERSION < QT_VERSION_CHECK(5, 5, 0))
        if(metaData)
            metaData->applyExifOrientation(&image);
#endif

        QGraphicsItem *item = GraphicsItemsFactory::instance().createImageItem(image);
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), metaData));
    }
};

DecoderAutoRegistrator registrator(new DecoderKImageFormatsImage, true);

} // namespace
