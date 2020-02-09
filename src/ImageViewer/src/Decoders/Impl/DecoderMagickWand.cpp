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

#include <wand/MagickWand.h>

#include <QFileInfo>
#include <QImage>
#include <QByteArray>
#include <QDebug>

#include "Utils/Global.h"

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

struct MagickWandGuard
{
    MagickWandGuard()
    {
        MagickWandGenesis();
    }

    ~MagickWandGuard()
    {
        MagickWandTerminus();
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

struct MagickWandDeleter
{
    static inline void cleanup(MagickWand *mw)
    {
        if(mw)
            DestroyMagickWand(mw);
    }
};

// ====================================================================================================

class MagickWandAnimationProvider : public AbstractAnimationProvider
{
    Q_DISABLE_COPY(MagickWandAnimationProvider)

public:
    explicit MagickWandAnimationProvider(const QString &filePath)
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

        QScopedPointer<MagickWand, MagickWandDeleter> mw(NewMagickWand());
        if(!MagickReadImageBlob(mw.data(), inBuffer.dataAs<const void*>(), inBuffer.sizeAs<size_t>()))
        {
            qWarning() << "[DecoderMagickWand] MagickReadImageBlob error";
            return false;
        }

        const size_t num = MagickGetNumberImages(mw.data());
        MagickResetIterator(mw.data());
        ssize_t x = 0, y = 0;
        size_t width = 0, height = 0;
        MagickGetImagePage(mw.data(), &width, &height, &x, &y);
        FramesCompositor compositor;
        compositor.startComposition(width, height);
        for(size_t i = 0; i < num; ++i)
        {
            MagickNextImage(mw.data());
            QImage qImage = convertImage(mw.data());
            if(qImage.isNull())
            {
                qWarning() << "[DecoderMagickWand] MagickGetImagePixels error";
                return false;
            }
            MagickGetImagePage(mw.data(), &width, &height, &x, &y);
            const QRect rect(static_cast<int>(x), static_cast<int>(y), qImage.width(), qImage.height());
            qImage = compositor.compositeFrame(qImage, rect, disposeType(mw.data()), blendType(mw.data()));
            const int delay = static_cast<int>(MagickGetImageDelay(mw.data()) * 1000 / MagickGetImageTicksPerSecond(mw.data()));
            m_frames.push_back(Frame(qImage, DelayCalculator::calculate(delay, DelayCalculator::MODE_CHROME)));
        }
        m_numFrames = static_cast<int>(num);
        m_numLoops = static_cast<int>(MagickGetImageIterations(mw.data()));
        return m_numFrames > 0;
    }

    QImage convertImage(MagickWand *mw) const
    {
        const size_t width = MagickGetImageWidth(mw);
        const size_t height = MagickGetImageHeight(mw);
        QImage qImage(static_cast<int>(width), static_cast<int>(height), QImage::Format_ARGB32);
        if(qImage.isNull())
            return qImage;

        qImage.fill(Qt::transparent);
        if(!MagickExportImagePixels(mw, 0, 0, width, height, "BGRA", CharPixel, qImage.bits()))
            return QImage();

        if(OrientationType orientation = MagickGetImageOrientation(mw))
        {
            qDebug() << "[DecoderMagickWand] Orientation found:" << orientation;
            ImageMetaData::applyExifOrientation(&qImage, static_cast<quint16>(orientation));
        }

        size_t length = 0;
        if(const unsigned char *datum = MagickGetImageProfile(mw, "ICC", &length))
        {
            qDebug() << "[DecoderMagickWand] ICC profile found";
            ICCProfile profile(QByteArray::fromRawData(reinterpret_cast<const char*>(datum), static_cast<int>(length)));
            profile.applyToImage(&qImage);
        }

        return qImage;
    }

    FramesCompositor::DisposeType disposeType(MagickWand *mw) const
    {
        switch(MagickGetImageDispose(mw))
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

    FramesCompositor::BlendType blendType(MagickWand *mw) const
    {
        if(MagickGetImageCompose(mw) == OverCompositeOp)
            return FramesCompositor::BLEND_OVER;
        return FramesCompositor::BLEND_NONE;
    }
};

// ====================================================================================================

class DecoderMagickWand : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderMagickWand");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        const MagickWandGuard magickWandGuard;
        QScopedPointer<ExceptionInfo, ExceptionInfoDeleter> exception(AcquireExceptionInfo());
        GetExceptionInfo(exception.data());

        QStringList formatNames;
        size_t num = 0;
        const MagickInfo **info = GetMagickInfoList("*", &num, exception.data());
        for(size_t i = 0; i < num; i++)
            formatNames.append(QString::fromLatin1(info[i]->name).toLower());

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
        IAnimationProvider *provider = new MagickWandAnimationProvider(filePath);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createAnimatedItem(provider);
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name()));
    }
};

DecoderAutoRegistrator registrator(new DecoderMagickWand, true);

// ====================================================================================================

} // namespace
