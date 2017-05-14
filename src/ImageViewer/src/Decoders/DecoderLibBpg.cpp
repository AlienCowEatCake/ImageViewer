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

extern "C" {
#include <libbpg.h>
}

#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QDebug>

#include "Utils/ScopedPointer.h"

#include "IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/Animation/AbstractAnimationProvider.h"
#include "Internal/Animation/AnimationUtils.h"
#include "Internal/Animation/FrameCompositor.h"
#include "Internal/CmsUtils.h"
#include "Internal/ExifUtils.h"

namespace {

// ====================================================================================================

class BpgAnimationProvider : public AbstractAnimationProvider
{
public:
    BpgAnimationProvider(const QString &filePath);

private:
    bool readBpg(const QString &filePath);
};

// ====================================================================================================

BpgAnimationProvider::BpgAnimationProvider(const QString &filePath)
{
    m_error = !readBpg(filePath);
}

bool BpgAnimationProvider::readBpg(const QString &filePath)
{
    QFile inFile(filePath);
    if(!inFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "Can't open" << filePath;
        return false;
    }
    QByteArray inBuffer = inFile.readAll();
    inFile.close();

    BPGDecoderContext *decoderContext;
    BPGImageInfo imageInfo;

    decoderContext = bpg_decoder_open();
    bpg_decoder_keep_extension_data(decoderContext, 1);
    if(bpg_decoder_decode(decoderContext, reinterpret_cast<const uint8_t*>(inBuffer.constData()), inBuffer.size()) < 0)
    {
        qWarning() << "Can't bpg_decoder_decode for" << filePath;
        bpg_decoder_close(decoderContext);
        return false;
    }
    bpg_decoder_get_info(decoderContext, &imageInfo);

    QScopedPointer<ICCProfile> profile;
    quint16 orientation = 1;
    for(BPGExtensionData *extension = bpg_decoder_get_extension_data(decoderContext); extension != NULL; extension = extension->next)
    {
        switch(extension->tag)
        {
        case BPG_EXTENSION_TAG_ICCP:
            qDebug() << "Found ICCP metadata";
            profile.reset(new ICCProfile(QByteArray::fromRawData(reinterpret_cast<const char*>(extension->buf), static_cast<int>(extension->buf_len))));
            break;
        case BPG_EXTENSION_TAG_EXIF:
            qDebug() << "Found EXIF metadata";
            orientation = ExifUtils::GetExifOrientation(QByteArray("Exif\0", 5) + QByteArray::fromRawData(reinterpret_cast<const char*>(extension->buf), static_cast<int>(extension->buf_len)));
            break;
        default:
            break;
        }
    }

    for(m_numFrames = 0; ; m_numFrames++)
    {
        if(bpg_decoder_start(decoderContext, BPG_OUTPUT_FORMAT_RGBA32) < 0)
            break;
        int delayNum, delayDen;
        bpg_decoder_get_frame_duration(decoderContext, &delayNum, &delayDen);
        QImage frame(static_cast<int>(imageInfo.width), static_cast<int>(imageInfo.height),
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
                     QImage::Format_RGBA8888);
#else
                     QImage::Format_ARGB32);
#endif
        for(int y = 0; y < frame.height(); y++)
            bpg_decoder_get_line(decoderContext, reinterpret_cast<uint8_t*>(frame.bits()) + y * frame.bytesPerLine());
#if (QT_VERSION < QT_VERSION_CHECK(5, 2, 0))
        QRgb *imageData = reinterpret_cast<QRgb*>(frame.bits());
        for(int i = 0; i < frame.width() * frame.height(); i++)
        {
            uchar *rawPixelData = reinterpret_cast<uchar*>(imageData);
            *(imageData++) = qRgba(rawPixelData[0], rawPixelData[1], rawPixelData[2], rawPixelData[3]);
        }
#endif
        if(profile)
            profile->applyToImage(&frame);
        ExifUtils::ApplyExifOrientation(&frame, orientation);
        m_frames.push_back(Frame(frame, delayNum * 1000 / delayDen));
    }
    bpg_decoder_close(decoderContext);
    return true;
}

// ====================================================================================================

class DecoderLibBpg : public IDecoder
{
public:
    QString name() const
    {
        return QString::fromLatin1("DecoderLibBpg");
    }

    QStringList supportedFormats() const
    {
        return QStringList()
                << QString::fromLatin1("bpg");
    }

    QGraphicsItem *loadImage(const QString &filePath)
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return NULL;
        return AnimationUtils::CreateGraphicsItem(new BpgAnimationProvider(filePath));
    }
};

DecoderAutoRegistrator registrator(new DecoderLibBpg);

// ====================================================================================================

} // namespace
