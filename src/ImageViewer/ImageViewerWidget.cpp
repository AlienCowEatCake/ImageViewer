#include "ImageViewerWidget.h"

#include <QGraphicsScene>

#include <QDebug>

struct ImageViewerWidget::Impl
{
    Impl(ImageViewerWidget *widget)
        : imageViewerWidget(widget)
        , scene(new QGraphicsScene(widget))
        , currentGraphicsItem(NULL)
        , currentZoomMode(ZOOM_IDENTITY)
        , previousZoomMode(-1)
        , currentZoomLevel(1)
        , previousZoomLevel(-1)
        , currentRotationAngle(0)
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
            const QRectF boundingRect = currentGraphicsItem->boundingRect();
            const QSize imageSize = boundingRect.size().toSize();
            const QSize windowSize = imageViewerWidget->size();
            if(imageSize.width() > windowSize.width() || imageSize.height() > windowSize.height())
            {
                imageViewerWidget->fitInView(boundingRect, Qt::KeepAspectRatio);
                const qreal deltaWidth = qreal(windowSize.width()) / qreal(imageSize.width());
                const qreal deltaHeight = qreal(windowSize.height()) / qreal(imageSize.height());
                currentZoomLevel = qMin(deltaWidth, deltaHeight);
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

        if(previousZoomLevel != currentZoomLevel)
        {
            emit imageViewerWidget->zoomLevelChanged(currentZoomLevel);
            previousZoomLevel = currentZoomLevel;
        }
        if(previousZoomMode != currentZoomMode)
        {
            emit imageViewerWidget->zoomModeChanged(currentZoomMode);
            previousZoomMode = currentZoomMode;
        }
    }

    ImageViewerWidget *imageViewerWidget;
    QGraphicsScene *scene;
    QGraphicsItem *currentGraphicsItem;
    ZoomMode currentZoomMode;
    int previousZoomMode;
    qreal currentZoomLevel;
    qreal previousZoomLevel;
    qreal currentRotationAngle;
};

ImageViewerWidget::ImageViewerWidget(QWidget *parent)
    : QGraphicsView(parent)
    , m_impl(new Impl(this))
{
    setViewportMargins(0, 0, 0, 0);
    setSmoothEnabled(true);
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
}

ImageViewerWidget::~ImageViewerWidget()
{}

void ImageViewerWidget::setZoomMode(ImageViewerWidget::ZoomMode mode, qreal zoomLevel)
{
    m_impl->currentZoomMode = mode;
    if(zoomLevel > 0)
        m_impl->currentZoomLevel = zoomLevel;
    m_impl->updateTransformations();
}

ImageViewerWidget::ZoomMode ImageViewerWidget::zoomMode() const
{
    return m_impl->currentZoomMode;
}

void ImageViewerWidget::setZoomLevel(qreal zoomLevel)
{
    setZoomMode(ZOOM_CUSTOM, zoomLevel);
}

qreal ImageViewerWidget::zoomLevel() const
{
    return m_impl->currentZoomLevel;
}

void ImageViewerWidget::setSmoothEnabled(bool isEnabled)
{
    setRenderHint(QPainter::Antialiasing, isEnabled);
    setRenderHint(QPainter::TextAntialiasing, isEnabled);
    setRenderHint(QPainter::SmoothPixmapTransform, isEnabled);
}

QSize ImageViewerWidget::imageSize() const
{
    if(!m_impl->currentGraphicsItem)
        return QSize();
    return m_impl->currentGraphicsItem->boundingRect().size().toSize();
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

void ImageViewerWidget::setGraphicsItem(QGraphicsItem *graphicsItem)
{
    m_impl->currentGraphicsItem = graphicsItem;
    scene()->clear();
    resetTransform();
    resetMatrix();
    if(!graphicsItem)
        return;
    graphicsItem->setFlags(QGraphicsItem::ItemClipsToShape);
    graphicsItem->setCacheMode(QGraphicsItem::NoCache);
    scene()->addItem(graphicsItem);
    setSceneRect(graphicsItem->boundingRect());
    m_impl->updateTransformations();
}

void ImageViewerWidget::clear()
{
    m_impl->currentGraphicsItem = NULL;
    scene()->clear();
    resetTransform();
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
