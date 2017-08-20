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

#include "GraphicsItemUtils.h"

#include <algorithm>
#include <cmath>

#include <QPainter>
#include <QPixmap>
#include <QRectF>

namespace GraphicsItemUtils {

qreal GetDeviceScaleFactor(const QPainter *painter)
{
    const QRectF identityRect = QRectF(0, 0, 1, 1);
    const QRectF deviceTransformedRect = painter->deviceTransform().mapRect(identityRect);
    return std::max(deviceTransformedRect.width(), deviceTransformedRect.height());
}

bool IsFuzzyEqualScaleFactors(const qreal scaleFactor1, const qreal scaleFactor2)
{
    return std::abs(scaleFactor1 - scaleFactor2) / std::max(scaleFactor1, scaleFactor2) <= 1e-2;
}

void DrawScaledPixmap(QPainter *painter, const QPixmap &scaledPixmap, const QRectF &originalRect, const qreal scaleFactor)
{
    const QRectF scaledRect = QRectF(originalRect.topLeft() * scaleFactor, originalRect.size() * scaleFactor);
    painter->drawPixmap(originalRect, scaledPixmap, scaledRect);
}

} // GraphicsItemUtils
