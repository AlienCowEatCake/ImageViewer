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

#define USE_STB_IMAGE_RESIZE

#include "ResampledImageGraphicsItem.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

#include <QObject>
#include <QPainter>
#include <QPixmap>
#include <QThread>
#include <QDebug>

#if defined (HAS_STB)
#include <stb_image_resize.h>
#elif defined (USE_STB_IMAGE_RESIZE)
#undef USE_STB_IMAGE_RESIZE
#endif

#include "../Scaling/AbstractScalingManager.h"
#include "../Scaling/AbstractScalingWorker.h"
#include "../Scaling/AutoUpdatedScalingWorkerHandler.h"
#include "GraphicsItemUtils.h"

// ====================================================================================================

namespace {

#if !defined (USE_STB_IMAGE_RESIZE)

double filterKernel(const double x)
{
//    // Mitchell-Netrevalli filter with B=1/3, C=1/3
//    const double absX = std::abs(x);
//    if(absX < 1.0)
//        return (16.0 + absX * absX * (21.0 * absX - 36.0)) / 18.0;
//    if(absX < 2.0)
//        return (32.0 + absX * (-60.0 + absX * (36.0 - 7.0 * absX))) / 18;
//    return 0.0;

    // https://en.wikipedia.org/wiki/Bicubic_interpolation#Bicubic_convolution_algorithm
    const double absX = std::abs(x);
    const double sqareAbsX = absX * absX;
    const double a = -0.5;
    if(absX <= 1.0)
        return (a + 2.0) * sqareAbsX * absX - (a + 3) * sqareAbsX + 1;
    if(absX < 2.0)
        return a * sqareAbsX * absX - 5.0 * a * sqareAbsX + 8.0 * a * absX - 4.0 * a;
    return 0.0;
}

double filterDomain()
{
    return 2.0;
}

const double FILTER_CUTOFF_TRESHOLD = 0.02;

struct WeightedCoordinate
{
    std::size_t coordinate;
    double weight;

    WeightedCoordinate(std::size_t coordinate = 0, double weight = 0.0)
        : coordinate(coordinate)
        , weight(weight)
    {}
};

#endif

class ResamplerWorker : public AbstractScalingWorker
{
public:
    void setImage(const QImage &newImage)
    {
        static const QList<QImage::Format> nativeImageFormats = QList<QImage::Format>()
#if !defined (USE_STB_IMAGE_RESIZE)
                /// @todo Через STB не работает, если stride != 0. Разобраться почему.
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
                << QImage::Format_Grayscale8
                << QImage::Format_Alpha8
#endif
                << QImage::Format_RGB888
#endif
                << QImage::Format_RGB32
                << QImage::Format_ARGB32
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
                << QImage::Format_RGBX8888
                << QImage::Format_RGBA8888
#endif
                   ;
        if(nativeImageFormats.contains(newImage.format()))
            m_image = newImage;
        else
            m_image = newImage.convertToFormat(QImage::Format_ARGB32);
    }

