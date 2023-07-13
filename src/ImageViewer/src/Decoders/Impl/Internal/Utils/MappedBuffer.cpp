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

#include "MappedBuffer.h"

#include <QFile>
#include <QByteArray>
#include <QString>
#include <QDebug>
#include <QXmlStreamReader>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QStringConverter>
#else
#include <QTextCodec>
#endif

#include "Utils/Global.h"

#if defined (HAS_ZLIB)
#include "ZLibUtils.h"
#endif

struct MappedBuffer::Impl
{
    QFile file;
    QByteArray data;
    uchar *mapped;
    qint64 size;
    bool isValid;

    explicit Impl(const QString &filePath)
        : file(filePath)
        , mapped(Q_NULLPTR)
        , size(0)
        , isValid(false)
    {
        if(!file.open(QIODevice::ReadOnly))
        {
            qWarning() << "[MappedBuffer] Can't open" << filePath;
            return;
        }
        size = file.size();
        mapped = file.map(0, size);
        if(!mapped)
        {
            qWarning() << "[MappedBuffer] Can't map" << filePath;
            data = file.readAll();
            size = data.size();
            mapped = reinterpret_cast<uchar*>(data.data());
        }
        isValid = true;
    }

    explicit Impl(const QByteArray &rawBuffer)
        : data(rawBuffer)
        , mapped(reinterpret_cast<uchar*>(data.data()))
        , size(data.size())
        , isValid(!data.isEmpty())
    {}
};

MappedBuffer::MappedBuffer(const QByteArray &rawBuffer, const MappedBuffer::Options &options)
    : m_impl(new Impl(rawBuffer))
{
    if(options.testFlag(AutoInflate))
        autoInflate();
    if(options.testFlag(AutoConvertXmlToUtf8))
        autoConvertXmlToUtf8();
}

MappedBuffer::MappedBuffer(const QString &filePath, const MappedBuffer::Options &options)
    : m_impl(new Impl(filePath))
{
    if(options.testFlag(AutoInflate))
        autoInflate();
    if(options.testFlag(AutoConvertXmlToUtf8))
        autoConvertXmlToUtf8();
}

MappedBuffer::~MappedBuffer()
{}

bool MappedBuffer::isValid() const
{
    return m_impl->isValid;
}

QByteArray MappedBuffer::byteArray() const
{
    if(!isValid())
        return QByteArray();
    if(!m_impl->data.isEmpty())
        return m_impl->data;
    return QByteArray(dataAs<const char*>(), sizeAs<int>());
}

QByteArray MappedBuffer::dataAsByteArray() const
{
    return QByteArray::fromRawData(dataAs<const char*>(), sizeAs<int>());
}

qint64 MappedBuffer::size() const
{
    return m_impl->size;
}

uchar *MappedBuffer::data() const
{
    return m_impl->mapped;
}

bool MappedBuffer::isDeflated() const
{
    return isValid() && size() > 2 && data()[0] == 0x1f && data()[1] == 0x8b;
}

bool MappedBuffer::doInflate()
{
    if(!isValid())
    {
        qWarning() << "[MappedBuffer] Invalid data";
        return false;
    }
#if defined (HAS_ZLIB)
    const QByteArray rawData = dataAsByteArray();
    const QByteArray inflatted = ZLibUtils::InflateData(rawData);
    if(inflatted.isEmpty())
    {
        qWarning() << "[MappedBuffer] Can't inflate data";
        return false;
    }
    m_impl->data = inflatted;
    m_impl->size = m_impl->data.size();
    m_impl->mapped = reinterpret_cast<uchar*>(m_impl->data.data());
    return true;
#else
    qWarning() << "[MappedBuffer] ZLib is not supported";
    return false;
#endif
}

QString MappedBuffer::getXmlEncoding() const
{
    if(!isValid())
    {
        qWarning() << "[MappedBuffer] Invalid data";
        return QString();
    }
    const QByteArray xmlData = dataAsByteArray();
    QXmlStreamReader reader(xmlData);
    while(reader.readNext() != QXmlStreamReader::StartDocument && !reader.atEnd());
    return reader.documentEncoding().toString().simplified().toLower();
}

bool MappedBuffer::convertXmlToUtf8()
{
    if(!isValid())
    {
        qWarning() << "[MappedBuffer] Invalid data";
        return false;
    }
    const QString encoding = getXmlEncoding();
    if(encoding.isEmpty())
    {
        qWarning() << "[MappedBuffer] Can't detect encoding";
        return false;
    }
    if(encoding != QString::fromLatin1("utf-8"))
    {
        const QByteArray xmlData = dataAsByteArray();
        QString xmlDataString;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        xmlDataString = QStringDecoder(encoding.toLatin1()).decode(xmlData);
#else
        xmlDataString = QTextCodec::codecForName(encoding.toLatin1())->toUnicode(xmlData);
#endif
        m_impl->data = xmlDataString.toUtf8();
        m_impl->size = m_impl->data.size();
        m_impl->mapped = reinterpret_cast<uchar*>(m_impl->data.data());
    }
    return true;
}

void MappedBuffer::autoInflate()
{
    if(m_impl->isValid && isDeflated())
        m_impl->isValid = doInflate();
}

void MappedBuffer::autoConvertXmlToUtf8()
{
    if(m_impl->isValid && !getXmlEncoding().isEmpty())
        m_impl->isValid = convertXmlToUtf8();
}
