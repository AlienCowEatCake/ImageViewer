/*
   Copyright (C) 2018-2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <cassert>

#include "Workarounds/BeginIgnoreDeprecated.h"
#include <libraw/libraw_datastream.h>
#include <libraw/libraw.h>
#include "Workarounds/EndIgnoreDeprecated.h"

#include <QtGlobal>
#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QPointer>
#include <QDebug>

#include "Utils/Global.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"
#include "Internal/Scaling/AbstractProgressiveImageProvider.h"
#include "Internal/Utils/MappedBuffer.h"

namespace
{

// ====================================================================================================

class RawImageProcessor
{
public:
    explicit RawImageProcessor(const QString &filePath)
        : m_isValid(false)
        , m_isCancelled(false)
    {
        m_isValid = openRawFile(filePath);
    }

    bool isValid() const
    {
        return m_isValid;
    }

    void cancel()
    {
        m_isCancelled = true;
    }

    bool isCancelled() const
    {
        return m_isCancelled;
    }

    QSize size() const
    {
        if(!isValid())
            return QSize();
        const libraw_image_sizes_t &sizes = m_rawProcessor.imgdata.sizes;
        const bool isRotated = sizes.flip == 5 || sizes.flip == 6;
        return isRotated ? QSize(sizes.height, sizes.width) : QSize(sizes.width, sizes.height);
    }

    QImage getThumbnail()
    {
#define CHECK_CANCEL if(isCancelled()) return QImage()
        if(!isValid())
            return QImage();
        try
        {
            CHECK_CANCEL;
            if(m_rawProcessor.unpack_thumb() != LIBRAW_SUCCESS)
                return QImage();
            CHECK_CANCEL;
            int errorCode = 0;
            libraw_processed_image_t *output = m_rawProcessor.dcraw_make_mem_thumb(&errorCode);
            if(!output)
            {
                qWarning() << "Error:" << libraw_strerror(errorCode);
                return QImage();
            }
            CHECK_CANCEL;
            QImage image = convertImage(output);
            m_rawProcessor.dcraw_clear_mem(output);
            return image;
        }
        catch(...)
        {
            qWarning() << "Exception";
        }
        return QImage();
#undef CHECK_CANCEL
    }

    QImage getImage()
    {
#define CHECK_CANCEL if(isCancelled()) return QImage()
        if(!isValid())
            return QImage();
        try
        {
            CHECK_CANCEL;
            if(m_rawProcessor.unpack() != LIBRAW_SUCCESS)
                return QImage();
            CHECK_CANCEL;
            if(m_rawProcessor.dcraw_process() != LIBRAW_SUCCESS)
                return QImage();
            CHECK_CANCEL;
            int errorCode = 0;
            libraw_processed_image_t *output = m_rawProcessor.dcraw_make_mem_image(&errorCode);
            if(!output)
            {
                qWarning() << "Error:" << libraw_strerror(errorCode);
                return QImage();
            }
            CHECK_CANCEL;
            QImage image = convertImage(output);
            m_rawProcessor.dcraw_clear_mem(output);
            return image;
        }
        catch(...)
        {
            qWarning() << "Exception";
        }
        return QImage();
#undef CHECK_CANCEL
    }

private:
    static int progressCallback(void *data, enum LibRaw_progress p, int iteration, int expected)
    {
        qDebug() << libraw_strprogress(p) << "pass" << iteration << "of" << expected;
        RawImageProcessor *processor = reinterpret_cast<RawImageProcessor*>(data);
        if(!processor->isCancelled())
            return 0;
        qDebug() << "Progress is cancelled!";
        return 1;
    }

    bool openRawFile(const QString &filePath)
    {
        try
        {
            const QByteArray filePathLocal8Bit = filePath.toLocal8Bit();
            if(QString::fromLocal8Bit(filePathLocal8Bit) != filePath || filePath.startsWith(QChar::fromLatin1(':')))
            {
                m_mappedBuffer.reset(new MappedBuffer(filePath));
                if(!m_mappedBuffer->isValid())
                    return false;
                m_dataStream.reset(new LibRaw_buffer_datastream(m_mappedBuffer->dataAs<void*>(), m_mappedBuffer->sizeAs<size_t>()));
            }
            else
            {
                m_dataStream.reset(new LibRaw_file_datastream(filePathLocal8Bit.constData()));
            }

            m_rawProcessor.set_progress_handler(progressCallback, this);
            if(m_rawProcessor.open_datastream(m_dataStream.data()) != LIBRAW_SUCCESS)
            {
                qWarning() << "Can't open datastream for" << filePath;
                return false;
            }

            // https://www.libraw.org/docs/API-datastruct-eng.html#libraw_output_params_t
            libraw_output_params_t &params = m_rawProcessor.imgdata.params;
            params.output_bps = 8;
            params.use_auto_wb = 1;
            params.use_camera_wb = 1;
            params.output_color = 1; // sRGB
            return true;
        }
        catch(...)
        {
            qWarning() << "Exception";
        }
        return false;
    }

    QImage convertImage(libraw_processed_image_t *output)
    {
        QImage image;
        if(!output)
            return image;

        if(output->type == LIBRAW_IMAGE_JPEG)
        {
            image.loadFromData(output->data, static_cast<int>(output->data_size), "JPG");
            if(image.isNull())
            {
                qWarning() << "Invalid image size";
                return image;
            }

            if(m_rawProcessor.imgdata.sizes.flip != 0)
            {
                int angle = 0;
                switch(m_rawProcessor.imgdata.sizes.flip)
                {
                case 3:
                    angle = 180;
                    break;
                case 5:
                    angle = -90;
                    break;
                case 6:
                    angle = 90;
                    break;
                default:
                    break;
                }
                if(angle != 0)
                {
                    QTransform rotation;
                    rotation.rotate(angle);
                    image = image.transformed(rotation);
                }
            }
        }
        else if(output->type == LIBRAW_IMAGE_BITMAP)
        {
            if(output->bits != 8)
            {
                qWarning() << "Error: Output BPS must be equals to 8!";
                assert(output->bits == 8);
                return image;
            }

            image = QImage(output->width, output->height, QImage::Format_RGB32);
            if(image.isNull())
            {
                qWarning() << "Invalid image size";
                return image;
            }

            if(output->colors != 1 && output->colors != 3)
                qWarning() << "Error: Unknown output->colors =" << output->colors;

            for(int i = 0; i < output->height; i++)
            {
                QRgb *line = reinterpret_cast<QRgb*>(image.scanLine(i));
                for(int j = 0; j < output->width; j++)
                {
                    unsigned char *source = output->data + (i * output->width + j) * output->colors;
                    switch(output->colors)
                    {
                    case 3:
                        line[j] = qRgb(source[0], source[1], source[2]);
                        break;
                    default:
                        line[j] = qRgb(source[0], source[0], source[0]);
                        break;
                    }
                }
            }
        }
        else
        {
            qWarning() << "Error: Unknown output->type =" << output->type;
        }
        return image;
    }

private:
    QScopedPointer<MappedBuffer> m_mappedBuffer;
    QScopedPointer<LibRaw_abstract_datastream> m_dataStream;
    LibRaw m_rawProcessor;
    bool m_isValid;
    bool m_isCancelled;
};

// ====================================================================================================

class RawImageProvider : public AbstractProgressiveImageProvider
{
    Q_DISABLE_COPY(RawImageProvider)

public:
    explicit RawImageProvider(const QString &filePath)
        : m_rawImageProcessor(new RawImageProcessor(filePath))
        , m_processingThread(Q_NULLPTR)
        , m_isFinal(true)
    {
        if(!m_rawImageProcessor->isValid())
        {
            m_rawImageProcessor.reset();
            return;
        }

        m_image = m_rawImageProcessor->getThumbnail();
        if(m_image.isNull())
        {
            m_image = m_rawImageProcessor->getImage();
            return;
        }
        else
        {
            m_isFinal = false;
            m_processingThread = new ProcessingThread(this);
            m_processingThread->start();
        }
    }

    ~RawImageProvider()
    {
        m_mutex.lock();
        if(m_rawImageProcessor)
            m_rawImageProcessor->cancel();
        m_mutex.unlock();
        if(m_processingThread)
            m_processingThread->wait();
    }

public: // AbstractImageProvider
    bool isValid() const Q_DECL_OVERRIDE
    {
        const QMutexLocker guard(&m_mutex);
        return !m_image.isNull();
    }

    bool isFinal() const Q_DECL_OVERRIDE
    {
        const QMutexLocker guard(&m_mutex);
        return m_isFinal;
    }

    QSize size() const Q_DECL_OVERRIDE
    {
        const QMutexLocker guard(&m_mutex);
        return m_rawImageProcessor ? m_rawImageProcessor->size() : m_image.size();
    }

    QImage image() const Q_DECL_OVERRIDE
    {
        const QMutexLocker guard(&m_mutex);
        return m_image;
    }

private:
    class ProcessingThread : public QThread
    {
    public:
        explicit ProcessingThread(RawImageProvider *provider)
            : QThread(provider)
            , m_provider(provider)
        {}

        void run() Q_DECL_OVERRIDE
        {
            const QImage img = m_provider->m_rawImageProcessor->getImage();
            if(m_provider->m_rawImageProcessor->isCancelled())
                return;
            m_provider->m_mutex.lock();
            if(!img.isNull())
                m_provider->m_image = img;
            m_provider->m_isFinal = true;
            m_provider->m_rawImageProcessor.reset();
            m_provider->m_mutex.unlock();
            Q_EMIT m_provider->updated();
        }

    private:
        RawImageProvider *m_provider;
    };

    QScopedPointer<RawImageProcessor> m_rawImageProcessor;
    QPointer<ProcessingThread> m_processingThread;
    mutable QMutex m_mutex;
    QImage m_image;
    bool m_isFinal;
};

// ====================================================================================================

class DecoderLibRaw : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderLibRaw");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        /// @note https://github.com/KDE/digikam/blob/v6.0.0/core/libs/rawengine/drawfiles.h
        return QStringList()
                << QString::fromLatin1("bay")  // Casio Digital Camera Raw File Format.
                << QString::fromLatin1("bmq")  // NuCore Raw Image File.
                << QString::fromLatin1("cr2")  // Canon Digital Camera RAW Image Format version 2.0. These images are based on the TIFF image standard.
                << QString::fromLatin1("crw")  // Canon Digital Camera RAW Image Format version 1.0.
                << QString::fromLatin1("cs1")  // Capture Shop Raw Image File.
                << QString::fromLatin1("dc2")  // Kodak DC25 Digital Camera File.
                << QString::fromLatin1("dcr")  // Kodak Digital Camera Raw Image Format for these models: Kodak DSC Pro SLR/c, Kodak DSC Pro SLR/n, Kodak DSC Pro 14N, Kodak DSC PRO 14nx.
                << QString::fromLatin1("dng")  // Adobe Digital Negative: DNG is publicly available archival format for the raw files generated by digital cameras. By addressing the lack of an open standard for the raw files created by individual camera models, DNG helps ensure that photographers will be able to access their files in the future.
                << QString::fromLatin1("erf")  // Epson Digital Camera Raw Image Format.
                << QString::fromLatin1("fff")  // Imacon Digital Camera Raw Image Format.
                << QString::fromLatin1("hdr")  // Leaf Raw Image File.
                << QString::fromLatin1("k25")  // Kodak DC25 Digital Camera Raw Image Format.
                << QString::fromLatin1("kdc")  // Kodak Digital Camera Raw Image Format.
                << QString::fromLatin1("mdc")  // Minolta RD175 Digital Camera Raw Image Format.
                << QString::fromLatin1("mos")  // Mamiya Digital Camera Raw Image Format.
                << QString::fromLatin1("mrw")  // Minolta Dimage Digital Camera Raw Image Format.
                << QString::fromLatin1("nef")  // Nikon Digital Camera Raw Image Format.
                << QString::fromLatin1("orf")  // Olympus Digital Camera Raw Image Format.
                << QString::fromLatin1("pef")  // Pentax Digital Camera Raw Image Format.
                << QString::fromLatin1("pxn")  // Logitech Digital Camera Raw Image Format.
                << QString::fromLatin1("raf")  // Fuji Digital Camera Raw Image Format.
                << QString::fromLatin1("raw")  // Panasonic Digital Camera Image Format.
                << QString::fromLatin1("rdc")  // Digital Foto Maker Raw Image File.
                << QString::fromLatin1("sr2")  // Sony Digital Camera Raw Image Format.
                << QString::fromLatin1("srf")  // Sony Digital Camera Raw Image Format for DSC-F828 8 megapixel digital camera or Sony DSC-R1
                << QString::fromLatin1("x3f")  // Sigma Digital Camera Raw Image Format for devices based on Foveon X3 direct image sensor.
                << QString::fromLatin1("arw")  // Sony Digital Camera Raw Image Format for Alpha devices.

                << QString::fromLatin1("3fr")  // Hasselblad Digital Camera Raw Image Format.
                << QString::fromLatin1("cine") // Phantom Software Raw Image File.
                << QString::fromLatin1("ia")   // Sinar Raw Image File.
                << QString::fromLatin1("kc2")  // Kodak DCS200 Digital Camera Raw Image Format.
                << QString::fromLatin1("mef")  // Mamiya Digital Camera Raw Image Format.
                << QString::fromLatin1("nrw")  // Nikon Digital Camera Raw Image Format.
                << QString::fromLatin1("qtk")  // Apple Quicktake 100/150 Digital Camera Raw Image Format.
                << QString::fromLatin1("rw2")  // Panasonic LX3 Digital Camera Raw Image Format.
                << QString::fromLatin1("sti")  // Sinar Capture Shop Raw Image File.

                << QString::fromLatin1("rwl")  // Leica Digital Camera Raw Image Format.

                << QString::fromLatin1("srw")  // Samnsung Raw Image Format.

//                /// @todo check if these format are supported
//                << QString::fromLatin1("drf")  // Kodak Digital Camera Raw Image Format.
//                << QString::fromLatin1("dsc")  // Kodak Digital Camera Raw Image Format.
//                << QString::fromLatin1("ptx")  // Pentax Digital Camera Raw Image Format.
//                << QString::fromLatin1("cap")  // Phase One Digital Camera Raw Image Format.
//                << QString::fromLatin1("iiq")  // Phase One Digital Camera Raw Image Format.
//                << QString::fromLatin1("rwz")  // Rawzor Digital Camera Raw Image Format.
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
        AbstractProgressiveImageProvider *provider = new RawImageProvider(filePath);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createProgressiveImageItem(provider);
        IImageMetaData *metaData = ImageMetaData::createMetaData(filePath);
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), metaData));
    }
};

DecoderAutoRegistrator registrator(new DecoderLibRaw);

// ====================================================================================================

} // namespace
