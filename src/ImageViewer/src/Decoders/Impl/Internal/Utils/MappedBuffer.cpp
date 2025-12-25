/*
   Copyright (C) 2019-2025 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <limits>

#include <QFile>
#include <QByteArray>
#include <QString>

#include "Utils/Global.h"
#include "Utils/Logging.h"

#include "XmlStreamReader.h"

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
            LOG_WARNING() << LOGGING_CTX << "Can't open" << filePath;
            return;
        }
        size = file.size();
        const quint64 maxSize = qMin(static_cast<quint64>(
#if (QT_VERSION >= QT_VERSION_CHECK(6, 8, 0))
                QByteArray::maxSize()
#elif (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                std::numeric_limits<qsizetype>::max()
#else
                std::numeric_limits<int>::max()
#endif
                ), static_cast<quint64>(std::numeric_limits<qint64>::max()));
        if(static_cast<quint64>(size) >= maxSize - 1)
        {
            LOG_WARNING() << LOGGING_CTX << "Wrong size =" << size << "maxSize =" << maxSize;
            size = 0;
            return;
        }
        mapped = file.map(0, size);
        if(!mapped)
        {
            LOG_WARNING() << LOGGING_CTX << "Can't map" << filePath;
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
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    return QByteArray(dataAs<const char*>(), sizeAs<qsizetype>());
#else
    return QByteArray(dataAs<const char*>(), sizeAs<int>());
#endif
}

QByteArray MappedBuffer::dataAsByteArray() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    return QByteArray::fromRawData(dataAs<const char*>(), sizeAs<qsizetype>());
#else
    return QByteArray::fromRawData(dataAs<const char*>(), sizeAs<int>());
#endif
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
        LOG_WARNING() << LOGGING_CTX << "Invalid data";
        return false;
    }
#if defined (HAS_ZLIB)
    const QByteArray rawData = dataAsByteArray();
    const QByteArray inflatted = ZLibUtils::InflateData(rawData);
    if(inflatted.isEmpty())
    {
        LOG_WARNING() << LOGGING_CTX << "Can't inflate data";
        return false;
    }
    m_impl->data = inflatted;
    m_impl->size = m_impl->data.size();
    m_impl->mapped = reinterpret_cast<uchar*>(m_impl->data.data());
    return true;
#else
    LOG_WARNING() << LOGGING_CTX << "ZLib is not supported";
    return false;
#endif
}

QString MappedBuffer::getXmlEncoding() const
{
    if(!isValid())
    {
        LOG_WARNING() << LOGGING_CTX << "Invalid data";
        return QString();
    }
    const QByteArray xmlData = dataAsByteArray();
    return XmlStreamReader::getEncoding(xmlData).toLower();
}

bool MappedBuffer::convertXmlToUtf8()
{
    if(!isValid())
    {
        LOG_WARNING() << LOGGING_CTX << "Invalid data";
        return false;
    }
    const QString encoding = getXmlEncoding();
    if(encoding.isEmpty())
    {
        LOG_WARNING() << LOGGING_CTX << "Can't detect encoding";
        return false;
    }
    if(encoding != QString::fromLatin1("utf-8"))
    {
        m_impl->data = XmlStreamReader::getDecodedString(dataAsByteArray()).toUtf8();
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
