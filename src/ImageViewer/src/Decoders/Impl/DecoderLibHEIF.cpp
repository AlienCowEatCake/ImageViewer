/*
   Copyright (C) 2018-2021 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "Utils/Global.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"
#include "Internal/PayloadWithMetaData.h"
#include "Internal/Utils/MappedBuffer.h"

#if defined (LIBHEIF_NUMERIC_VERSION) && (LIBHEIF_NUMERIC_VERSION >= 0x01030000)
#define USE_STREAM_READER_API
#endif

namespace
{

#if defined (USE_STREAM_READER_API)

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

#endif

PayloadWithMetaData<QImage> readHeifFile(const QString &filePath)
{
#if defined (USE_STREAM_READER_API)
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
    heif_error error = heif_context_read_from_reader(ctx, &reader, &inFile, Q_NULLPTR);
#else
    const MappedBuffer inBuffer(filePath);
    if(!inBuffer.isValid())
        return QImage();

    heif_context *ctx = heif_context_alloc();
    heif_error error = heif_context_read_from_memory(ctx, inBuffer.dataAs<const void*>(), inBuffer.sizeAs<size_t>(), Q_NULLPTR);
#endif
    if(error.code != heif_error_Ok)
    {
        qWarning() << "Can't read:" << error.message;
        heif_context_free(ctx);
        return QImage();
    }

    heif_image_handle *handle = Q_NULLPTR;
    error = heif_context_get_primary_image_handle(ctx, &handle);
    if(error.code != heif_error_Ok)
    {
        qWarning() << "Can't get primary image handle:" << error.message;
        heif_context_free(ctx);
        return QImage();
    }

    heif_image *img = Q_NULLPTR;
    heif_decoding_options *options = heif_decoding_options_alloc();
#if defined (LIBHEIF_NUMERIC_VERSION) && (LIBHEIF_NUMERIC_VERSION >= 0x01070000)
    if(options->version >= 2)
        options->convert_hdr_to_8bit = 1;
#endif
    error = heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGBA, options);
    heif_decoding_options_free(options);
    if(error.code != heif_error_Ok)
    {
        qWarning() << "Can't decode image:" << error.message;
        heif_image_handle_release(handle);
        heif_context_free(ctx);
        return QImage();
    }

    const int width = static_cast<int>(heif_image_get_width(img, heif_channel_interleaved));
    const int height = static_cast<int>(heif_image_get_height(img, heif_channel_interleaved));
    int stride = 0;
    const uint8_t *planeData = heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride);
    QImage result = (heif_image_get_chroma_format(img) == heif_chroma_interleaved_RGB) ///< libheif-1.7.0, WTF?
            ? QImage(reinterpret_cast<const uchar*>(planeData), width, height, stride, QImage::Format_RGB888).convertToFormat(QImage::Format_RGB32)
            : QImage(reinterpret_cast<const uchar*>(planeData), width, height, stride, QImage::Format_ARGB32).rgbSwapped();
    if(result.isNull())
    {
        qWarning() << "Invalid image size:" << width << "x" << height << "chroma_format:" << heif_image_get_chroma_format(img);
        heif_image_release(img);
        heif_image_handle_release(handle);
        heif_context_free(ctx);
        return QImage();
    }

#if defined (USE_STREAM_READER_API)
    ImageMetaData *metaData = ImageMetaData::createMetaData(filePath);
#else
    ImageMetaData *metaData = ImageMetaData::createMetaData(QByteArray::fromRawData(inBuffer.dataAs<const char*>(), inBuffer.sizeAs<int>()));
#endif
    const int numberOfMetadataBlocks = heif_image_handle_get_number_of_metadata_blocks(handle, Q_NULLPTR);
    if(numberOfMetadataBlocks > 0)
    {
        heif_item_id *ids = new heif_item_id[numberOfMetadataBlocks];
        heif_image_handle_get_list_of_metadata_block_IDs(handle, Q_NULLPTR, ids, numberOfMetadataBlocks);
        for(int i = 0; i < numberOfMetadataBlocks; ++i)
        {
            if(strcmp(heif_image_handle_get_metadata_type(handle, ids[i]), "Exif") == 0)
            {
                qDebug() << "Found EXIF metadata";
                const size_t metadataSize = heif_image_handle_get_metadata_size(handle, ids[i]);
                QByteArray rawMetadata(static_cast<int>(metadataSize), 0);
                error = heif_image_handle_get_metadata(handle, ids[i], rawMetadata.data());
                if(error.code != heif_error_Ok)
                {
                    qWarning() << "Can't get EXIF metadata:" << error.message;
                }
                else
                {
                    const int offset = 10;
                    metaData = ImageMetaData::joinMetaData(metaData, ImageMetaData::createExifMetaData(QByteArray::fromRawData(rawMetadata.constData() + offset, rawMetadata.size() - offset)));
                }
            }
            else if(strcmp(heif_image_handle_get_metadata_type(handle, ids[i]), "XMP") == 0)
            {
                qDebug() << "Found XMP metadata";
                const size_t metadataSize = heif_image_handle_get_metadata_size(handle, ids[i]);
                QByteArray rawMetadata(static_cast<int>(metadataSize), 0);
                error = heif_image_handle_get_metadata(handle, ids[i], rawMetadata.data());
                if(error.code != heif_error_Ok)
                    qWarning() << "Can't get XMP metadata:" << error.message;
                else
                    metaData = ImageMetaData::joinMetaData(metaData, ImageMetaData::createXmpMetaData(QByteArray::fromRawData(rawMetadata.constData(), rawMetadata.size())));
            }
        }
        delete [] ids;
    }

    heif_image_release(img);
    heif_image_handle_release(handle);
    heif_context_free(ctx);
    return PayloadWithMetaData<QImage>(result, metaData);
}

class DecoderLibHEIF : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderLibHEIF");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
                << QString::fromLatin1("heif")
                << QString::fromLatin1("heic")
                << QString::fromLatin1("heix")
#if defined (LIBHEIF_NUMERIC_VERSION) && (LIBHEIF_NUMERIC_VERSION >= 0x01070000)
                << QString::fromLatin1("avif")
#endif
                   ;
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
        const PayloadWithMetaData<QImage> readData = readHeifFile(filePath);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createImageItem(readData);
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), readData.metaData()));
    }
};

DecoderAutoRegistrator registrator(new DecoderLibHEIF);

} // namespace
