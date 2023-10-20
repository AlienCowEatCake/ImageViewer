/*
   Copyright (C) 2018-2023 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined(QML_WEBENGINE_SVG_GRAPHICS_ITEM_H_INCLUDED)
#define QML_WEBENGINE_SVG_GRAPHICS_ITEM_H_INCLUDED

#include <QGraphicsObject>
#include <QScopedPointer>

#include "Utils/Global.h"

#include "AbstractSVGWebBrowserNoJS.h"

class QByteArray;
class QUrl;

class QMLWebEngineSVGGraphicsItem : public QGraphicsObject, public AbstractSVGWebBrowserNoJS
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
    Q_DISABLE_COPY(QMLWebEngineSVGGraphicsItem)

public:
    explicit QMLWebEngineSVGGraphicsItem(QGraphicsItem *parentItem = Q_NULLPTR);
    ~QMLWebEngineSVGGraphicsItem();

    static bool isAvailable();
    bool load(const QByteArray &svgData, const QUrl &baseUrl);

    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) Q_DECL_OVERRIDE;

private:
    QByteArray getSvgData() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void onUpdateRequested();

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // QML_WEBENGINE_SVG_GRAPHICS_ITEM_H_INCLUDED
