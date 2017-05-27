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

#include "AbstractAnimationProvider.h"

#include <QPixmap>

AbstractAnimationProvider::Frame::Frame()
    : delay(-1)
{}

AbstractAnimationProvider::Frame::Frame(int width, int height, int delay)
    : image(width, height, QImage::Format_ARGB32)
    , delay(delay)
{}

AbstractAnimationProvider::Frame::Frame(const QImage &image, int delay)
    : image(image)
    , delay(delay)
{}

AbstractAnimationProvider::~AbstractAnimationProvider()
{}

bool AbstractAnimationProvider::isValid() const
{
    return !m_error;
}

bool AbstractAnimationProvider::isSingleFrame() const
{
    return m_numFrames == 1;
}

int AbstractAnimationProvider::nextImageDelay() const
{
    return m_frames[m_currentFrame].delay;
}

bool AbstractAnimationProvider::jumpToNextImage()
{
    m_currentFrame++;
    if(m_currentFrame == m_numFrames)
    {
        m_currentFrame = 0;
        m_currentLoop++;
    }
    return m_numLoops <= 0 || m_currentLoop <= m_numLoops;
}

QPixmap AbstractAnimationProvider::currentPixmap() const
{
    return QPixmap::fromImage(m_frames[m_currentFrame].image);
}

AbstractAnimationProvider::AbstractAnimationProvider()
    : m_numFrames(1)
    , m_numLoops(0)
    , m_error(true)
    , m_currentFrame(0)
    , m_currentLoop(0)
{}
