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

#if !defined(DECODER_XMLSTREAMREADER_H_INCLUDED)
#define DECODER_XMLSTREAMREADER_H_INCLUDED

#include <QByteArray>
#include <QXmlStreamReader>

class XmlStreamReader : public QXmlStreamReader
{
    Q_DISABLE_COPY(XmlStreamReader)

public:
    static QString getEncoding(const QByteArray& data);
    static QString getDecodedString(const QByteArray& data);

    explicit XmlStreamReader(const QByteArray& data);
};

#endif
