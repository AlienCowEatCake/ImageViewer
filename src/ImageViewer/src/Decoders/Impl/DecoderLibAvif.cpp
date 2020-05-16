/*
   Copyright (C) 2020 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <avif/avif.h>

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

class AvifAnimationProvider : public AbstractAnimationProvider
{
    Q_DISABLE_COPY(AvifAnimationProvider)

public:
    AvifAnimationProvider()
    {}

    PayloadWithMetaData<bool> readAvifFile(const QString &filePath)
    {
        const MappedBuffer inBuffer(filePath);
        if(!inBuffer.isValid())
            return false;

        avifROData raw;
        raw.data = inBuffer.dataAs<const uint8_t*>();
        raw.size = inBuffer.sizeAs<size_t>();

        avifDecoder *decoder = avifDecoderCreate();
        avifResult decodeResult = avifDecoderParse(decoder, &raw);
        if(decodeResult != AVIF_RESULT_OK)
        {
            qWarning() << "ERROR: Failed to decode:" << avifResultToString(decodeResult);
            avifDecoderDestroy(decoder);
            return false;
        }

        ImageMetaData *metaData = ImageMetaData::createMetaData(filePath);

        for(m_numFrames = 0; ; m_numFrames++)
        {
            avifResult nextImageResult = avifDecoderNextImage(decoder);
            if(nextImageResult == AVIF_RESULT_NO_IMAGES_REMAINING)
                break;

            if(nextImageResult != AVIF_RESULT_OK)
            {
                qWarning() << "ERROR: Failed to decode all frames:" << avifResultToString(nextImageResult);
                break;
            }

            QImage frame(static_cast<int>(decoder->image->width), static_cast<int>(decoder->image->height), QImage::Format_ARGB32);
            if(frame.isNull())
            {
                qWarning() << "Invalid image size";
                avifDecoderDestroy(decoder);
                return PayloadWithMetaData<bool>(false, metaData);
            }

            avifRGBImage rgb;
            avifRGBImageSetDefaults(&rgb, decoder->image);
            rgb.format = AVIF_RGB_FORMAT_BGRA;
            rgb.depth = 8;
            rgb.pixels = reinterpret_cast<uint8_t*>(frame.bits());
            rgb.rowBytes = static_cast<uint32_t>(frame.width() * 4);
            avifImageYUVToRGB(decoder->image, &rgb);

            if(decoder->image->profileFormat == AVIF_PROFILE_FORMAT_ICC)
            {
                qDebug() << "Found ICC profile for frame" << m_numFrames;
                ICCProfile(QByteArray::fromRawData(reinterpret_cast<const char*>(decoder->image->icc.data), static_cast<int>(decoder->image->icc.size))).applyToImage(&frame);
            }
            else if(decoder->image->profileFormat == AVIF_PROFILE_FORMAT_NCLX)
            {
                qDebug() << "Found NCLX profile for frame" << m_numFrames;
                /// @todo
            }

            if(decoder->image->exif.size)
            {
                qDebug() << "Found EXIF metadata for frame" << m_numFrames;
                metaData = ImageMetaData::joinMetaData(metaData, ImageMetaData::createExifMetaData(QByteArray::fromRawData(reinterpret_cast<const char*>(decoder->image->exif.data), static_cast<int>(decoder->image->exif.size))));
            }
            if(decoder->image->xmp.size)
            {
                qDebug() << "Found XMP metadata for frame" << m_numFrames;
                metaData = ImageMetaData::joinMetaData(metaData, ImageMetaData::createXmpMetaData(QByteArray::fromRawData(reinterpret_cast<const char*>(decoder->image->xmp.data), static_cast<int>(decoder->image->xmp.size))));
            }

            m_frames.push_back(Frame(frame, DelayCalculator::calculate(static_cast<int>(decoder->imageTiming.duration * 1000.0), DelayCalculator::MODE_NORMAL)));
        }

        avifDecoderDestroy(decoder);

        m_error = false;
        return PayloadWithMetaData<bool>(true, metaData);
    }
};

class DecoderLibAvif : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderLibAvif");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
                << QString::fromLatin1("avif")
                << QString::fromLatin1("avifs");
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
        AvifAnimationProvider *provider = new AvifAnimationProvider();
        const PayloadWithMetaData<bool> readResult = provider->readAvifFile(filePath);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createAnimatedItem(provider);
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), readResult.metaData()));
    }
};

DecoderAutoRegistrator registrator(new DecoderLibAvif);

} // namespace

