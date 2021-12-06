/*
   Copyright (C) 2017-2021 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined(IMAGEVIEWERWIDGET_H_INCLUDED)
#define IMAGEVIEWERWIDGET_H_INCLUDED

#include <QGraphicsView>
#include <QImage>

#include "Utils/Global.h"
#include "Utils/ScopedPointer.h"

class QGraphicsItem;

class ImageViewerWidget : public QGraphicsView
{
    Q_OBJECT
    Q_DISABLE_COPY(ImageViewerWidget)
    Q_PROPERTY(QImage backgroundTexture READ backgroundTexture WRITE setBackgroundTexture)

public:
    enum ZoomMode
    {
        ZOOM_IDENTITY       = 1,
        ZOOM_FIT_TO_WINDOW  = 2,
        ZOOM_CUSTOM         = 3
    };

    enum WheelMode
    {
        WHEEL_SCROLL        = 1,
        WHEEL_ZOOM          = 2
    };

Q_SIGNALS:
    void zoomLevelChanged(qreal zoomLevel);

public:

    explicit ImageViewerWidget(QWidget *parent = Q_NULLPTR);
    ~ImageViewerWidget();

    ZoomMode zoomMode() const;
    qreal zoomLevel() const;

    WheelMode wheelMode() const;

    QSize imageSize() const;
    QImage grabImage() const;

public Q_SLOTS:

    QGraphicsItem *graphicsItem() const;
    void setGraphicsItem(QGraphicsItem *graphicsItem);
    void clear();

    void setZoomMode(ImageViewerWidget::ZoomMode mode);
    void setZoomLevel(qreal zoomLevel);

    void setWheelMode(ImageViewerWidget::WheelMode mode);

    void rotateClockwise();
    void rotateCounterclockwise();

    void flipHorizontal();
    void flipVertical();

    void zoomIn();
    void zoomOut();
    void resetZoom();

    void scrollLeft();
    void scrollRight();
    void scrollUp();
    void scrollDown();

    void setBackgroundColor(const QColor &color);

    QImage backgroundTexture() const;
    void setBackgroundTexture(const QImage &image);

    void setSmoothTransformation(bool enabled);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) Q_DECL_OVERRIDE;

    bool event(QEvent* event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // IMAGEVIEWERWIDGET_H_INCLUDED
