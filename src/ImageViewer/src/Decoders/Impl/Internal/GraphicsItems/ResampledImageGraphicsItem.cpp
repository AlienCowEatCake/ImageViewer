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
#include <cassert>

#include <QObject>
#include <QPainter>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

#if defined (HAS_STB)
#include <stb_image_resize.h>
#endif

#include "Utils/ScopedPointer.h"

#include "AbstractScalingManager.h"
#include "AbstractScalingWorker.h"
#include "AbstractScalingWorkerHandler.h"

// ====================================================================================================

namespace {

class ResamplerWorker : public AbstractScalingWorker
{
public:
    ResamplerWorker(QObject *parent = NULL)
        : AbstractScalingWorker(parent)
    {}

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
        QScopedPointer<ScaledImageData> data(new ScaledImageData(QImage(scaledImageSize, m_image.format()), newScaleFactor));

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
        const int outputStride = data->image.bytesPerLine() - scaledImageSize.width() * numChannels;

        /// @todo Не работает, если stride != 0. Разобраться почему.
        assert(inputStride == 0);
        assert(outputStride == 0);

        CHECK_ABORT_STATE;
        stbir_resize_uint8(m_image.bits(), m_image.width(), m_image.height(), inputStride,
                           data->image.bits(), scaledImageSize.width(), scaledImageSize.height(), outputStride, numChannels);

        CHECK_ABORT_STATE;
        lockScaledImage();
        m_scaledData.swap(data);
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

class ResamplerWorkerHandler : public AbstractScalingWorkerHandler
{
public:
    ResamplerWorkerHandler(ResampledImageGraphicsItem *item, ResamplerWorker *worker, QThread *thread, QObject *parent = NULL)
        : AbstractScalingWorkerHandler(worker, thread, parent)
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

    ResampledImageGraphicsItem *m_item;
};

} // namespace

// ====================================================================================================

namespace {

class ResamplerManager : public AbstractScalingManager
{
public:
    ResamplerManager(ResamplerWorker *worker, ResamplerWorkerHandler* handler, QThread *thread, QObject *parent = NULL)
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
    ResamplerWorkerHandler *handler = new ResamplerWorkerHandler(item, worker, thread);
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
        painter->setRenderHint(QPainter::SmoothPixmapTransform, transformationMode == Qt::SmoothTransformation);
        painter->drawImage(pixmap.rect(), resamplerManager->getScaledImage(), QRectF(QPointF(0, 0), QSizeF(pixmap.size()) * resamplerManager->getScaledScaleFactor()));
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

    if(m_impl->transformationMode != Qt::SmoothTransformation)
        return m_impl->paintDefault(painter);

    const QRectF identityRect = QRectF(0, 0, 1, 1);
    const QRectF deviceTransformedRect = painter->deviceTransform().mapRect(identityRect);
    const qreal newScaleFactor = std::max(deviceTransformedRect.width(), deviceTransformedRect.height());

    if(newScaleFactor >= 1 || newScaleFactor <= 0)
        return m_impl->paintDefault(painter);

    m_impl->resamplerManager->beginScaledImageProcessing();
    const qreal scaleFactor = !m_impl->resamplerManager->hasScaledData() ? newScaleFactor : m_impl->resamplerManager->getScaledScaleFactor();
    if(!(!m_impl->resamplerManager->hasScaledData() || std::abs(newScaleFactor - scaleFactor) / std::max(newScaleFactor, scaleFactor) > 1e-2))
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
