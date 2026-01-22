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

#include "ImageViewerWidget.h"

#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QScrollBar>
#include <QGraphicsPixmapItem>
#include <QStyleOptionGraphicsItem>
#include <QMouseEvent>
#include <QWheelEvent>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#define IMAGEVIEWERWIDGET_NO_QMATRIX
#endif
#if defined (IMAGEVIEWERWIDGET_NO_QMATRIX)
#include <QTransform>
typedef QTransform TransformMatrix;
#else
#include <QMatrix>
typedef QMatrix TransformMatrix;
#endif

#if (QT_VERSION < QT_VERSION_CHECK(4, 6, 0)) || defined (QT_NO_GESTURES)
#define IMAGEVIEWERWIDGET_NO_GESTURES
#endif
#if !defined (IMAGEVIEWERWIDGET_NO_GESTURES)
#include <QGesture>
#include <QGestureEvent>
#include <QPinchGesture>
#endif

#include "Utils/Global.h"

#include "Decoders/GraphicsItemFeatures/IGrabImage.h"
#include "Decoders/GraphicsItemFeatures/ITransformationMode.h"

namespace {

const int SCROLL_STEP = 10;
const qreal ZOOM_STEP = 1.1;
const qreal ZOOM_CHANGE_EPSILON = 1e-5;
const int ZOOM_FIT_SIZE_CORRECTION = 1;
const qreal ROTATION_TRESHOLD = 45;
const qreal WHEEL_NAVIGATE_EPSILON = 1e-3;
const qreal MOUSE_NAVIGATION_ZONE_WIDTH = 32;

enum MouseNavigationZone
{
    MOUSE_NAVIGATION_ZONE_NONE,
    MOUSE_NAVIGATION_ZONE_PREVIOUS,
    MOUSE_NAVIGATION_ZONE_NEXT,
    MOUSE_NAVIGATION_ZONE_INVALID
};

} // namespace

struct ImageViewerWidget::Impl
{
    explicit Impl(ImageViewerWidget *widget)
        : imageViewerWidget(widget)
        , scene(new QGraphicsScene(widget))
        , currentGraphicsItem(Q_NULLPTR)
        , currentZoomMode(ZOOM_IDENTITY)
        , lastZoomMode(currentZoomMode)
        , currentZoomLevel(1)
        , previousZoomLevel(-1)
        , currentRotationAngle(0)
        , flipHorizontal(false)
        , flipVertical(false)
        , transformationMode(Qt::SmoothTransformation)
        , wheelMode(WHEEL_SCROLL)
        , wheelAccumulatedValue(0)
        , upscaleOnFitToWindow(false)
        , navigatePreviousEnabled(false)
        , navigateNextEnabled(false)
        , navigationZone(MOUSE_NAVIGATION_ZONE_NONE)
    {
        widget->setScene(scene);
    }

    qreal getFitToWindowZoomLevel() const
    {
        TransformMatrix rotationMatrix;
        rotationMatrix.rotate(currentRotationAngle);
        const QRectF boundingRect = rotationMatrix.mapRect(currentGraphicsItem->boundingRect());
        const QSize imageSize = boundingRect.size().toSize();
        const QSize windowSize = imageViewerWidget->viewport()->size();
        if(upscaleOnFitToWindow || imageSize.width() > windowSize.width() || imageSize.height() > windowSize.height())
        {
            const qreal deltaWidth = static_cast<qreal>(windowSize.width() - ZOOM_FIT_SIZE_CORRECTION) / static_cast<qreal>(imageSize.width());
            const qreal deltaHeight = static_cast<qreal>(windowSize.height() - ZOOM_FIT_SIZE_CORRECTION) / static_cast<qreal>(imageSize.height());
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

        TransformMatrix matrix;
        matrix.scale(currentZoomLevel, currentZoomLevel);
        matrix.rotate(currentRotationAngle);
        matrix.scale(flipHorizontal ? -1 : 1, flipVertical ? -1 : 1);
#if defined (IMAGEVIEWERWIDGET_NO_QMATRIX)
        imageViewerWidget->setTransform(matrix);
#else
        imageViewerWidget->setMatrix(matrix);
#endif

        if(qAbs(previousZoomLevel - currentZoomLevel) / qMax(previousZoomLevel, currentZoomLevel) > ZOOM_CHANGE_EPSILON)
        {
            Q_EMIT imageViewerWidget->zoomLevelChanged(currentZoomLevel);
            previousZoomLevel = currentZoomLevel;
        }
    }

    void applyTransformationMode()
    {
        if(!currentGraphicsItem)
            return;
        QGraphicsPixmapItem *pixmapItem = dynamic_cast<QGraphicsPixmapItem*>(currentGraphicsItem);
        if(pixmapItem)
        {
            pixmapItem->setTransformationMode(transformationMode);
            return;
        }
        ITransformationMode *itemWithTransformationMode = dynamic_cast<ITransformationMode*>(currentGraphicsItem);
        if(itemWithTransformationMode)
        {
            itemWithTransformationMode->setTransformationMode(transformationMode);
            return;
        }
    }

    inline QPointF getMouseEventPos(const QMouseEvent *event) const
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        return event->position();
#else
        return QPointF(event->pos());
#endif
    }

