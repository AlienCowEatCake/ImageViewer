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
#include <QScrollBar>
#include <QGraphicsPixmapItem>
#include <QStyleOptionGraphicsItem>
#include <QMatrix>
#include <QMouseEvent>
#include <QWheelEvent>

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

#include "Decoders/Impl/Internal/GraphicsItems/ResampledImageGraphicsItem.h"

namespace {

const int SCROLL_STEP = 10;
const qreal ZOOM_STEP = 1.1;
const qreal ZOOM_CHANGE_EPSILON = 1e-5;
const int ZOOM_FIT_SIZE_CORRECTION = 1;
const qreal ROTATION_TRESHOLD = 30;

} // namespace

struct ImageViewerWidget::Impl
{
    Impl(ImageViewerWidget *widget)
        : imageViewerWidget(widget)
        , scene(new QGraphicsScene(widget))
        , currentGraphicsItem(NULL)
        , currentZoomMode(ZOOM_IDENTITY)
        , lastZoomMode(currentZoomMode)
        , currentZoomLevel(1)
        , previousZoomLevel(-1)
        , currentRotationAngle(0)
        , flipHorizontal(false)
        , flipVertical(false)
        , transformationMode(Qt::SmoothTransformation)
        , wheelMode(WHEEL_SCROLL)
    {
        widget->setScene(scene);
    }

    qreal getFitToWindowZoomLevel() const
    {
        QMatrix rotationMatrix;
        rotationMatrix.rotate(currentRotationAngle);
        const QRectF boundingRect = rotationMatrix.mapRect(currentGraphicsItem->boundingRect());
        const QSize imageSize = boundingRect.size().toSize();
        const QSize windowSize = imageViewerWidget->viewport()->size();
        if(imageSize.width() > windowSize.width() || imageSize.height() > windowSize.height())
        {
            const qreal deltaWidth = qreal(windowSize.width() - ZOOM_FIT_SIZE_CORRECTION) / qreal(imageSize.width());
            const qreal deltaHeight = qreal(windowSize.height() - ZOOM_FIT_SIZE_CORRECTION) / qreal(imageSize.height());
            return qMin(deltaWidth, deltaHeight);
        }
        return 1;
    }

    void updateTransformations()
    {
        if(!currentGraphicsItem)
            return;

        switch(currentZoomMode)
        {
        case ZOOM_IDENTITY:
            currentZoomLevel = 1;
            break;
        case ZOOM_FIT_TO_WINDOW:
            currentZoomLevel = getFitToWindowZoomLevel();
            break;
        case ZOOM_CUSTOM:
            break;
        }

        QMatrix matrix;
        matrix.scale(currentZoomLevel, currentZoomLevel);
        matrix.rotate(currentRotationAngle);
        matrix.scale(flipHorizontal ? -1 : 1, flipVertical ? -1 : 1);
        imageViewerWidget->setMatrix(matrix);

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
        {
            pixItem->setTransformationMode(transformationMode);
            return;
        }
        ResampledImageGraphicsItem* imgItem = dynamic_cast<ResampledImageGraphicsItem*>(currentGraphicsItem);
        if(imgItem)
        {
            imgItem->setTransformationMode(transformationMode);
            return;
        }
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
            if(rotationAngle > ROTATION_TRESHOLD)
            {
                imageViewerWidget->rotateClockwise();
                gesture->setRotationAngle(0);
            }
            if(rotationAngle < -ROTATION_TRESHOLD)
            {
                imageViewerWidget->rotateCounterclockwise();
                gesture->setRotationAngle(0);
            }
        }
#else
        Q_UNUSED(gesture);
        Q_UNUSED(ROTATION_TRESHOLD);
#endif
    }

    ImageViewerWidget *imageViewerWidget;
    QGraphicsScene *scene;
    QGraphicsItem *currentGraphicsItem;
    ZoomMode currentZoomMode;
    ZoomMode lastZoomMode;
    qreal currentZoomLevel;
    qreal previousZoomLevel;
    int currentRotationAngle;
    bool flipHorizontal;
    bool flipVertical;
    Qt::TransformationMode transformationMode;
    WheelMode wheelMode;
};

ImageViewerWidget::ImageViewerWidget(QWidget *parent)
    : QGraphicsView(parent)
    , m_impl(new Impl(this))
{
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::TextAntialiasing, true);
    setRenderHint(QPainter::SmoothPixmapTransform, true);
    setDragMode(QGraphicsView::NoDrag);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
#if !defined (IMAGEVIEWERWIDGET_NO_GESTURES)
    grabGesture(Qt::PinchGesture);
#endif
}

ImageViewerWidget::~ImageViewerWidget()
{}

