/*
   Copyright (C) 2021 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <FLIF/flif.h>
#include <FLIF/flif_dec.h>

#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QDebug>

#include "Utils/Global.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"
#include "Internal/PayloadWithMetaData.h"
#include "Internal/Animation/AbstractAnimationProvider.h"
#include "Internal/Animation/DelayCalculator.h"
#include "Internal/Utils/CmsUtils.h"
#include "Internal/Utils/MappedBuffer.h"

namespace {

class FlifAnimationProvider : public AbstractAnimationProvider
{
    Q_DISABLE_COPY(FlifAnimationProvider)

public:
    FlifAnimationProvider()
    {}

    PayloadWithMetaData<bool> readFlifFile(const QString &filePath)
    {
        const MappedBuffer inBuffer(filePath);
        if(!inBuffer.isValid())
            return false;

        FLIF_DECODER *d = flif_create_decoder();
        if(!flif_decoder_decode_memory(d, inBuffer.dataAs<const void*>(), inBuffer.sizeAs<size_t>()))
        {
            qWarning() << "ERROR: Cannot decode FLIF image";
            flif_destroy_decoder(d);
            return false;
        }

        ImageMetaData *metaData = ImageMetaData::createMetaData(filePath);

        m_numFrames = static_cast<int>(flif_decoder_num_images(d));
        m_numLoops = static_cast<int>(flif_decoder_num_loops(d));
        for(int i = 0; i < m_numFrames; ++i)
        {
            FLIF_IMAGE* image = flif_decoder_get_image(d, i);
            if(!image)
            {
                qWarning() << "ERROR: Failed to get frame" << i;
                continue;
            }

            const uint32_t width = flif_image_get_width(image);
            const uint32_t height = flif_image_get_height(image);
            const uint32_t frameDelay = flif_image_get_frame_delay(image);

            QImage frame(static_cast<int>(width), static_cast<int>(height), QImage::Format_ARGB32);
            if(frame.isNull())
            {
                qWarning() << "Invalid image size for frame" << i;
                continue;
            }

            QByteArray buffer(width * 4, 0);
            for(uint32_t y = 0; y < height; ++y)
            {
                flif_image_read_row_RGBA8(image, y, reinterpret_cast<void*>(buffer.data()), static_cast<size_t>(buffer.size()));
                QRgb *line = reinterpret_cast<QRgb*>(frame.scanLine(y));
                for(uint32_t x = 0; x < width; ++x)
                    line[x] = qRgba(buffer[x * 4], buffer[x * 4 + 1], buffer[x * 4 + 2], buffer[x * 4 + 3]);
            }

            unsigned char *data = Q_NULLPTR;
            size_t length = 0;
            if(flif_image_get_metadata(image, "iCCP", &data, &length))
            {
                qDebug() << "Found ICC profile for frame" << i;
                ICCProfile(QByteArray::fromRawData(reinterpret_cast<const char*>(data), static_cast<int>(length))).applyToImage(&frame);
                flif_image_free_metadata(image, data);
                data = Q_NULLPTR;
                length = 0;
            }
            if(flif_image_get_metadata(image, "eXmp", &data, &length))
            {
                qDebug() << "Found XMP metadata for frame" << i;
                metaData = ImageMetaData::joinMetaData(metaData, ImageMetaData::createXmpMetaData(QByteArray::fromRawData(reinterpret_cast<const char*>(data), static_cast<int>(length))));
                flif_image_free_metadata(image, data);
                data = Q_NULLPTR;
                length = 0;
            }
            if(flif_image_get_metadata(image, "eXif", &data, &length))
            {
                qDebug() << "Found EXIF metadata for frame" << i;
                metaData = ImageMetaData::joinMetaData(metaData, ImageMetaData::createExifMetaData(QByteArray::fromRawData(reinterpret_cast<const char*>(data), static_cast<int>(length))));
                flif_image_free_metadata(image, data);
                data = Q_NULLPTR;
                length = 0;
            }

            m_frames.push_back(Frame(frame, DelayCalculator::calculate(static_cast<int>(frameDelay), DelayCalculator::MODE_NORMAL)));
        }

        flif_destroy_decoder(d);
        m_error = m_frames.empty();
        return PayloadWithMetaData<bool>(true, metaData);
    }
};

class DecoderFLIF : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderFLIF");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
                << QString::fromLatin1("flif");
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
        FlifAnimationProvider *provider = new FlifAnimationProvider();
        const PayloadWithMetaData<bool> readResult = provider->readFlifFile(filePath);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createAnimatedItem(provider);
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), readResult.metaData()));
    }
};

DecoderAutoRegistrator registrator(new DecoderFLIF);

} // namespace

