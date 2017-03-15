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

#include "ImageViewerWidget.h"

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QStyleOptionGraphicsItem>

#if (QT_VERSION < QT_VERSION_CHECK(4, 6, 0))
#define IMAGEVIEWERWIDGET_NO_GESTURES
#endif
#if !defined (IMAGEVIEWERWIDGET_NO_GESTURES)
#include <QGesture>
#include <QGestureEvent>
#include <QPinchGesture>
#else
class QGestureEvent {};
class QPinchGesture {};
#endif

namespace {

const qreal ZOOM_CHANGE_EPSILON = 1e-5;
const qreal ZOOM_FIT_CORRECTION = 0.999;

} // namespace

struct ImageViewerWidget::Impl
{
    Impl(ImageViewerWidget *widget)
        : imageViewerWidget(widget)
        , scene(new QGraphicsScene(widget))
        , currentGraphicsItem(NULL)
        , currentZoomMode(ZOOM_IDENTITY)
        , currentZoomLevel(1)
        , previousZoomLevel(-1)
        , currentRotationAngle(0)
        , transformationMode(Qt::SmoothTransformation)
    {
        widget->setScene(scene);
    }

    void updateTransformations()
    {
        if(!currentGraphicsItem)
            return;

        imageViewerWidget->resetTransform();
        imageViewerWidget->rotate(currentRotationAngle);
        switch(currentZoomMode)
        {
        case ZOOM_IDENTITY:
        {
            currentZoomLevel = 1;
            break;
        }
        case ZOOM_FIT_TO_WINDOW:
        {
            QMatrix rotationMatrix;
            rotationMatrix.rotate(currentRotationAngle);
            const QRectF boundingRect = rotationMatrix.mapRect(currentGraphicsItem->boundingRect());
            const QSize imageSize = boundingRect.size().toSize();
            const QSize windowSize = imageViewerWidget->viewport()->size();
            if(imageSize.width() > windowSize.width() || imageSize.height() > windowSize.height())
            {
                const qreal deltaWidth = qreal(windowSize.width()) / qreal(imageSize.width());
                const qreal deltaHeight = qreal(windowSize.height()) / qreal(imageSize.height());
                currentZoomLevel = qMin(deltaWidth, deltaHeight) * ZOOM_FIT_CORRECTION;
                imageViewerWidget->scale(currentZoomLevel, currentZoomLevel);
            }
            else
            {
                currentZoomLevel = 1;
            }
            break;
        }
        case ZOOM_CUSTOM:
        {
            imageViewerWidget->scale(currentZoomLevel, currentZoomLevel);
            break;
        }
        }

        if(qAbs(previousZoomLevel - currentZoomLevel) / qMax(previousZoomLevel, currentZoomLevel) > ZOOM_CHANGE_EPSILON)
        {
            emit imageViewerWidget->zoomLevelChanged(currentZoomLevel);
            previousZoomLevel = currentZoomLevel;
        }
    }

    void applyTransformationMode()
    {
        if(!currentGraphicsItem)
            return;
        QGraphicsPixmapItem *pixItem = dynamic_cast<QGraphicsPixmapItem*>(currentGraphicsItem);
        if(pixItem)
            pixItem->setTransformationMode(transformationMode);
    }

    bool gestureEvent(QGestureEvent* event)
    {
#if !defined (IMAGEVIEWERWIDGET_NO_GESTURES)
        if(QGesture* pinch = event->gesture(Qt::PinchGesture))
            pinchTriggered(static_cast<QPinchGesture*>(pinch));
        return true;
#else
        Q_UNUSED(event);
        return true;
#endif
    }

    void pinchTriggered(QPinchGesture* gesture)
    {
#if !defined (IMAGEVIEWERWIDGET_NO_GESTURES)
        const QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();
        if(changeFlags.testFlag(QPinchGesture::ScaleFactorChanged))
        {
            const qreal newZoomLevel = currentZoomLevel * gesture->scaleFactor();
            imageViewerWidget->setZoomLevel(newZoomLevel);
            updateTransformations();
        }
        if(changeFlags.testFlag(QPinchGesture::RotationAngleChanged))
        {
            const qreal rotationAngle = gesture->rotationAngle();
            const qreal rotationTreshhold = 30;
            if(rotationAngle > rotationTreshhold)
            {
                imageViewerWidget->rotateClockwise();
                gesture->setRotationAngle(0);
            }
            if(rotationAngle < -rotationTreshhold)
            {
                imageViewerWidget->rotateCounterclockwise();
                gesture->setRotationAngle(0);
            }
        }
        /// @todo QPinchGesture::CenterPointChanged
#else
        Q_UNUSED(gesture);
#endif
    }

