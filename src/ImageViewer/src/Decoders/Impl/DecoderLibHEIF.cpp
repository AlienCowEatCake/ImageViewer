/*
   Copyright (C) 2018 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <libheif/heif.h>

#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QDebug>

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/Utils/ExifUtils.h"

namespace
{

int64_t getPositionCallback(void *userdata)
{
    QIODevice *device = static_cast<QIODevice*>(userdata);
    return static_cast<int64_t>(device->pos());
}

int readCallback(void *data, size_t size, void *userdata)
{
    QIODevice *device = static_cast<QIODevice*>(userdata);
    return (device->isReadable() && device->read(static_cast<char*>(data), static_cast<qint64>(size))) ? 0 : -1;
}

int seekCallback(int64_t position, void *userdata)
{
    QIODevice *device = static_cast<QIODevice*>(userdata);
    return device->seek(static_cast<qint64>(position)) ? 0 : -1;
}

heif_reader_grow_status waitForFileSizeCallback(int64_t target_size, void *userdata)
{
    QIODevice *device = static_cast<QIODevice*>(userdata);
    return (device->size() >= static_cast<qint64>(target_size))
            ? heif_reader_grow_status_size_reached
            : heif_reader_grow_status_size_beyond_eof;
}

QImage readHeifFile(const QString &filePath)
{
    QFile inFile(filePath);
    if(!inFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "Can't open" << filePath;
        return QImage();
    }

    heif_context *ctx = heif_context_alloc();
    heif_reader reader;
    memset(&reader, 0, sizeof(heif_reader));
    reader.reader_api_version = 1;
    reader.get_position = &getPositionCallback;
    reader.read = &readCallback;
    reader.seek = &seekCallback;
    reader.wait_for_file_size = &waitForFileSizeCallback;
    heif_error error = heif_context_read_from_reader(ctx, &reader, &inFile, NULL);
    if(error.code != heif_error_Ok)
    {
        qWarning() << "Can't read:" << error.message;
        heif_context_free(ctx);
        return QImage();
    }

    heif_image_handle *handle = NULL;
    error = heif_context_get_primary_image_handle(ctx, &handle);
    if(error.code != heif_error_Ok)
    {
        qWarning() << "Can't get primary image handle:" << error.message;
        heif_context_free(ctx);
        return QImage();
    }

    heif_image *img = NULL;
    error = heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGBA, NULL);
    if(error.code != heif_error_Ok)
    {
        qWarning() << "Can't decode image:" << error.message;
        heif_image_handle_release(handle);
        heif_context_free(ctx);
        return QImage();
    }

    const int width = static_cast<int>(heif_image_get_width(img, heif_channel_interleaved));
    const int height = static_cast<int>(heif_image_get_height(img, heif_channel_interleaved));
    QImage result(width, height, QImage::Format_ARGB32);
    if(result.isNull())
    {
        qWarning() << "Invalid image size:" << width << "x" << height;
        heif_image_release(img);
        heif_image_handle_release(handle);
        heif_context_free(ctx);
        return QImage();
    }

    int stride = 0;
    const uint8_t *planeData = heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride);
    QRgb *imageData = reinterpret_cast<QRgb*>(result.bits());
    for(int j = 0; j < result.height(); j++)
    {
        const uint8_t *currentPlaneByte = planeData + j * stride;
        for(int i = 0; i < result.width(); i++)
        {
            const int r = *(currentPlaneByte++);
            const int g = *(currentPlaneByte++);
            const int b = *(currentPlaneByte++);
            const int a = *(currentPlaneByte++);
            *(imageData++) = qRgba(r, g, b, a);
        }
    }

    /// @note Orientation is already applied by default in decoding options
//    quint16 orientation = 1;
//    const int numberOfMetadataBlocks = heif_image_handle_get_number_of_metadata_blocks(handle, NULL);
//    if(numberOfMetadataBlocks > 0)
//    {
//        heif_item_id *ids = new heif_item_id[numberOfMetadataBlocks];
//        heif_image_handle_get_list_of_metadata_block_IDs(handle, NULL, ids, numberOfMetadataBlocks);
//        for(int i = 0; i < numberOfMetadataBlocks; ++i)
//        {
//            if(strcmp(heif_image_handle_get_metadata_type(handle, ids[i]), "Exif") == 0)
//            {
//                qDebug() << "Found EXIF metadata";
//                const size_t metadataSize = heif_image_handle_get_metadata_size(handle, ids[i]);
//                QByteArray metadata(static_cast<int>(metadataSize), 0);
//                error = heif_image_handle_get_metadata(handle, ids[i], metadata.data());
//                if(error.code != heif_error_Ok)
//                {
//                    qWarning() << "Can't get EXIF metadata:" << error.message;
//                }
//                else
//                {
//                    const int offset = 10;
//                    orientation = ExifUtils::GetExifOrientation(QByteArray::fromRawData(metadata.constData() + offset, metadata.size() - offset));
//                }
//            }
//        }
//        delete [] ids;
//    }
//    ExifUtils::ApplyExifOrientation(&result, orientation);

    heif_image_release(img);
    heif_image_handle_release(handle);
    heif_context_free(ctx);
    return result;
}

class DecoderLibHEIF : public IDecoder
{
public:
    QString name() const
    {
        return QString::fromLatin1("DecoderLibHEIF");
    }

    QStringList supportedFormats() const
    {
        return QStringList()
                << QString::fromLatin1("heif")
                << QString::fromLatin1("heic");
    }

    QStringList advancedFormats() const
    {
        return QStringList();
    }

    bool isAvailable() const
    {
        return true;
    }

    QSharedPointer<IImageData> loadImage(const QString &filePath)
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return QSharedPointer<IImageData>();
        return QSharedPointer<IImageData>(new ImageData(GraphicsItemsFactory::instance().createImageItem(readHeifFile(filePath)), name()));
    }
};

DecoderAutoRegistrator registrator(new DecoderLibHEIF);

} // namespace
