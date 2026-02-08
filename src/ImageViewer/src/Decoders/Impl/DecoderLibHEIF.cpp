/*
   Copyright (C) 2018-2026 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <algorithm>
#include <cstring>

#include <libheif/heif.h>
/// @note Sequences API is available since 1.20.0, but it is mostly broken until 1.21.0
#if defined (LIBHEIF_HAVE_VERSION) && (LIBHEIF_HAVE_VERSION(1, 21, 0))
#include <libheif/heif_sequences.h>
#define USE_LIBHEIF_SEQUENCES
#endif

#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QPixmap>

#include "Utils/Global.h"
#include "Utils/Logging.h"
#include "Utils/ScopedPointer.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"
#include "Internal/PayloadWithMetaData.h"
#include "Internal/Animation/DelayCalculator.h"
#include "Internal/Animation/IAnimationProvider.h"
#include "Internal/Utils/MappedBuffer.h"

#if defined (LIBHEIF_NUMERIC_VERSION) && (LIBHEIF_NUMERIC_VERSION >= 0x01030000)
#define USE_STREAM_READER_API
#endif
#if defined (LIBHEIF_HAVE_VERSION) && (LIBHEIF_HAVE_VERSION(1, 13, 0))
#define USE_LIBHEIF_INIT_API
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

struct HeifInitGuard
{
#if defined (USE_LIBHEIF_INIT_API)
    HeifInitGuard()     { heif_init(Q_NULLPTR); }
    ~HeifInitGuard()    { heif_deinit(); }
#endif
};

template<typename T>
class SP
{
    Q_DISABLE_COPY(SP)
public:
    SP()
        : m_dataRef(Q_NULLPTR)
        , m_deleter(Q_NULLPTR)
    {}

    SP(T *dataRef, void (*deleter)(T*))
        : m_dataRef(dataRef)
        , m_deleter(deleter)
    {}

    SP(T *dataRef, void (*deleter)(const T*))
        : m_dataRef(dataRef)
        , m_deleter((void (*)(T*))deleter)
    {}

    ~SP()
    {
        reset();
    }

    inline operator bool() const    { return !!m_dataRef; }
    inline operator T * () const    { return m_dataRef; }
    inline T ** operator & ()       { return &m_dataRef; }
    inline T * operator -> () const { return m_dataRef; }

    void swap(SP<T> &other)
    {
        std::swap(m_dataRef, other.m_dataRef);
        std::swap(m_deleter, other.m_deleter);
    }

    void reset()
    {
        if(m_dataRef && m_deleter)
            m_deleter(m_dataRef);
        m_dataRef = Q_NULLPTR;
        m_deleter = Q_NULLPTR;
    }

    void reset(T *dataRef, void (*deleter)(T*))
    {
        reset();
        m_dataRef = dataRef;
        m_deleter = deleter;
    }

    void reset(T *dataRef, void (*deleter)(const T*))
    {
        reset(dataRef, (void (*)(T*))deleter);
    }

private:
    T *m_dataRef;
    void (*m_deleter)(T*);
};

class HeifAnimationProvider : public IAnimationProvider
{
    Q_DISABLE_COPY(HeifAnimationProvider)

public:
    HeifAnimationProvider()
        : m_nextImageDelay(-1)
#if defined (USE_LIBHEIF_SEQUENCES)
        , m_timescale(0)
#endif
    {
    }

    bool isValid() const Q_DECL_OVERRIDE
    {
        return !m_currentImage.isNull();
    }

    bool isSingleFrame() const Q_DECL_OVERRIDE
    {
#if defined (USE_LIBHEIF_SEQUENCES)
        if(m_ctx && heif_context_has_sequence(m_ctx) && m_timescale && m_track)
            return false;
#endif
        return true;
    }

    int nextImageDelay() const Q_DECL_OVERRIDE
    {
        return m_nextImageDelay;
    }

    bool jumpToNextImage() Q_DECL_OVERRIDE
    {
#if defined (USE_LIBHEIF_SEQUENCES)
        if(isSingleFrame())
            return false;

        SP<heif_image> img(Q_NULLPTR, &heif_image_release);
        heif_error error = heif_track_decode_next_image(m_track, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGBA, m_options);
        if(error.code == heif_error_End_of_sequence)
        {
            LOG_DEBUG() << LOGGING_CTX << "End_of_sequence reached";
            return false;
        }
        if(error.code != heif_error_Ok)
        {
            LOG_WARNING() << LOGGING_CTX << "Can't decode next image:" << error.message;
            return false;
        }

        QImage frame = convertImage(img);
        if(frame.isNull())
            return false;
        m_currentImage = frame;
        const uint32_t duration = heif_image_get_duration(img);
        const int durationMs = static_cast<int>(static_cast<uint64_t>(duration) * static_cast<uint64_t>(1000) / static_cast<uint64_t>(m_timescale));
        m_nextImageDelay = DelayCalculator::calculate(durationMs, DelayCalculator::MODE_CHROME);
        return true;
#else
        return false;
#endif
    }

    QPixmap currentPixmap() const Q_DECL_OVERRIDE
    {
        return QPixmap::fromImage(currentImage());
    }

    QImage currentImage() const Q_DECL_OVERRIDE
    {
        return m_currentImage;
    }

    PayloadWithMetaData<bool> readHeifFile(const QString &filePath)
    {
#if defined (USE_STREAM_READER_API)
        m_inFile.reset(new QFile(filePath));
        if(!m_inFile->open(QIODevice::ReadOnly))
        {
            LOG_WARNING() << LOGGING_CTX << "Can't open" << filePath;
            return false;
        }

        m_ctx.reset(heif_context_alloc(), &heif_context_free);
        memset(&m_reader, 0, sizeof(m_reader));
        m_reader.reader_api_version = 1;
        m_reader.get_position = &getPositionCallback;
        m_reader.read = &readCallback;
        m_reader.seek = &seekCallback;
        m_reader.wait_for_file_size = &waitForFileSizeCallback;
        heif_error error = heif_context_read_from_reader(m_ctx, &m_reader, m_inFile.data(), Q_NULLPTR);
#else
        m_inBuffer.reset(new MappedBuffer(filePath));
        if(!m_inBuffer->isValid())
            return false;

        m_ctx.reset(heif_context_alloc(), &heif_context_free);
        heif_error error = heif_context_read_from_memory(m_ctx, m_inBuffer->dataAs<const void*>(), m_inBuffer->sizeAs<size_t>(), Q_NULLPTR);
#endif
        if(error.code != heif_error_Ok)
        {
            LOG_WARNING() << LOGGING_CTX << "Can't read:" << error.message;
            return false;
        }

        m_options.reset(heif_decoding_options_alloc(), &heif_decoding_options_free);
#if defined (LIBHEIF_NUMERIC_VERSION) && (LIBHEIF_NUMERIC_VERSION >= 0x01070000)
        if(m_options->version >= 2)
            m_options->convert_hdr_to_8bit = 1;
#endif

#if defined (USE_LIBHEIF_SEQUENCES)
        if(heif_context_has_sequence(m_ctx))
        {
            const int tracksCount = heif_context_number_of_sequence_tracks(m_ctx);
            if(tracksCount > 0)
            {
                QScopedArrayPointer<uint32_t> trackIds(new uint32_t[tracksCount]);
                memset(trackIds.data(), 0, sizeof(uint32_t) * tracksCount);
                heif_context_get_track_ids(m_ctx, trackIds.data());
                for(int trackNum = 0; trackNum < tracksCount; ++trackNum)
                {
                    SP<heif_track> track(heif_context_get_track(m_ctx, trackIds[trackNum]), &heif_track_release);
                    if(!track)
                        continue;
                    if(heif_track_get_track_handler_type(track) != heif_track_type_image_sequence)
                        continue;
                    m_timescale = heif_track_get_timescale(track);
                    m_track.swap(track);
                    if(jumpToNextImage())
                        break;
                    m_timescale = 0;
                    m_track.reset();
                    continue;
                }
            }
        }

        if(m_currentImage.isNull())
#endif
        {
            SP<heif_image_handle> handle(Q_NULLPTR, &heif_image_handle_release);
            error = heif_context_get_primary_image_handle(m_ctx, &handle);
            if(error.code != heif_error_Ok)
            {
                LOG_WARNING() << LOGGING_CTX << "Can't get primary image handle:" << error.message;
                return false;
            }

            SP<heif_image> img(Q_NULLPTR, &heif_image_release);
            error = heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGBA, m_options);
            if(error.code != heif_error_Ok)
            {
                LOG_WARNING() << LOGGING_CTX << "Can't decode image:" << error.message;
                return false;
            }

            m_currentImage = convertImage(img);
            if(m_currentImage.isNull())
                return false;
        }

#if defined (USE_STREAM_READER_API)
        ImageMetaData *metaData = ImageMetaData::createMetaData(filePath);
#else
        ImageMetaData *metaData = ImageMetaData::createMetaData(m_inBuffer->dataAsByteArray());
#endif
        if(!metaData)
        {
            SP<heif_image_handle> handle(Q_NULLPTR, &heif_image_handle_release);
            error = heif_context_get_primary_image_handle(m_ctx, &handle);
            if(error.code != heif_error_Ok)
            {
                LOG_WARNING() << LOGGING_CTX << "Can't get primary image handle:" << error.message;
            }
            else
            {
                const int numberOfMetadataBlocks = heif_image_handle_get_number_of_metadata_blocks(handle, Q_NULLPTR);
                if(numberOfMetadataBlocks > 0)
                {
                    QScopedArrayPointer<heif_item_id> ids(new heif_item_id[numberOfMetadataBlocks]);
                    heif_image_handle_get_list_of_metadata_block_IDs(handle, Q_NULLPTR, ids.data(), numberOfMetadataBlocks);
                    for(int i = 0; i < numberOfMetadataBlocks; ++i)
                    {
                        if(strcmp(heif_image_handle_get_metadata_type(handle, ids[i]), "Exif") == 0)
                        {
                            LOG_DEBUG() << LOGGING_CTX << "Found EXIF metadata";
                            const size_t metadataSize = heif_image_handle_get_metadata_size(handle, ids[i]);
                            QByteArray rawMetadata(static_cast<int>(metadataSize), 0);
                            error = heif_image_handle_get_metadata(handle, ids[i], rawMetadata.data());
                            if(error.code != heif_error_Ok)
                            {
                                LOG_WARNING() << LOGGING_CTX << "Can't get EXIF metadata:" << error.message;
                            }
                            else
                            {
                                const int offset = 10;
                                metaData = ImageMetaData::joinMetaData(metaData, ImageMetaData::createExifMetaData(QByteArray::fromRawData(rawMetadata.constData() + offset, rawMetadata.size() - offset)));
                            }
                        }
                        else if(strcmp(heif_image_handle_get_metadata_type(handle, ids[i]), "XMP") == 0)
                        {
                            LOG_DEBUG() << LOGGING_CTX << "Found XMP metadata";
                            const size_t metadataSize = heif_image_handle_get_metadata_size(handle, ids[i]);
                            QByteArray rawMetadata(static_cast<int>(metadataSize), 0);
                            error = heif_image_handle_get_metadata(handle, ids[i], rawMetadata.data());
                            if(error.code != heif_error_Ok)
                                LOG_WARNING() << LOGGING_CTX << "Can't get XMP metadata:" << error.message;
                            else
                                metaData = ImageMetaData::joinMetaData(metaData, ImageMetaData::createXmpMetaData(QByteArray::fromRawData(rawMetadata.constData(), rawMetadata.size())));
                        }
                    }
                }
            }
        }

        return PayloadWithMetaData<bool>(true, metaData);
    }

private:
    static QImage convertImage(heif_image *img)
    {
        if(!img)
            return QImage();

        const int width = static_cast<int>(heif_image_get_width(img, heif_channel_interleaved));
        const int height = static_cast<int>(heif_image_get_height(img, heif_channel_interleaved));
        int stride = 0;
        const uint8_t *planeData = heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride);
        QImage result;
        if(heif_image_get_chroma_format(img) == heif_chroma_interleaved_RGB) ///< libheif-1.7.0, WTF?
        {
            result = QImage(reinterpret_cast<const uchar*>(planeData), width, height, stride, QImage::Format_RGB888).convertToFormat(QImage::Format_RGB32);
        }
        else
        {
#if (Q_BYTE_ORDER != Q_BIG_ENDIAN)
            result = QImage(reinterpret_cast<const uchar*>(planeData), width, height, stride, QImage::Format_ARGB32).rgbSwapped();
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
            result = QImage(reinterpret_cast<const uchar*>(planeData), width, height, stride, QImage::Format_RGBA8888).copy();
#else
            result = QImage(width, height, QImage::Format_ARGB32);
            for(int y = 0; y < result.height(); ++y)
            {
                QRgb* lineOut = reinterpret_cast<QRgb*>(result.scanLine(y));
                const QRgb* lineIn = reinterpret_cast<const QRgb*>(planeData + stride * y);
                for(int x = 0; x < result.width(); ++x)
                    lineOut[x] = qRgba(qAlpha(lineIn[x]), qRed(lineIn[x]), qGreen(lineIn[x]), qBlue(lineIn[x]));
            }
#endif
        }
        if(result.isNull())
            LOG_WARNING() << LOGGING_CTX << "Invalid image size:" << width << "x" << height << "chroma_format:" << heif_image_get_chroma_format(img);

        return result;
    }

private:
    QImage m_currentImage;
    int m_nextImageDelay;

#if defined (USE_STREAM_READER_API)
    QScopedPointer<QFile> m_inFile;
    heif_reader m_reader;
#else
    QScopedPointer<MappedBuffer> m_inBuffer;
#endif

    HeifInitGuard m_initGuard;
    SP<heif_context> m_ctx;
    SP<heif_decoding_options> m_options;
#if defined (USE_LIBHEIF_SEQUENCES)
    uint32_t m_timescale;
    SP<heif_track> m_track;
#endif
};

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
                << QString::fromLatin1("heifs")
                << QString::fromLatin1("heics")
                << QString::fromLatin1("hif")
#if defined (LIBHEIF_NUMERIC_VERSION) && (LIBHEIF_NUMERIC_VERSION >= 0x01070000)
                << QString::fromLatin1("avif")
                << QString::fromLatin1("avifs")
#endif
#if defined (LIBHEIF_NUMERIC_VERSION) && (LIBHEIF_NUMERIC_VERSION >= 0x010D0000)
                << QString::fromLatin1("hej2")
#endif
#if defined (LIBHEIF_HAVE_VERSION) && (LIBHEIF_HAVE_VERSION(1, 18, 0))
                << QString::fromLatin1("vvic")
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
        HeifAnimationProvider *provider = new HeifAnimationProvider();
        const PayloadWithMetaData<bool> readResult = provider->readHeifFile(filePath);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createAnimatedItem(provider);
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), readResult.metaData()));
    }
};

DecoderAutoRegistrator registrator(new DecoderLibHEIF);

} // namespace
