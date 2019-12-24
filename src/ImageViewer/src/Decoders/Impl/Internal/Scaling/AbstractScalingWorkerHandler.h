/*
   Copyright (C) 2017-2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined(ABSTRACT_SCALING_WORKER_HANDLER_H_INCLUDED)
#define ABSTRACT_SCALING_WORKER_HANDLER_H_INCLUDED

#include <QObject>

#include "Utils/ScopedPointer.h"

class QThread;
class AbstractScalingWorker;

class AbstractScalingWorkerHandler : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AbstractScalingWorkerHandler)

public:
    AbstractScalingWorkerHandler(AbstractScalingWorker *worker, QThread *thread);
    ~AbstractScalingWorkerHandler();

    bool isRunning() const;

    void setDestroyOnFinished();

protected:
    virtual void onStarted();
    virtual void onFinished();
    virtual void onAborted();
    virtual void prepareTermination();

private Q_SLOTS:
    void onStartedReceived();
    void onFinishedReceived();
    void onAbortedReceived();

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // ABSTRACT_SCALING_WORKER_HANDLER_H_INCLUDED
