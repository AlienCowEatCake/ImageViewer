/*
   Copyright (C) 2020-2023 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

        avifDecoder *decoder = avifDecoderCreate();
#if QT_VERSION_CHECK(AVIF_VERSION_MAJOR, AVIF_VERSION_MINOR, AVIF_VERSION_PATCH) >= QT_VERSION_CHECK(0, 4, 3)
        /// @note Default dav1d can crash in some configurations, so try to use aom
        ///
        /// (lldb) bt
        /// * thread #1, stop reason = Exception 0xc0000005 encountered at address 0x59033779: Access violation reading location 0xffffffff
        ///   * frame #0: 0x59033779 libdav1d.dll`dav1d_data_wrap + 9
        ///     frame #1: 0x6143acbd libavif.dll`avifEncoderWrite + 3341
        ///     frame #2: 0x6142c5f5 libavif.dll`avifDecoderNextImage + 1013
        ///
        if(avifCodecName(AVIF_CODEC_CHOICE_AOM, AVIF_CODEC_FLAG_CAN_DECODE))
            decoder->codecChoice = AVIF_CODEC_CHOICE_AOM;
#endif
#if QT_VERSION_CHECK(AVIF_VERSION_MAJOR, AVIF_VERSION_MINOR, AVIF_VERSION_PATCH) >= QT_VERSION_CHECK(0, 9, 1)
        decoder->strictFlags = AVIF_STRICT_DISABLED;
#endif
#if QT_VERSION_CHECK(AVIF_VERSION_MAJOR, AVIF_VERSION_MINOR, AVIF_VERSION_PATCH) < QT_VERSION_CHECK(0, 8, 2)
        avifROData raw;
        raw.data = inBuffer.dataAs<const uint8_t*>();
        raw.size = inBuffer.sizeAs<size_t>();
        avifResult decodeResult = avifDecoderParse(decoder, &raw);
#else
        avifResult decodeResult = avifDecoderSetIOMemory(decoder, inBuffer.dataAs<const uint8_t*>(), inBuffer.sizeAs<size_t>());
        if(decodeResult != AVIF_RESULT_OK)
        {
            qWarning() << "ERROR: Cannot set IO on avifDecoder:" << avifResultToString(decodeResult);
            avifDecoderDestroy(decoder);
            return false;
        }
        decodeResult = avifDecoderParse(decoder);
#endif
        if(decodeResult != AVIF_RESULT_OK)
        {
            qWarning() << "ERROR: Failed to decode:" << avifResultToString(decodeResult);
            avifDecoderDestroy(decoder);
            return false;
        }

        ImageMetaData *metaData = ImageMetaData::createMetaData(inBuffer.dataAsByteArray());

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

            if(decoder->image->transformFlags & AVIF_TRANSFORM_PASP)
            {
                qDebug() << "Found PixelAspectRatioBox transform for frame" << m_numFrames;
                if(decoder->image->pasp.hSpacing != 1 && decoder->image->pasp.vSpacing != 1)
                {
                    qDebug() << "pasp.hSpacing =" << decoder->image->pasp.hSpacing;
                    qDebug() << "pasp.vSpacing =" << decoder->image->pasp.vSpacing;
                    /// @todo ?
                }
            }
            if(decoder->image->transformFlags & AVIF_TRANSFORM_CLAP)
            {
                qDebug() << "Found CleanApertureBox transform for frame" << m_numFrames;
                const avifCleanApertureBox &clap = decoder->image->clap;
                const int width = static_cast<int>(clap.widthN / clap.widthD);
                const int height = static_cast<int>(clap.heightN / clap.heightD);
                const int horizOff = static_cast<int>(clap.horizOffN / clap.horizOffD);
                const int vertOff = static_cast<int>(clap.vertOffN / clap.vertOffD);
                const int left = horizOff + (static_cast<int>(decoder->image->width)  - 1) / 2 - (width  - 1) / 2;
                const int top  = vertOff  + (static_cast<int>(decoder->image->height) - 1) / 2 - (height - 1) / 2;
                frame = frame.copy(QRect(left, top, width, height));
            }
            if(decoder->image->transformFlags & AVIF_TRANSFORM_IROT)
            {
                qDebug() << "Found ImageRotation transform for frame" << m_numFrames;
                QTransform transform;
                transform.rotate(-static_cast<qreal>(decoder->image->irot.angle) * 90);
                frame = frame.transformed(transform);
            }
            if(decoder->image->transformFlags & AVIF_TRANSFORM_IMIR)
            {
                qDebug() << "Found ImageMirror transform for frame" << m_numFrames;
#if QT_VERSION_CHECK(AVIF_VERSION_MAJOR, AVIF_VERSION_MINOR, AVIF_VERSION_PATCH) < QT_VERSION_CHECK(0, 9, 2) || QT_VERSION_CHECK(AVIF_VERSION_MAJOR, AVIF_VERSION_MINOR, AVIF_VERSION_PATCH) > QT_VERSION_CHECK(1, 0, 0)
                // See discussion:
                // https://github.com/AOMediaCodec/libavif/pull/665
                // https://github.com/AOMediaCodec/libavif/issues/667
                frame = frame.mirrored(decoder->image->imir.axis == 1, decoder->image->imir.axis == 0);
#else
                frame = frame.mirrored(decoder->image->imir.mode == 1, decoder->image->imir.mode == 0);
#endif
            }

#if QT_VERSION_CHECK(AVIF_VERSION_MAJOR, AVIF_VERSION_MINOR, AVIF_VERSION_PATCH) < QT_VERSION_CHECK(0, 8, 0)
            if(decoder->image->profileFormat == AVIF_PROFILE_FORMAT_ICC)
#else
            if(decoder->image->icc.data && decoder->image->icc.size)
#endif
            {
                qDebug() << "Found ICC profile for frame" << m_numFrames;
                ICCProfile(QByteArray::fromRawData(reinterpret_cast<const char*>(decoder->image->icc.data), static_cast<int>(decoder->image->icc.size))).applyToImage(&frame);
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

#if QT_VERSION_CHECK(AVIF_VERSION_MAJOR, AVIF_VERSION_MINOR, AVIF_VERSION_PATCH) > QT_VERSION_CHECK(1, 0, 0)
        if(decoder->repetitionCount >= 0)
            m_numLoops = decoder->repetitionCount + 1;
#endif

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
        /// @note https://aomediacodec.github.io/av1-avif/
        return QStringList()
                << QString::fromLatin1("heif")
                << QString::fromLatin1("heifs")
                << QString::fromLatin1("hif");
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

