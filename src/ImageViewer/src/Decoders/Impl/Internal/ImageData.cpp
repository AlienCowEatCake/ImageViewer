/*
   Copyright (C) 2018-2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "../../IImageMetaData.h"

ImageData::ImageData()
    : m_graphicsItem(Q_NULLPTR)
    , m_metaData(Q_NULLPTR)
{}

ImageData::ImageData(QGraphicsItem *graphicsItem, const QString &filePath, const QString &decoderName, IImageMetaData *metaData)
    : m_graphicsItem(graphicsItem)
    , m_decoderName(decoderName)
    , m_filePath(filePath)
    , m_size(graphicsItem ? graphicsItem->boundingRect().toAlignedRect().size() : QSize())
    , m_metaData(metaData)
{}

ImageData::~ImageData()
{
    if(m_graphicsItem && !m_graphicsItem->scene())
        delete m_graphicsItem;
    if(m_metaData)
        delete m_metaData;
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

QString ImageData::filePath() const
{
    return m_filePath;
}

QSize ImageData::size() const
{
    return m_size;
}

IImageMetaData *ImageData::metaData() const
{
    return m_metaData;
}
