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

#include <cassert>

extern "C" {
#include <libbpg.h>
}

#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QDebug>

#include "Utils/Global.h"
#include "Utils/ScopedPointer.h"

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

// ====================================================================================================

class BpgAnimationProvider : public AbstractAnimationProvider
{
    Q_DISABLE_COPY(BpgAnimationProvider)

public:
    BpgAnimationProvider();
    PayloadWithMetaData<bool> readBpgFile(const QString &filePath);
};

// ====================================================================================================

BpgAnimationProvider::BpgAnimationProvider()
{}

PayloadWithMetaData<bool> BpgAnimationProvider::readBpgFile(const QString &filePath)
{
    const MappedBuffer inBuffer(filePath);
    if(!inBuffer.isValid())
        return false;

    BPGDecoderContext *decoderContext;
    BPGImageInfo imageInfo;

    decoderContext = bpg_decoder_open();
    bpg_decoder_keep_extension_data(decoderContext, 1);
    if(bpg_decoder_decode(decoderContext, inBuffer.dataAs<const uint8_t*>(), inBuffer.sizeAs<int>()) < 0)
    {
        qWarning() << "Can't bpg_decoder_decode for" << filePath;
        bpg_decoder_close(decoderContext);
        return false;
    }
    bpg_decoder_get_info(decoderContext, &imageInfo);

    QScopedPointer<ICCProfile> profile;
    ImageMetaData *metaData = ImageMetaData::createMetaData(filePath);

    for(BPGExtensionData *extension = bpg_decoder_get_extension_data(decoderContext); extension != Q_NULLPTR; extension = extension->next)
    {
        switch(extension->tag)
        {
        case BPG_EXTENSION_TAG_ICCP:
            qDebug() << "Found ICCP metadata";
            profile.reset(new ICCProfile(QByteArray::fromRawData(reinterpret_cast<const char*>(extension->buf), static_cast<int>(extension->buf_len))));
            break;
        case BPG_EXTENSION_TAG_EXIF:
            qDebug() << "Found EXIF metadata";
            if(!metaData)
                metaData = ImageMetaData::createExifMetaData(QByteArray::fromRawData(reinterpret_cast<const char*>(extension->buf + 1), static_cast<int>(extension->buf_len - 1)));
            break;
        case BPG_EXTENSION_TAG_XMP:
            /// @todo
            qDebug() << "Found XMP metadata";
            break;
        default:
            break;
        }
    }

    for(m_numFrames = 0; ; m_numFrames++)
    {
        if(bpg_decoder_start(decoderContext, BPG_OUTPUT_FORMAT_RGBA32) < 0)
            break;

        int delayNum = 0, delayDen = 0;
        bpg_decoder_get_frame_duration(decoderContext, &delayNum, &delayDen);
        if(delayNum < 0)
            delayNum = -delayNum;
        if(delayDen < 0)
            delayDen = -delayDen;
        else if(delayDen == 0)
            delayDen = 1;

#define USE_RGBA_8888 (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0) && Q_BYTE_ORDER == Q_LITTLE_ENDIAN)

        QImage frame(static_cast<int>(imageInfo.width), static_cast<int>(imageInfo.height),
#if (USE_RGBA_8888)
                     QImage::Format_RGBA8888);
#else
                     QImage::Format_ARGB32);
#endif
        if(frame.isNull())
        {
            qWarning() << "Invalid image size";
            bpg_decoder_close(decoderContext);
            return PayloadWithMetaData<bool>(false, metaData);
        }

        for(int y = 0; y < frame.height(); y++)
            bpg_decoder_get_line(decoderContext, reinterpret_cast<uint8_t*>(frame.bits()) + y * frame.bytesPerLine());
#if (!USE_RGBA_8888)
        frame = frame.rgbSwapped();
#endif

#undef USE_RGBA_8888

        if(profile)
            profile->applyToImage(&frame);
        if(metaData)
            metaData->applyExifOrientation(&frame);

        m_frames.push_back(Frame(frame, DelayCalculator::calculate(delayNum * 1000 / delayDen, DelayCalculator::MODE_NORMAL)));
    }
    bpg_decoder_close(decoderContext);

    m_error = false;
    return PayloadWithMetaData<bool>(true, metaData);
}

// ====================================================================================================

class DecoderLibBpg : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderLibBpg");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
                << QString::fromLatin1("bpg");
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
        BpgAnimationProvider *provider = new BpgAnimationProvider();
        const PayloadWithMetaData<bool> readResult = provider->readBpgFile(filePath);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createAnimatedItem(provider);
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), readResult.metaData()));
    }
};

DecoderAutoRegistrator registrator(new DecoderLibBpg);

// ====================================================================================================

} // namespace
