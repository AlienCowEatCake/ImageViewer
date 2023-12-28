/*
   Copyright (C) 2019-2023 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "XmlStreamReader.h"

#include <algorithm>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QStringConverter>
#endif
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0)) || defined (QT_CORE5COMPAT_LIB)
#include <QTextCodec>
#endif

#include "Utils/Global.h"

QString XmlStreamReader::getEncoding(const QByteArray &data)
{
    QXmlStreamReader reader(data);
    while(reader.readNext() != QXmlStreamReader::StartDocument && !reader.atEnd());
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    if(reader.documentEncoding().empty() && reader.hasError() && reader.error() == QXmlStreamReader::NotWellFormedError)
    {
        /// @note Encoding is ignored if input is QString
        const qint64 xmlStrLen = std::min(static_cast<qint64>(data.size()), static_cast<qint64>(reader.characterOffset() + 1));
        const QString xmlStr = QString::fromLatin1(data.data(), xmlStrLen);
        reader.clear();
        reader.addData(xmlStr);
        while(reader.readNext() != QXmlStreamReader::StartDocument && !reader.atEnd());
    }
#endif
    return reader.documentEncoding().toString().simplified();
}

QString XmlStreamReader::getDecodedString(const QByteArray &data)
{
    const QString encoding = getEncoding(data).toLower();
    if(!encoding.isEmpty() && encoding != QString::fromLatin1("utf-8"))
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        QStringDecoder decoder = QStringDecoder(encoding.toLatin1(), QStringConverter::Flag::Stateless);
        if(decoder.isValid())
            return decoder.decode(data);
#endif
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0)) || defined (QT_CORE5COMPAT_LIB)
        if(QTextCodec *codec = QTextCodec::codecForName(encoding.toLatin1()))
            return codec->toUnicode(data);
#endif
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const std::optional<QStringConverter::Encoding> encodingForData = QStringConverter::encodingForData(data);
    if(encodingForData.has_value())
        return QStringDecoder(encodingForData.value(), QStringConverter::Flag::Stateless).decode(data);
#endif
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0)) || defined (QT_CORE5COMPAT_LIB)
#if (QT_VERSION >= QT_VERSION_CHECK(4, 6, 0))
    if(QTextCodec *codec = QTextCodec::codecForUtfText(data, Q_NULLPTR))
#else
    if(QTextCodec *codec = QTextCodec::codecForUtfText(data))
#endif
        return codec->toUnicode(data);
#endif

    return QString::fromUtf8(data);
}

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0)) || !defined (QT_CORE5COMPAT_LIB)

XmlStreamReader::XmlStreamReader(const QByteArray &data)
    : QXmlStreamReader(data)
{}

#else

XmlStreamReader::XmlStreamReader(const QByteArray &data)
{
    addData(getDecodedString(data));
}

#endif
