/*
   Copyright (C) 2018-2021 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "../../Impl/Internal/ImageMetaData.h"

namespace {

QPair<qreal, qreal> getDpi(IImageMetaData *metaData)
{
    if(metaData)
    {
        const QPair<qreal, qreal> result = metaData->dpi();
        if(result.first > 0 && result.second > 0)
            return result;
    }
    return ImageMetaData().dpi();
}

}

ImageData::ImageData()
    : m_graphicsItem(Q_NULLPTR)
    , m_dpi(getDpi(Q_NULLPTR))
    , m_metaData(Q_NULLPTR)
{}

ImageData::ImageData(QGraphicsItem *graphicsItem, const QString &filePath, const QString &decoderName, IImageMetaData *metaData)
    : m_graphicsItem(graphicsItem)
    , m_decoderName(decoderName)
    , m_filePath(filePath)
    , m_size(graphicsItem ? graphicsItem->boundingRect().toAlignedRect().size() : QSize())
    , m_dpi(getDpi(metaData))
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

QPair<qreal, qreal> ImageData::dpi() const
{
    return m_dpi;
}

IImageMetaData *ImageData::metaData() const
{
    return m_metaData;
}
