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

#include <cstring>

#include <jxl/decode.h>

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

namespace
{

class JxlAnimationProvider : public AbstractAnimationProvider
{
    Q_DISABLE_COPY(JxlAnimationProvider)

public:
    JxlAnimationProvider()
    {}

    PayloadWithMetaData<bool> readJxlFile(const QString &filePath)
    {
        const MappedBuffer inBuffer(filePath);
        if(!inBuffer.isValid())
            return false;

        JxlDecoder *decoder = JxlDecoderCreate(Q_NULLPTR);
        if(!decoder)
        {
            qWarning() << "ERROR: JxlDecoderCreate";
            return false;
        }

        JxlDecoderSubscribeEvents(decoder, JXL_DEC_BASIC_INFO | JXL_DEC_COLOR_ENCODING | JXL_DEC_FRAME | JXL_DEC_FULL_IMAGE);
        JxlDecoderSetInput(decoder, inBuffer.dataAs<const uint8_t*>(), inBuffer.sizeAs<size_t>());

        if(JxlDecoderProcessInput(decoder) != JXL_DEC_BASIC_INFO)
        {
            qWarning() << "ERROR: JxlDecoderProcessInput";
            JxlDecoderDestroy(decoder);
            return false;
        }

        JxlBasicInfo info;
        if(JxlDecoderGetBasicInfo(decoder, &info) != JXL_DEC_SUCCESS)
        {
            qWarning() << "ERROR: JxlDecoderGetBasicInfo";
            JxlDecoderDestroy(decoder);
            return false;
        }

        qreal ticksPerSecond = 1;
        if(info.have_animation)
        {
            m_numLoops = info.animation.num_loops;
            ticksPerSecond = static_cast<qreal>(info.animation.tps_numerator) / static_cast<qreal>(info.animation.tps_denominator);
        }

        if(JxlDecoderProcessInput(decoder) != JXL_DEC_COLOR_ENCODING)
        {
            qWarning() << "ERROR: JxlDecoderProcessInput != JXL_DEC_COLOR_ENCODING";
            JxlDecoderDestroy(decoder);
            return false;
        }

        while(true)
        {
            JxlPixelFormat format;
            memset(&format, 0, sizeof(format));
            format.num_channels = 4;
            format.data_type = JXL_TYPE_UINT8;
            format.endianness = JXL_NATIVE_ENDIAN;
            format.align = 0;
            size_t iccBufferSize = 0;
            if(JxlDecoderGetICCProfileSize(decoder, &format, JXL_COLOR_PROFILE_TARGET_DATA, &iccBufferSize) != JXL_DEC_SUCCESS)
            {
                qWarning() << "ERROR: JxlDecoderGetICCProfileSize";
                JxlDecoderDestroy(decoder);
                return false;
            }

            QByteArray iccBuffer(static_cast<int>(iccBufferSize), 0);
            if(JxlDecoderGetColorAsICCProfile(decoder, &format, JXL_COLOR_PROFILE_TARGET_DATA, reinterpret_cast<uint8_t*>(iccBuffer.data()), static_cast<size_t>(iccBuffer.size())) != JXL_DEC_SUCCESS)
            {
                qWarning() << "ERROR: JxlDecoderGetICCProfileSize";
                JxlDecoderDestroy(decoder);
                return false;
            }

            QImage frame(static_cast<int>(info.xsize), static_cast<int>(info.ysize), /*info.alpha_premultiplied ? QImage::Format_ARGB32_Premultiplied :*/ QImage::Format_ARGB32);
            if(frame.isNull())
            {
                qWarning() << "Invalid image size";
                JxlDecoderDestroy(decoder);
                return false;
            }

            if(JxlDecoderProcessInput(decoder) != JXL_DEC_FRAME)
            {
                qWarning() << "ERROR: JxlDecoderProcessInput != JXL_DEC_FRAME";
                JxlDecoderDestroy(decoder);
                return false;
            }

            JxlFrameHeader frameHeader;
            memset(&frameHeader, 0, sizeof(frameHeader));
            if(JxlDecoderGetFrameHeader(decoder, &frameHeader) != JXL_DEC_SUCCESS)
            {
                qWarning() << "ERROR: JxlDecoderGetFrameHeader";
                JxlDecoderDestroy(decoder);
                return false;
            }

            if(JxlDecoderProcessInput(decoder) != JXL_DEC_NEED_IMAGE_OUT_BUFFER)
            {
                qWarning() << "ERROR: JxlDecoderProcessInput != JXL_DEC_NEED_IMAGE_OUT_BUFFER";
                JxlDecoderDestroy(decoder);
                return false;
            }
            if(JxlDecoderSetImageOutBuffer(decoder, &format, frame.bits(), frame.bytesPerLine() * frame.height()) != JXL_DEC_SUCCESS)
            {
                qWarning() << "ERROR: JxlDecoderProcessInput";
                JxlDecoderDestroy(decoder);
                return false;
            }
            if(JxlDecoderProcessInput(decoder) != JXL_DEC_FULL_IMAGE)
            {
                qWarning() << "ERROR: JxlDecoderProcessInput != JXL_DEC_FULL_IMAGE";
                JxlDecoderDestroy(decoder);
                return false;
            }

            frame = frame.rgbSwapped();
            ICCProfile iccProfile(iccBuffer);
            iccProfile.applyToImage(&frame);

            const qreal duration = info.have_animation ? frameHeader.duration / ticksPerSecond * 1000 : 0;
            m_frames.push_back(Frame(frame, DelayCalculator::calculate(static_cast<int>(duration), DelayCalculator::MODE_NORMAL)));

            if(!info.have_animation || frameHeader.is_last)
                break;
        }

        ImageMetaData *metaData = ImageMetaData::createMetaData(filePath);
        m_error = m_frames.empty();
        m_numFrames = m_frames.size();
        return PayloadWithMetaData<bool>(true, metaData);
    }
};

class DecoderLibJxl : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderLibJxl");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
                << QString::fromLatin1("jxl");
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
        JxlAnimationProvider *provider = new JxlAnimationProvider();
        const PayloadWithMetaData<bool> readResult = provider->readJxlFile(filePath);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createAnimatedItem(provider);
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), readResult.metaData()));
    }
};

DecoderAutoRegistrator registrator(new DecoderLibJxl);

} // namespace
