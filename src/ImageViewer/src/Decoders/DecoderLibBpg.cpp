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

#include "IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/Animation/AbstractAnimationProvider.h"
#include "Internal/Animation/AnimationUtils.h"
#include "Internal/Animation/FrameCompositor.h"

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

    /// @attention BEGIN DEBUG ROUTINES

    static const char *format_str[6] = {
        "Gray",
        "4:2:0",
        "4:2:2",
        "4:4:4",
        "4:2:0_video",
        "4:2:2_video",
    };
    static const char *color_space_str[BPG_CS_COUNT] = {
        "YCbCr",
        "RGB",
        "YCgCo",
        "YCbCr_BT709",
        "YCbCr_BT2020",
    };
    static const char *extension_tag_str[] = {
        "Unknown",
        "EXIF",
        "ICC profile",
        "XMP",
        "Thumbnail",
        "Animation control",
    };

    BPGExtensionData *first_md = bpg_decoder_get_extension_data(decoderContext);

    qDebug() << QString::fromLatin1("size=%1x%2 color_space=%3")
                .arg(imageInfo.width)
                .arg(imageInfo.height)
                .arg(QString::fromLatin1(imageInfo.format == BPG_FORMAT_GRAY ? "Gray" : color_space_str[imageInfo.color_space]))
                .toLatin1().data();
    if(imageInfo.has_w_plane)
        qDebug() << QString::fromLatin1("w_plane=%1")
                    .arg(imageInfo.has_w_plane)
                    .toLatin1().data();
    if(imageInfo.has_alpha)
        qDebug() << QString::fromLatin1("alpha=%1 premul=%2")
                    .arg(imageInfo.has_alpha)
                    .arg(imageInfo.premultiplied_alpha)
                    .toLatin1().data();
    qDebug() << QString::fromLatin1("format=%1 limited_range=%2 bit_depth=%3 animation=%4")
                .arg(QString::fromLatin1(format_str[imageInfo.format]))
                .arg(imageInfo.limited_range)
                .arg(imageInfo.bit_depth)
                .arg(imageInfo.has_animation)
                .toLatin1().data();

    if(first_md)
    {
        const char *tag_name;
        qDebug() << "Extension data:";
        for(BPGExtensionData *md = first_md; md != NULL; md = md->next)
        {
            if(md->tag <= 5)
                tag_name = extension_tag_str[md->tag];
            else
                tag_name = extension_tag_str[0];
            qDebug() << QString::fromLatin1("tag=%1 (%2) length=%3")
                        .arg(md->tag)
                        .arg(QString::fromLatin1(tag_name))
                        .arg(md->buf_len)
                        .toLatin1().data();
        }
    }

    /// @attention END DEBUG ROUTINES

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
