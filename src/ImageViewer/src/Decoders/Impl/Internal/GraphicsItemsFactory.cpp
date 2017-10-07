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

#include "GraphicsItemsFactory.h"

#include <QGraphicsItem>
#include <QGraphicsProxyWidget>
#include <QImage>
#include <QPixmap>

#include "Animation/IAnimationProvider.h"
#include "Animation/AnimationWidget.h"
#include "Animation/AnimationObject.h"
#include "Scaling/IScaledImageProvider.h"
#include "GraphicsItems/RasterizedImageGraphicsItem.h"
#include "GraphicsItems/ResampledImageGraphicsItem.h"

GraphicsItemsFactory &GraphicsItemsFactory::instance()
{
    static GraphicsItemsFactory factory;
    return factory;
}

QGraphicsItem *GraphicsItemsFactory::createImageItem(const QImage &image)
{
    if(image.isNull())
        return NULL;
    return new ResampledImageGraphicsItem(image);
}

QGraphicsItem *GraphicsItemsFactory::createPixmapItem(const QPixmap &pixmap)
{
    if(pixmap.isNull())
        return NULL;
    return new ResampledImageGraphicsItem(pixmap);
}

QGraphicsItem *GraphicsItemsFactory::createAnimatedItem(IAnimationProvider *animationProvider)
{
    if(!animationProvider || !animationProvider->isValid())
    {
        if(animationProvider)
            delete animationProvider;
        return NULL;
    }

    if(animationProvider->isSingleFrame())
    {
        const QImage image = animationProvider->currentImage();
        delete animationProvider;
        return createImageItem(image);
    }

    AnimationWidget *widget = new AnimationWidget();
    widget->setStyleSheet(QString::fromLatin1("background: transparent; border: none;"));
    widget->setAnimationProvider(animationProvider);
    QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget();
    proxy->setWidget(widget);
    return proxy;
}

QGraphicsItem *GraphicsItemsFactory::createScalableItem(IScaledImageProvider *scaledImageProvider)
{
    if(!scaledImageProvider->isValid())
        return NULL;
    return new RasterizedImageGraphicsItem(scaledImageProvider);
}

GraphicsItemsFactory::GraphicsItemsFactory()
{}

GraphicsItemsFactory::~GraphicsItemsFactory()
{}
