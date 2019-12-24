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

#if !defined(ABSTRACT_SCALING_WORKER_H_INCLUDED)
#define ABSTRACT_SCALING_WORKER_H_INCLUDED

#include <QObject>
#include <QImage>
#include <QMutex>

#include "Utils/ScopedPointer.h"

class AbstractScalingWorker : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AbstractScalingWorker)

public:
    AbstractScalingWorker();
    ~AbstractScalingWorker();

    void setScaleFactor(const qreal newScaleFactor);
    qreal getScaleFactor() const;

    void lockScaledImage();
    void unlockScaledImage();

    bool hasScaledData() const;
    QImage getScaledImage() const;
    qreal getScaledScaleFactor() const;
    qint64 getScaledDataId() const;

public Q_SLOTS:
    void process();
    void abort();

Q_SIGNALS:
    void started();
    void finished();
    void aborted();

protected:
    bool isAborted() const;
    virtual bool scaleImpl() = 0;

    qreal m_scaleFactor;
    struct ScaledImageData
    {
        QImage image;
        qreal scaleFactor;
        ScaledImageData(const QImage& image, const qreal scaleFactor);
    };
    QScopedPointer<ScaledImageData> m_scaledData;

private:
    bool m_workerAborted;
    QMutex m_scaledDataLock;
};

#endif // ABSTRACT_SCALING_WORKER_H_INCLUDED
