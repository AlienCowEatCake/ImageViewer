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

#if !defined(ABSTRACT_SCALING_MANAGER_H_INCLUDED)
#define ABSTRACT_SCALING_MANAGER_H_INCLUDED

#include <QImage>

#include "Utils/ScopedPointer.h"

class QThread;
class AbstractScalingWorker;
class AbstractScalingWorkerHandler;

class AbstractScalingManager
{
    Q_DISABLE_COPY(AbstractScalingManager)

public:
    AbstractScalingManager(AbstractScalingWorker *worker, AbstractScalingWorkerHandler *handler, QThread *thread);
    ~AbstractScalingManager();

    void startTask(const qreal scaleFactor);
    void abortTask();

    void beginScaledImageProcessing();
    void endScaledImageProcessing();

    bool hasScaledData() const;
    QImage getScaledImage() const;
    qreal getScaledScaleFactor() const;

protected:
    AbstractScalingWorker *m_scalingWorker;
    QThread *m_scalingThread;
    AbstractScalingWorkerHandler *m_scalingWorkerHandler;
};

#endif // ABSTRACT_SCALING_MANAGER_H_INCLUDED
