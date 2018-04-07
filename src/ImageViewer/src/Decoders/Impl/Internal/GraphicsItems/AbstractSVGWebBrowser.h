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

#if !defined(ABSTRACT_SVG_WEB_BROWSER_H_INCLUDED)
#define ABSTRACT_SVG_WEB_BROWSER_H_INCLUDED

#include <QtGlobal>

class QVariant;
class QString;
class QSizeF;
class QRectF;

class AbstractSVGWebBrowser
{
    Q_DISABLE_COPY(AbstractSVGWebBrowser)

public:
    AbstractSVGWebBrowser();
    virtual ~AbstractSVGWebBrowser();

protected:
    virtual QVariant evalJS(const QString &scriptSource);
    virtual QVariant evalJS(const char *scriptSource);
    virtual QVariant evalJSImpl(const QString &scriptSource) = 0;

    virtual QSizeF defaultSvgSize() const;
    virtual QRectF detectSvgRect();

    virtual QSizeF svgSizeAttribute();
    virtual QRectF svgViewBoxAttribute();
    virtual QRectF svgBoundingBoxRect();

    virtual bool rootElementIsSvg();

    static QRectF QRectFIntegerized(const QRectF &rect);
    static qreal parseLength(const QString &str);
};

#endif // ABSTRACT_SVG_WEB_BROWSER_H_INCLUDED
