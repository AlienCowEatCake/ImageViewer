/*
   Copyright (C) 2017-2026 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <QtGlobal>
#if defined (QT_SVG_LIB)
#include <QGraphicsSvgItem>
#include <QSvgRenderer>
#else
#include <QGraphicsItem>
#endif
#include <QFileInfo>

#include "Utils/Global.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"
#include "Internal/Utils/MappedBuffer.h"
#include "Internal/Utils/XmlStreamReader.h"

namespace {

#if defined (QT_SVG_LIB)
class SVGGraphicsItem : public QGraphicsSvgItem
{
public:
    SVGGraphicsItem(const QString &filePath, QGraphicsItem *parentItem = Q_NULLPTR)
        : QGraphicsSvgItem(parentItem)
    {
        m_inBuffer.reset(new MappedBuffer(filePath, MappedBuffer::AutoInflate));
        if(!m_inBuffer->isValid())
            return;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        m_xmlStreamReader.reset(new XmlStreamReader(m_inBuffer->dataAsByteArray()));
        QSvgRenderer *svgRenderer = new QSvgRenderer(this);
        QObject::connect(svgRenderer, &QSvgRenderer::repaintNeeded, this, [this]{ update(); });
        svgRenderer->load(m_xmlStreamReader.data());
        setSharedRenderer(svgRenderer);
#else
        renderer()->load(m_inBuffer->dataAsByteArray());
        // For d->updateDefaultSize();
        setElementId(elementId());
#endif
    }

private:
    QScopedPointer<MappedBuffer> m_inBuffer;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QScopedPointer<XmlStreamReader> m_xmlStreamReader;
#endif
};
#endif

class DecoderQtSVG : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderQtSVG");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
#if defined (QT_SVG_LIB)
                << QString::fromLatin1("svg")
                << QString::fromLatin1("svgz")
#endif
                ;
    }

    QStringList advancedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList();
    }

    bool isAvailable() const Q_DECL_OVERRIDE
    {
#if defined (QT_SVG_LIB)
        return true;
#else
        return false;
#endif
    }

    QSharedPointer<IImageData> loadImage(const QString &filePath) Q_DECL_OVERRIDE
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return QSharedPointer<IImageData>();
#if defined (QT_SVG_LIB)
        SVGGraphicsItem *graphicsSvgItem = new SVGGraphicsItem(filePath);
        if(graphicsSvgItem->renderer()->isValid())
        {
            IImageMetaData *metaData = ImageMetaData::createMetaData(filePath);
            return QSharedPointer<IImageData>(new ImageData(graphicsSvgItem, filePath, name(), metaData));
        }
        delete graphicsSvgItem;
#endif
        return QSharedPointer<IImageData>();
    }
};

DecoderAutoRegistrator registrator(new DecoderQtSVG);

} // namespace
