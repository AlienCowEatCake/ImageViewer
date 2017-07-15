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

#if !defined(IMAGE_RESAMPLER_WORKER_H_INCLUDED)
#define IMAGE_RESAMPLER_WORKER_H_INCLUDED

#include <QObject>
#include <QImage>

#include "Utils/ScopedPointer.h"

class ImageResamplerWorker : public QObject
{
    Q_OBJECT

public:
    ImageResamplerWorker(QObject *parent = NULL);
    ~ImageResamplerWorker();

    void setImage(const QImage &newImage);
    QImage getImage() const;

    void setScaleFactor(const qreal newScaleFactor);
    qreal getScaleFactor() const;

    void lockResampledImage();
    void unlockResampledImage();

    bool hasResampledData() const;
    QImage getResampledImage() const;
    qreal getResampledScaleFactor() const;

public slots:
    void process();
    void abort();

signals:
    void started();
    void finished();
    void aborted();

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

class ImageResamplerWorkerHandler : public QObject
{
    Q_OBJECT

public:
    ImageResamplerWorkerHandler(ImageResamplerWorker *worker, QObject *parent = NULL);

    virtual void onStarted() = 0;
    virtual void onFinished() = 0;
    virtual void onAborted() = 0;

private slots:
    void onStartedReceived();
    void onFinishedReceived();
    void onAbortedReceived();
};

#endif // IMAGE_RESAMPLER_WORKER_H_INCLUDED
