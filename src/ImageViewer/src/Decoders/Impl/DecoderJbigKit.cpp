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

#include <QtGlobal>

#include <cstring>

extern "C" {
#include <jbig.h>
}

#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QByteArray>

#include "Utils/Global.h"
#include "Utils/Logging.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"
#include "Internal/PayloadWithMetaData.h"
#include "Internal/Utils/MappedBuffer.h"

namespace
{

PayloadWithMetaData<QImage> readJbigFile(const QString &filePath)
{
    const MappedBuffer inBuffer(filePath);
    if(!inBuffer.isValid())
        return QImage();

    jbg_dec_state decoder;
    jbg_dec_init(&decoder);
    jbg_newlen(inBuffer.dataAs<unsigned char*>(), inBuffer.sizeAs<std::size_t>());

    int decodeStatus = jbg_dec_in(&decoder, inBuffer.dataAs<unsigned char*>(), inBuffer.sizeAs<std::size_t>(), Q_NULLPTR);
    if(decodeStatus != JBG_EOK)
    {
        LOG_WARNING() << LOGGING_CTX << QString::fromLatin1("Error (%1) decoding: %2")
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
        LOG_WARNING() << LOGGING_CTX << "Invalid image size";
        jbg_dec_free(&decoder);
        return QImage();
    }

    memcpy(result.bits(), jbg_dec_getimage(&decoder, 0), jbg_dec_getsize(&decoder));
    result.invertPixels();

    jbg_dec_free(&decoder);

    ImageMetaData *metaData = ImageMetaData::createMetaData(inBuffer.dataAsByteArray());
    return PayloadWithMetaData<QImage>(result, metaData);
}

class DecoderJbigKit : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderJbigKit");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
                << QString::fromLatin1("jbg")
                << QString::fromLatin1("jbig");
    }

    QStringList advancedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList();
    }

    bool isAvailable() const Q_DECL_OVERRIDE
    {
        return true;
    }

    QSharedPointer<IImageData> loadImage(const QString &filePath) Q_DECL_OVERRIDE
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return QSharedPointer<IImageData>();
        const PayloadWithMetaData<QImage> readResult = readJbigFile(filePath);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createImageItem(readResult);
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), readResult.metaData()));
    }
};

DecoderAutoRegistrator registrator(new DecoderJbigKit);

} // namespace
