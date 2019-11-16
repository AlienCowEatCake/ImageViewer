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

#if !defined(ABSTRACT_SVG_WEB_BROWSER_NO_JS_H_INCLUDED)
#define ABSTRACT_SVG_WEB_BROWSER_NO_JS_H_INCLUDED

#include "Utils/Global.h"

#include "AbstractSVGWebBrowser.h"

class QByteArray;

class AbstractSVGWebBrowserNoJS : public AbstractSVGWebBrowser
{
    Q_DISABLE_COPY(AbstractSVGWebBrowserNoJS)

public:
    AbstractSVGWebBrowserNoJS();
    ~AbstractSVGWebBrowserNoJS();

protected:
    virtual QByteArray getSvgData() = 0;

    QVariant evalJSImpl(const QString &scriptSource) Q_DECL_OVERRIDE;

    QSizeF svgSizeAttribute() Q_DECL_OVERRIDE;
    QRectF svgViewBoxAttribute() Q_DECL_OVERRIDE;
    QRectF svgBoundingBoxRect() Q_DECL_OVERRIDE;
    QRectF svgBoundingClientRect() Q_DECL_OVERRIDE;

    bool rootElementIsSvg() Q_DECL_OVERRIDE;
};

#endif // ABSTRACT_SVG_WEB_BROWSER_NO_JS_H_INCLUDED