    QImage getImage() const
    {
        return m_image;
    }

private:
    bool scaleImpl()
    {
#define CHECK_ABORT_STATE if(isAborted()) return false
        CHECK_ABORT_STATE;
        const qreal newScaleFactor = m_scaleFactor;
        const QSize scaledImageSize = m_image.size() * newScaleFactor;
        CHECK_ABORT_STATE;
        QImage scaledImage(scaledImageSize, m_image.format());

        CHECK_ABORT_STATE;
        int numChannels = 0;
        switch(m_image.format())
        {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
        case QImage::Format_Grayscale8:
        case QImage::Format_Alpha8:
            numChannels = 1;
            break;
#endif
        case QImage::Format_RGB888:
            numChannels = 3;
            break;
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32:
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
        case QImage::Format_RGBX8888:
        case QImage::Format_RGBA8888:
#endif
            numChannels = 4;
            break;
        default:
            qWarning() << "[ImageResamplerWorker::scaleImpl()]: Unsupported image format";
            return false;
        }

#if defined (USE_STB_IMAGE_RESIZE)

        CHECK_ABORT_STATE;
        const int inputStride = m_image.bytesPerLine() - m_image.width() * numChannels;
        const int outputStride = scaledImage.bytesPerLine() - scaledImageSize.width() * numChannels;

        /// @todo Не работает, если stride != 0. Разобраться почему.
        assert(inputStride == 0);
        assert(outputStride == 0);

        CHECK_ABORT_STATE;
        stbir_resize_uint8(m_image.bits(), m_image.width(), m_image.height(), inputStride,
                           scaledImage.bits(), scaledImageSize.width(), scaledImageSize.height(), outputStride, numChannels);

#else

        assert(newScaleFactor < 1);
        if(newScaleFactor >= 1)
            return false;

        CHECK_ABORT_STATE;
        const QRect scaledExposedRect = QRect(QPoint(0, 0), scaledImageSize); /// @todo Exposed?
        const std::size_t outWidth = static_cast<std::size_t>(scaledExposedRect.width());
        const std::size_t outHeight = static_cast<std::size_t>(scaledExposedRect.height());

        CHECK_ABORT_STATE;
        std::vector<std::vector<WeightedCoordinate> > horizontal(outWidth);
        for(std::size_t i = 0; i < outWidth; i++)
        {
            const double left = static_cast<double>(scaledExposedRect.left());
            const double di = static_cast<double>(i);
            const double center = (di + left) / newScaleFactor;
            const std::size_t start = static_cast<std::size_t>(std::max(static_cast<int>((di - filterDomain() + left) / newScaleFactor - 0.5), 0));
            const std::size_t stop  = static_cast<std::size_t>(std::min(static_cast<int>((di + filterDomain() + left) / newScaleFactor + 0.5), m_image.width() - 1));

            horizontal[i].reserve(stop - start + 1);
            for(std::size_t j = start; j <= stop; j++)
            {
                WeightedCoordinate wc(j, filterKernel((j - center) * newScaleFactor));
                if(std::abs(wc.weight) > FILTER_CUTOFF_TRESHOLD)
                    horizontal[i].push_back(wc);
            }
        }

        CHECK_ABORT_STATE;
        std::vector<std::vector<WeightedCoordinate> > vertical(outHeight);
        for(std::size_t i = 0; i < outHeight; i++)
        {
            const double top = static_cast<double>(scaledExposedRect.top());
            const double di = static_cast<double>(i);
            const double center = (di + top) / newScaleFactor;
            const std::size_t start = static_cast<std::size_t>(std::max(static_cast<int>((di - filterDomain() + top) / newScaleFactor - 0.5), 0));
            const std::size_t stop  = static_cast<std::size_t>(std::min(static_cast<int>((di + filterDomain() + top) / newScaleFactor + 0.5), m_image.height() - 1));

            vertical[i].reserve(stop - start + 1);
            for(std::size_t j = start; j <= stop; j++)
            {
                WeightedCoordinate wc(j, filterKernel((j - center) * newScaleFactor));
                if(std::abs(wc.weight) > FILTER_CUTOFF_TRESHOLD)
                    vertical[i].push_back(wc);
            }
        }

        CHECK_ABORT_STATE;
        const std::size_t channelsSize = static_cast<std::size_t>(numChannels);
        std::vector<double> channels(channelsSize);
        for(std::size_t outRow = 0; outRow < outHeight; outRow++)
        {
            uchar *outScanLine = scaledImage.scanLine(static_cast<int>(outRow));
            const std::vector<WeightedCoordinate> &verticalData = vertical[outRow];
            for(std::size_t outCol = 0; outCol < outWidth; outCol++)
            {
                const std::vector<WeightedCoordinate> &horizontalData = horizontal[outCol];
                for(std::size_t k = 0; k < channelsSize; k++)
                    channels[k] = 0.0;
                double sum = 0.0;
                for(std::size_t i = 0, iEnd = verticalData.size(); i < iEnd; i++)
                {
                    const WeightedCoordinate &vwci = verticalData[i];
                    const uchar *inScanLine = m_image.scanLine(static_cast<int>(vwci.coordinate));
                    const double vwciWeight = vwci.weight;

                    for(std::size_t j = 0, jEnd = horizontalData.size(); j < jEnd; j++)
                    {
                        const WeightedCoordinate& hwcj = horizontalData[j];
                        const double w = hwcj.weight * vwciWeight;
                        const uchar *inPixel = inScanLine + (hwcj.coordinate * channelsSize);
                        for(std::size_t k = 0; k < channelsSize; k++)
                            channels[k] += w * inPixel[k];
                        sum += w;
                    }
                }
                sum = 1.0 / sum;
                uchar *outPixel = outScanLine + (outCol * channelsSize);
                for(std::size_t k = 0; k < channelsSize; k++)
                    outPixel[k] = static_cast<uchar>(qBound(0, static_cast<int>(channels[k] * sum), 255));
                CHECK_ABORT_STATE;
            }
        }

#endif

        CHECK_ABORT_STATE;
        lockScaledImage();
        m_scaledData.reset(new ScaledImageData(scaledImage, newScaleFactor));
        unlockScaledImage();
        CHECK_ABORT_STATE;

        return true;
#undef CHECK_ABORT_STATE
    }

    QImage m_image;
};

} // namespace

// ====================================================================================================

namespace {

class ResamplerManager : public AbstractScalingManager
{
public:
    ResamplerManager(ResamplerWorker *worker, AutoUpdatedScalingWorkerHandler* handler, QThread *thread)
        : AbstractScalingManager(worker, handler, thread)
    {}

