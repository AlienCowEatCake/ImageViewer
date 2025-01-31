/*
   Copyright (C) 2019-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <QImage>
#include <QFileInfo>

#include <nanosvg.h>
#include <nanosvgrast.h>

#include "Utils/Global.h"
#include "Utils/Logging.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"
#include "Internal/Scaling/IScaledImageProvider.h"
#include "Internal/Utils/MappedBuffer.h"

namespace {

const qreal MAX_IMAGE_DIMENSION = 16384;
const qreal MIN_IMAGE_DIMENSION = 1;

class NanoSVGPixmapProvider : public IScaledImageProvider
{
public:
    explicit NanoSVGPixmapProvider(const QString &filePath)
        : m_image(Q_NULLPTR)
        , m_rasterizer(Q_NULLPTR)
        , m_width(0)
        , m_height(0)
        , m_minScaleFactor(1)
        , m_maxScaleFactor(1)
    {
        QByteArray inBuffer = MappedBuffer(filePath, MappedBuffer::AutoInflate | MappedBuffer::AutoConvertXmlToUtf8).byteArray();
        if(inBuffer.isEmpty())
        {
            LOG_WARNING() << LOGGING_CTX << "Can't read" << filePath;
            return;
        }

        inBuffer += '\0'; // Must be null terminated.

        m_image = nsvgParse(inBuffer.data(), "px", 96.0f);
        if(!m_image)
        {
            LOG_WARNING() << LOGGING_CTX << "Could not open SVG image";
            return;
        }

        m_rasterizer = nsvgCreateRasterizer();
        if(!m_rasterizer)
        {
            LOG_WARNING() << LOGGING_CTX << "Could not init rasterizer";
            return;
        }

        m_width = static_cast<int>(m_image->width);
        m_height = static_cast<int>(m_image->height);
        m_minScaleFactor = std::max(MIN_IMAGE_DIMENSION / m_width, MIN_IMAGE_DIMENSION / m_height);
        m_maxScaleFactor = std::min(MAX_IMAGE_DIMENSION / m_width, MAX_IMAGE_DIMENSION / m_height);
    }

    ~NanoSVGPixmapProvider()
    {
        if(m_rasterizer)
            nsvgDeleteRasterizer(m_rasterizer);
        if(m_image)
            nsvgDelete(m_image);
    }

    bool isValid() const Q_DECL_OVERRIDE
    {
        return m_image && m_rasterizer;
    }

    bool requiresMainThread() const Q_DECL_OVERRIDE
    {
        return false;
    }

    QImage image(const qreal scaleFactor) Q_DECL_OVERRIDE
    {
#define USE_RGBA_8888 (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
        const int w = static_cast<int>(m_width * scaleFactor);
        const int h = static_cast<int>(m_height * scaleFactor);
        QImage img(w, h,
#if (USE_RGBA_8888)
                   QImage::Format_RGBA8888);
#else
                   QImage::Format_ARGB32);
#endif

        if(img.isNull())
        {
            LOG_WARNING() << LOGGING_CTX << "Invalid image size";
            return img;
        }
        nsvgRasterize(m_rasterizer, m_image, 0, 0, static_cast<float>(scaleFactor), img.bits(), w, h, img.bytesPerLine());
#if (!USE_RGBA_8888)
#if (Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
        QImage_rgbSwap(img);
#else
        for(int y = 0; y < img.height(); ++y)
        {
            QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(y));
            for(int x = 0; x < img.width(); ++x)
                line[x] = qRgba(qAlpha(line[x]), qRed(line[x]), qGreen(line[x]), qBlue(line[x]));
        }
#endif
#endif
        return img;
#undef USE_RGBA_8888
    }

    QRectF boundingRect() const Q_DECL_OVERRIDE
    {
        return QRectF(0, 0, m_width, m_height);
    }

    qreal minScaleFactor() const Q_DECL_OVERRIDE
    {
        return m_minScaleFactor;
    }

    qreal maxScaleFactor() const Q_DECL_OVERRIDE
    {
        return m_maxScaleFactor;
    }

private:
    NSVGimage *m_image;
    NSVGrasterizer *m_rasterizer;
    int m_width;
    int m_height;
    qreal m_minScaleFactor;
    qreal m_maxScaleFactor;
};

class DecoderNanoSVG : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderNanoSVG");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
                << QString::fromLatin1("svg")
#if defined (HAS_ZLIB)
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
        return true;
    }

    QSharedPointer<IImageData> loadImage(const QString &filePath) Q_DECL_OVERRIDE
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return QSharedPointer<IImageData>();
        IScaledImageProvider *provider = new NanoSVGPixmapProvider(filePath);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createScalableItem(provider);
        IImageMetaData *metaData = item ? ImageMetaData::createMetaData(filePath) : Q_NULLPTR;
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), metaData));
    }
};

DecoderAutoRegistrator registrator(new DecoderNanoSVG);

} // namespace
