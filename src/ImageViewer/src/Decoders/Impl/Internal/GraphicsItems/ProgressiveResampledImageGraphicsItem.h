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

#if !defined(PROGRESSIVE_RESAMPLED_IMAGE_GRAPHICS_ITEM_H_INCLUDED)
#define PROGRESSIVE_RESAMPLED_IMAGE_GRAPHICS_ITEM_H_INCLUDED

#include "ResampledImageGraphicsItem.h"

class AbstractProgressiveImageProvider;

class ProgressiveResampledImageGraphicsItem : public QObject, public ResampledImageGraphicsItem
{
    Q_OBJECT
    Q_DISABLE_COPY(ProgressiveResampledImageGraphicsItem)

public:
    ProgressiveResampledImageGraphicsItem(QGraphicsItem *parentItem = NULL);
    ProgressiveResampledImageGraphicsItem(AbstractProgressiveImageProvider *provider, QGraphicsItem *parentItem = NULL);
    ~ProgressiveResampledImageGraphicsItem();

    AbstractProgressiveImageProvider *provider() const;
    void setProvider(AbstractProgressiveImageProvider *provider);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = NULL);

private slots:
    void onUpdateRequested();

private:
    QScopedPointer<AbstractProgressiveImageProvider> m_provider;
};

#endif // PROGRESSIVE_RESAMPLED_IMAGE_GRAPHICS_ITEM_H_INCLUDED
