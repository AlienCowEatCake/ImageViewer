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

#include <cmath>
#include <algorithm>
#include <cassert>

#include <QObject>
#include <QPainter>
#include <QThread>

#include "Utils/ScopedPointer.h"
#include "Utils/SharedPointer.h"

#include "AbstractScalingManager.h"
#include "AbstractScalingWorker.h"
#include "AbstractScalingWorkerHandler.h"

// ====================================================================================================

RasterizedImageGraphicsItem::IRasterizedPixmapProvider::~IRasterizedPixmapProvider()
{}

typedef RasterizedImageGraphicsItem::IRasterizedPixmapProvider IProvider;

// ====================================================================================================

namespace {

class RasterizerWorker : public AbstractScalingWorker
{
public:
    void setProvider(const QSharedPointer<IProvider> &provider)
    {
        m_provider = provider;
    }

    bool generateScaledPixmap(const bool checkAbortedState)
    {
#define CHECK_ABORT_STATE if(checkAbortedState && isAborted()) return false
        CHECK_ABORT_STATE;
        if(!m_provider)
            return false;
        CHECK_ABORT_STATE;
        const qreal newScaleFactor = m_scaleFactor;
        CHECK_ABORT_STATE;
        QPixmap pixmap = m_provider->pixmap(newScaleFactor);
        if(pixmap.isNull())
            return false;
        CHECK_ABORT_STATE;
        lockScaledImage();
        m_scaledData.reset(new ScaledImageData(pixmap, newScaleFactor));
        unlockScaledImage();
        CHECK_ABORT_STATE;
        return true;
#undef CHECK_ABORT_STATE
    }

private:
    bool scaleImpl()
    {
        return generateScaledPixmap(true);
    }

    QSharedPointer<IProvider> m_provider;
};

} // namespace

// ====================================================================================================

namespace {

class RasterizerWorkerHandler : public AbstractScalingWorkerHandler
{
public:
    RasterizerWorkerHandler(RasterizedImageGraphicsItem *item, RasterizerWorker *worker, QThread *thread)
        : AbstractScalingWorkerHandler(worker, thread)
        , m_item(item)
    {}

private:
    void onFinished()
    {
        if(m_item)
            m_item->update();
    }

    void prepareTermination()
    {
        m_item = NULL;
    }

    RasterizedImageGraphicsItem *m_item;
};

} // namespace

// ====================================================================================================

namespace {

class RasterizerManager : public AbstractScalingManager
{
public:
    RasterizerManager(RasterizerWorker *worker, RasterizerWorkerHandler* handler, QThread *thread)
        : AbstractScalingManager(worker, handler, thread)
    {}

    void setProvider(const QSharedPointer<IProvider> &provider)
    {
        static_cast<RasterizerWorker*>(m_scalingWorker)->setProvider(provider);
    }

    void execTaskSynchronously(const qreal scaleFactor)
    {
        abortTask();
        m_scalingWorker->setScaleFactor(scaleFactor);
        static_cast<RasterizerWorker*>(m_scalingWorker)->generateScaledPixmap(false);
    }
};

RasterizerManager *createRasterizerManager(RasterizedImageGraphicsItem *item)
{
    RasterizerWorker *worker = new RasterizerWorker();
    QThread *thread = new QThread();
    RasterizerWorkerHandler *handler = new RasterizerWorkerHandler(item, worker, thread);
    return new RasterizerManager(worker, handler, thread);
}

} // namespace

// ====================================================================================================

struct RasterizedImageGraphicsItem::Impl
{
    QSharedPointer<IProvider> provider;
    Qt::TransformationMode transformationMode;
    QScopedPointer<RasterizerManager> rasterizerManager;

    Impl(RasterizedImageGraphicsItem *rasterizedImageGraphicsItem)
        : transformationMode(Qt::FastTransformation)
        , rasterizerManager(createRasterizerManager(rasterizedImageGraphicsItem))
    {}
};

// ====================================================================================================

RasterizedImageGraphicsItem::RasterizedImageGraphicsItem(QGraphicsItem *parentItem)
    : QGraphicsItem(parentItem)
    , m_impl(new Impl(this))
{}

RasterizedImageGraphicsItem::RasterizedImageGraphicsItem(QSharedPointer<IRasterizedPixmapProvider> provider, QGraphicsItem *parentItem)
    : QGraphicsItem(parentItem)
    , m_impl(new Impl(this))
{
    setProvider(provider);
}

RasterizedImageGraphicsItem::~RasterizedImageGraphicsItem()
{}

QSharedPointer<RasterizedImageGraphicsItem::IRasterizedPixmapProvider> RasterizedImageGraphicsItem::provider() const
{
    return m_impl->provider;
}

void RasterizedImageGraphicsItem::setProvider(QSharedPointer<IRasterizedPixmapProvider> provider)
{
    m_impl->provider = provider;
    m_impl->rasterizerManager->setProvider(provider);
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

    const QRectF identityRect = QRectF(0, 0, 1, 1);
    const QRectF deviceTransformedRect = painter->deviceTransform().mapRect(identityRect);
    const qreal deviceScaleFactor = std::max(deviceTransformedRect.width(), deviceTransformedRect.height());
    const qreal maxScaleFactor = m_impl->provider->maxScaleFactor();
    const qreal minScaleFactor = m_impl->provider->minScaleFactor();
    const qreal newScaleFactor = std::max(std::min(deviceScaleFactor, maxScaleFactor), minScaleFactor);

    if(!m_impl->rasterizerManager->hasScaledData())
        m_impl->rasterizerManager->execTaskSynchronously(newScaleFactor);

    if(!m_impl->rasterizerManager->hasScaledData())
        return;

    painter->setRenderHint(QPainter::SmoothPixmapTransform, m_impl->transformationMode == Qt::SmoothTransformation);
    const QRectF originalRect = boundingRect();
    m_impl->rasterizerManager->beginScaledImageProcessing();
    const qreal actualScaleFactor = m_impl->rasterizerManager->getScaledScaleFactor();
    const QRectF scaledRect = QRectF(originalRect.topLeft() * actualScaleFactor, originalRect.size() * actualScaleFactor);
    painter->drawPixmap(originalRect, m_impl->rasterizerManager->getScaledPixmap(), scaledRect);
    m_impl->rasterizerManager->endScaledImageProcessing();

    if(std::abs(newScaleFactor - actualScaleFactor) / std::max(newScaleFactor, actualScaleFactor) > 1e-2)
        m_impl->rasterizerManager->startTask(newScaleFactor);
}

// ====================================================================================================
