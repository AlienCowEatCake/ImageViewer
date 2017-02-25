#ifndef IMAGEVIEWERWIDGET_H_INCLUDED
#define IMAGEVIEWERWIDGET_H_INCLUDED

#include <QGraphicsView>
#include <QGraphicsItem>

#include "Utils/ScopedPointer.h"

class ImageViewerWidget : public QGraphicsView
{
    Q_OBJECT

public:
    enum ZoomMode
    {
        ZOOM_IDENTITY       = 1,
        ZOOM_FIT_TO_WINDOW  = 2,
        ZOOM_CUSTOM         = 3
    };

signals:
    void zoomModeChanged(ImageViewerWidget::ZoomMode mode);
    void zoomLevelChanged(qreal zoomLevel);

public:

    ImageViewerWidget(QWidget *parent = NULL);
    ~ImageViewerWidget();

    void setGraphicsItem(QGraphicsItem *graphicsItem);
    void clear();

    void setZoomMode(ZoomMode mode, qreal zoomLevel = -1);
    ZoomMode zoomMode() const;
    void setZoomLevel(qreal zoomLevel);
    qreal zoomLevel() const;

    void setSmoothEnabled(bool isEnabled);

    QSize imageSize() const;

public slots:
    void rotateClockwise();
    void rotateCounterclockwise();

    void zoomIn();
    void zoomOut();

protected:
    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // IMAGEVIEWERWIDGET_H_INCLUDED
