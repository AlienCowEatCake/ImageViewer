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

#include "QtWebKitSVGGraphicsItem.h"

#include <cmath>
#include <algorithm>

#include <QUrl>
#include <QString>
#include <QByteArray>

#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include <QWebFrame>
#include <QWebPage>

#include <QDebug>

struct QtWebKitSVGGraphicsItem::Impl
{
    QWebPage page;
    QRectF svgRect;
};

QtWebKitSVGGraphicsItem::QtWebKitSVGGraphicsItem(QGraphicsItem *parentItem)
    : QGraphicsItem(parentItem)
    , m_impl(new Impl)
{
#if (QT_VERSION >= QT_VERSION_CHECK(4, 6, 0))
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption, true);
#endif

    QPalette palette = m_impl->page.palette();
    palette.setColor(QPalette::Background, Qt::transparent);
    palette.setBrush(QPalette::Base, Qt::transparent);
    m_impl->page.setPalette(palette);

    connect(&m_impl->page, SIGNAL(repaintRequested(const QRect&)), this, SLOT(onUpdateRequested(const QRect&)));
}

QtWebKitSVGGraphicsItem::~QtWebKitSVGGraphicsItem()
{}

bool QtWebKitSVGGraphicsItem::load(const QByteArray &svgData, const QUrl &baseUrl)
{
    m_impl->svgRect = QRectF(QPointF(), defaultSvgSize());
    m_impl->page.setViewportSize(m_impl->svgRect.size().toSize());

    m_impl->page.mainFrame()->setContent(svgData, QString::fromLatin1("image/svg+xml"), baseUrl);

    if(!rootElementIsSvg())
    {
        qWarning() << "[QtWebKitSVGGraphicsItem] Error: not an SVG";
        return false;
    }

    removeRootOverflowAttribute();

    m_impl->svgRect = detectSvgRect();
    m_impl->svgRect = QRectF(m_impl->svgRect.topLeft(), m_impl->svgRect.size().expandedTo(QSizeF(1, 1)));
    m_impl->page.setViewportSize(m_impl->svgRect.united(QRectF(0, 0, 1, 1)).size().toSize());

    m_impl->page.mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
    m_impl->page.mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);

    return true;
}

QRectF QtWebKitSVGGraphicsItem::boundingRect() const
{
    return m_impl->svgRect;
}

void QtWebKitSVGGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    m_impl->page.mainFrame()->render(painter, option->exposedRect.toRect());
}

void QtWebKitSVGGraphicsItem::onUpdateRequested(const QRect& rect)
{
    update(rect);
}

QVariant QtWebKitSVGGraphicsItem::evalJSImpl(const QString &scriptSource)
{
    return m_impl->page.mainFrame()->evaluateJavaScript(scriptSource);
}
