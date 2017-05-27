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

#include "AnimationObject.h"

#include <algorithm>

#include <QImage>
#include <QPixmap>
#include <QTimer>
#include <QTime>

#include "IAnimationProvider.h"

struct AnimationObject::Impl
{
    Impl(AnimationObject *animation)
        : animation(animation)
        , provider(NULL)
        , nextImageDelay(-1)
    {
        nextImageTimer.setSingleShot(true);
        QObject::connect(&nextImageTimer, SIGNAL(timeout()), animation, SLOT(loadNextFrame()));
    }

    ~Impl()
    {
        cleanup();
    }

    bool jumpToNextImage()
    {
        if(provider && provider->jumpToNextImage())
        {
            nextImageDelay = provider->nextImageDelay();
            currentPixmap = provider->currentPixmap();
            return true;
        }
        nextImageDelay = -1;
        currentPixmap = QPixmap();
        return false;
    }

    void cleanup()
    {
        nextImageTimer.stop();
        if(provider)
            delete provider;
        provider = NULL;
        currentPixmap = QPixmap();
        nextImageDelay = -1;
    }

    AnimationObject *animation;
    IAnimationProvider *provider;
    QPixmap currentPixmap;
    QTimer nextImageTimer;
    int nextImageDelay;
};

AnimationObject::AnimationObject(QObject *parent)
    : QObject(parent)
    , m_impl(new Impl(this))
{}

AnimationObject::AnimationObject(IAnimationProvider *provider, QObject *parent)
    : QObject(parent)
    , m_impl(new Impl(this))
{
    setProvider(provider);
}

AnimationObject::~AnimationObject()
{}

void AnimationObject::setProvider(IAnimationProvider *provider)
{
    m_impl->cleanup();
    if(!provider || !provider->isValid())
    {
        if(provider)
            delete provider;
        return;
    }
    m_impl->provider = provider;
    m_impl->currentPixmap = provider->currentPixmap();
    m_impl->nextImageDelay = provider->nextImageDelay();
    if(m_impl->nextImageDelay >= 0)
        m_impl->nextImageTimer.start(m_impl->nextImageDelay);
    emit updated();
}

IAnimationProvider *AnimationObject::provider() const
{
    return m_impl->provider;
}

QPixmap AnimationObject::currentPixmap() const
{
    return m_impl->currentPixmap;
}

bool AnimationObject::isValid() const
{
    return true
            && m_impl->provider
            && m_impl->provider->isValid()
            && !m_impl->currentPixmap.isNull();
}

void AnimationObject::loadNextFrame()
{
    QTime time;
    time.start();
    const bool processingStatus = m_impl->jumpToNextImage();
    const int processingTime = time.elapsed();
    if(processingStatus)
    {
        if(m_impl->nextImageDelay >= 0)
            m_impl->nextImageTimer.start(std::max(0, m_impl->nextImageDelay - processingTime));
        emit updated();
    }
}
