/*
   Copyright (C) 2017-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        MappedBuffer inBuffer(filePath, MappedBuffer::AutoInflate);
        if(!inBuffer.isValid())
            return QSharedPointer<IImageData>();
        QGraphicsSvgItem *graphicsSvgItem = new QGraphicsSvgItem();
        QSvgRenderer* renderer = new QSvgRenderer(graphicsSvgItem);
        QObject::connect(renderer, &QSvgRenderer::repaintNeeded, graphicsSvgItem, [graphicsSvgItem]{ graphicsSvgItem->update(); });
        XmlStreamReader reader(inBuffer.dataAsByteArray());
        renderer->load(&reader);
        graphicsSvgItem->setSharedRenderer(renderer);
#else
        QGraphicsSvgItem *graphicsSvgItem = new QGraphicsSvgItem(filePath);
#endif
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
