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
#include <QStringList>
#include <QRegExp>

#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include <QWebFrame>
#include <QWebPage>

#include <QDebug>

namespace {

const int DEFAULT_SVG_SIZE = 512;

const qreal POINTS_PER_INCH = 72.0;
const qreal CM_PER_INCH     = 2.54;
const qreal MM_PER_INCH     = 25.4;
const qreal PICA_PER_INCH   = 6.0;
const qreal PIXELS_PER_INCH = 96.0;

QRectF QRectFIntegerized(const QRectF &rect)
{
    const qreal left = std::floor(rect.left());
    const qreal top = std::floor(rect.top());
    const qreal width = std::ceil(rect.width() + std::abs(rect.left() - left));
    const qreal height = std::ceil(rect.height() + std::abs(rect.top() - top));
    return QRectF(left, top, width, height);
}

qreal parseLength(const QString &str)
{
    const QString data = str.simplified().toLower();
    if(data.endsWith(QString::fromLatin1("px")))
        return static_cast<qreal>(data.left(data.length() - 2).toDouble());
    if(data.endsWith(QString::fromLatin1("pt")))
        return static_cast<qreal>(data.left(data.length() - 2).toDouble()) / POINTS_PER_INCH * PIXELS_PER_INCH;
    if(data.endsWith(QString::fromLatin1("in")))
        return static_cast<qreal>(data.left(data.length() - 2).toDouble()) * PIXELS_PER_INCH;
    if(data.endsWith(QString::fromLatin1("cm")))
        return static_cast<qreal>(data.left(data.length() - 2).toDouble()) / CM_PER_INCH * PIXELS_PER_INCH;
    if(data.endsWith(QString::fromLatin1("mm")))
        return static_cast<qreal>(data.left(data.length() - 2).toDouble()) / MM_PER_INCH * PIXELS_PER_INCH;
    if(data.endsWith(QString::fromLatin1("pc")))
        return static_cast<qreal>(data.left(data.length() - 2).toDouble()) / PICA_PER_INCH * PIXELS_PER_INCH;
    if(data.endsWith(QString::fromLatin1("em")))
        return 0; /// @todo
    if(data.endsWith(QString::fromLatin1("ex")))
        return 0; /// @todo
    if(data.endsWith(QString::fromLatin1("%")))
        return 0; /// @todo
    return static_cast<qreal>(data.toDouble());
}

} // namespace

struct QtWebKitSVGGraphicsItem::Impl
{
    QWebPage page;
    QRectF svgRect;

    QRectF detectRect()
    {
#if 0
        const QRectF rect = QRectF(static_cast<qreal>(evalJS("document.rootElement.x.baseVal.value").toDouble()),
                                   static_cast<qreal>(evalJS("document.rootElement.y.baseVal.value").toDouble()),
                                   static_cast<qreal>(evalJS("document.rootElement.width.baseVal.value").toDouble()),
                                   static_cast<qreal>(evalJS("document.rootElement.height.baseVal.value").toDouble()));
        if(!rect.isEmpty())
            return QRectF(QPointF(), rect.size());

        const QRectF viewBox = QRectF(static_cast<qreal>(evalJS("document.rootElement.viewBox.baseVal.x").toDouble()),
                                      static_cast<qreal>(evalJS("document.rootElement.viewBox.baseVal.y").toDouble()),
                                      static_cast<qreal>(evalJS("document.rootElement.viewBox.baseVal.width").toDouble()),
                                      static_cast<qreal>(evalJS("document.rootElement.viewBox.baseVal.height").toDouble()));
        if(!viewBox.isEmpty())
            return QRectF(QPointF(), viewBox.size());
#else
        const QSizeF size = QSizeF(parseLength(evalJS("document.rootElement.getAttribute('width');").toString()),
                                   parseLength(evalJS("document.rootElement.getAttribute('height');").toString()));
        if(!size.isEmpty())
            return QRectF(QPointF(), size);

        const QStringList viewBoxData = evalJS("document.rootElement.getAttribute('viewBox');").toString()
                .split(QRegExp(QString::fromLatin1("\\s")), QString::SkipEmptyParts);
        const QRectF viewBox = (viewBoxData.size() == 4)
                ? QRectF(parseLength(viewBoxData.at(0)),
                         parseLength(viewBoxData.at(1)),
                         parseLength(viewBoxData.at(2)),
                         parseLength(viewBoxData.at(3)))
                : QRectF();
        if(!viewBox.isEmpty())
        {
            if(!size.width() && size.height())
            {
                const qreal aspectRatio = viewBox.width() / viewBox.height();
                return QRectF(0, 0, aspectRatio * size.height(), size.height());
            }
            if(size.width() && !size.height())
            {
                const qreal aspectRatio = viewBox.width() / viewBox.height();
                return QRectF(0, 0, size.width(), size.height() / aspectRatio);
            }
            return QRectF(QPointF(), viewBox.size());
        }
#endif

        const QRectF bBox(static_cast<qreal>(evalJS("document.rootElement.getBBox().x;").toDouble()),
                          static_cast<qreal>(evalJS("document.rootElement.getBBox().y;").toDouble()),
                          static_cast<qreal>(evalJS("document.rootElement.getBBox().width;").toDouble()),
                          static_cast<qreal>(evalJS("document.rootElement.getBBox().height;").toDouble()));
        if(!bBox.isEmpty())
            return QRectFIntegerized(bBox);

        return QRectF(0, 0, DEFAULT_SVG_SIZE, DEFAULT_SVG_SIZE);
    }

    QVariant evalJS(const char *scriptSource)
    {
        return page.mainFrame()->evaluateJavaScript(QString::fromLatin1(scriptSource));
    }
};

QtWebKitSVGGraphicsItem::QtWebKitSVGGraphicsItem(QGraphicsItem *parentItem)
    : QGraphicsItem(parentItem)
    , m_impl(new Impl)
{
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
    m_impl->svgRect = QRectF(0, 0, DEFAULT_SVG_SIZE, DEFAULT_SVG_SIZE);
    m_impl->page.setViewportSize(m_impl->svgRect.size().toSize());

    m_impl->page.mainFrame()->setContent(svgData, QString::fromLatin1("image/svg+xml"), baseUrl);

    const QString root = m_impl->evalJS("document.rootElement.nodeName").toString();
    if(root.isEmpty() || root.compare(QString::fromLatin1("svg"), Qt::CaseInsensitive))
    {
        qWarning() << "[QtWebKitSVGGraphicsItem] Error: not an SVG";
        return false;
    }

    m_impl->svgRect = m_impl->detectRect();
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
