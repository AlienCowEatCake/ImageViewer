#ifndef IMAGEVIEWERWIDGET_H_INCLUDED
#define IMAGEVIEWERWIDGET_H_INCLUDED

#include <QGraphicsView>
#include <QGraphicsItem>

#include <QScopedPointer>

class ImageViewerWidget : public QGraphicsView
{
    Q_OBJECT

public:
    enum ZoomMode
    {
        ZOOM_IDENTITY,
        ZOOM_FIT_TO_WINDOW,
        ZOOM_CUSTOM
    };

signals:
    void zoomModeChanged(ImageViewerWidget::ZoomMode mode);

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
