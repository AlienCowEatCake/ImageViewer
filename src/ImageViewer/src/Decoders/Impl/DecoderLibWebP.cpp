/*
   Copyright (C) 2017-2018 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <algorithm>

#include <webp/decode.h>
#include <webp/demux.h>

#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QDebug>

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/Animation/AbstractAnimationProvider.h"
#include "Internal/Animation/DelayCalculator.h"
#include "Internal/Animation/FramesCompositor.h"

namespace
{

// ====================================================================================================

class WebPAnimationProvider : public AbstractAnimationProvider
{
    Q_DISABLE_COPY(WebPAnimationProvider)

public:
    WebPAnimationProvider(const QString &filePath);

private:
    bool readWebP(const QString &filePath);
};

// ====================================================================================================

WebPAnimationProvider::WebPAnimationProvider(const QString &filePath)
{
    m_error = !readWebP(filePath);
}

bool WebPAnimationProvider::readWebP(const QString &filePath)
{
    QFile inFile(filePath);
    if(!inFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "Can't open" << filePath;
        return false;
    }
    QByteArray inBuffer = inFile.readAll();
    inFile.close();

    WebPBitstreamFeatures features;
    VP8StatusCode status = WebPGetFeatures(reinterpret_cast<const uint8_t*>(inBuffer.constData()), static_cast<std::size_t>(inBuffer.size()), &features);
    if(status != VP8_STATUS_OK)
    {
        qWarning() << "Can't WebPGetFeatures for" << filePath;
        qWarning() << "Error code:" << status;
        return false;
    }

//    qDebug() << "features.width:" << features.width;
//    qDebug() << "features.height:" << features.height;
//    qDebug() << "features.has_alpha:" << features.has_alpha;
//    qDebug() << "features.has_animation:" << features.has_animation;
//    qDebug() << "features.format:" << features.format;

    if(!features.has_animation)
    {
        m_frames.push_back(Frame(features.width, features.height));
        QImage &frame = m_frames[0].image;
        if(frame.isNull())
        {
            qWarning() << "Invalid image size";
            return false;
        }

        const uint8_t *data = reinterpret_cast<const uint8_t*>(inBuffer.constData());
        const std::size_t dataSize = static_cast<std::size_t>(inBuffer.size());
        uint8_t *output = reinterpret_cast<uint8_t*>(frame.bits());
        const std::size_t size = static_cast<std::size_t>(frame.bytesPerLine() * frame.height());
        const int stride = frame.bytesPerLine();
#if (Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
        if(!WebPDecodeBGRAInto(data, dataSize, output, size, stride))
        {
            qWarning() << "Can't WebPDecodeBGRAInto for" << filePath;
#else
        if(!WebPDecodeARGBInto(data, dataSize, output, size, stride))
        {
            qWarning() << "Can't WebPDecodeBGRAInto for" << filePath;
#endif
            return false;
        }
    }
    else
    {
        WebPData webpData;
        webpData.bytes = reinterpret_cast<const uint8_t*>(inBuffer.constData());
        webpData.size = static_cast<std::size_t>(inBuffer.size());
        WebPDemuxer *demuxer = WebPDemux(&webpData);
        if(!demuxer)
        {
            qWarning() << "Can't WebPDemux for" << filePath;
            return false;
        }

        m_numLoops = static_cast<int>(WebPDemuxGetI(demuxer, WEBP_FF_LOOP_COUNT));
        m_numFrames = static_cast<int>(WebPDemuxGetI(demuxer, WEBP_FF_FRAME_COUNT));

        WebPIterator iter;
        iter.duration = 100;
        if(!WebPDemuxGetFrame(demuxer, 1, &iter))
        {
            qWarning() << "Can't WebPDemuxGetFrame for" << filePath;
            WebPDemuxDelete(demuxer);
            return false;
        }

        FramesCompositor compositor;
        compositor.startComposition(QSize(features.width, features.height));

        for(int i = 0; i < m_numFrames; i++)
        {
            status = WebPGetFeatures(iter.fragment.bytes, iter.fragment.size, &features);
            if(status != VP8_STATUS_OK)
            {
                qWarning() << "Can't WebPGetFeatures for" << filePath;
                qWarning() << "Error fragment:" << i;
                qWarning() << "Error code:" << status;
                WebPDemuxReleaseIterator(&iter);
                WebPDemuxDelete(demuxer);
                return false;
            }

            QImage frame(iter.width, iter.height, QImage::Format_ARGB32);
            if(frame.isNull())
            {
                qWarning() << "Invalid image size";
                WebPDemuxReleaseIterator(&iter);
                WebPDemuxDelete(demuxer);
                return false;
            }

            const uint8_t *data = iter.fragment.bytes;
            const std::size_t dataSize = iter.fragment.size;
            uint8_t *output = reinterpret_cast<uint8_t*>(frame.bits());
            const std::size_t size = static_cast<std::size_t>(frame.bytesPerLine() * frame.height());
            const int stride = frame.bytesPerLine();
#if (Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
            if(!WebPDecodeBGRAInto(data, dataSize, output, size, stride))
            {
                qWarning() << "Can't WebPDecodeBGRAInto for" << filePath;
#else
            if(!WebPDecodeARGBInto(data, dataSize, output, size, stride))
            {
                qWarning() << "Can't WebPDecodeBGRAInto for" << filePath;
#endif
                qWarning() << "Error fragment:" << i;
                WebPDemuxReleaseIterator(&iter);
                WebPDemuxDelete(demuxer);
                return false;
            }

            const QRect frameRect(iter.x_offset, iter.y_offset, iter.width, iter.height);
            FramesCompositor::DisposeType compositorDisposeType = FramesCompositor::DISPOSE_NONE;
            switch(iter.dispose_method)
            {
            case WEBP_MUX_DISPOSE_NONE:         compositorDisposeType = FramesCompositor::DISPOSE_NONE;         break;
            case WEBP_MUX_DISPOSE_BACKGROUND:   compositorDisposeType = FramesCompositor::DISPOSE_BACKGROUND;   break;
            }
            FramesCompositor::BlendType compositorBlendType = FramesCompositor::BLEND_NONE;
            switch(iter.blend_method)
            {
            case WEBP_MUX_NO_BLEND: compositorBlendType = FramesCompositor::BLEND_NONE; break;
            case WEBP_MUX_BLEND:    compositorBlendType = FramesCompositor::BLEND_OVER; break;
            }
            const int realDuration = DelayCalculator::calculate(iter.duration, DelayCalculator::MODE_CHROME);
            m_frames.push_back(Frame(compositor.compositeFrame(frame, frameRect, compositorDisposeType, compositorBlendType), realDuration));

            if(i != m_numFrames - 1 && !WebPDemuxNextFrame(&iter))
            {
                qWarning() << "Can't WebPDemuxNextFrame for" << filePath;
                qWarning() << "Error fragment:" << i;
                WebPDemuxReleaseIterator(&iter);
                WebPDemuxDelete(demuxer);
                return false;
            }
        }

        WebPDemuxDelete(demuxer);
    }

    return true;
}

// ====================================================================================================

class DecoderLibWebP : public IDecoder
{
public:
    QString name() const
    {
        return QString::fromLatin1("DecoderLibWebP");
    }

    QStringList supportedFormats() const
    {
        return QStringList()
                << QString::fromLatin1("webp");
    }

    QStringList advancedFormats() const
    {
        return QStringList();
    }

    bool isAvailable() const
    {
        return true;
    }

    QGraphicsItem *loadImage(const QString &filePath)
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return NULL;
        return GraphicsItemsFactory::instance().createAnimatedItem(new WebPAnimationProvider(filePath));
    }
};

DecoderAutoRegistrator registrator(new DecoderLibWebP);

// ====================================================================================================

} // namespace
