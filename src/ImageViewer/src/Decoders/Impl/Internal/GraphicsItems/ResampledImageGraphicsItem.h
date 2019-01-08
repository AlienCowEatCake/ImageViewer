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

#if !defined(RESAMPLED_IMAGE_GRAPHICS_ITEM_H_INCLUDED)
#define RESAMPLED_IMAGE_GRAPHICS_ITEM_H_INCLUDED

#include <QGraphicsItem>
#include <QImage>
#include <QPixmap>

#include "Utils/Global.h"
#include "Utils/ScopedPointer.h"

class ResampledImageGraphicsItem : public QGraphicsItem
{
    Q_DISABLE_COPY(ResampledImageGraphicsItem)

public:
    ResampledImageGraphicsItem(QGraphicsItem *parentItem = Q_NULLPTR);
    ResampledImageGraphicsItem(const QImage &image, QGraphicsItem *parentItem = Q_NULLPTR);
    ResampledImageGraphicsItem(const QPixmap &pixmap, QGraphicsItem *parentItem = Q_NULLPTR);
    ~ResampledImageGraphicsItem();

    QImage image() const;
    void setImage(const QImage &image);

    QPixmap pixmap() const;
    void setPixmap(const QPixmap &pixmap);

    Qt::TransformationMode transformationMode() const;
    void setTransformationMode(Qt::TransformationMode mode);

    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) Q_DECL_OVERRIDE;

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // RESAMPLED_IMAGE_GRAPHICS_ITEM_H_INCLUDED