ImageViewerWidget::ZoomMode ImageViewerWidget::zoomMode() const
{
    return m_impl->currentZoomMode;
}

qreal ImageViewerWidget::zoomLevel() const
{
    return m_impl->currentZoomLevel;
}

ImageViewerWidget::WheelMode ImageViewerWidget::wheelMode() const
{
    return m_impl->wheelMode;
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
    if(m_impl->currentRotationAngle)
    {
        QTransform transform;
        transform.rotate(m_impl->currentRotationAngle);
        image = image.transformed(transform);
    }
    if(m_impl->flipHorizontal || m_impl->flipVertical)
        image = image.mirrored(m_impl->flipHorizontal, m_impl->flipVertical);
    return image;
}

void ImageViewerWidget::setGraphicsItem(QGraphicsItem *graphicsItem)
{
    clear();
    m_impl->currentGraphicsItem = graphicsItem;
    if(!graphicsItem)
        return;
    graphicsItem->setFlag(QGraphicsItem::ItemClipsToShape, true);
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
    resetMatrix();
    ensureVisible(QRectF(0, 0, 0, 0));
    setSceneRect(0, 0, 1, 1);
    m_impl->currentRotationAngle = 0;
    m_impl->flipHorizontal = false;
    m_impl->flipVertical = false;
}

void ImageViewerWidget::setZoomMode(ImageViewerWidget::ZoomMode mode)
{
    m_impl->currentZoomMode = mode;
    m_impl->lastZoomMode = mode;
    m_impl->updateTransformations();
}

void ImageViewerWidget::setZoomLevel(qreal zoomLevel)
{
    m_impl->currentZoomMode = ZOOM_CUSTOM;
    m_impl->currentZoomLevel = zoomLevel;
    m_impl->updateTransformations();
}

void ImageViewerWidget::setWheelMode(ImageViewerWidget::WheelMode mode)
{
    m_impl->wheelMode = mode;
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

void ImageViewerWidget::flipHorizontal()
{
    m_impl->flipHorizontal = !m_impl->flipHorizontal;
    m_impl->updateTransformations();
}

void ImageViewerWidget::flipVertical()
{
    m_impl->flipVertical = !m_impl->flipVertical;
    m_impl->updateTransformations();
}

void ImageViewerWidget::zoomIn()
{
    setZoomLevel(m_impl->currentZoomLevel * ZOOM_STEP);
}

void ImageViewerWidget::zoomOut()
{
    setZoomLevel(m_impl->currentZoomLevel / ZOOM_STEP);
}

void ImageViewerWidget::resetZoom()
{
    m_impl->currentZoomMode = m_impl->lastZoomMode;
    m_impl->updateTransformations();
}

void ImageViewerWidget::scrollLeft()
{
    QScrollBar* scrollBar = horizontalScrollBar();
    scrollBar->setValue(scrollBar->value() - SCROLL_STEP);
}

void ImageViewerWidget::scrollRight()
{
    QScrollBar* scrollBar = horizontalScrollBar();
    scrollBar->setValue(scrollBar->value() + SCROLL_STEP);
}

void ImageViewerWidget::scrollUp()
{
    QScrollBar* scrollBar = verticalScrollBar();
    scrollBar->setValue(scrollBar->value() - SCROLL_STEP);
}

void ImageViewerWidget::scrollDown()
{
    QScrollBar* scrollBar = verticalScrollBar();
    scrollBar->setValue(scrollBar->value() + SCROLL_STEP);
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

void ImageViewerWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        setDragMode(QGraphicsView::ScrollHandDrag);
    QGraphicsView::mousePressEvent(event);
}

void ImageViewerWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    setDragMode(QGraphicsView::NoDrag);
}

void ImageViewerWidget::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    if(dragMode() == QGraphicsView::ScrollHandDrag && event->buttons() == Qt::MouseButtons())
        setDragMode(QGraphicsView::NoDrag);
}

void ImageViewerWidget::wheelEvent(QWheelEvent *event)
{
    if(wheelMode() == WHEEL_ZOOM || event->modifiers() & Qt::ControlModifier)
    {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        const QPointF numDegrees = QPointF(event->angleDelta()) / 8.0;
        const qreal stepsDistance = (numDegrees.x() + numDegrees.y()) / 15.0;
#else
        const qreal stepsDistance = event->delta() / 8.0 / 15.0;
#endif
        const qreal scaleFactor = (stepsDistance > 0 ? (1.0 + stepsDistance / 10.0) : 1.0 / (1.0 - stepsDistance / 10.0));
        setZoomLevel(m_impl->currentZoomLevel * scaleFactor);
        event->accept();
        return;
    }
    QGraphicsView::wheelEvent(event);
}
