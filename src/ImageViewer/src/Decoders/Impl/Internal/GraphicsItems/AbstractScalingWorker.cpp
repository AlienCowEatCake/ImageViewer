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

#include "AbstractScalingWorker.h"

#include <cassert>

#include <QImage>
#include <QMutexLocker>

namespace {

const qreal INVALID_SCALE_FACTOR = -1;

} // namespace

AbstractScalingWorker::ScaledImageData::ScaledImageData(const QPixmap &pixmap, const qreal scaleFactor)
    : pixmap(pixmap)
    , scaleFactor(scaleFactor)
{}

AbstractScalingWorker::AbstractScalingWorker(QObject *parent)
    : QObject(parent)
    , m_scaleFactor(INVALID_SCALE_FACTOR)
    , m_workerAborted(false)
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

QPixmap AbstractScalingWorker::getScaledPixmap() const
{
    if(hasScaledData())
        return m_scaledData->pixmap;
    return QPixmap();
}

qreal AbstractScalingWorker::getScaledScaleFactor() const
{
    if(hasScaledData())
        return m_scaledData->scaleFactor;
    return INVALID_SCALE_FACTOR;
}

void AbstractScalingWorker::process()
{
    m_workerAborted = false;
    emit started();
    if(isAborted())
    {
        emit aborted();
        return;
    }
    if(!scaleImpl())
    {
        emit aborted();
        return;
    }
    if(isAborted())
    {
        emit aborted();
        return;
    }
    emit finished();
}

void AbstractScalingWorker::abort()
{
    m_workerAborted = true;
}

bool AbstractScalingWorker::isAborted() const
{
    return m_workerAborted;
}
