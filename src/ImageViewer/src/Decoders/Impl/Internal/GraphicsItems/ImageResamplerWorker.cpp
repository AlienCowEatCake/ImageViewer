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

#include "ImageResamplerWorker.h"

#include <cassert>

#include <QImage>
#include <QMutex>
#include <QMutexLocker>
#include <QList>
#include <QDebug>

#include <stb_image_resize.h>

// ====================================================================================================

struct ImageResamplerWorker::Impl
{
    ImageResamplerWorker *worker;
    bool workerAborted;

    qreal scaleFactor;
    QImage image;

    struct ResampledImageData
    {
        QImage image;
        qreal scaleFactor;

        ResampledImageData(const QImage& image, const qreal scaleFactor)
            : image(image)
            , scaleFactor(scaleFactor)
        {}
    };
    QScopedPointer<ResampledImageData> resampledData;
    QMutex resampledDataLock;

    Impl(ImageResamplerWorker *worker)
        : worker(worker)
        , workerAborted(false)
        , scaleFactor(-1)
    {}
};

// ====================================================================================================

ImageResamplerWorker::ImageResamplerWorker(QObject *parent)
    : QObject(parent)
    , m_impl(new Impl(this))
{}

ImageResamplerWorker::~ImageResamplerWorker()
{}

void ImageResamplerWorker::setImage(const QImage &newImage)
{
    static const QList<QImage::Format> nativeImageFormats = QList<QImage::Format>()
            /// @todo Не работает, если stride != 0. Разобраться почему.
//#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
//            << QImage::Format_Grayscale8
//            << QImage::Format_Alpha8
//#endif
//            << QImage::Format_RGB888
            << QImage::Format_RGB32
            << QImage::Format_ARGB32
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
            << QImage::Format_RGBX8888
            << QImage::Format_RGBA8888
#endif
               ;
    if(nativeImageFormats.contains(newImage.format()))
        m_impl->image = newImage;
    else
        m_impl->image = newImage.convertToFormat(QImage::Format_ARGB32);
}

QImage ImageResamplerWorker::getImage() const
{
    return m_impl->image;
}

void ImageResamplerWorker::setScaleFactor(const qreal newScaleFactor)
{
    m_impl->scaleFactor = newScaleFactor;
}

qreal ImageResamplerWorker::getScaleFactor() const
{
    return m_impl->scaleFactor;
}

void ImageResamplerWorker::lockResampledImage()
{
    m_impl->resampledDataLock.lock();
}

void ImageResamplerWorker::unlockResampledImage()
{
    m_impl->resampledDataLock.unlock();
}

bool ImageResamplerWorker::hasResampledData() const
{
    return !m_impl->resampledData.isNull();
}

QImage ImageResamplerWorker::getResampledImage() const
{
    return m_impl->resampledData->image;
}

qreal ImageResamplerWorker::getResampledScaleFactor() const
{
    return m_impl->resampledData->scaleFactor;
}

void ImageResamplerWorker::process()
{
#define CHECK_ABORT_STATE \
    if(m_impl->workerAborted) \
    { \
        emit aborted(); \
        return; \
    }

    m_impl->workerAborted = false;
    emit started();
    CHECK_ABORT_STATE;
    const qreal newScaleFactor = m_impl->scaleFactor;
    const QSize scaledImageSize = m_impl->image.size() * newScaleFactor;
    CHECK_ABORT_STATE;
    QScopedPointer<Impl::ResampledImageData> data(new Impl::ResampledImageData(QImage(scaledImageSize, m_impl->image.format()), newScaleFactor));

    CHECK_ABORT_STATE;
    int numChannels = 0;
    switch(m_impl->image.format())
    {
    /// @todo Не работает, если stride != 0. Разобраться почему.
//#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
//    case QImage::Format_Grayscale8:
//    case QImage::Format_Alpha8:
//        numChannels = 1;
//        break;
//#endif
//    case QImage::Format_RGB888:
//        numChannels = 3;
//        break;
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    case QImage::Format_RGBX8888:
    case QImage::Format_RGBA8888:
#endif
        numChannels = 4;
        break;
    default:
        qWarning() << "[ImageResamplerWorker::process()]: Unsupported image format";
        emit aborted();
        return;
    }

    CHECK_ABORT_STATE;
    const int inputStride = m_impl->image.bytesPerLine() - m_impl->image.width() * numChannels;
    const int outputStride = data->image.bytesPerLine() - scaledImageSize.width() * numChannels;

    /// @todo Не работает, если stride != 0. Разобраться почему.
    assert(inputStride == 0);
    assert(outputStride == 0);

    CHECK_ABORT_STATE;
    stbir_resize_uint8(m_impl->image.bits(), m_impl->image.width(), m_impl->image.height(), inputStride,
                       data->image.bits(), scaledImageSize.width(), scaledImageSize.height(), outputStride, numChannels);

    CHECK_ABORT_STATE;
    QMutexLocker guard(&m_impl->resampledDataLock);
    CHECK_ABORT_STATE;
    m_impl->resampledData.swap(data);
    CHECK_ABORT_STATE;
    guard.unlock();

    CHECK_ABORT_STATE;
    emit finished();

#undef CHECK_ABORT_STATE
}

void ImageResamplerWorker::abort()
{
    m_impl->workerAborted = true;
}

// ====================================================================================================

ImageResamplerWorkerHandler::ImageResamplerWorkerHandler(ImageResamplerWorker *worker, QObject *parent)
    : QObject(parent)
{
    connect(worker, SIGNAL(started()), this, SLOT(onStartedReceived()));
    connect(worker, SIGNAL(finished()), this, SLOT(onFinishedReceived()));
    connect(worker, SIGNAL(aborted()), this, SLOT(onAbortedReceived()));
}

void ImageResamplerWorkerHandler::onStartedReceived()
{
    onStarted();
}

void ImageResamplerWorkerHandler::onFinishedReceived()
{
    onFinished();
}

void ImageResamplerWorkerHandler::onAbortedReceived()
{
    onAborted();
}

// ====================================================================================================
