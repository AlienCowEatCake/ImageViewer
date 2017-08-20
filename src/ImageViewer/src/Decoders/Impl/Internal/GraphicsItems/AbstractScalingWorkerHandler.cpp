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

#include "AbstractScalingWorkerHandler.h"

#include <QThread>
#include <QMutex>
#include <QMutexLocker>

#include "AbstractScalingWorker.h"

struct AbstractScalingWorkerHandler::Impl
{
    AbstractScalingWorkerHandler *handler;
    QMutex handlerMutex;
    bool isRunning;
    bool destroyOnFinished;
    AbstractScalingWorker *worker;
    QThread *thread;

    Impl(AbstractScalingWorkerHandler *handler, AbstractScalingWorker *worker, QThread *thread)
        : handler(handler)
        , isRunning(false)
        , destroyOnFinished(false)
        , worker(worker)
        , thread(thread)
    {}

    void checkAndDestroy()
    {
        if(!destroyOnFinished)
            return;

        worker->disconnect();
        thread->disconnect();
        handler->disconnect();

        thread->quit();
        thread->wait();
        delete worker;
        thread->deleteLater();
        handler->deleteLater();

        destroyOnFinished = false;
    }
};

AbstractScalingWorkerHandler::AbstractScalingWorkerHandler(AbstractScalingWorker *worker, QThread *thread, QObject *parent)
    : QObject(parent)
    , m_impl(new Impl(this, worker, thread))
{
    connect(worker, SIGNAL(started()), this, SLOT(onStartedReceived()));
    connect(worker, SIGNAL(finished()), this, SLOT(onFinishedReceived()));
    connect(worker, SIGNAL(aborted()), this, SLOT(onAbortedReceived()));
}

AbstractScalingWorkerHandler::~AbstractScalingWorkerHandler()
{}

bool AbstractScalingWorkerHandler::isRunning() const
{
    return m_impl->isRunning;
}

void AbstractScalingWorkerHandler::setDestroyOnFinished()
{
    QMutexLocker guard(&m_impl->handlerMutex);
    prepareTermination();
    m_impl->destroyOnFinished = true;
    if(!isRunning())
        m_impl->checkAndDestroy();
}

void AbstractScalingWorkerHandler::onStarted()
{}

void AbstractScalingWorkerHandler::onFinished()
{}

void AbstractScalingWorkerHandler::onAborted()
{}

void AbstractScalingWorkerHandler::prepareTermination()
{}

void AbstractScalingWorkerHandler::onStartedReceived()
{
    QMutexLocker guard(&m_impl->handlerMutex);
    onStarted();
    m_impl->isRunning = true;
}

void AbstractScalingWorkerHandler::onFinishedReceived()
{
    QMutexLocker guard(&m_impl->handlerMutex);
    m_impl->isRunning = false;
    m_impl->thread->quit();
    onFinished();
    m_impl->checkAndDestroy();
}

void AbstractScalingWorkerHandler::onAbortedReceived()
{
    QMutexLocker guard(&m_impl->handlerMutex);
    m_impl->isRunning = false;
    m_impl->thread->quit();
    onAborted();
    m_impl->checkAndDestroy();
}
