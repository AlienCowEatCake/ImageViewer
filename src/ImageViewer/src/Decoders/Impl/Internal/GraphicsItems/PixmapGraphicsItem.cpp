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

#include "PixmapGraphicsItem.h"

#include <algorithm>

#include <QPainter>
#include <QImage>
#include <QPixmap>

#include "GraphicsItemUtils.h"

PixmapGraphicsItem::PixmapGraphicsItem(QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent)
{}

PixmapGraphicsItem::PixmapGraphicsItem(const QPixmap &pixmap, QGraphicsItem *parent)
    : QGraphicsPixmapItem(pixmap, parent)
{}

void PixmapGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    /// @note QPainter не умеет масштабирование изображений в меньшую сторону. Какой renderHint ему не ставь, хоть
    /// SmoothPixmapTransform, хоть FastTransformation, качество результата будет как при FastTransformation.
    /// https://bugreports.qt.io/browse/QTBUG-30682
    /// https://bugs.webkit.org/show_bug.cgi?id=119263

    if(pixmap().isNull() || transformationMode() != Qt::SmoothTransformation)
        return QGraphicsPixmapItem::paint(painter, option, widget);

    const qreal newScaleFactor = GraphicsItemUtils::GetDeviceScaleFactor(painter);
    const QSize originalPixmapSize = pixmap().size();
    const QSize scaledPixmapSize = originalPixmapSize * newScaleFactor;
    if(newScaleFactor >= 1 || newScaleFactor <= 0 || scaledPixmapSize == originalPixmapSize)
        return QGraphicsPixmapItem::paint(painter, option, widget);

    const QPixmap scaledPixmap = pixmap().scaled(scaledPixmapSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter->drawPixmap(pixmap().rect(), scaledPixmap, scaledPixmap.rect());
}
