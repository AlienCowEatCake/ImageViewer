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

#include "DecoderSTB.h"

#include <QGraphicsPixmapItem>
#include <QImage>
#include <QPixmap>
#include <QFileInfo>
#include <QDebug>

#if defined (HAS_THIRDPARTY_STB)
#include "stb_image.h"
#endif

#include "DecoderAutoRegistrator.h"
#include "ExifUtils.h"

#define DECODER_STB_PRIORITY 80

namespace {

DecoderAutoRegistrator registrator(new DecoderSTB, DECODER_STB_PRIORITY);

} // namespace

QString DecoderSTB::name() const
{
    return QString::fromLatin1("DecoderSTB");
}

QList<DecoderFormatInfo> DecoderSTB::supportedFormats() const
{
#if defined (HAS_THIRDPARTY_STB)
    const QStringList stbImageFormats = QStringList()
            << QString::fromLatin1("jpg")
            << QString::fromLatin1("jpeg")
            << QString::fromLatin1("jpe")   /// @todo Check this!
            << QString::fromLatin1("png")
            << QString::fromLatin1("bmp")
            << QString::fromLatin1("dib")   /// @todo Check this!
            << QString::fromLatin1("psd")
            << QString::fromLatin1("tga")
            << QString::fromLatin1("targa") /// @todo Check this!
            << QString::fromLatin1("gif")
            << QString::fromLatin1("hdr")
            << QString::fromLatin1("pic")
            << QString::fromLatin1("pbm")   /// @todo Check this!
            << QString::fromLatin1("ppm")
            << QString::fromLatin1("pgm")
            << QString::fromLatin1("pnm");  /// @todo Check this!
    QList<DecoderFormatInfo> result;
    for(QStringList::ConstIterator it = stbImageFormats.constBegin(); it != stbImageFormats.constEnd(); ++it)
    {
        DecoderFormatInfo info;
        info.decoderPriority = DECODER_STB_PRIORITY;
        info.format = *it;
        result.append(info);
    }
    return result;
#else
    return QList<DecoderFormatInfo>();
#endif
}

QGraphicsItem *DecoderSTB::loadImage(const QString &filename)
{
    const QFileInfo fileInfo(filename);
    if(!fileInfo.exists() || !fileInfo.isReadable())
        return NULL;

#if defined (HAS_THIRDPARTY_STB)
    int x, y, n;
    unsigned char *data = ::stbi_load(filename.toLocal8Bit().data(), &x, &y, &n, 4);
    if(!data)
    {
        qDebug() << ::stbi_failure_reason();
        return NULL;
    }

    std::size_t imageBytesSize = static_cast<std::size_t>(x) * static_cast<std::size_t>(y) * 4;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    QImage image(x, y, QImage::Format_RGBA8888);
    memcpy(image.bits(), data, imageBytesSize);
#else
    QImage image(x, y, QImage::Format_ARGB32);
    uchar *outBits = image.bits();
    uchar *inBits = reinterpret_cast<uchar*>(data);
    for(std::size_t i = 0; i < imageBytesSize; i += 4)
    {
        outBits[0] = inBits[2]; // B
        outBits[1] = inBits[1]; // G
        outBits[2] = inBits[0]; // R
        outBits[3] = inBits[3]; // A
        outBits += 4;
        inBits += 4;
    }
#endif
    stbi_image_free(data);

    if(image.isNull())
        return NULL;

    quint16 orientation = ExifUtils::GetExifOrientation(filename);
    if(orientation != 1)
        ExifUtils::ApplyExifOrientation(&image, orientation);

    return new QGraphicsPixmapItem(QPixmap::fromImage(image));
#else
    return NULL;
#endif
}
