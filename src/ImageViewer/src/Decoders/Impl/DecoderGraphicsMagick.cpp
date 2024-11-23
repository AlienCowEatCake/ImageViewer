/*
   Copyright (C) 2020-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <cstdio>
#include <cstdlib>

#include <magick/api.h>

#include <QFileInfo>
#include <QImage>
#include <QByteArray>
#include <QSysInfo>

#include "Utils/Logging.h"

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

#if defined (MagickLibVersion) && ((MagickLibVersion) == 0x0100010)
#undef MagickLibVersion
#define MagickLibVersion 0x010010
#endif
#if defined (MagickLibVersion) && ((MagickLibVersion) == 0x0100011)
#undef MagickLibVersion
#define MagickLibVersion 0x010011
#endif
#if defined (MagickLibVersion) && ((MagickLibVersion) == 0x0100012)
#undef MagickLibVersion
#define MagickLibVersion 0x010012
#endif
#if defined (MagickLibVersion) && ((MagickLibVersion) == 0x0100013)
#undef MagickLibVersion
#define MagickLibVersion 0x010013
#endif
#if defined (MagickLibVersion) && ((MagickLibVersion) == 0x0100014)
#undef MagickLibVersion
#define MagickLibVersion 0x010014
#endif

namespace
{

// ====================================================================================================

struct GraphicsMagickGuard
{
    explicit GraphicsMagickGuard(const char *path)
    {
        InitializeMagick(path);
    }

    ~GraphicsMagickGuard()
    {
        DestroyMagick();
    }
};

class ScopedExceptionInfo
{
public:
    ScopedExceptionInfo()
    {
        GetExceptionInfo(&m_exception);
    }

    ~ScopedExceptionInfo()
    {
        DestroyExceptionInfo(&m_exception);
    }

    operator ExceptionInfo * ()
    {
        return &m_exception;
    }

    ExceptionInfo * operator -> ()
    {
        return &m_exception;
    }

private:
    ExceptionInfo m_exception;
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

class GraphicsMagickAnimationProvider : public AbstractAnimationProvider
{
    Q_DISABLE_COPY(GraphicsMagickAnimationProvider)

public:
    explicit GraphicsMagickAnimationProvider(const QString &filePath)
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

        const GraphicsMagickGuard graphicsMagickGuard(Q_NULLPTR);
        ScopedExceptionInfo exception;
        QScopedPointer<ImageInfo, ImageInfoDeleter> info(CloneImageInfo(Q_NULLPTR));
        QScopedPointer<Image, ImageDeleter> image(BlobToImage(info.data(), inBuffer.dataAs<const void*>(), inBuffer.sizeAs<size_t>(), exception));
        if(!image)
        {
            if(exception->severity != UndefinedException)
                LOG_WARNING() << LOGGING_CTX << "Exception:" << exception->reason << exception->description;
            else
                LOG_WARNING() << LOGGING_CTX << "BlobToImage error";
            return false;
        }

        FramesCompositor compositor;
        compositor.startComposition(image->page.width, image->page.height);
        for(Image *frame = image.data(); frame; frame = frame->next)
        {
            QImage qImage = convertImage(frame, exception);
            if(qImage.isNull())
            {
                if(exception->severity != UndefinedException)
                    LOG_WARNING() << LOGGING_CTX << "Exception:" << exception->reason << exception->description;
                else
                    LOG_WARNING() << LOGGING_CTX << "ExportImagePixels error";
                return false;
            }
            const QRect rect(static_cast<int>(frame->page.x), static_cast<int>(frame->page.y), qImage.width(), qImage.height());
            qImage = compositor.compositeFrame(qImage, rect, disposeType(frame), blendType(frame));
            const int delay = static_cast<int>(static_cast<ssize_t>(frame->delay));
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
        DispatchImage(image, 0, 0, width, height, ((QSysInfo::ByteOrder == QSysInfo::LittleEndian) ? "BGRA" : "ARGB"), CharPixel, qImage.bits(), exception);
        if(exception->severity != UndefinedException)
            return QImage();

#if defined (MagickLibVersion) && ((MagickLibVersion) >= 0x020000)
        if(image->orientation)
        {
            LOG_DEBUG() << LOGGING_CTX << "Orientation found:" << image->orientation;
            ImageMetaData::applyExifOrientation(&qImage, static_cast<quint16>(image->orientation));
        }
#endif

        size_t profileLength = 0;
        const unsigned char *profileData = GetImageProfile(image, "ICC", &profileLength);
        if(profileData)
        {
            LOG_DEBUG() << LOGGING_CTX << "ICC profile found";
            ICCProfile profile(QByteArray::fromRawData(reinterpret_cast<const char*>(profileData), static_cast<int>(profileLength)));
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

class DecoderGraphicsMagick : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderGraphicsMagick");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        const GraphicsMagickGuard graphicsMagickGuard(Q_NULLPTR);
        ScopedExceptionInfo exception;

        QStringList formatNames;
        MagickInfo **info = GetMagickInfoArray(exception);
        if(info)
        {
            for(size_t i = 0; info[i]; ++i)
                formatNames.append(QString::fromLatin1(info[i]->name).toLower());
#if defined (MagickLibVersion) && ((MagickLibVersion) >= 0x020000)
            MagickFree(info);
#else
            free(info);
#endif
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
        IAnimationProvider *provider = new GraphicsMagickAnimationProvider(filePath);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createAnimatedItem(provider);
        IImageMetaData *metaData = item ? ImageMetaData::createMetaData(filePath) : Q_NULLPTR;
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), metaData));
    }
};

DecoderAutoRegistrator registrator(new DecoderGraphicsMagick, true);

// ====================================================================================================

} // namespace
