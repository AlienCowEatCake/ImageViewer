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

#include "AnimationUtils.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QPixmap>

#include "IAnimationProvider.h"
#include "AnimationWidget.h"
#include "AnimationObject.h"

namespace AnimationUtils {

QGraphicsItem *CreateGraphicsItem(IAnimationProvider *provider)
{
    if(!provider || !provider->isValid())
    {
        if(provider)
            delete provider;
        return NULL;
    }

    if(provider->isSingleFrame())
    {
        const QPixmap pixmap = provider->currentPixmap();
        delete provider;
        if(!pixmap.isNull())
            return new QGraphicsPixmapItem(pixmap);
        return NULL;
    }

    AnimationWidget *widget = new AnimationWidget();
    widget->setAttribute(Qt::WA_NoSystemBackground, true);
    widget->setAnimationProvider(provider);
    QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget();
    proxy->setWidget(widget);
    return proxy;
}

} // namespace AnimationUtils
