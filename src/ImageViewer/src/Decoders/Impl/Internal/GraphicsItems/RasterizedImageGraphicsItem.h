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

#if !defined(RASTERIZED_IMAGE_GRAPHICS_ITEM_H_INCLUDED)
#define RASTERIZED_IMAGE_GRAPHICS_ITEM_H_INCLUDED

#include <QGraphicsItem>
#include <QImage>

#include "Utils/Global.h"
#include "Utils/ScopedPointer.h"

#include "../../../GraphicsItemFeatures/IGrabImage.h"
#include "../../../GraphicsItemFeatures/ITransformationMode.h"

class IScaledImageProvider;

class RasterizedImageGraphicsItem :
        public QGraphicsItem,
        public ITransformationMode,
        public IGrabImage
{
    Q_DISABLE_COPY(RasterizedImageGraphicsItem)

public:
    explicit RasterizedImageGraphicsItem(QGraphicsItem *parentItem = Q_NULLPTR);
    RasterizedImageGraphicsItem(IScaledImageProvider *provider, QGraphicsItem *parentItem = Q_NULLPTR);
    ~RasterizedImageGraphicsItem();

    IScaledImageProvider *provider() const;
    void setProvider(IScaledImageProvider *provider);

    Qt::TransformationMode transformationMode() const Q_DECL_OVERRIDE;
    void setTransformationMode(Qt::TransformationMode mode) Q_DECL_OVERRIDE;

    QImage grabImage() Q_DECL_OVERRIDE;

    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) Q_DECL_OVERRIDE;

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // RASTERIZED_IMAGE_GRAPHICS_ITEM_H_INCLUDED
