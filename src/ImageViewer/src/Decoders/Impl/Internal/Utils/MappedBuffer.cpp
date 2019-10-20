/*
   Copyright (C) 2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "Utils/Global.h"

#if defined (HAS_ZLIB)
#include "ZLibUtils.h"
#endif

struct MappedBuffer::Impl
{
    QFile file;
    uchar *mapped;
    qint64 size;
    QByteArray data;
    bool isValid;

    Impl(const QString &filePath)
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
};

MappedBuffer::MappedBuffer(const QString &filePath)
    : m_impl(new Impl(filePath))
{}

MappedBuffer::~MappedBuffer()
{}

bool MappedBuffer::isValid() const
{
    return m_impl->isValid;
}

qint64 MappedBuffer::size() const
{
    return m_impl->size;
}

uchar *MappedBuffer::data() const
{
    return m_impl->mapped;
}

bool MappedBuffer::doInflate()
{
    if(!isValid())
    {
        qWarning() << "[MappedBuffer] Invalid data";
        return false;
    }
#if defined (HAS_ZLIB)
    const QByteArray rawData = QByteArray::fromRawData(dataAs<const char*>(), sizeAs<int>());
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
