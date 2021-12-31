/*
   Copyright (C) 2020-2021 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "Utils/Global.h"

#include <cstdlib>

#if QT_HAS_INCLUDE(<MagickCore/MagickCore.h>)
#include <MagickCore/MagickCore.h>
#else
#include <magick/MagickCore.h>
#endif
/// @note https://github.com/umlaeute/Gem/blob/v0.94/plugins/imageMAGICK/imageMAGICK.cpp#L52
#if (defined (MagickLibInterface) && (MagickLibInterface > 3)) || (defined (MagickLibVersion) && (MagickLibVersion >= 0x662))
typedef size_t magick_size_t;
#else
typedef unsigned long magick_size_t;
#endif

#include <QFileInfo>
#include <QImage>
#include <QByteArray>
#include <QDebug>

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"
#include "Internal/Animation/AbstractAnimationProvider.h"
#include "Internal/Animation/DelayCalculator.h"
#include "Internal/Animation/FramesCompositor.h"
#include "Internal/Utils/CmsUtils.h"
#include "Internal/Utils/MappedBuffer.h"

namespace
{

// ====================================================================================================

struct MagickCoreGuard
{
    MagickCoreGuard(const char *path, const MagickBooleanType establishSignalHandlers)
    {
        MagickCoreGenesis(path, establishSignalHandlers);
    }

    ~MagickCoreGuard()
    {
        MagickCoreTerminus();
    }
};

struct ExceptionInfoDeleter
{
    static inline void cleanup(ExceptionInfo *exception)
    {
        if(exception)
            DestroyExceptionInfo(exception);
    }
};

struct ImageInfoDeleter
{
    static inline void cleanup(ImageInfo *info)
    {
        if(info)
            DestroyImageInfo(info);
    }
};

struct ImageDeleter
{
    static inline void cleanup(Image *image)
    {
        if(image)
            DestroyImage(image);
    }
};

// ====================================================================================================

class MagickCoreAnimationProvider : public AbstractAnimationProvider
{
    Q_DISABLE_COPY(MagickCoreAnimationProvider)

public:
    explicit MagickCoreAnimationProvider(const QString &filePath)
    {
        m_numLoops = 1;
        m_error = !readImage(filePath);
    }

private:
    bool readImage(const QString &filePath)
    {
        const MappedBuffer inBuffer(filePath);
        if(!inBuffer.isValid())
            return false;

        const MagickCoreGuard magickCoreGuard(Q_NULLPTR, MagickFalse);
        QScopedPointer<ExceptionInfo, ExceptionInfoDeleter> exception(AcquireExceptionInfo());
        QScopedPointer<ImageInfo, ImageInfoDeleter> info(CloneImageInfo(Q_NULLPTR));
        QScopedPointer<Image, ImageDeleter> image(BlobToImage(info.data(), inBuffer.dataAs<const void*>(), inBuffer.sizeAs<size_t>(), exception.data()));
        if(!image)
        {
            if(exception->severity != UndefinedException)
                qWarning() << "[DecoderMagickCore] Exception:" << exception->reason << exception->description;
            else
                qWarning() << "[DecoderMagickCore] BlobToImage error";
            return false;
        }

        FramesCompositor compositor;
        compositor.startComposition(image->page.width, image->page.height);
        for(Image *frame = image.data(); frame; frame = frame->next)
        {
            QImage qImage = convertImage(frame, exception.data());
            if(qImage.isNull())
            {
                if(exception->severity != UndefinedException)
                    qWarning() << "[DecoderMagickCore] Exception:" << exception->reason << exception->description;
                else
                    qWarning() << "[DecoderMagickCore] ExportImagePixels error";
                return false;
            }
            const QRect rect(static_cast<int>(frame->page.x), static_cast<int>(frame->page.y), qImage.width(), qImage.height());
            qImage = compositor.compositeFrame(qImage, rect, disposeType(frame), blendType(frame));
            const int delay = static_cast<int>(static_cast<ssize_t>(frame->delay) * 1000 / frame->ticks_per_second);
            m_frames.push_back(Frame(qImage, DelayCalculator::calculate(delay, DelayCalculator::MODE_CHROME)));
        }
        m_numFrames = m_frames.size();
        m_numLoops = static_cast<int>(image->iterations);
        return m_numFrames > 0;
    }

    QImage convertImage(const Image *image, ExceptionInfo *exception) const
    {
        const size_t width = image->magick_columns;
        const size_t height = image->magick_rows;
        QImage qImage(static_cast<int>(width), static_cast<int>(height), QImage::Format_ARGB32);
        if(qImage.isNull())
            return qImage;

        qImage.fill(Qt::transparent);
        ExportImagePixels(image, 0, 0, width, height, "BGRA", CharPixel, qImage.bits(), exception);
        if(exception->severity != UndefinedException)
            return QImage();

        if(image->orientation)
        {
            qDebug() << "[DecoderMagickCore] Orientation found:" << image->orientation;
            ImageMetaData::applyExifOrientation(&qImage, static_cast<quint16>(image->orientation));
        }

        if(const StringInfo *pinfo = GetImageProfile(image, "ICC"))
        {
            qDebug() << "[DecoderMagickCore] ICC profile found";
            ICCProfile profile(QByteArray::fromRawData(reinterpret_cast<const char*>(pinfo->datum), static_cast<int>(pinfo->length)));
            profile.applyToImage(&qImage);
        }

        return qImage;
    }

    FramesCompositor::DisposeType disposeType(const Image *image) const
    {
        switch(image->dispose)
        {
        case NoneDispose:
            return FramesCompositor::DISPOSE_NONE;
        case BackgroundDispose:
            return FramesCompositor::DISPOSE_BACKGROUND;
        case PreviousDispose:
            return FramesCompositor::DISPOSE_PREVIOUS;
        default:
            break;
        }
        return FramesCompositor::DISPOSE_NONE;
    }

    FramesCompositor::BlendType blendType(const Image *image) const
    {
        if(image->compose == OverCompositeOp)
            return FramesCompositor::BLEND_OVER;
        return FramesCompositor::BLEND_NONE;
    }
};

// ====================================================================================================

class DecoderMagickCore : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderMagickCore");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        const MagickCoreGuard magickCoreGuard(Q_NULLPTR, MagickFalse);
        QScopedPointer<ExceptionInfo, ExceptionInfoDeleter> exception(AcquireExceptionInfo());

        QStringList formatNames;
        magick_size_t num = 0;
        const MagickInfo **info = GetMagickInfoList("*", &num, exception.data());
        if(info)
        {
            for(size_t i = 0; i < num; i++)
                formatNames.append(QString::fromLatin1(info[i]->name).toLower());
            RelinquishMagickMemory(info);
        }
        return formatNames;
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
        if(!fileInfo.exists() || !fileInfo.isReadable() || !isAvailable())
            return QSharedPointer<IImageData>();
        IAnimationProvider *provider = new MagickCoreAnimationProvider(filePath);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createAnimatedItem(provider);
        IImageMetaData *metaData = ImageMetaData::createMetaData(filePath);
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), metaData));
    }
};

DecoderAutoRegistrator registrator(new DecoderMagickCore, true);

// ====================================================================================================

} // namespace
