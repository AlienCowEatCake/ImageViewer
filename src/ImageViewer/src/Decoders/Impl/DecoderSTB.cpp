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

#include <cassert>

#include <QGraphicsPixmapItem>
#include <QImage>
#include <QPixmap>
#include <QFileInfo>
#include <QDebug>

#include "stb_image.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/Utils/ExifUtils.h"

namespace {

class DecoderSTB : public IDecoder
{
public:
    QString name() const
    {
        return QString::fromLatin1("DecoderSTB");
    }

    QStringList supportedFormats() const
    {
        return QStringList()
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

        int x, y, n;
        unsigned char *data = ::stbi_load(filePath.toLocal8Bit().data(), &x, &y, &n, 0);
        if(!data)
        {
            qDebug() << ::stbi_failure_reason();
            return NULL;
        }

        QImage image(x, y, QImage::Format_ARGB32);
        for(int i = 0; i < y; i++)
        {
            QRgb *line = reinterpret_cast<QRgb*>(image.scanLine(i));
            for(int j = 0; j < x; j++)
            {
                unsigned char *source = data + (i * x + j) * n;
                switch(n)
                {
                case 1: // grey
                    line[j] = qRgb(source[0], source[0], source[0]);
                    break;
                case 2: // grey, alpha
                    line[j] = qRgba(source[0], source[0], source[0], source[1]);
                    break;
                case 3: // red, green, blue
                    line[j] = qRgb(source[0], source[1], source[2]);
                    break;
                case 4: // red, green, blue, alpha
                    line[j] = qRgba(source[0], source[1], source[2], source[3]);
                    break;
                default:
                    assert(false);
                    image = QImage();
                    break;
                }
            }
        }
        ::stbi_image_free(data);

        if(image.isNull())
            return NULL;

        ExifUtils::ApplyExifOrientation(&image, ExifUtils::GetExifOrientation(filePath));

        return new QGraphicsPixmapItem(QPixmap::fromImage(image));
    }
};

DecoderAutoRegistrator registrator(new DecoderSTB, true);

} // namespace
