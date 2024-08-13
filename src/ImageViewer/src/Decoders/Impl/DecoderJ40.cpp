/*
   Copyright (C) 2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined(__STDC_LIMIT_MACROS)
#define __STDC_LIMIT_MACROS
#endif
#include <cstring>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#define J40_CONFIRM_THAT_THIS_IS_EXPERIMENTAL_AND_POTENTIALLY_UNSAFE
#include <j40.h>

#include "Utils/Global.h"

#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QDebug>

#include "Utils/Global.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"
#include "Internal/PayloadWithMetaData.h"
#include "Internal/Utils/MappedBuffer.h"

namespace {

PayloadWithMetaData<QImage> readJ40File(const QString &filePath)
{
    const MappedBuffer inBuffer(filePath);
    if(!inBuffer.isValid())
        return QImage();

    j40_image image;
    memset(&image, 0, sizeof(image));
    j40_err err = 0;

    err = j40_from_memory(&image, inBuffer.dataAs<void*>(), inBuffer.sizeAs<size_t>(), Q_NULLPTR);
    if(err)
    {
        qWarning() << "ERROR: j40_from_memory:" << j40_error_string(&image);
        return QImage();
    }

    err = j40_output_format(&image, J40_RGBA, J40_U8X4);
    if(err)
    {
        qWarning() << "ERROR: j40_output_format:" << j40_error_string(&image);
        j40_free(&image);
        return QImage();
    }

    if(!j40_next_frame(&image))
    {
        qWarning() << "ERROR: j40_next_frame:" << j40_error_string(&image);
        j40_free(&image);
        return QImage();
    }

    j40_frame frame = j40_current_frame(&image);
    j40_pixels_u8x4 pixels = j40_frame_pixels_u8x4(&frame, J40_RGBA);

#define USE_RGBA_8888 (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))

    QImage result = QImage(static_cast<int>(pixels.width), static_cast<int>(pixels.height),
#if (USE_RGBA_8888)
                           QImage::Format_RGBA8888);
#else
                           QImage::Format_ARGB32);
#endif
    for(int y = 0; y < result.height(); ++y)
        memcpy(result.scanLine(y), j40_row_u8x4(pixels, y), result.width() * 4);
#if (!USE_RGBA_8888)
#if (Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
    QImage_rgbSwap(result);
#else
    for(int y = 0; y < result.height(); ++y)
    {
        QRgb* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for(int x = 0; x < result.width(); ++x)
            line[x] = qRgba(qAlpha(line[x]), qRed(line[x]), qGreen(line[x]), qBlue(line[x]));
    }
#endif
#endif

#undef USE_RGBA_8888

    j40_free(&image);

    ImageMetaData *metaData = ImageMetaData::createMetaData(inBuffer.dataAsByteArray());
    return PayloadWithMetaData<QImage>(result, metaData);
}

class DecoderJ40 : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderJ40");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
               << QString::fromLatin1("jxl");
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
        const PayloadWithMetaData<QImage> readResult = readJ40File(filePath);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createImageItem(readResult);
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), readResult.metaData()));
    }
};

DecoderAutoRegistrator registrator(new DecoderJ40);

} // namespace
