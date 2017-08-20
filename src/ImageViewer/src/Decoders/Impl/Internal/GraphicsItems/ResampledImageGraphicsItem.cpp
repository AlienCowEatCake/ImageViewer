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

#include <cmath>
#include <algorithm>

#include <QObject>
#include <QPainter>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>

#include <stb_image_resize.h>

#include "Utils/ScopedPointer.h"
#include "ImageResamplerWorker.h"

// ====================================================================================================

namespace {

class ResamplerWorkerHandler : public ImageResamplerWorkerHandler
{
public:
    ResamplerWorkerHandler(QGraphicsItem *item, ImageResamplerWorker *worker, QThread *thread, QObject *parent = NULL)
        : ImageResamplerWorkerHandler(worker, parent)
        , m_item(item)
        , m_isRunning(false)
        , m_destroyOnFinished(false)
        , m_worker(worker)
        , m_thread(thread)
    {}

    void onStarted()
    {
        QMutexLocker guard(&m_handlerMutex);
        m_isRunning = true;
    }

    void onFinished()
    {
        QMutexLocker guard(&m_handlerMutex);
        m_isRunning = false;
        m_thread->quit();
        if(m_item)
            m_item->update();
        checkAndDestroy();
    }

    void onAborted()
    {
        QMutexLocker guard(&m_handlerMutex);
        m_isRunning = false;
        m_thread->quit();
        checkAndDestroy();
    }

    bool isRunning() const
    {
        return m_isRunning;
    }

    void setDestroyOnFinished()
    {
        QMutexLocker guard(&m_handlerMutex);
        m_item = NULL;
        m_destroyOnFinished = true;
        if(!isRunning())
            checkAndDestroy();
    }

private:
    void checkAndDestroy()
    {
        if(!m_destroyOnFinished)
            return;

        m_worker->disconnect();
        m_thread->disconnect();
        disconnect();

        m_thread->quit();
        m_thread->wait();
        delete m_worker;
        m_thread->deleteLater();
        deleteLater();
    }

    QMutex m_handlerMutex;
    QGraphicsItem *m_item;
    bool m_isRunning;
    bool m_destroyOnFinished;
    ImageResamplerWorker *m_worker;
    QThread *m_thread;
};

} // namespace

// ====================================================================================================

namespace {

class ResamplerManager
{
public:
    ResamplerManager(QGraphicsItem *item)
        : resamplerWorker(new ImageResamplerWorker)
        , resamplerThread(new QThread)
        , resamplerWorkerHandler(new ResamplerWorkerHandler(item, resamplerWorker, resamplerThread))
    {
        resamplerWorker->moveToThread(resamplerThread);
        QObject::connect(resamplerThread, SIGNAL(started()), resamplerWorker, SLOT(process()));
    }

    ~ResamplerManager()
    {
        abortTask();
        resamplerWorkerHandler->setDestroyOnFinished();
    }

    void setImage(const QImage &image)
    {
        resamplerWorker->setImage(image);
    }

    QImage getImage() const
    {
        return resamplerWorker->getImage();
    }

    void startTask(qreal scaleFactor)
    {
        if(resamplerWorkerHandler->isRunning())
            return;

        abortTask();
        resamplerWorker->setScaleFactor(scaleFactor);
        resamplerThread->start();
    }

    void abortTask()
    {
        resamplerWorker->abort();
    }

    void beginResampledImageProcessing()
    {
        resamplerWorker->lockResampledImage();
    }

    void endResampledImageProcessing()
    {
        resamplerWorker->unlockResampledImage();
    }

    bool hasResampledData() const
    {
        return resamplerWorker->hasResampledData();
    }

    QImage getResampledImage() const
    {
        return resamplerWorker->getResampledImage();
    }

    qreal getResampledScaleFactor() const
    {
        return resamplerWorker->getResampledScaleFactor();
    }

private:

    ImageResamplerWorker *resamplerWorker;
    QThread *resamplerThread;
    ResamplerWorkerHandler *resamplerWorkerHandler;
};

} // namespace

// ====================================================================================================

struct ResampledImageGraphicsItem::Impl
{
    QPixmap pixmap;
    Qt::TransformationMode transformationMode;

    ResamplerManager resamplerManager;

    Impl(ResampledImageGraphicsItem *resampledImageGraphicsItem)
        : transformationMode(Qt::FastTransformation)
        , resamplerManager(resampledImageGraphicsItem)
    {}

    void paintDefault(QPainter *painter) const
    {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, transformationMode == Qt::SmoothTransformation);
        painter->drawPixmap(0, 0, pixmap);
    }

    void paintResampled(QPainter *painter) const
    {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, transformationMode == Qt::SmoothTransformation);
        painter->drawImage(pixmap.rect(), resamplerManager.getResampledImage(), QRectF(QPointF(0, 0), QSizeF(pixmap.size()) * resamplerManager.getResampledScaleFactor()));
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
    return m_impl->resamplerManager.getImage();
}

void ResampledImageGraphicsItem::setImage(const QImage &image)
{
    m_impl->pixmap = QPixmap::fromImage(image);
    m_impl->resamplerManager.setImage(image);
    update();
}

QPixmap ResampledImageGraphicsItem::pixmap() const
{
    return m_impl->pixmap;
}

void ResampledImageGraphicsItem::setPixmap(const QPixmap &pixmap)
{
    m_impl->pixmap = pixmap;
    m_impl->resamplerManager.setImage(pixmap.toImage());
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

    if(m_impl->transformationMode != Qt::SmoothTransformation)
        return m_impl->paintDefault(painter);

    const QRectF identityRect = QRectF(0, 0, 1, 1);
    const QRectF deviceTransformedRect = painter->deviceTransform().mapRect(identityRect);
    const qreal newScaleFactor = std::max(deviceTransformedRect.width(), deviceTransformedRect.height());

    if(newScaleFactor >= 1 || newScaleFactor <= 0)
        return m_impl->paintDefault(painter);

    m_impl->resamplerManager.beginResampledImageProcessing();
    const qreal scaleFactor = !m_impl->resamplerManager.hasResampledData() ? newScaleFactor : m_impl->resamplerManager.getResampledScaleFactor();
    if(!(!m_impl->resamplerManager.hasResampledData() || std::abs(newScaleFactor - scaleFactor) / std::max(newScaleFactor, scaleFactor) > 1e-2))
    {
        m_impl->paintResampled(painter);
        m_impl->resamplerManager.endResampledImageProcessing();
        return;
    }
    m_impl->resamplerManager.endResampledImageProcessing();

    m_impl->paintDefault(painter);
    m_impl->resamplerManager.startTask(newScaleFactor);
}

// ====================================================================================================