    ImageViewerWidget *imageViewerWidget;
    QGraphicsScene *scene;
    QGraphicsItem *currentGraphicsItem;
    ZoomMode currentZoomMode;
    qreal currentZoomLevel;
    qreal previousZoomLevel;
    qreal currentRotationAngle;
    Qt::TransformationMode transformationMode;
};

ImageViewerWidget::ImageViewerWidget(QWidget *parent)
    : QGraphicsView(parent)
    , m_impl(new Impl(this))
{
    setViewportMargins(0, 0, 0, 0);
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::TextAntialiasing, true);
    setRenderHint(QPainter::SmoothPixmapTransform, true);
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
#if !defined (IMAGEVIEWERWIDGET_NO_GESTURES)
    grabGesture(Qt::PinchGesture);
#endif
}

ImageViewerWidget::~ImageViewerWidget()
{}

void ImageViewerWidget::setGraphicsItem(QGraphicsItem *graphicsItem)
{
    clear();
    m_impl->currentGraphicsItem = graphicsItem;
    if(!graphicsItem)
        return;
    graphicsItem->setFlags(QGraphicsItem::ItemClipsToShape);
    graphicsItem->setCacheMode(QGraphicsItem::NoCache);
    scene()->addItem(graphicsItem);
    setSceneRect(graphicsItem->boundingRect());
    m_impl->applyTransformationMode();
    m_impl->updateTransformations();
}

void ImageViewerWidget::clear()
{
    m_impl->currentGraphicsItem = NULL;
    scene()->clear();
    resetTransform();
    resetMatrix();
    setSceneRect(0, 0, 1, 1);
    m_impl->currentRotationAngle = 0;
}

void ImageViewerWidget::setZoomMode(ImageViewerWidget::ZoomMode mode)
{
    m_impl->currentZoomMode = mode;
    m_impl->updateTransformations();
}

ImageViewerWidget::ZoomMode ImageViewerWidget::zoomMode() const
{
    return m_impl->currentZoomMode;
}

void ImageViewerWidget::setZoomLevel(qreal zoomLevel)
{
    m_impl->currentZoomMode = ZOOM_CUSTOM;
    m_impl->currentZoomLevel = zoomLevel;
    m_impl->updateTransformations();
}

qreal ImageViewerWidget::zoomLevel() const
{
    return m_impl->currentZoomLevel;
}

QSize ImageViewerWidget::imageSize() const
{
    if(!m_impl->currentGraphicsItem)
        return QSize();
    return m_impl->currentGraphicsItem->boundingRect().size().toSize();
}

QImage ImageViewerWidget::grabImage() const
{
    if(!m_impl->currentGraphicsItem)
        return QImage();
    QImage image(imageSize(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    QStyleOptionGraphicsItem options;
    options.exposedRect = m_impl->currentGraphicsItem->boundingRect();
    m_impl->currentGraphicsItem->paint(&painter, &options);
    painter.end();
    int rotationAngle = static_cast<int>(m_impl->currentRotationAngle);
    if(rotationAngle)
    {
        QTransform transform;
        transform.rotate(rotationAngle);
        image = image.transformed(transform);
    }
    return image;
}

void ImageViewerWidget::rotateClockwise()
{
    m_impl->currentRotationAngle += 90;
    if(m_impl->currentRotationAngle >= 360)
        m_impl->currentRotationAngle -= 360;
    m_impl->updateTransformations();
}

void ImageViewerWidget::rotateCounterclockwise()
{
    m_impl->currentRotationAngle -= 90;
    if(m_impl->currentRotationAngle <= -360)
        m_impl->currentRotationAngle += 360;
    m_impl->updateTransformations();
}

void ImageViewerWidget::zoomIn()
{
    setZoomLevel(m_impl->currentZoomLevel * 1.1);
}

void ImageViewerWidget::zoomOut()
{
    setZoomLevel(m_impl->currentZoomLevel / 1.1);
}

void ImageViewerWidget::setBackgroundColor(const QColor &color)
{
    setBackgroundBrush(QBrush(color));
}

void ImageViewerWidget::setSmoothTransformation(bool enabled)
{
    m_impl->transformationMode = (enabled ? Qt::SmoothTransformation : Qt::FastTransformation);
    m_impl->applyTransformationMode();
}

bool ImageViewerWidget::event(QEvent *event)
{
#if !defined (IMAGEVIEWERWIDGET_NO_GESTURES)
    if(event->type() == QEvent::Gesture)
        return m_impl->gestureEvent(static_cast<QGestureEvent*>(event));
#endif
    return QGraphicsView::event(event);
}

void ImageViewerWidget::showEvent(QShowEvent *event)
{
    m_impl->updateTransformations();
    QGraphicsView::showEvent(event);
}

void ImageViewerWidget::resizeEvent(QResizeEvent *event)
{
    m_impl->updateTransformations();
    QGraphicsView::resizeEvent(event);
}
