/*
   Copyright (C) 2017-2026 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "AbstractScalingWorker.h"

#include <cassert>

#include <QImage>
#include <QMutexLocker>

namespace {

const qreal INVALID_SCALE_FACTOR = -1;
const qint64 INVALID_DATA_ID = 0;

} // namespace

AbstractScalingWorker::ScaledImageData::ScaledImageData(const QImage &image, const qreal scaleFactor)
    : image(image)
    , scaleFactor(scaleFactor)
{}

AbstractScalingWorker::AbstractScalingWorker()
    : m_scaleFactor(INVALID_SCALE_FACTOR)
    , m_workerAborted(0)
{}

AbstractScalingWorker::~AbstractScalingWorker()
{}

void AbstractScalingWorker::setScaleFactor(const qreal newScaleFactor)
{
    m_scaleFactor = newScaleFactor;
}

qreal AbstractScalingWorker::getScaleFactor() const
{
    return m_scaleFactor;
}

void AbstractScalingWorker::lockScaledImage()
{
    m_scaledDataLock.lock();
}

void AbstractScalingWorker::unlockScaledImage()
{
    m_scaledDataLock.unlock();
}

bool AbstractScalingWorker::hasScaledData() const
{
    return !m_scaledData.isNull();
}

QImage AbstractScalingWorker::getScaledImage() const
{
    if(hasScaledData())
        return m_scaledData->image;
    return QImage();
}

qreal AbstractScalingWorker::getScaledScaleFactor() const
{
    if(hasScaledData())
        return m_scaledData->scaleFactor;
    return INVALID_SCALE_FACTOR;
}

qint64 AbstractScalingWorker::getScaledDataId() const
{
    if(hasScaledData())
        return m_scaledData->image.cacheKey();
    return INVALID_DATA_ID;
}

void AbstractScalingWorker::process()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    m_workerAborted.storeRelease(0);
#else
    m_workerAborted = 0;
#endif
    Q_EMIT started();
    if(isAborted())
    {
        Q_EMIT aborted();
        return;
    }
    if(!scaleImpl())
    {
        Q_EMIT aborted();
        return;
    }
    if(isAborted())
    {
        Q_EMIT aborted();
        return;
    }
    Q_EMIT finished();
}

void AbstractScalingWorker::abort()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    m_workerAborted.storeRelease(1);
#else
    m_workerAborted = 1;
#endif
}

bool AbstractScalingWorker::isAborted() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    return m_workerAborted.loadAcquire() != 0;
#else
    return m_workerAborted != 0;
#endif
}
