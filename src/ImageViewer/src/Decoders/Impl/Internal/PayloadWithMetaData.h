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

#if !defined(PAYLOAD_WITH_METADATA_H_INCLUDED)
#define PAYLOAD_WITH_METADATA_H_INCLUDED

#include <cstddef>

class IImageMetaData;

template<typename T>
class PayloadWithMetaData
{
public:
    PayloadWithMetaData()
        : m_payload(T())
        , m_metaData(NULL)
    {}

    PayloadWithMetaData(const T &payload)
        : m_payload(payload)
        , m_metaData(NULL)
    {}

    PayloadWithMetaData(const T &payload, IImageMetaData *metaData)
        : m_payload(payload)
        , m_metaData(metaData)
    {}

    PayloadWithMetaData(const PayloadWithMetaData &other)
        : m_payload(other.m_payload)
        , m_metaData(other.m_metaData)
    {}

    T &operator=(const PayloadWithMetaData &other)
    {
        if(this != &other)
        {
            m_payload = other.m_payload;
            m_metaData = other.m_metaData;
        }
        return *this;
    }

    operator const T&() const
    {
        return m_payload;
    }

    IImageMetaData *metaData() const
    {
        return m_metaData;
    }

private:
    T const m_payload;
    IImageMetaData * const m_metaData;
};

#endif // PAYLOAD_WITH_METADATA_H_INCLUDED
