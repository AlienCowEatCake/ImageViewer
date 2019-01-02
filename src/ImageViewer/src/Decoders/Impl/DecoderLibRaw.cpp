/*
   Copyright (C) 2018-2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <libraw/libraw_datastream.h>
#include <libraw/libraw.h>

#include <QtGlobal>
#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QDebug>

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"

namespace
{

// ====================================================================================================

const qreal MAX_IMAGE_DIMENSION = 32767;

QImage readRawFile(const QString &filePath)
{
    try
    {
        const QByteArray filePathLocal8Bit = filePath.toLocal8Bit();
        QScopedPointer<LibRaw_abstract_datastream> dataStream;
        QByteArray inBuffer;
        if(QString::fromLocal8Bit(filePathLocal8Bit) != filePath || filePath.startsWith(QChar::fromLatin1(':')))
        {
            QFile inFile(filePath);
            if(!inFile.open(QIODevice::ReadOnly))
            {
                qWarning() << "Can't open" << filePath;
                return QImage();
            }
            inBuffer = inFile.readAll();
            dataStream.reset(new LibRaw_buffer_datastream(inBuffer.data(), static_cast<size_t>(inBuffer.size())));
        }
        else
        {
            dataStream.reset(new LibRaw_file_datastream(filePathLocal8Bit.constData()));
        }

        LibRaw rawProcessor;
        if(rawProcessor.open_datastream(dataStream.data()) != LIBRAW_SUCCESS)
        {
            qWarning() << "Can't open datastream for" << filePath;
            return QImage();
        }

        rawProcessor.imgdata.params.output_bps = 8;
        const libraw_data_t &imgdata = rawProcessor.imgdata;
        libraw_processed_image_t *output;
        int errorCode = 0;
        if(imgdata.sizes.width > MAX_IMAGE_DIMENSION || imgdata.sizes.height > MAX_IMAGE_DIMENSION)
        {
            qDebug() << "Image has too large size: using thumbnail";
            rawProcessor.unpack_thumb();
            output = rawProcessor.dcraw_make_mem_thumb(&errorCode);
        }
        else
        {
            qDebug() << "Image has acceptable size: decoding raw data";
            rawProcessor.unpack();
            rawProcessor.dcraw_process();
            output = rawProcessor.dcraw_make_mem_image(&errorCode);
        }
        if(!output)
        {
            qWarning() << "Error:" << libraw_strerror(errorCode);
            return QImage();
        }

        QImage image;
        if(output->type == LIBRAW_IMAGE_JPEG)
        {
            image.loadFromData(output->data, static_cast<int>(output->data_size), "JPG");
            if(image.isNull())
            {
                qWarning() << "Invalid image size";
                rawProcessor.dcraw_clear_mem(output);
                return QImage();
            }

            if(imgdata.sizes.flip != 0)
            {
                int angle = 0;
                switch(imgdata.sizes.flip)
                {
                case 3:
                    angle = 180;
                    break;
                case 5:
                    angle = -90;
                    break;
                case 6:
                    angle = 90;
                    break;
                default:
                    break;
                }
                if(angle != 0)
                {
                    QTransform rotation;
                    rotation.rotate(angle);
                    image = image.transformed(rotation);
                }
            }
        }
        else if(output->type == LIBRAW_IMAGE_BITMAP)
        {
            if(output->bits != 8)
            {
                qWarning() << "Error: Output BPS must be equals to 8!";
                assert(output->bits == 8);
                rawProcessor.dcraw_clear_mem(output);
                return QImage();
            }

            image = QImage(output->width, output->height, QImage::Format_RGB32);
            if(image.isNull())
            {
                qWarning() << "Invalid image size";
                rawProcessor.dcraw_clear_mem(output);
                return QImage();
            }

            if(output->colors != 1 && output->colors != 3)
                qWarning() << "Error: Unknown output->colors =" << output->colors;

            for(int i = 0; i < output->height; i++)
            {
                QRgb *line = reinterpret_cast<QRgb*>(image.scanLine(i));
                for(int j = 0; j < output->width; j++)
                {
                    unsigned char *source = output->data + (i * output->width + j) * output->colors;
                    switch(output->colors)
                    {
                    case 3:
                        line[j] = qRgb(source[0], source[1], source[2]);
                        break;
                    default:
                        line[j] = qRgb(source[0], source[0], source[0]);
                        break;
                    }
                }
            }
        }
        else
        {
            qWarning() << "Error: Unknown output->type =" << output->type;
        }

        rawProcessor.dcraw_clear_mem(output);
        return image;
    }
    catch(...)
    {
        qWarning() << "Exception";
    }
    return QImage();
}

// ====================================================================================================

class DecoderLibRaw : public IDecoder
{
public:
    QString name() const
    {
        return QString::fromLatin1("DecoderLibRaw");
    }

    QStringList supportedFormats() const
    {
        return QStringList()
                << QString::fromLatin1("raw")
                << QString::fromLatin1("crw")
                << QString::fromLatin1("cr2")
                << QString::fromLatin1("arw")
                << QString::fromLatin1("nef")
                << QString::fromLatin1("raf")
                << QString::fromLatin1("dng");
    }

    QStringList advancedFormats() const
    {
        return QStringList();
    }

    bool isAvailable() const
    {
        return true;
    }

    QSharedPointer<IImageData> loadImage(const QString &filePath)
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return QSharedPointer<IImageData>();
        return QSharedPointer<IImageData>(new ImageData(GraphicsItemsFactory::instance().createImageItem(readRawFile(filePath)), filePath, name()));
    }
};

DecoderAutoRegistrator registrator(new DecoderLibRaw);

// ====================================================================================================

} // namespace
