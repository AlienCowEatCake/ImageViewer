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

#include "FramesCompositor.h"

#include <cassert>
#include <vector>

struct FramesCompositor::Impl
{
    Impl()
        : isFirstFrame(true)
    {}

    void blendOver(uchar *dst, const uchar *src, std::size_t blendOffsetX, std::size_t blendOffsetY,
                   std::size_t blendWidth, std::size_t blendHeight, std::size_t dataWidth)
    {
        for(std::size_t j = 0; j < blendHeight; j++)
        {
            const std::size_t offset = ((j + blendOffsetY) * dataWidth + blendOffsetX) * 4;
            const QRgb *sp = reinterpret_cast<const QRgb*>(src + offset);
            QRgb *dp = reinterpret_cast<QRgb*>(dst + offset);
            for(std::size_t i = 0; i < blendWidth; i++, sp++, dp++)
            {
                if(qAlpha(*sp) == 255)
                {
                    *dp = *sp;
                }
                else if(qAlpha(*sp) != 0)
                {
                    if(qAlpha(*dp) != 0)
                    {
                        int u = qAlpha(*sp) * 255;
                        int v = (255 - qAlpha(*sp)) * qAlpha(*dp);
                        int al = u + v;
                        int r = (qRed(*sp)   * u + qRed(*dp)   * v) / al;
                        int g = (qGreen(*sp) * u + qGreen(*dp) * v) / al;
                        int b = (qBlue(*sp)  * u + qBlue(*dp)  * v) / al;
                        int a = al / 255;
                        *dp = qRgba(r, g, b, a);
                    }
                    else
                    {
                        *dp = *sp;
                    }
                }
            }
        }
    }

    QImage previousFrame;
    QSize size;
    bool isFirstFrame;
};

FramesCompositor::FramesCompositor()
    : m_impl(new Impl)
{}

FramesCompositor::~FramesCompositor()
{}

void FramesCompositor::startComposition(const QSize &size)
{
    m_impl->previousFrame = QImage(size, QImage::Format_ARGB32);
    m_impl->previousFrame.fill(Qt::transparent);
    m_impl->isFirstFrame = true;
    m_impl->size = m_impl->previousFrame.size();
}

void FramesCompositor::startComposition(std::size_t width, std::size_t height)
{
    startComposition(QSize(static_cast<int>(width), static_cast<int>(height)));
}

bool FramesCompositor::isStarted() const
{
    return !m_impl->size.isEmpty();
}

QImage FramesCompositor::compositeFrame(const QImage &frame, const QRect &rect, FramesCompositor::DisposeType dispose, FramesCompositor::BlendType blend)
{
    assert(frame.format() == QImage::Format_ARGB32 || frame.format() == QImage::Format_RGB32);
    if(!isStarted())
        return frame;

    const std::size_t width = static_cast<std::size_t>(m_impl->size.width());
    const std::size_t height = static_cast<std::size_t>(m_impl->size.height());
    const std::size_t dataSize = width * height * 4;
    const std::size_t nextFrameWidth = static_cast<std::size_t>(rect.width());
    const std::size_t nextFrameHeight = static_cast<std::size_t>(rect.height());
    const std::size_t nextFrameOffsetX = static_cast<std::size_t>(rect.x());
    const std::size_t nextFrameOffsetY = static_cast<std::size_t>(rect.y());

    if(m_impl->isFirstFrame)
    {
        blend = BLEND_NONE;
        if(dispose == DISPOSE_PREVIOUS)
            dispose = DISPOSE_BACKGROUND;
        m_impl->isFirstFrame = false;
    }

    QImage image;
    if(frame.size() == m_impl->size)
    {
        image = frame;
    }
    else if(frame.size() == rect.size() && m_impl->previousFrame.rect().contains(rect))
    {
        image = QImage(m_impl->size, QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        for(std::size_t j = 0; j < nextFrameHeight; j++)
        {
            const std::size_t imageOffset = ((j + nextFrameOffsetY) * width + nextFrameOffsetX) * 4;
            const std::size_t frameOffset = j * nextFrameWidth * 4;
            memcpy(image.bits() + imageOffset, frame.bits() + frameOffset, nextFrameWidth * 4);
        }
    }
    else
    {
        assert(false);
    }

    std::vector<uchar> tempBuffer(dataSize);

    uchar *imageBits = image.bits();
    uchar *tempBits = &(tempBuffer[0]);
    uchar *prevBits = m_impl->previousFrame.bits();

    if(dispose == DISPOSE_PREVIOUS)
        memcpy(tempBits, prevBits, dataSize);

    if(blend == BLEND_OVER)
    {
        m_impl->blendOver(prevBits, imageBits, nextFrameOffsetX, nextFrameOffsetY, nextFrameWidth, nextFrameHeight, width);
    }
    else
    {
        for(std::size_t j = 0; j < nextFrameHeight; j++)
        {
            const std::size_t offset = ((j + nextFrameOffsetY) * width + nextFrameOffsetX) * 4;
            memcpy(prevBits + offset, imageBits + offset, nextFrameWidth * 4);
        }
    }

    memcpy(imageBits, prevBits, dataSize);

    if(dispose == DISPOSE_PREVIOUS)
    {
        memcpy(prevBits, tempBits, dataSize);
    }
    else if(dispose == DISPOSE_BACKGROUND)
    {
        for(std::size_t j = 0; j < nextFrameHeight; j++)
        {
            const std::size_t offset = ((j + nextFrameOffsetY) * width + nextFrameOffsetX) * 4;
            memset(prevBits + offset, 0, nextFrameWidth * 4);
        }
    }

    return image;
}
