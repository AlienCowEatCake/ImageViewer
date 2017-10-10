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

#include <QFileInfo>
#include <QColor>
#include <QDebug>

#include "QtImageFormatsImageReader.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"

namespace {

class DecoderQtImageFormatsImage : public IDecoder
{
public:
    QString name() const
    {
        return QString::fromLatin1("DecoderQtImageFormatsImage");
    }

    QStringList supportedFormats() const
    {
        const QList<QByteArray> readerFormats = QtImageFormatsImageReader::supportedImageFormats();
        QStringList result;
        for(QList<QByteArray>::ConstIterator it = readerFormats.constBegin(); it != readerFormats.constEnd(); ++it)
            result.append(QString::fromLatin1(*it).toLower());
        return result;
    }

    QStringList advancedFormats() const
    {
        return QStringList();
    }

    QGraphicsItem *loadImage(const QString &filePath)
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return NULL;
        QtImageFormatsImageReader imageReader(filePath);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 6, 0))
        imageReader.setDecideFormatFromContent(true);
#endif
        imageReader.setBackgroundColor(Qt::transparent);
        imageReader.setQuality(100);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
        imageReader.setAutoTransform(true);
#endif
        QImage image;
        image = imageReader.read();
        if(image.isNull())
        {
            qDebug() << imageReader.errorString();
            return NULL;
        }
        return GraphicsItemsFactory::instance().createImageItem(image);
    }
};

DecoderAutoRegistrator registrator(new DecoderQtImageFormatsImage, true);

} // namespace
