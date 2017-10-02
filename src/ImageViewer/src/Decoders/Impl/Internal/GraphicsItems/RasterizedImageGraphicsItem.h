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

#if !defined(RASTERIZED_IMAGE_GRAPHICS_ITEM_H_INCLUDED)
#define RASTERIZED_IMAGE_GRAPHICS_ITEM_H_INCLUDED

#include <QGraphicsItem>
#include <QImage>
#include "Utils/ScopedPointer.h"

class IScaledImageProvider;

class RasterizedImageGraphicsItem : public QGraphicsItem
{
    Q_DISABLE_COPY(RasterizedImageGraphicsItem)

public:
    RasterizedImageGraphicsItem(QGraphicsItem *parentItem = NULL);
    RasterizedImageGraphicsItem(IScaledImageProvider *provider, QGraphicsItem *parentItem = NULL);
    ~RasterizedImageGraphicsItem();

    IScaledImageProvider *provider() const;
    void setProvider(IScaledImageProvider *provider);

    Qt::TransformationMode transformationMode() const;
    void setTransformationMode(Qt::TransformationMode mode);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = NULL);

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // RASTERIZED_IMAGE_GRAPHICS_ITEM_H_INCLUDED
