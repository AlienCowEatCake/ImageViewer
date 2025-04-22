/*
   Copyright (C) 2017-2025 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <QPainter>
#include <QThread>

#include "../Scaling/AbstractScalingManager.h"
#include "../Scaling/AbstractScalingWorker.h"
#include "../Scaling/AutoUpdatedScalingWorkerHandler.h"
#include "GraphicsItemUtils.h"

// ====================================================================================================

namespace {

class ResamplerWorker : public AbstractScalingWorker
{
public:
    void setImage(const QImage &newImage)
    {
        m_image = newImage;
    }

    QImage getImage() const
    {
        return m_image;
    }

private:
    bool scaleImpl() Q_DECL_OVERRIDE
    {
#define CHECK_ABORT_STATE if(isAborted()) return false
        CHECK_ABORT_STATE;
        const qreal newScaleFactor = m_scaleFactor;
        const QSize scaledImageSize = m_image.size() * newScaleFactor;
        CHECK_ABORT_STATE;
        QImage scaledImage = m_image.scaled(scaledImageSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        CHECK_ABORT_STATE;
        lockScaledImage();
        m_scaledData.reset(new ScaledImageData(scaledImage, newScaleFactor));
        unlockScaledImage();
        CHECK_ABORT_STATE;
        return true;
#undef CHECK_ABORT_STATE
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

    QPixmap lastPaintedResampledPixmap;
    qint64 lastPaintedResampledDataId;

    explicit Impl(ResampledImageGraphicsItem *resampledImageGraphicsItem)
        : transformationMode(Qt::FastTransformation)
        , resamplerManager(createResamplerManager(resampledImageGraphicsItem))
        , lastPaintedResampledDataId(0)
    {}

    void paintDefault(QPainter *painter) const
    {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, transformationMode == Qt::SmoothTransformation);
        painter->drawPixmap(0, 0, pixmap);
    }

    void paintResampled(QPainter *painter)
    {
        const qint64 currentDataId = resamplerManager->getScaledDataId();
        if(currentDataId != lastPaintedResampledDataId || lastPaintedResampledPixmap.isNull())
        {
            const QImage scaledImage = resamplerManager->getScaledImage();
            if(scaledImage.isNull())
                return paintDefault(painter);

            lastPaintedResampledPixmap = QPixmap::fromImage(scaledImage);
            lastPaintedResampledDataId = currentDataId;
        }
        painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
        painter->drawPixmap(pixmap.rect(), lastPaintedResampledPixmap, lastPaintedResampledPixmap.rect());
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

QImage ResampledImageGraphicsItem::grabImage()
{
    return image();
}

QRectF ResampledImageGraphicsItem::boundingRect() const
{
    return QRectF(m_impl->pixmap.rect());
}

void ResampledImageGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    /// @note QPainter with SmoothPixmapTransform render hint downscales images
    /// with same quality as QPainter with FastTransformation render hint.
    /// https://bugreports.qt.io/browse/QTBUG-30682
    /// https://bugs.webkit.org/show_bug.cgi?id=119263

    if(m_impl->pixmap.isNull())
        return;

    if(m_impl->transformationMode != Qt::SmoothTransformation)
        return m_impl->paintDefault(painter);

    const qreal newScaleFactor = GraphicsItemUtils::GetDeviceScaleFactor(painter);
    const QSize originalPixmapSize = m_impl->pixmap.size();
    const QSize scaledPixmapSize = originalPixmapSize * newScaleFactor;
    if(newScaleFactor >= 1 || newScaleFactor <= 0 || scaledPixmapSize == originalPixmapSize)
        return m_impl->paintDefault(painter);

    ResamplerManager *resamplerManager = m_impl->resamplerManager.data();

    resamplerManager->beginScaledImageProcessing();
    const qreal scaleFactor = !resamplerManager->hasScaledData() ? newScaleFactor : resamplerManager->getScaledScaleFactor();
    if(resamplerManager->hasScaledData() && GraphicsItemUtils::IsFuzzyEqualScaleFactors(newScaleFactor, scaleFactor))
    {
        m_impl->paintResampled(painter);
        resamplerManager->endScaledImageProcessing();
        return;
    }
    resamplerManager->endScaledImageProcessing();

    m_impl->paintDefault(painter);
    resamplerManager->startTask(newScaleFactor);
}

// ====================================================================================================
