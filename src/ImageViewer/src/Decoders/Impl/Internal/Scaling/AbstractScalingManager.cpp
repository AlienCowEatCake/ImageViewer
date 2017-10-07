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

#include "AbstractScalingManager.h"

#include <QObject>
#include <QThread>

#include "AbstractScalingWorker.h"
#include "AbstractScalingWorkerHandler.h"

AbstractScalingManager::AbstractScalingManager(AbstractScalingWorker *worker, AbstractScalingWorkerHandler *handler, QThread *thread)
    : m_scalingWorker(worker)
    , m_scalingThread(thread)
    , m_scalingWorkerHandler(handler)
{
    m_scalingWorker->moveToThread(m_scalingThread);
    QObject::connect(m_scalingThread, SIGNAL(started()), m_scalingWorker, SLOT(process()));
}

AbstractScalingManager::~AbstractScalingManager()
{
    abortTask();
    m_scalingWorkerHandler->setDestroyOnFinished();
}

void AbstractScalingManager::startTask(const qreal scaleFactor)
{
    if(m_scalingWorkerHandler->isRunning())
        return;

    abortTask();
    m_scalingWorker->setScaleFactor(scaleFactor);
    m_scalingThread->start();
}

void AbstractScalingManager::abortTask()
{
    m_scalingWorker->abort();
}

void AbstractScalingManager::beginScaledImageProcessing()
{
    m_scalingWorker->lockScaledImage();
}

void AbstractScalingManager::endScaledImageProcessing()
{
    m_scalingWorker->unlockScaledImage();
}

bool AbstractScalingManager::hasScaledData() const
{
    return m_scalingWorker->hasScaledData();
}

QImage AbstractScalingManager::getScaledImage() const
{
    return m_scalingWorker->getScaledImage();
}

qreal AbstractScalingManager::getScaledScaleFactor() const
{
    return m_scalingWorker->getScaledScaleFactor();
}
