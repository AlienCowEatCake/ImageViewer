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

#if !defined(IMAGEVIEWERWIDGET_H_INCLUDED)
#define IMAGEVIEWERWIDGET_H_INCLUDED

#include <QGraphicsView>
#include <QImage>
#include "Utils/ScopedPointer.h"

class QGraphicsItem;

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
    void zoomLevelChanged(qreal zoomLevel);

public:

    ImageViewerWidget(QWidget *parent = NULL);
    ~ImageViewerWidget();

    void setGraphicsItem(QGraphicsItem *graphicsItem);
    void clear();

    void setZoomMode(ZoomMode mode);
    ZoomMode zoomMode() const;
    void setZoomLevel(qreal zoomLevel);
    qreal zoomLevel() const;

    QSize imageSize() const;
    QImage grabImage() const;

public slots:
    void rotateClockwise();
    void rotateCounterclockwise();

    void flipHorizontal();
    void flipVertical();

    void zoomIn();
    void zoomOut();
    void resetZoom();

    void setBackgroundColor(const QColor &color);

    void setSmoothTransformation(bool enabled);

protected:
    bool event(QEvent* event);
    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // IMAGEVIEWERWIDGET_H_INCLUDED
