/*
   Copyright (C) 2017-2023 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined(MAC_WEBVIEW_RASTERIZER_GRAPHICS_ITEM_H_INCLUDED)
#define MAC_WEBVIEW_RASTERIZER_GRAPHICS_ITEM_H_INCLUDED

#include <QObject>
#include <QGraphicsItem>

#include "Utils/Global.h"
#include "Utils/ScopedPointer.h"

#include "../../../GraphicsItemFeatures/IGrabImage.h"
#include "../../../GraphicsItemFeatures/IGrabScaledImage.h"

#include "AbstractSVGWebBrowser.h"

class QByteArray;
class QUrl;

class MacWebViewRasterizerGraphicsItem :
        public QObject,
        public QGraphicsItem,
        public AbstractSVGWebBrowser,
        public IGrabImage,
        public IGrabScaledImage
{
    Q_OBJECT
//    Q_INTERFACES(QGraphicsItem)
    Q_DISABLE_COPY(MacWebViewRasterizerGraphicsItem)

public:
    MacWebViewRasterizerGraphicsItem(QGraphicsItem *parentItem = Q_NULLPTR);
    ~MacWebViewRasterizerGraphicsItem();

    static bool isAvailable();
    bool load(const QByteArray &svgData, const QUrl &baseUrl);

    QImage grabImage() Q_DECL_OVERRIDE;
    QImage grabImage(qreal scaleFactor) Q_DECL_OVERRIDE;

    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) Q_DECL_OVERRIDE;

private:
    QVariant evalJSImpl(const QString &scriptSource) Q_DECL_OVERRIDE;

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // MAC_WEBVIEW_RASTERIZER_GRAPHICS_ITEM_H_INCLUDED
