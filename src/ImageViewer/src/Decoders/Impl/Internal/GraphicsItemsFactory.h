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

#if !defined(GRAPHICS_ITEMS_FACTORY_H_INCLUDED)
#define GRAPHICS_ITEMS_FACTORY_H_INCLUDED

#include <QtGlobal>

class QGraphicsItem;
class QImage;
class QPixmap;
class IAnimationProvider;
class IScaledImageProvider;

class GraphicsItemsFactory
{
    Q_DISABLE_COPY(GraphicsItemsFactory)

public:
    static GraphicsItemsFactory &instance();

    QGraphicsItem *createImageItem(const QImage &image);
    QGraphicsItem *createPixmapItem(const QPixmap &pixmap);
    QGraphicsItem *createAnimatedItem(IAnimationProvider *animationProvider);
    QGraphicsItem *createScalableItem(IScaledImageProvider *scaledImageProvider);

private:
    GraphicsItemsFactory();
    ~GraphicsItemsFactory();
};

#endif // GRAPHICS_ITEMS_FACTORY_H_INCLUDED
