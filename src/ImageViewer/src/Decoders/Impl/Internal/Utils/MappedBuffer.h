/*
   Copyright (C) 2019-2026 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <QFlags>

#include "Utils/ScopedPointer.h"

class QByteArray;
class QString;

class MappedBuffer
{
    Q_DISABLE_COPY(MappedBuffer)

public:
    enum Option {
        NoOptions               = 0,
        AutoInflate             = 1 << 0,
        AutoConvertXmlToUtf8    = 1 << 1
    };
    Q_DECLARE_FLAGS(Options, Option)

    explicit MappedBuffer(const QByteArray &rawBuffer, const Options &options = NoOptions);
    explicit MappedBuffer(const QString &filePath, const Options &options = NoOptions);
    ~MappedBuffer();

    bool isValid() const;

    QByteArray byteArray() const;
    QByteArray dataAsByteArray() const;

    qint64 size() const;
    uchar *data() const;

    template<typename T>
    T sizeAs() const { return static_cast<T>(size()); }

    template<typename T>
    T dataAs() const { return reinterpret_cast<T>(data()); }

    bool isDeflated() const;
    bool doInflate(); ///< @note Not inflate() due to conflict with zlib

    QString getXmlEncoding() const;
    bool convertXmlToUtf8();

private:
    bool convertXmlToUtf8(const QString &encoding);

    void autoInflate();
    void autoConvertXmlToUtf8();

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MappedBuffer::Options)

#endif // DECODER_MAPPED_BUFFER_H_INCLUDED
