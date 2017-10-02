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

#if !defined(MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_H_INCLUDED)
#define MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_H_INCLUDED

#include <QGraphicsObject>
#include "Utils/ScopedPointer.h"

class MacWebKitRasterizerGraphicsItem : public QGraphicsObject
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
    Q_DISABLE_COPY(MacWebKitRasterizerGraphicsItem)

public:
    enum State
    {
        STATE_LOADING,
        STATE_FAILED,
        STATE_SUCCEED
    };

    enum DataType
    {
        DATA_TYPE_UNKNOWN,
        DATA_TYPE_HTML,
        DATA_TYPE_XHTML,
        DATA_TYPE_XML,
        DATA_TYPE_SVG
    };

    class Impl;

    MacWebKitRasterizerGraphicsItem(const QUrl &url, QGraphicsItem *parentItem = NULL);
    MacWebKitRasterizerGraphicsItem(const QByteArray &htmlData, DataType dataType = DATA_TYPE_UNKNOWN, QGraphicsItem *parentItem = NULL);

    ~MacWebKitRasterizerGraphicsItem();

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = NULL);

    State state() const;

private:
    QScopedPointer<Impl> m_impl;
};

#endif // MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_H_INCLUDED
