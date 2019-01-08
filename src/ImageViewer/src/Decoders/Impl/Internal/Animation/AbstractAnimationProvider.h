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

#if !defined(ABSTRACT_ANIMATION_PROVIDER_H_INCLUDED)
#define ABSTRACT_ANIMATION_PROVIDER_H_INCLUDED

#include "IAnimationProvider.h"

#include <QImage>
#include <QVector>

#include "Utils/Global.h"

class AbstractAnimationProvider : public IAnimationProvider
{
    Q_DISABLE_COPY(AbstractAnimationProvider)

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

    bool isValid() const Q_DECL_OVERRIDE;
    bool isSingleFrame() const Q_DECL_OVERRIDE;
    int nextImageDelay() const Q_DECL_OVERRIDE;
    bool jumpToNextImage() Q_DECL_OVERRIDE;
    QPixmap currentPixmap() const Q_DECL_OVERRIDE;
    QImage currentImage() const Q_DECL_OVERRIDE;

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
