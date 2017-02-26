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

#include "DecoderQtPixmap.h"

#include <QImageReader>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QFileInfo>
#include "Utils/ScopedPointer.h"

#include "DecoderAutoRegistrator.h"

namespace {

const int DECODER_QT_PIXMAP_PRIORITY = 100;

DecoderAutoRegistrator registrator(new DecoderQtPixmap, true);

} // namespace

QString DecoderQtPixmap::name() const
{
    return QString::fromLatin1("DecoderQtPixmap");
}

QList<DecoderFormatInfo> DecoderQtPixmap::supportedFormats() const
{
    const QList<QByteArray> readerFormats = QImageReader::supportedImageFormats();
    QList<DecoderFormatInfo> result;
    for(QList<QByteArray>::ConstIterator it = readerFormats.constBegin(); it != readerFormats.constEnd(); ++it)
    {
        DecoderFormatInfo info;
        info.decoderPriority = DECODER_QT_PIXMAP_PRIORITY;
        info.format = QString::fromLatin1(*it).toLower();
        result.append(info);
    }
    return result;
}

QGraphicsItem *DecoderQtPixmap::loadImage(const QString &filename)
{
    const QFileInfo fileInfo(filename);
    if(!fileInfo.exists() || !fileInfo.isReadable())
        return NULL;
    QPixmap pixmap(filename);
    if(pixmap.isNull())
        return NULL;
    return new QGraphicsPixmapItem(pixmap);
}
