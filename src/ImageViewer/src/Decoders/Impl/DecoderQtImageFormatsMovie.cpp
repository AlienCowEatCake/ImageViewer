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

#include <QGraphicsProxyWidget>
#include <QFileInfo>

#include "QtImageFormatsMovie.h"

#include "Utils/Global.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"
#include "Internal/Animation/MovieAnimationProvider.h"

namespace {

class DecoderQtImageFormatsMovie : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderQtImageFormatsMovie");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        const QList<QByteArray> readerFormats = QtImageFormatsMovie::supportedFormats();
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
        QtImageFormatsMovie *movie = new QtImageFormatsMovie(filePath);
        if(!movie->isValid() || movie->frameCount() == 1)
        {
            movie->deleteLater();
            return QSharedPointer<IImageData>();
        }
        IAnimationProvider *provider = new MovieAnimationProvider<QtImageFormatsMovie>(movie);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createAnimatedItem(provider);
        IImageMetaData *metaData = ImageMetaData::createMetaData(filePath);
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), metaData));

    }
};

DecoderAutoRegistrator registrator(new DecoderQtImageFormatsMovie);

} // namespace
