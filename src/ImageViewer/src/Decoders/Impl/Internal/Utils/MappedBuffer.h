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

#if !defined(DECODER_MAPPED_BUFFER_H_INCLUDED)
#define DECODER_MAPPED_BUFFER_H_INCLUDED

#include "Utils/ScopedPointer.h"

class QString;

class MappedBuffer
{
    Q_DISABLE_COPY(MappedBuffer)

public:
    explicit MappedBuffer(const QString &filePath);
    ~MappedBuffer();

    bool isValid() const;

    qint64 size() const;
    uchar *data() const;

    template<typename T>
    T sizeAs() const { return static_cast<T>(size()); }

    template<typename T>
    T dataAs() const { return reinterpret_cast<T>(data()); }

    bool doInflate(); ///< @note Not inflate() due to conflict with zlib

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // DECODER_MAPPED_BUFFER_H_INCLUDED