    inline qreal getMouseNavigationZoneWidth() const
    {
        const qreal width = static_cast<qreal>(imageViewerWidget->width());
        return qMin(static_cast<qreal>(width / 3.0), MOUSE_NAVIGATION_ZONE_WIDTH);
    }

    inline bool isLeftNavigationZone(const QPointF &p) const
    {
        const qreal x = static_cast<qreal>(p.x());
        const qreal navigationZoneWidth = getMouseNavigationZoneWidth();
        return x >= 0 && x < navigationZoneWidth;
    }

    inline bool isRightNavigationZone(const QPointF &p) const
    {
        const qreal x = static_cast<qreal>(p.x());
        const qreal width = static_cast<qreal>(imageViewerWidget->width());
        const qreal navigationZoneWidth = getMouseNavigationZoneWidth();
        return x <= width && x > width - navigationZoneWidth;
    }

    inline bool isNavigationNextZone(const QPointF &p) const
    {
        return navigateNextEnabled &&
               (imageViewerWidget->layoutDirection() == Qt::RightToLeft ? isLeftNavigationZone(p) : isRightNavigationZone(p));
    }

    inline bool isNavigationPreviousZone(const QPointF &p) const
    {
        return navigatePreviousEnabled &&
               (imageViewerWidget->layoutDirection() == Qt::RightToLeft ? isRightNavigationZone(p) : isLeftNavigationZone(p));
    }

    inline bool isNavigationZone(const QPointF &p) const
    {
        return isNavigationNextZone(p) || isNavigationPreviousZone(p);
    }

    inline bool isNavigationZone(const QMouseEvent *event) const
    {
        return isNavigationZone(getMouseEventPos(event));
    }

    void updateCursorAndToolTip(const QMouseEvent *event)
    {
        const QPointF p = getMouseEventPos(event);
        if(isNavigationNextZone(p))
        {
            if(navigationZone != MOUSE_NAVIGATION_ZONE_NEXT)
            {
                imageViewerWidget->setCursor(Qt::PointingHandCursor);
                imageViewerWidget->setToolTip(qApp->translate("ImageViewerWidget", "Next"));
                navigationZone = MOUSE_NAVIGATION_ZONE_NEXT;
            }
        }
        else if(isNavigationPreviousZone(p))
        {
            if(navigationZone != MOUSE_NAVIGATION_ZONE_PREVIOUS)
            {
                imageViewerWidget->setCursor(Qt::PointingHandCursor);
                imageViewerWidget->setToolTip(qApp->translate("ImageViewerWidget", "Previous"));
                navigationZone = MOUSE_NAVIGATION_ZONE_PREVIOUS;
            }
        }
        else
        {
            if(navigationZone != MOUSE_NAVIGATION_ZONE_NONE)
            {
                imageViewerWidget->unsetCursor();
                imageViewerWidget->setToolTip(QString());
                navigationZone = MOUSE_NAVIGATION_ZONE_NONE;
            }
        }
    }

#if !defined (IMAGEVIEWERWIDGET_NO_GESTURES)
    bool gestureEvent(QGestureEvent *event)
    {
        if(QGesture *pinch = event->gesture(Qt::PinchGesture))
            pinchTriggered(static_cast<QPinchGesture*>(pinch));
        return true;
    }
#endif

#if !defined (IMAGEVIEWERWIDGET_NO_GESTURES)
    void pinchTriggered(QPinchGesture *gesture)
    {
        const QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();
        if(changeFlags.testFlag(QPinchGesture::ScaleFactorChanged))
        {
            const qreal newZoomLevel = currentZoomLevel * gesture->scaleFactor();
            imageViewerWidget->setZoomLevel(newZoomLevel);
            updateTransformations();
        }
        if(changeFlags.testFlag(QPinchGesture::RotationAngleChanged))
        {
            qreal rotationAngle = gesture->totalRotationAngle();
            while(rotationAngle > 180.0)
                rotationAngle -= 360.0;
            while(rotationAngle < -180.0)
                rotationAngle += 360.0;
            if(rotationAngle >= ROTATION_TRESHOLD)
            {
                imageViewerWidget->rotateClockwise();
                gesture->setTotalRotationAngle(0);
            }
            if(rotationAngle <= -ROTATION_TRESHOLD)
            {
                imageViewerWidget->rotateCounterclockwise();
                gesture->setTotalRotationAngle(0);
            }
        }
    }
#endif

