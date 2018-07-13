/*
   Copyright (C) 2018 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "ImageData.h"

#include <QGraphicsItem>

ImageData::ImageData()
    : m_graphicsItem(NULL)
{}

ImageData::ImageData(QGraphicsItem *graphicsItem, const QString &decoderName)
    : m_graphicsItem(graphicsItem)
    , m_decoderName(decoderName)
    , m_size(graphicsItem ? graphicsItem->boundingRect().toAlignedRect().size() : QSize())
{}

ImageData::~ImageData()
{
    if(m_graphicsItem && !m_graphicsItem->scene())
        delete m_graphicsItem;
}

bool ImageData::isEmpty() const
{
    return !m_graphicsItem;
}

QGraphicsItem *ImageData::graphicsItem() const
{
    return m_graphicsItem;
}

QString ImageData::decoderName() const
{
    return m_decoderName;
}

QSize ImageData::size() const
{
    return m_size;
}
