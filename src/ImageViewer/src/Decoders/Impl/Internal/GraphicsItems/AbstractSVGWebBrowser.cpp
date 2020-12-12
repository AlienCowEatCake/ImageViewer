/*
   Copyright (C) 2018-2020 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "AbstractSVGWebBrowser.h"

#include <algorithm>
#include <cmath>

#include <QVariant>
#include <QString>
#include <QStringList>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QRegularExpression>
#else
#include <QRegExp>
#endif
#include <QSizeF>
#include <QRectF>

namespace {

const int DEFAULT_SVG_SIZE = 512;

const qreal POINTS_PER_INCH = 72.0;
const qreal CM_PER_INCH     = 2.54;
const qreal MM_PER_INCH     = 25.4;
const qreal PICA_PER_INCH   = 6.0;
const qreal PIXELS_PER_INCH = 96.0;

} // namespace

AbstractSVGWebBrowser::AbstractSVGWebBrowser()
{}

AbstractSVGWebBrowser::~AbstractSVGWebBrowser()
{}

QVariant AbstractSVGWebBrowser::evalJS(const QString &scriptSource)
{
    return evalJSImpl(scriptSource);
}

QVariant AbstractSVGWebBrowser::evalJS(const char *scriptSource)
{
    return evalJSImpl(QString::fromLocal8Bit(scriptSource));
}

QSizeF AbstractSVGWebBrowser::defaultSvgSize() const
{
    return QSizeF(DEFAULT_SVG_SIZE, DEFAULT_SVG_SIZE);
}

QRectF AbstractSVGWebBrowser::detectFallbackSvgRect()
{
    return svgBoundingBoxRect();
}

QRectF AbstractSVGWebBrowser::detectSvgRect()
{
    const QSizeF size = svgSizeAttribute();
    if(!size.isEmpty())
        return QRectF(QPointF(), size);

    const QRectF viewBox = svgViewBoxAttribute();
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

    const QRectF fallbackRect = detectFallbackSvgRect();
    if(!fallbackRect.isEmpty())
        return QRectFIntegerized(fallbackRect);

    return QRectF(QPointF(), defaultSvgSize());
}

QSizeF AbstractSVGWebBrowser::svgSizeAttribute()
{
    const QSizeF size = QSizeF(parseLength(evalJS("document.rootElement.getAttribute('width');").toString()),
                               parseLength(evalJS("document.rootElement.getAttribute('height');").toString()));
    return size;
}

QRectF AbstractSVGWebBrowser::svgViewBoxAttribute()
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    typedef QRegularExpression QRE;
#else
    typedef QRegExp QRE;
#endif
    const QStringList viewBoxData = evalJS("document.rootElement.getAttribute('viewBox');").toString()
            .split(QRE(QString::fromLatin1("\\s")),
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
                   Qt::SkipEmptyParts);
#else
                   QString::SkipEmptyParts);
#endif
    return (viewBoxData.size() == 4)
            ? QRectF(parseLength(viewBoxData.at(0)),
                     parseLength(viewBoxData.at(1)),
                     parseLength(viewBoxData.at(2)),
                     parseLength(viewBoxData.at(3)))
            : QRectF();
}

QRectF AbstractSVGWebBrowser::svgBoundingBoxRect()
{
    const QRectF bBox(static_cast<qreal>(evalJS("document.rootElement.getBBox().x;").toDouble()),
                      static_cast<qreal>(evalJS("document.rootElement.getBBox().y;").toDouble()),
                      static_cast<qreal>(evalJS("document.rootElement.getBBox().width;").toDouble()),
                      static_cast<qreal>(evalJS("document.rootElement.getBBox().height;").toDouble()));
    return bBox.isEmpty() ? QRectF() : QRectFIntegerized(bBox);
}

QRectF AbstractSVGWebBrowser::svgBoundingClientRect()
{
    const QRectF boundingClientRect(static_cast<qreal>(evalJS("document.rootElement.getBoundingClientRect().x;").toDouble()),
                                    static_cast<qreal>(evalJS("document.rootElement.getBoundingClientRect().y;").toDouble()),
                                    static_cast<qreal>(evalJS("document.rootElement.getBoundingClientRect().width;").toDouble()),
                                    static_cast<qreal>(evalJS("document.rootElement.getBoundingClientRect().height;").toDouble()));
    return boundingClientRect.isEmpty() ? QRectF() : boundingClientRect;
}

bool AbstractSVGWebBrowser::rootElementIsSvg()
{
    const QString root = evalJS("document.rootElement.nodeName").toString();
    return !root.isEmpty() && root.compare(QString::fromLatin1("svg"), Qt::CaseInsensitive) == 0;
}

void AbstractSVGWebBrowser::removeRootOverflowAttribute()
{
    evalJS(
        "if(document.rootElement.hasAttribute('overflow')) {"
            "document.rootElement.removeAttribute('overflow');"
        "}"
        );
}

QRectF AbstractSVGWebBrowser::QRectFIntegerized(const QRectF &rect)
{
    const qreal left = std::floor(rect.left());
    const qreal top = std::floor(rect.top());
    const qreal width = std::ceil(rect.width() + std::abs(rect.left() - left));
    const qreal height = std::ceil(rect.height() + std::abs(rect.top() - top));
    return QRectF(left, top, width, height);
}

qreal AbstractSVGWebBrowser::parseLength(const QString &str)
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
