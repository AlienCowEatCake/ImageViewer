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

#if !defined(ABSTRACT_ANIMATION_PROVIDER_H_INCLUDED)
#define ABSTRACT_ANIMATION_PROVIDER_H_INCLUDED

#include "IAnimationProvider.h"

#include <QImage>
#include <QVector>

class AbstractAnimationProvider : public IAnimationProvider
{
public:
    struct Frame
    {
        Frame();
        Frame(int width, int height, int delay = -1);
        Frame(const QImage &image, int delay = -1);
        QImage image;
        int delay;
    };

    ~AbstractAnimationProvider();

    bool isValid() const;
    bool isSingleFrame() const;
    int nextImageDelay() const;
    bool jumpToNextImage();
    QPixmap currentPixmap() const;

protected:
    AbstractAnimationProvider();

    QVector<Frame> m_frames;
    int m_numFrames;
    int m_numLoops;
    bool m_error;

private:
    int m_currentFrame;
    int m_currentLoop;
};

#endif // ABSTRACT_ANIMATION_PROVIDER_H_INCLUDED
