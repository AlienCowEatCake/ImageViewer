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

#include "RasterizedImageGraphicsItem.h"

#include <algorithm>
#include <cassert>

#include <QObject>
#include <QPainter>
#include <QThread>

#include "Utils/SharedPointer.h"

#include "../Scaling/IScaledImageProvider.h"
#include "../Scaling/AbstractScalingManager.h"
#include "../Scaling/AbstractScalingWorker.h"
#include "../Scaling/AutoUpdatedScalingWorkerHandler.h"
#include "GraphicsItemUtils.h"

// ====================================================================================================

namespace {

class RasterizerWorker : public AbstractScalingWorker
{
public:
    void setProvider(const QSharedPointer<IScaledImageProvider> &provider)
    {
        m_provider = provider;
    }

    bool generateScaledImage(const bool checkAbortedState)
    {
#define CHECK_ABORT_STATE if(checkAbortedState && isAborted()) return false
        CHECK_ABORT_STATE;
        if(!m_provider)
            return false;
        CHECK_ABORT_STATE;
        const qreal newScaleFactor = m_scaleFactor;
        CHECK_ABORT_STATE;
        QImage image = m_provider->image(newScaleFactor);
        if(image.isNull())
            return false;
        CHECK_ABORT_STATE;
        lockScaledImage();
        m_scaledData.reset(new ScaledImageData(image, newScaleFactor));
        unlockScaledImage();
        CHECK_ABORT_STATE;
        return true;
#undef CHECK_ABORT_STATE
    }

private:
    bool scaleImpl()
    {
        return generateScaledImage(true);
    }

    QSharedPointer<IScaledImageProvider> m_provider;
};

} // namespace

// ====================================================================================================

namespace {

class RasterizerManager : public AbstractScalingManager
{
public:
    RasterizerManager(RasterizerWorker *worker, AutoUpdatedScalingWorkerHandler* handler, QThread *thread)
        : AbstractScalingManager(worker, handler, thread)
    {}

    void setProvider(const QSharedPointer<IScaledImageProvider> &provider)
    {
        static_cast<RasterizerWorker*>(m_scalingWorker)->setProvider(provider);
    }

    void execTaskSynchronously(const qreal scaleFactor)
    {
        abortTask();
        m_scalingWorker->setScaleFactor(scaleFactor);
        static_cast<RasterizerWorker*>(m_scalingWorker)->generateScaledImage(false);
    }
};

RasterizerManager *createRasterizerManager(RasterizedImageGraphicsItem *item)
{
    RasterizerWorker *worker = new RasterizerWorker();
    QThread *thread = new QThread();
    AutoUpdatedScalingWorkerHandler *handler = new AutoUpdatedScalingWorkerHandler(item, worker, thread);
    return new RasterizerManager(worker, handler, thread);
}

} // namespace

// ====================================================================================================

struct RasterizedImageGraphicsItem::Impl
{
    QSharedPointer<IScaledImageProvider> provider;
    Qt::TransformationMode transformationMode;
    QScopedPointer<RasterizerManager> rasterizerManager;

    QPixmap lastPaintedPixmap;
    qint64 lastPaintedDataId;

    Impl(RasterizedImageGraphicsItem *rasterizedImageGraphicsItem)
        : transformationMode(Qt::FastTransformation)
        , rasterizerManager(createRasterizerManager(rasterizedImageGraphicsItem))
        , lastPaintedDataId(0)
    {}

    void drawScaledImage(QPainter *painter)
    {
        const qint64 currentDataId = rasterizerManager->getScaledDataId();
        if(currentDataId != lastPaintedDataId || lastPaintedPixmap.isNull())
        {
            const QImage scaledImage = rasterizerManager->getScaledImage();
            if(scaledImage.isNull())
                return;

            lastPaintedPixmap = QPixmap::fromImage(scaledImage);
            lastPaintedDataId = currentDataId;
        }

        const QRectF originalRect = provider->boundingRect();
        const qreal scaleFactor = rasterizerManager->getScaledScaleFactor();
        const QRectF scaledRect = QRectF(originalRect.topLeft() * scaleFactor, originalRect.size() * scaleFactor);
        painter->drawPixmap(originalRect, lastPaintedPixmap, scaledRect);
    }
};

// ====================================================================================================

RasterizedImageGraphicsItem::RasterizedImageGraphicsItem(QGraphicsItem *parentItem)
    : QGraphicsItem(parentItem)
    , m_impl(new Impl(this))
{}

RasterizedImageGraphicsItem::RasterizedImageGraphicsItem(IScaledImageProvider *provider, QGraphicsItem *parentItem)
    : QGraphicsItem(parentItem)
    , m_impl(new Impl(this))
{
    setProvider(provider);
}

RasterizedImageGraphicsItem::~RasterizedImageGraphicsItem()
{}

IScaledImageProvider *RasterizedImageGraphicsItem::provider() const
{
    return m_impl->provider.data();
}

void RasterizedImageGraphicsItem::setProvider(IScaledImageProvider *provider)
{
    QSharedPointer<IScaledImageProvider> sharedProvider(provider);
    m_impl->provider = sharedProvider;
    m_impl->rasterizerManager->setProvider(sharedProvider);
    update();
}

Qt::TransformationMode RasterizedImageGraphicsItem::transformationMode() const
{
    return m_impl->transformationMode;
}

void RasterizedImageGraphicsItem::setTransformationMode(Qt::TransformationMode mode)
{
    m_impl->transformationMode = mode;
    update();
}

QRectF RasterizedImageGraphicsItem::boundingRect() const
{
    if(!m_impl->provider)
        return QRectF();
    return m_impl->provider->boundingRect();
}

void RasterizedImageGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if(!m_impl->provider || !m_impl->provider->isValid())
        return;

    const qreal deviceScaleFactor = GraphicsItemUtils::GetDeviceScaleFactor(painter);
    const qreal maxScaleFactor = m_impl->provider->maxScaleFactor();
    const qreal minScaleFactor = m_impl->provider->minScaleFactor();
    const qreal newScaleFactor = std::max(std::min(deviceScaleFactor, maxScaleFactor), minScaleFactor);

    if(!m_impl->rasterizerManager->hasScaledData())
        m_impl->rasterizerManager->execTaskSynchronously(newScaleFactor);

    if(!m_impl->rasterizerManager->hasScaledData())
        return;

    painter->setRenderHint(QPainter::SmoothPixmapTransform, m_impl->transformationMode == Qt::SmoothTransformation);
    m_impl->rasterizerManager->beginScaledImageProcessing();
    m_impl->drawScaledImage(painter);
    m_impl->rasterizerManager->endScaledImageProcessing();

    if(!GraphicsItemUtils::IsFuzzyEqualScaleFactors(newScaleFactor, m_impl->rasterizerManager->getScaledScaleFactor()))
        m_impl->rasterizerManager->startTask(newScaleFactor);
}

// ====================================================================================================