    ImageViewerWidget *imageViewerWidget;
    QGraphicsScene *scene;
    QGraphicsItem *currentGraphicsItem;
    QImage backgroundTexture;
    ZoomMode currentZoomMode;
    ZoomMode lastZoomMode;
    qreal currentZoomLevel;
    qreal previousZoomLevel;
    int currentRotationAngle;
    bool flipHorizontal;
    bool flipVertical;
    Qt::TransformationMode transformationMode;
    WheelMode wheelMode;
    qreal wheelAccumulatedValue;
    bool upscaleOnFitToWindow;
    bool navigatePreviousEnabled;
    bool navigateNextEnabled;
    MouseNavigationZone navigationZone;
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
    QImage image;
    if(IGrabImage *itemWithGrabImage = dynamic_cast<IGrabImage*>(m_impl->currentGraphicsItem))
    {
        image = itemWithGrabImage->grabImage();
    }
    else
    {
        image = QImage(imageSize(), QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        const QRectF boundingRect = m_impl->currentGraphicsItem->boundingRect();
        QPainter painter;
        painter.begin(&image);
        painter.translate(-boundingRect.x(), -boundingRect.y());
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        QStyleOptionGraphicsItem options;
        options.exposedRect = boundingRect;
        m_impl->currentGraphicsItem->paint(&painter, &options);
        painter.end();
    }
    if(m_impl->currentRotationAngle)
    {
        QTransform transform;
        transform.rotate(m_impl->currentRotationAngle);
        image = image.transformed(transform);
    }
    if(m_impl->flipHorizontal || m_impl->flipVertical)
    {
        Qt::Orientations orientations;
        if(m_impl->flipHorizontal)
            orientations |= Qt::Horizontal;
        if(m_impl->flipVertical)
            orientations |= Qt::Vertical;
        QImage_flip(image, orientations);
    }
    return image;
}

QGraphicsItem *ImageViewerWidget::graphicsItem() const
{
    return m_impl->currentGraphicsItem;
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
    m_impl->currentGraphicsItem = Q_NULLPTR;
    scene()->clear();
#if defined (IMAGEVIEWERWIDGET_NO_QMATRIX)
    resetTransform();
#else
    resetMatrix();
#endif
    ensureVisible(QRectF(0, 0, 0, 0));
    setSceneRect(0, 0, 1, 1);
    m_impl->currentRotationAngle = 0;
    m_impl->flipHorizontal = false;
    m_impl->flipVertical = false;
    m_impl->wheelAccumulatedValue = 0;
}

void ImageViewerWidget::setZoomMode(ImageViewerWidget::ZoomMode mode)
{
    m_impl->currentZoomMode = mode;
    m_impl->lastZoomMode = mode;
    m_impl->updateTransformations();
}

void ImageViewerWidget::setZoomLevel(qreal zoomLevel)
{
    if(qFuzzyIsNull(zoomLevel))
        return;
    m_impl->currentZoomMode = ZOOM_CUSTOM;
    m_impl->currentZoomLevel = zoomLevel;
    m_impl->updateTransformations();
}

void ImageViewerWidget::setWheelMode(ImageViewerWidget::WheelMode mode)
{
    m_impl->wheelAccumulatedValue = 0;
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

QImage ImageViewerWidget::backgroundTexture() const
{
    return m_impl->backgroundTexture;
}

void ImageViewerWidget::setBackgroundTexture(const QImage &image)
{
    m_impl->backgroundTexture = image;
}

void ImageViewerWidget::setSmoothTransformation(bool enabled)
{
    m_impl->transformationMode = (enabled ? Qt::SmoothTransformation : Qt::FastTransformation);
    m_impl->applyTransformationMode();
}

void ImageViewerWidget::setUpscaleOnFitToWindow(bool enabled)
{
    m_impl->upscaleOnFitToWindow = enabled;
    m_impl->updateTransformations();
}

void ImageViewerWidget::setNavigatePreviousEnabled(bool enabled)
{
    m_impl->navigatePreviousEnabled = enabled;
    m_impl->navigationZone = MOUSE_NAVIGATION_ZONE_INVALID;
}

void ImageViewerWidget::setNavigateNextEnabled(bool enabled)
{
    m_impl->navigateNextEnabled = enabled;
    m_impl->navigationZone = MOUSE_NAVIGATION_ZONE_INVALID;
}

void ImageViewerWidget::drawBackground(QPainter *painter, const QRectF &rect)
{
    if(!m_impl->backgroundTexture.isNull() && m_impl->currentGraphicsItem)
    {
        const QRect itemBounds = mapFromScene(m_impl->currentGraphicsItem->boundingRect()).boundingRect().adjusted(0, 0, -1, -1);
        const QRect targetArea = itemBounds.intersected(mapFromScene(rect).boundingRect());
        painter->save();
        painter->resetTransform();
        painter->translate(itemBounds.topLeft());
        painter->setPen(Qt::NoPen);
        painter->setBrush(m_impl->backgroundTexture);
        painter->drawRect(targetArea.x() - itemBounds.x(), targetArea.y() - itemBounds.y(), targetArea.width(), targetArea.height());
        painter->restore();
    }
    return QGraphicsView::drawBackground(painter, rect);
}

bool ImageViewerWidget::event(QEvent *event)
{
#if !defined (IMAGEVIEWERWIDGET_NO_GESTURES)
    if(event->type() == QEvent::Gesture)
        return m_impl->gestureEvent(static_cast<QGestureEvent*>(event));
#endif
    return QGraphicsView::event(event);
}

void ImageViewerWidget::changeEvent(QEvent *event)
{
    QGraphicsView::changeEvent(event);
    switch(event->type())
    {
    case QEvent::LayoutDirectionChange:
    case QEvent::LanguageChange:
        m_impl->navigationZone = MOUSE_NAVIGATION_ZONE_INVALID;
        break;
    default:
        break;
    }
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

void ImageViewerWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && !m_impl->isNavigationZone(event))
    {
        switch(m_impl->currentZoomMode)
        {
        case ZOOM_IDENTITY:
        case ZOOM_CUSTOM:
            m_impl->currentZoomMode = ZOOM_FIT_TO_WINDOW;
            break;
        case ZOOM_FIT_TO_WINDOW:
            m_impl->currentZoomMode = ZOOM_IDENTITY;
            break;
        }
        m_impl->updateTransformations();
    }
    m_impl->updateCursorAndToolTip(event);
    QGraphicsView::mouseDoubleClickEvent(event);
}

void ImageViewerWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && !m_impl->isNavigationZone(event))
        setDragMode(QGraphicsView::ScrollHandDrag);
    m_impl->updateCursorAndToolTip(event);
    QGraphicsView::mousePressEvent(event);
}

void ImageViewerWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    if(dragMode() == QGraphicsView::NoDrag)
    {
        const QPointF p = m_impl->getMouseEventPos(event);
        if(m_impl->isNavigationNextZone(p))
            Q_EMIT selectNextRequested();
        else if(m_impl->isNavigationPreviousZone(p))
            Q_EMIT selectPreviousRequested();
    }
    else
    {
        setDragMode(QGraphicsView::NoDrag);
    }
    m_impl->updateCursorAndToolTip(event);
}

void ImageViewerWidget::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    if(event->buttons() == Qt::MouseButtons())
    {
        if(dragMode() == QGraphicsView::ScrollHandDrag)
            setDragMode(QGraphicsView::NoDrag);
        m_impl->updateCursorAndToolTip(event);
    }
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
    if(wheelMode() == WHEEL_NAVIGATE)
    {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        const QPointF numDegrees = QPointF(event->angleDelta()) / 8.0;
        const qreal stepsDistance = (numDegrees.x() + numDegrees.y()) / 15.0;
#else
        const qreal stepsDistance = event->delta() / 8.0 / 15.0;
#endif
        if((stepsDistance < -WHEEL_NAVIGATE_EPSILON && m_impl->wheelAccumulatedValue > WHEEL_NAVIGATE_EPSILON) ||
           (stepsDistance > WHEEL_NAVIGATE_EPSILON && m_impl->wheelAccumulatedValue < -WHEEL_NAVIGATE_EPSILON))
            m_impl->wheelAccumulatedValue = 0;
        m_impl->wheelAccumulatedValue += stepsDistance;
        if(qAbs(m_impl->wheelAccumulatedValue) >= 1.0 - WHEEL_NAVIGATE_EPSILON)
        {
            if(m_impl->wheelAccumulatedValue > 0)
                Q_EMIT selectPreviousRequested();
            else
                Q_EMIT selectNextRequested();
            m_impl->wheelAccumulatedValue = 0;
        }
        event->accept();
        return;
    }
    QGraphicsView::wheelEvent(event);
}