    void setImage(const QImage &image)
    {
        static_cast<ResamplerWorker*>(m_scalingWorker)->setImage(image);
    }

    QImage getImage() const
    {
        return static_cast<ResamplerWorker*>(m_scalingWorker)->getImage();
    }
};

ResamplerManager *createResamplerManager(ResampledImageGraphicsItem *item)
{
    ResamplerWorker *worker = new ResamplerWorker();
    QThread *thread = new QThread();
    AutoUpdatedScalingWorkerHandler *handler = new AutoUpdatedScalingWorkerHandler(item, worker, thread);
    return new ResamplerManager(worker, handler, thread);
}

} // namespace

// ====================================================================================================

struct ResampledImageGraphicsItem::Impl
{
    QPixmap pixmap;
    Qt::TransformationMode transformationMode;

    QScopedPointer<ResamplerManager> resamplerManager;

    Impl(ResampledImageGraphicsItem *resampledImageGraphicsItem)
        : transformationMode(Qt::FastTransformation)
        , resamplerManager(createResamplerManager(resampledImageGraphicsItem))
    {}

    void paintDefault(QPainter *painter) const
    {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, transformationMode == Qt::SmoothTransformation);
        painter->drawPixmap(0, 0, pixmap);
    }

    void paintResampled(QPainter *painter) const
    {
        const QImage scaledImage = resamplerManager->getScaledImage();
        if(scaledImage.isNull())
            return paintDefault(painter);
        painter->setRenderHint(QPainter::SmoothPixmapTransform, transformationMode == Qt::SmoothTransformation);
        GraphicsItemUtils::DrawScaledImage(painter, scaledImage, pixmap.rect(), resamplerManager->getScaledScaleFactor());
    }
};

// ====================================================================================================

ResampledImageGraphicsItem::ResampledImageGraphicsItem(QGraphicsItem *parentItem)
    : QGraphicsItem(parentItem)
    , m_impl(new Impl(this))
{}

ResampledImageGraphicsItem::ResampledImageGraphicsItem(const QImage &image, QGraphicsItem *parentItem)
    : QGraphicsItem(parentItem)
    , m_impl(new Impl(this))
{
    setImage(image);
}

ResampledImageGraphicsItem::ResampledImageGraphicsItem(const QPixmap &pixmap, QGraphicsItem *parentItem)
    : QGraphicsItem(parentItem)
    , m_impl(new Impl(this))
{
    setPixmap(pixmap);
}

ResampledImageGraphicsItem::~ResampledImageGraphicsItem()
{}

QImage ResampledImageGraphicsItem::image() const
{
    return m_impl->resamplerManager->getImage();
}

void ResampledImageGraphicsItem::setImage(const QImage &image)
{
    m_impl->pixmap = QPixmap::fromImage(image);
    m_impl->resamplerManager->setImage(image);
    update();
}

QPixmap ResampledImageGraphicsItem::pixmap() const
{
    return m_impl->pixmap;
}

void ResampledImageGraphicsItem::setPixmap(const QPixmap &pixmap)
{
    m_impl->pixmap = pixmap;
    m_impl->resamplerManager->setImage(pixmap.toImage());
    update();
}

Qt::TransformationMode ResampledImageGraphicsItem::transformationMode() const
{
    return m_impl->transformationMode;
}

void ResampledImageGraphicsItem::setTransformationMode(Qt::TransformationMode mode)
{
    m_impl->transformationMode = mode;
    update();
}

QRectF ResampledImageGraphicsItem::boundingRect() const
{
    return QRectF(m_impl->pixmap.rect());
}

void ResampledImageGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if(m_impl->pixmap.isNull())
        return;

    if(m_impl->transformationMode != Qt::SmoothTransformation)
        return m_impl->paintDefault(painter);

    const qreal newScaleFactor = GraphicsItemUtils::GetDeviceScaleFactor(painter);
    const QSize originalPixmapSize = m_impl->pixmap.size();
    const QSize scaledPixmapSize = originalPixmapSize * newScaleFactor;
    if(newScaleFactor >= 1 || newScaleFactor <= 0 || scaledPixmapSize == originalPixmapSize)
        return m_impl->paintDefault(painter);

    m_impl->resamplerManager->beginScaledImageProcessing();
    const qreal scaleFactor = !m_impl->resamplerManager->hasScaledData() ? newScaleFactor : m_impl->resamplerManager->getScaledScaleFactor();
    if(m_impl->resamplerManager->hasScaledData() && GraphicsItemUtils::IsFuzzyEqualScaleFactors(newScaleFactor, scaleFactor))
    {
        m_impl->paintResampled(painter);
        m_impl->resamplerManager->endScaledImageProcessing();
        return;
    }
    m_impl->resamplerManager->endScaledImageProcessing();

    m_impl->paintDefault(painter);
    m_impl->resamplerManager->startTask(newScaleFactor);
}

// ====================================================================================================
