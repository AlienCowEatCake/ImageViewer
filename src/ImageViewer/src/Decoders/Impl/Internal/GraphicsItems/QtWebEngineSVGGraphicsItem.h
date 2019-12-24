/*
   Copyright (C) 2018-2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "Utils/Global.h"

#include "AbstractSVGWebBrowser.h"

class QByteArray;
class QUrl;

class QtWebEngineSVGGraphicsItem : public QGraphicsObject, public AbstractSVGWebBrowser
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
    Q_DISABLE_COPY(QtWebEngineSVGGraphicsItem)

public:
    explicit QtWebEngineSVGGraphicsItem(QGraphicsItem *parentItem = Q_NULLPTR);
    ~QtWebEngineSVGGraphicsItem();

    bool load(const QByteArray &svgData, const QUrl &baseUrl);

    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void onUpdateRequested();
    void onRenderProcessTerminated(int terminationStatus);

private:
    QVariant evalJSImpl(const QString &scriptSource) Q_DECL_OVERRIDE;

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // QT_WEBKIT_SVG_GRAPHICS_ITEM_H_INCLUDED
