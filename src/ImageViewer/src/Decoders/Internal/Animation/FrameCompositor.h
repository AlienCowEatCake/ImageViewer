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

#if !defined(FRAME_COMPOSITOR_H_INCLUDED)
#define FRAME_COMPOSITOR_H_INCLUDED

#include <QImage>
#include <QRect>
#include <QSize>

#include "Utils/ScopedPointer.h"

class FrameCompositor
{
public:
    enum DisposeType
    {
        DISPOSE_NONE,
        DISPOSE_BACKGROUND,
        DISPOSE_PREVIOUS
    };

    enum BlendType
    {
        BLEND_NONE,
        BLEND_OVER
    };

    FrameCompositor();
    ~FrameCompositor();

    void startComposition(const QSize& size);
    void startComposition(std::size_t width, std::size_t height);
    bool isStarted() const;

    QImage compositeFrame(const QImage &frame, const QRect &rect, DisposeType dispose = DISPOSE_NONE, BlendType blend = BLEND_NONE);

private:
    Q_DISABLE_COPY(FrameCompositor)
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // FRAME_COMPOSITOR_H_INCLUDED
