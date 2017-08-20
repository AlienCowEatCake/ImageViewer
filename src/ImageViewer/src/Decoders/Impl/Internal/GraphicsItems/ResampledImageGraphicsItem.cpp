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

#include "ResampledImageGraphicsItem.h"

#include <cassert>

#include <QObject>
#include <QPainter>
#include <QThread>
#include <QDebug>

#if defined (HAS_STB)
#include <stb_image_resize.h>
#endif

#include "Utils/ScopedPointer.h"

#include "AbstractScalingManager.h"
#include "AbstractScalingWorker.h"
#include "AutoUpdatedScalingWorkerHandler.h"
#include "GraphicsItemUtils.h"

// ====================================================================================================

namespace {

class ResamplerWorker : public AbstractScalingWorker
{
public:
    void setImage(const QImage &newImage)
    {
        static const QList<QImage::Format> nativeImageFormats = QList<QImage::Format>()
                /// @todo Не работает, если stride != 0. Разобраться почему.
//#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
//                << QImage::Format_Grayscale8
//                << QImage::Format_Alpha8
//#endif
//                << QImage::Format_RGB888
                << QImage::Format_RGB32
                << QImage::Format_ARGB32
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
                << QImage::Format_RGBX8888
                << QImage::Format_RGBA8888
#endif
                   ;
        if(nativeImageFormats.contains(newImage.format()))
            m_image = newImage;
        else
            m_image = newImage.convertToFormat(QImage::Format_ARGB32);
    }

    QImage getImage() const
    {
        return m_image;
    }

private:
    bool scaleImpl()
    {
#if defined (HAS_STB)

#define CHECK_ABORT_STATE if(isAborted()) return false
        CHECK_ABORT_STATE;
        const qreal newScaleFactor = m_scaleFactor;
        const QSize scaledImageSize = m_image.size() * newScaleFactor;
        CHECK_ABORT_STATE;
        QImage scaledImage(scaledImageSize, m_image.format());

        CHECK_ABORT_STATE;
        int numChannels = 0;
        switch(m_image.format())
        {
        /// @todo Не работает, если stride != 0. Разобраться почему.
//#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
//        case QImage::Format_Grayscale8:
//        case QImage::Format_Alpha8:
//            numChannels = 1;
//            break;
//#endif
//        case QImage::Format_RGB888:
//            numChannels = 3;
//            break;
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32:
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
        case QImage::Format_RGBX8888:
        case QImage::Format_RGBA8888:
#endif
            numChannels = 4;
            break;
        default:
            qWarning() << "[ImageResamplerWorker::scaleImpl()]: Unsupported image format";
            return false;
        }

        CHECK_ABORT_STATE;
        const int inputStride = m_image.bytesPerLine() - m_image.width() * numChannels;
        const int outputStride = scaledImage.bytesPerLine() - scaledImageSize.width() * numChannels;

        /// @todo Не работает, если stride != 0. Разобраться почему.
        assert(inputStride == 0);
        assert(outputStride == 0);

        CHECK_ABORT_STATE;
        stbir_resize_uint8(m_image.bits(), m_image.width(), m_image.height(), inputStride,
                           scaledImage.bits(), scaledImageSize.width(), scaledImageSize.height(), outputStride, numChannels);

        CHECK_ABORT_STATE;
        lockScaledImage();
        m_scaledData.reset(new ScaledImageData(QPixmap::fromImage(scaledImage), newScaleFactor));
        unlockScaledImage();
        CHECK_ABORT_STATE;

        return true;
#undef CHECK_ABORT_STATE

#else
        return false;
#endif
    }

    QImage m_image;
};

} // namespace

// ====================================================================================================

namespace {

class ResamplerManager : public AbstractScalingManager
{
public:
    ResamplerManager(ResamplerWorker *worker, AutoUpdatedScalingWorkerHandler* handler, QThread *thread)
        : AbstractScalingManager(worker, handler, thread)
    {}

    void setImage(const QImage &image)
    {
        static_cast<ResamplerWorker*>(m_scalingWorker)->setImage(image);
    }

    QImage getImage() const
    {
        return static_cast<ResamplerWorker*>(m_scalingWorker)->getImage();
    }
};

ResamplerManager *createResamplerManager(ResampledImageGraphicsItem *item)
{
    ResamplerWorker *worker = new ResamplerWorker();
    QThread *thread = new QThread();
    AutoUpdatedScalingWorkerHandler *handler = new AutoUpdatedScalingWorkerHandler(item, worker, thread);
    return new ResamplerManager(worker, handler, thread);
}

} // namespace

