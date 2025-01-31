/*
   Copyright (C) 2017-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "ZLibUtils.h"

#include <cassert>

#include <QDataStream>
#include <QFile>

#include <zlib.h>

#include "Utils/Logging.h"

#define CHUNK 16384

namespace {

QByteArray zerr(int ret)
{
    QByteArray description = "zpipe: ";
    switch(ret)
    {
    case Z_ERRNO:
        description.append("error reading stream");
        break;
    case Z_STREAM_ERROR:
        description.append("invalid compression level");
        break;
    case Z_DATA_ERROR:
        description.append("invalid or incomplete deflate data");
        break;
    case Z_MEM_ERROR:
        description.append("out of memory");
        break;
    case Z_VERSION_ERROR:
        description.append("zlib version mismatch!");
        break;
    }
    return description;
}

QByteArray InflateDataStream(QDataStream& source)
{
    int ret;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    // allocate inflate state
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit2(&strm, 16 + MAX_WBITS);
    if(ret != Z_OK)
    {
        LOG_WARNING() << LOGGING_CTX << zerr(ret);
        return QByteArray();
    }

    QByteArray dest;

    // decompress until deflate stream ends or end of file
    do
    {
        int count = source.readRawData(reinterpret_cast<char*>(in), CHUNK);
        if(count < 0)
        {
            inflateEnd(&strm);
            LOG_WARNING() << LOGGING_CTX << zerr(Z_ERRNO);
            return QByteArray();
        }
        if(count == 0)
            break;
        strm.avail_in = static_cast<unsigned>(count);
        strm.next_in = in;

        // run inflate() on input until output buffer not full
        do
        {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR); // state not clobbered
            switch(ret)
            {
            case Z_NEED_DICT:
                inflateEnd(&strm);
                LOG_WARNING() << LOGGING_CTX << zerr(Z_DATA_ERROR);
                return QByteArray();
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                inflateEnd(&strm);
                LOG_WARNING() << LOGGING_CTX << zerr(ret);
                return QByteArray();
            }
            int have = static_cast<int>(CHUNK - strm.avail_out);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
            dest.append(reinterpret_cast<const char*>(out), have);
#else
            dest.append(QByteArray(reinterpret_cast<const char*>(out), have));
#endif
        }
        while(strm.avail_out == 0);

        // done when inflate() says it's done
    }
    while(ret != Z_STREAM_END);

    // clean up and return
    inflateEnd(&strm);
    if(ret != Z_STREAM_END)
    {
        LOG_WARNING() << LOGGING_CTX << zerr(Z_DATA_ERROR);
        return QByteArray();
    }
    return dest;
}

} // namespace

namespace ZLibUtils {

QByteArray InflateDevice(QIODevice *device)
{
    if(!device)
        return QByteArray();
    QDataStream source(device);
    return InflateDataStream(source);
}

QByteArray InflateFile(const QString &filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        LOG_WARNING() << LOGGING_CTX << "Can't open" << filePath;
        return QByteArray();
    }
    return InflateDevice(&file);
}

QByteArray InflateData(const QByteArray &byteArray)
{
    QDataStream source(byteArray);
    return InflateDataStream(source);
}

} // namespace ZLibUtils
