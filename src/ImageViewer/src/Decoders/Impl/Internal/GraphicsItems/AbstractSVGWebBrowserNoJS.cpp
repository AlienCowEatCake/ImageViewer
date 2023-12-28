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

#include "AbstractSVGWebBrowserNoJS.h"

#include <QByteArray>
#include <QDebug>
#include <QRect>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QRegularExpression>
#else
#include <QRegExp>
#endif
#include <QSize>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QXmlStreamAttributes>

#include "../Utils/XmlStreamReader.h"

AbstractSVGWebBrowserNoJS::AbstractSVGWebBrowserNoJS()
{}

AbstractSVGWebBrowserNoJS::~AbstractSVGWebBrowserNoJS()
{}

QVariant AbstractSVGWebBrowserNoJS::evalJSImpl(const QString &scriptSource)
{
    Q_UNUSED(scriptSource);
    qWarning() << __FUNCTION__ << "JavaScript is not available for NoJS implementation";
    return QVariant();
}

QSizeF AbstractSVGWebBrowserNoJS::svgSizeAttribute()
{
    const QByteArray svgData = getSvgData();
    XmlStreamReader reader(svgData);
    while(reader.readNext() != QXmlStreamReader::StartElement && !reader.atEnd());
    if(reader.atEnd())
        return QSizeF();
    const QXmlStreamAttributes attributes = reader.attributes();
    const QSizeF size = QSizeF(parseLength(attributes.value(QString::fromLatin1("width")).toString()),
                               parseLength(attributes.value(QString::fromLatin1("height")).toString()));
    return size;
}

QRectF AbstractSVGWebBrowserNoJS::svgViewBoxAttribute()
{
    const QByteArray svgData = getSvgData();
    XmlStreamReader reader(svgData);
    while(reader.readNext() != QXmlStreamReader::StartElement && !reader.atEnd());
    if(reader.atEnd())
        return QRectF();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    typedef QRegularExpression QRE;
#else
    typedef QRegExp QRE;
#endif
    const QStringList viewBoxData = reader.attributes().value(QString::fromLatin1("viewBox")).toString()
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

QRectF AbstractSVGWebBrowserNoJS::svgBoundingBoxRect()
{
    return QRectF(); /// @todo
}

QRectF AbstractSVGWebBrowserNoJS::svgBoundingClientRect()
{
    return QRectF(); /// @todo
}

bool AbstractSVGWebBrowserNoJS::rootElementIsSvg()
{
    const QByteArray svgData = getSvgData();
    XmlStreamReader reader(svgData);
    while(reader.readNext() != QXmlStreamReader::StartElement && !reader.atEnd());
    return !reader.name().toString().compare(QString::fromLatin1("svg"), Qt::CaseInsensitive);
}
