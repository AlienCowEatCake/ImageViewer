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

#include <set>

#include <QImageReader>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QFileInfo>
#include <QDebug>

#if defined (HAS_THIRDPARTY_QTEXTENDED)
#include "qexifimageheader.h"
#endif

#include "DecoderAutoRegistrator.h"

namespace {

const int DECODER_QT_PIXMAP_PRIORITY = 100;

DecoderAutoRegistrator registrator(new DecoderQtPixmap, true);

#if defined (HAS_THIRDPARTY_QTEXTENDED)

void applyExifOrientation(QImage *image, quint16 exifOrientation)
{
    // This is not an optimized implementation, but easiest to maintain
    QTransform transform;

    switch (exifOrientation) {
        case 1: // normal
            break;
        case 2: // mirror horizontal
            *image = image->mirrored(true, false);
            break;
        case 3: // rotate 180
            transform.rotate(180);
            *image = image->transformed(transform);
            break;
        case 4: // mirror vertical
            *image = image->mirrored(false, true);
            break;
        case 5: // mirror horizontal and rotate 270 CCW
            *image = image->mirrored(true, false);
            transform.rotate(270);
            *image = image->transformed(transform);
            break;
        case 6: // rotate 90 CW
            transform.rotate(90);
            *image = image->transformed(transform);
            break;
        case 7: // mirror horizontal and rotate 90 CW
            *image = image->mirrored(true, false);
            transform.rotate(90);
            *image = image->transformed(transform);
            break;
        case 8: // rotate 270 CW
            transform.rotate(-90);
            *image = image->transformed(transform);
            break;
        default:
            qWarning("This should never happen");
    }
    exifOrientation = 1;
}

#endif

} // namespace

QString DecoderQtPixmap::name() const
{
    return QString::fromLatin1("DecoderQtPixmap");
}

QList<DecoderFormatInfo> DecoderQtPixmap::supportedFormats() const
{
    // https://doc.qt.io/archives/qtextended4.4/qimagereader.html#supportedImageFormats
    const QStringList defaultReaderFormats = QStringList()
            << QString::fromLatin1("bmp")
            << QString::fromLatin1("jpg")
            << QString::fromLatin1("jpeg")
//            << QString::fromLatin1("mng")
            << QString::fromLatin1("png")
            << QString::fromLatin1("pbm")
            << QString::fromLatin1("pgm")
            << QString::fromLatin1("ppm")
//            << QString::fromLatin1("tiff")
            << QString::fromLatin1("xbm")
            << QString::fromLatin1("xpm");
    const QList<QByteArray> readerFormats = QImageReader::supportedImageFormats();
    std::set<QString> allReaderFormats;
    for(QList<QByteArray>::ConstIterator it = readerFormats.constBegin(); it != readerFormats.constEnd(); ++it)
        allReaderFormats.insert(QString::fromLatin1(*it).toLower());
    for(QStringList::ConstIterator it = defaultReaderFormats.constBegin(); it != defaultReaderFormats.constEnd(); ++it)
        allReaderFormats.insert(it->toLower());
    QList<DecoderFormatInfo> result;
    for(std::set<QString>::const_iterator it = allReaderFormats.begin(); it != allReaderFormats.end(); ++it)
    {
        DecoderFormatInfo info;
        info.decoderPriority = DECODER_QT_PIXMAP_PRIORITY;
        info.format = *it;
        result.append(info);
    }
    return result;
}

QGraphicsItem *DecoderQtPixmap::loadImage(const QString &filename)
{
    const QFileInfo fileInfo(filename);
    if(!fileInfo.exists() || !fileInfo.isReadable())
        return NULL;
    QPixmap pixmap;
    bool status = false;

#if defined (HAS_THIRDPARTY_QTEXTENDED)
    QExifImageHeader exifHeader;
    status = exifHeader.loadFromJpeg(filename) && exifHeader.contains(QExifImageHeader::Orientation);
    if(status)
    {
        quint16 orientation = exifHeader.value(QExifImageHeader::Orientation).toShort();
        if((status = (orientation >= 2 && orientation <= 8)))
        {
            QImage image(filename);
            if((status = !image.isNull()))
            {
                applyExifOrientation(&image, exifHeader.value(QExifImageHeader::Orientation).toShort());
                pixmap = QPixmap::fromImage(image);
            }
        }
    }
#endif

    if(!status)
        pixmap.load(filename);
    if(pixmap.isNull())
        return NULL;
    return new QGraphicsPixmapItem(pixmap);
}
