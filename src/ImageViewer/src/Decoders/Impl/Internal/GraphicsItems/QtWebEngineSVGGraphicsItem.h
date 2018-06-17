/*
   Copyright (C) 2018 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined(QT_WEBKIT_SVG_GRAPHICS_ITEM_H_INCLUDED)
#define QT_WEBKIT_SVG_GRAPHICS_ITEM_H_INCLUDED

#include <QGraphicsObject>
#include <QScopedPointer>
#include "AbstractSVGWebBrowser.h"

class QByteArray;
class QUrl;

class QtWebEngineSVGGraphicsItem : public QGraphicsObject, public AbstractSVGWebBrowser
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
    Q_DISABLE_COPY(QtWebEngineSVGGraphicsItem)

public:
    QtWebEngineSVGGraphicsItem(QGraphicsItem *parentItem = NULL);
    ~QtWebEngineSVGGraphicsItem();

    bool load(const QByteArray &svgData, const QUrl &baseUrl);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = NULL);

private slots:
    void onUpdateRequested();
    void onRenderProcessTerminated(int terminationStatus);

private:
    QVariant evalJSImpl(const QString &scriptSource);

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // QT_WEBKIT_SVG_GRAPHICS_ITEM_H_INCLUDED
