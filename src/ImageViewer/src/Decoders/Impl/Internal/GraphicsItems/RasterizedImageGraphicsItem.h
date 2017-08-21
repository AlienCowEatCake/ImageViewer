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
#include "Utils/SharedPointer.h"

class RasterizedImageGraphicsItem : public QGraphicsItem
{
public:
    class IRasterizedImageProvider
    {
    public:
        virtual ~IRasterizedImageProvider();
        virtual bool isValid() const = 0;
        virtual QImage image(const qreal scaleFactor) = 0;
        virtual QRectF boundingRect() const = 0;
        virtual qreal minScaleFactor() const = 0;
        virtual qreal maxScaleFactor() const = 0;
    };

    RasterizedImageGraphicsItem(QGraphicsItem *parentItem = NULL);
    RasterizedImageGraphicsItem(QSharedPointer<IRasterizedImageProvider> provider, QGraphicsItem *parentItem = NULL);
    ~RasterizedImageGraphicsItem();

    QSharedPointer<IRasterizedImageProvider> provider() const;
    void setProvider(QSharedPointer<IRasterizedImageProvider> provider);

    Qt::TransformationMode transformationMode() const;
    void setTransformationMode(Qt::TransformationMode mode);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = NULL);

private:
    Q_DISABLE_COPY(RasterizedImageGraphicsItem)
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // RASTERIZED_IMAGE_GRAPHICS_ITEM_H_INCLUDED
