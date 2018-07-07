/*
   Copyright (C) 2017-2018 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <QtGlobal>

#include <cstring>

extern "C" {
#include <jbig.h>
}

#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QDebug>

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"

namespace
{

QImage readJbigFile(const QString &filePath)
{
    QFile inFile(filePath);
    if(!inFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "Can't open" << filePath;
        return QImage();
    }
    QByteArray inBuffer = inFile.readAll();
    unsigned char *bufferData = reinterpret_cast<unsigned char*>(inBuffer.data());
    const std::size_t bufferSize = static_cast<std::size_t>(inBuffer.size());

    jbg_dec_state decoder;
    jbg_dec_init(&decoder);
    jbg_newlen(bufferData, bufferSize);

    int decodeStatus = jbg_dec_in(&decoder, bufferData, bufferSize, NULL);
    if(decodeStatus != JBG_EOK)
    {
        qWarning() << QString::fromLatin1("Error (%1) decoding: %2")
                      .arg(decodeStatus)
                      .arg(QString::fromLocal8Bit(jbg_strerror(decodeStatus)))
                      .toLocal8Bit().data();
        jbg_dec_free(&decoder);
        return QImage();
    }

    const int width = static_cast<int>(jbg_dec_getwidth(&decoder));
    const int height = static_cast<int>(jbg_dec_getheight(&decoder));
    QImage result(width, height, QImage::Format_Mono);
    if(result.isNull())
    {
        qWarning() << "Invalid image size";
        jbg_dec_free(&decoder);
        return QImage();
    }

    memcpy(result.bits(), jbg_dec_getimage(&decoder, 0), jbg_dec_getsize(&decoder));
    result.invertPixels();

    jbg_dec_free(&decoder);
    return result;
}

class DecoderJbigKit : public IDecoder
{
public:
    QString name() const
    {
        return QString::fromLatin1("DecoderJbigKit");
    }

    QStringList supportedFormats() const
    {
        return QStringList()
                << QString::fromLatin1("jbg")
                << QString::fromLatin1("jbig");
    }

    QStringList advancedFormats() const
    {
        return QStringList();
    }

    bool isAvailable() const
    {
        return true;
    }

    QGraphicsItem *loadImage(const QString &filePath)
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return NULL;
        return GraphicsItemsFactory::instance().createImageItem(readJbigFile(filePath));
    }
};

DecoderAutoRegistrator registrator(new DecoderJbigKit);

} // namespace
