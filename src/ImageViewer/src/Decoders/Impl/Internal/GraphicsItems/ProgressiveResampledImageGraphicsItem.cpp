/*
   Copyright (C) 2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "ProgressiveResampledImageGraphicsItem.h"

#include <QPainter>
#include <QSize>
#include <QSizeF>
#include <QRect>
#include <QRectF>
#include <QPoint>
#include <QPointF>

#include "../Scaling/AbstractProgressiveImageProvider.h"

ProgressiveResampledImageGraphicsItem::ProgressiveResampledImageGraphicsItem(QGraphicsItem *parentItem)
    : ResampledImageGraphicsItem(parentItem)
{}

ProgressiveResampledImageGraphicsItem::ProgressiveResampledImageGraphicsItem(AbstractProgressiveImageProvider *provider, QGraphicsItem *parentItem)
    : ResampledImageGraphicsItem(parentItem)
{
    setProvider(provider);
}

ProgressiveResampledImageGraphicsItem::~ProgressiveResampledImageGraphicsItem()
{}

AbstractProgressiveImageProvider *ProgressiveResampledImageGraphicsItem::provider() const
{
    return m_provider.data();
}

void ProgressiveResampledImageGraphicsItem::setProvider(AbstractProgressiveImageProvider *provider)
{
    if(m_provider)
        disconnect(m_provider.data(), SIGNAL(updated()), this, SLOT(onUpdateRequested()));
    m_provider.reset(provider);
    if(m_provider)
        connect(m_provider.data(), SIGNAL(updated()), this, SLOT(onUpdateRequested()));
    onUpdateRequested();
}

QRectF ProgressiveResampledImageGraphicsItem::boundingRect() const
{
    return m_provider ? QRectF(QPoint(), m_provider->size()) : ResampledImageGraphicsItem::boundingRect();
}

void ProgressiveResampledImageGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if(!m_provider)
        return;
    if(m_provider->isFinal())
        return ResampledImageGraphicsItem::paint(painter, option, widget);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    const QImage image = m_provider->image();
    painter->drawImage(boundingRect(), image, image.rect());
}

void ProgressiveResampledImageGraphicsItem::onUpdateRequested()
{
    if(m_provider && m_provider->isFinal())
        setImage(m_provider->image());
    else
        update();
}