// ====================================================================================================

struct ResampledImageGraphicsItem::Impl
{
    QPixmap pixmap;
    Qt::TransformationMode transformationMode;

    QScopedPointer<ResamplerManager> resamplerManager;

    Impl(ResampledImageGraphicsItem *resampledImageGraphicsItem)
        : transformationMode(Qt::FastTransformation)
        , resamplerManager(createResamplerManager(resampledImageGraphicsItem))
    {}

    void paintDefault(QPainter *painter) const
    {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, transformationMode == Qt::SmoothTransformation);
        painter->drawPixmap(0, 0, pixmap);
    }

    void paintResampled(QPainter *painter) const
    {
        const QPixmap scaledPixmap = resamplerManager->getScaledPixmap();
        if(scaledPixmap.isNull())
            return paintDefault(painter);
        painter->setRenderHint(QPainter::SmoothPixmapTransform, transformationMode == Qt::SmoothTransformation);
        GraphicsItemUtils::DrawScaledPixmap(painter, scaledPixmap, pixmap.rect(), resamplerManager->getScaledScaleFactor());
    }
};

// ====================================================================================================

ResampledImageGraphicsItem::ResampledImageGraphicsItem(QGraphicsItem *parentItem)
    : QGraphicsItem(parentItem)
    , m_impl(new Impl(this))
{}

ResampledImageGraphicsItem::ResampledImageGraphicsItem(const QImage &image, QGraphicsItem *parentItem)
    : QGraphicsItem(parentItem)
    , m_impl(new Impl(this))
{
    setImage(image);
}

ResampledImageGraphicsItem::ResampledImageGraphicsItem(const QPixmap &pixmap, QGraphicsItem *parentItem)
    : QGraphicsItem(parentItem)
    , m_impl(new Impl(this))
{
    setPixmap(pixmap);
}

ResampledImageGraphicsItem::~ResampledImageGraphicsItem()
{}

QImage ResampledImageGraphicsItem::image() const
{
    return m_impl->resamplerManager->getImage();
}

void ResampledImageGraphicsItem::setImage(const QImage &image)
{
    m_impl->pixmap = QPixmap::fromImage(image);
    m_impl->resamplerManager->setImage(image);
    update();
}

QPixmap ResampledImageGraphicsItem::pixmap() const
{
    return m_impl->pixmap;
}

void ResampledImageGraphicsItem::setPixmap(const QPixmap &pixmap)
{
    m_impl->pixmap = pixmap;
    m_impl->resamplerManager->setImage(pixmap.toImage());
    update();
}

Qt::TransformationMode ResampledImageGraphicsItem::transformationMode() const
{
    return m_impl->transformationMode;
}

void ResampledImageGraphicsItem::setTransformationMode(Qt::TransformationMode mode)
{
    m_impl->transformationMode = mode;
    update();
}

QRectF ResampledImageGraphicsItem::boundingRect() const
{
    return QRectF(m_impl->pixmap.rect());
}

void ResampledImageGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if(m_impl->pixmap.isNull())
        return;

    if(m_impl->transformationMode != Qt::SmoothTransformation)
        return m_impl->paintDefault(painter);

    const qreal newScaleFactor = GraphicsItemUtils::GetDeviceScaleFactor(painter);

    if(newScaleFactor >= 1 || newScaleFactor <= 0)
        return m_impl->paintDefault(painter);

    m_impl->resamplerManager->beginScaledImageProcessing();
    const qreal scaleFactor = !m_impl->resamplerManager->hasScaledData() ? newScaleFactor : m_impl->resamplerManager->getScaledScaleFactor();
    if(m_impl->resamplerManager->hasScaledData() && GraphicsItemUtils::IsFuzzyEqualScaleFactors(newScaleFactor, scaleFactor))
    {
        m_impl->paintResampled(painter);
        m_impl->resamplerManager->endScaledImageProcessing();
        return;
    }
    m_impl->resamplerManager->endScaledImageProcessing();

    m_impl->paintDefault(painter);
    m_impl->resamplerManager->startTask(newScaleFactor);
}

// ====================================================================================================
