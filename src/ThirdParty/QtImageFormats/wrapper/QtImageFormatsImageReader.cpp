/// @note This is modified QImageReader code.

#include "QtImageFormatsImageReader.h"

//#define QTIMAGEFORMATS_IMAGEREADER_DEBUG

#include <QCoreApplication>
#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageIOHandler>
#include <QList>
#include <QMap>
#include <QVector>
#include <QRect>
#include <QSize>
#include <QColor>
#include <QVariant>
#include <QStringList>

#if defined (WRAPPER_USE_DDS_HANDLER)
#include "../qtimageformats/src/plugins/imageformats/dds/qddshandler.h"
#endif
#if defined (WRAPPER_USE_ICNS_HANDLER)
#include "../qtimageformats/src/plugins/imageformats/icns/qicnshandler_p.h"
#endif
#if defined (WRAPPER_USE_JP2_HANDLER)
#include "../qtimageformats/src/plugins/imageformats/jp2/qjp2handler_p.h"
#endif
#if defined (WRAPPER_USE_MNG_HANDLER)
#include "../qtimageformats/src/plugins/imageformats/mng/qmnghandler_p.h"
#endif
#if defined (WRAPPER_USE_TGA_HANDLER)
#include "../qtimageformats/src/plugins/imageformats/tga/qtgahandler.h"
#endif
#if defined (WRAPPER_USE_TIFF_HANDLER)
#include "../qtimageformats/src/plugins/imageformats/tiff/qtiffhandler_p.h"
#endif
#if defined (WRAPPER_USE_WBMP_HANDLER)
#include "../qtimageformats/src/plugins/imageformats/wbmp/qwbmphandler_p.h"
#endif
#if defined (WRAPPER_USE_WEBP_HANDLER)
#include "../qtimageformats/src/plugins/imageformats/webp/qwebphandler_p.h"
#endif

QT_BEGIN_NAMESPACE

enum BuiltInFormatType
{
#if defined (WRAPPER_USE_DDS_HANDLER)
    DdsFormat,
#endif
#if defined (WRAPPER_USE_ICNS_HANDLER)
    IcnsFormat,
#endif
#if defined (WRAPPER_USE_JP2_HANDLER)
    Jp2Format,
    Jp2CFormat,
#endif
#if defined (WRAPPER_USE_MNG_HANDLER)
    MngFormat,
#endif
#if defined (WRAPPER_USE_TGA_HANDLER)
    TgaFormat,
#endif
#if defined (WRAPPER_USE_TIFF_HANDLER)
    TiffFormat,
#endif
#if defined (WRAPPER_USE_WBMP_HANDLER)
    WbmpFormat,
#endif
#if defined (WRAPPER_USE_WEBP_HANDLER)
    WebpFormat,
#endif
    NumFormats,
    NoFormat = -1
};

struct BuiltInFormatStruct
{
    BuiltInFormatType format;
    QList<QByteArray> extensions;
};

static const BuiltInFormatStruct BuiltInFormats[] =
{
#if defined (WRAPPER_USE_DDS_HANDLER)
    { DdsFormat , QList<QByteArray>() << "dds" },
#endif
#if defined (WRAPPER_USE_ICNS_HANDLER)
    { IcnsFormat, QList<QByteArray>() << "icns" },
#endif
#if defined (WRAPPER_USE_JP2_HANDLER)
    { Jp2Format , QList<QByteArray>() << "jp2" },
    { Jp2CFormat, QList<QByteArray>() << "j2k" << "jpc" << "j2c" },
#endif
#if defined (WRAPPER_USE_MNG_HANDLER)
    { MngFormat , QList<QByteArray>() << "mng" },
#endif
#if defined (WRAPPER_USE_TGA_HANDLER)
    { TgaFormat , QList<QByteArray>() << "tga" },
#endif
#if defined (WRAPPER_USE_TIFF_HANDLER)
    { TiffFormat, QList<QByteArray>() << "tif" << "tiff" },
#endif
#if defined (WRAPPER_USE_WBMP_HANDLER)
    { WbmpFormat, QList<QByteArray>() << "wbmp" },
#endif
#if defined (WRAPPER_USE_WEBP_HANDLER)
    { WebpFormat, QList<QByteArray>() << "webp" },
#endif
};
Q_STATIC_ASSERT(NumFormats == sizeof BuiltInFormats / sizeof *BuiltInFormats);

static QImageIOHandler *createReadHandlerHelper(QIODevice *device,
                                                const QByteArray &format,
                                                bool autoDetectImageFormat,
                                                bool ignoresFormatAndExtension)
{
    if(!autoDetectImageFormat && format.isEmpty())
        return 0;

    QByteArray form = format.toLower();
    QImageIOHandler *handler = 0;
    QByteArray suffix;

    if(device && format.isEmpty() && autoDetectImageFormat && !ignoresFormatAndExtension)
    {
        // if there's no format, see if \a device is a file, and if so, find
        // the file suffix and find support for that format among our plugins.
        // this allows plugins to override our built-in handlers.
        if(QFile *file = qobject_cast<QFile *>(device))
        {
#if defined (QTIMAGEFORMATS_IMAGEREADER_DEBUG)
            qDebug() << "QImageReader::createReadHandler: device is a file:" << file->fileName();
#endif
            suffix = QFileInfo(file->fileName()).suffix().toLower().toLatin1();
        }
    }

    QByteArray testFormat = !form.isEmpty() ? form : suffix;

    if(ignoresFormatAndExtension)
        testFormat = QByteArray();

    // if we don't have a handler yet, check if we have built-in support for
    // the format
    if(!handler && !testFormat.isEmpty())
    {
        BuiltInFormatType formatType = NoFormat;
        for(int i = 0; i < NumFormats; i++)
        {
            if(BuiltInFormats[i].extensions.contains(testFormat))
            {
                formatType = BuiltInFormats[i].format;
                break;
            }
        }
        switch(formatType)
        {
#if defined (WRAPPER_USE_DDS_HANDLER)
        case DdsFormat:
            handler = new QDDSHandler;
            break;
#endif
#if defined (WRAPPER_USE_ICNS_HANDLER)
        case IcnsFormat:
            handler = new QICNSHandler;
            break;
#endif
#if defined (WRAPPER_USE_JP2_HANDLER)
        case Jp2Format:
            handler = new QJp2Handler;
            handler->setOption(QImageIOHandler::SubType, "jp2");
            break;
        case Jp2CFormat:
            handler = new QJp2Handler;
            handler->setOption(QImageIOHandler::SubType, "j2k");
            break;
#endif
#if defined (WRAPPER_USE_MNG_HANDLER)
        case MngFormat:
            handler = new QMngHandler;
            break;
#endif
#if defined (WRAPPER_USE_TGA_HANDLER)
        case TgaFormat:
            handler = new QTgaHandler;
            break;
#endif
#if defined (WRAPPER_USE_TIFF_HANDLER)
        case TiffFormat:
            handler = new QTiffHandler;
            break;
#endif
#if defined (WRAPPER_USE_WBMP_HANDLER)
        case WbmpFormat:
            handler = new QWbmpHandler(device); /// @todo WTF?
            break;
#endif
#if defined (WRAPPER_USE_WEBP_HANDLER)
        case WebpFormat:
            handler = new QWebpHandler;
            break;
#endif
        default:
            break;
        }
#if defined (QTIMAGEFORMATS_IMAGEREADER_DEBUG)
        if(handler)
            qDebug() << "QImageReader::createReadHandler: using the built-in handler for" << testFormat;
#endif
    }

    if(!handler && (autoDetectImageFormat || ignoresFormatAndExtension))
    {
        // check if any of our built-in handlers recognize the file from its
        // contents.
        int currentFormat = 0;
        if(!suffix.isEmpty())
        {
            // If reading from a file with a suffix, start testing our
            // built-in handler for that suffix first.
            for(int i = 0; i < NumFormats; ++i)
            {
                if(BuiltInFormats[i].extensions.contains(suffix))
                {
                    currentFormat = BuiltInFormats[i].format;
                    break;
                }
            }
        }

        QByteArray subType;
        int numFormats = NumFormats;
        while (device && numFormats >= 0)
        {
            const qint64 pos = device->pos();
            switch(currentFormat)
            {
#if defined (WRAPPER_USE_DDS_HANDLER)
            case DdsFormat:
                if(QDDSHandler::canRead(device))
                    handler = new QDDSHandler;
                break;
#endif
#if defined (WRAPPER_USE_ICNS_HANDLER)
            case IcnsFormat:
                if(QICNSHandler::canRead(device))
                    handler = new QICNSHandler;
                break;
#endif
#if defined (WRAPPER_USE_JP2_HANDLER)
            case Jp2Format:
            case Jp2CFormat:
                if(QJp2Handler::canRead(device, &subType))
                {
                    handler = new QJp2Handler;
                    handler->setOption(QImageIOHandler::SubType, subType);
                }
                break;
#endif
#if defined (WRAPPER_USE_MNG_HANDLER)
            case MngFormat:
                if(QMngHandler::canRead(device))
                    handler = new QMngHandler;
                break;
#endif
#if defined (WRAPPER_USE_TGA_HANDLER)
            case TgaFormat:
                if(QTgaHandler::canRead(device))
                    handler = new QTgaHandler;
                break;
#endif
#if defined (WRAPPER_USE_TIFF_HANDLER)
            case TiffFormat:
                if(QTiffHandler::canRead(device))
                    handler = new QTiffHandler;
                break;
#endif
#if defined (WRAPPER_USE_WBMP_HANDLER)
            case WbmpFormat:
                if(QWbmpHandler::canRead(device))
                    handler = new QWbmpHandler(device); /// @todo WTF?
                break;
#endif
#if defined (WRAPPER_USE_WEBP_HANDLER)
            case WebpFormat:
                if(QWebpHandler::canRead(device))
                    handler = new QWebpHandler;
                break;
#endif
            default:
                break;
            }
            if(!device->isSequential())
                device->seek(pos);

            if(handler)
            {
#if defined (QTIMAGEFORMATS_IMAGEREADER_DEBUG)
                qDebug("QImageReader::createReadHandler: the %s built-in handler can read this data", handler->name().constData());
#endif
                break;
            }

            --numFormats;
            ++currentFormat;
            currentFormat %= NumFormats;
            if(currentFormat >= NumFormats)
                currentFormat = 0;
        }
    }

    if(!handler)
    {
#if defined (QTIMAGEFORMATS_IMAGEREADER_DEBUG)
        qDebug("QImageReader::createReadHandler: no handlers found. giving up.");
#endif
        // no handler: give up.
        return 0;
    }

    handler->setDevice(device);
    if(!form.isEmpty())
        handler->setFormat(form);
    return handler;
}

static QMap<QString, QString> getImageTextFromDescription(const QString &description)
{
    QMap<QString, QString> text;
    QStringList pairs = description.split(QLatin1String("\n\n"));
    for(QStringList::ConstIterator it = pairs.constBegin(); it != pairs.constEnd(); ++it)
    {
        const QString &pair = *it;
        int index = pair.indexOf(QLatin1Char(':'));
        if(index >= 0 && pair.indexOf(QLatin1Char(' ')) < index)
        {
            if(!pair.trimmed().isEmpty())
                text.insert(QLatin1String("Description"), pair.simplified());
        }
        else
        {
            const QString key = pair.left(index);
            if(!key.trimmed().isEmpty())
                text.insert(key, pair.mid(index + 2).simplified());
        }
    }
    return text;
}

struct QtImageFormatsImageReader::Impl
{
    Impl(QtImageFormatsImageReader *reader);
    ~Impl();

    // device
    QByteArray format;
    bool autoDetectImageFormat;
    bool ignoresFormatAndExtension;
    QIODevice *device;
    bool deleteDevice;
    QImageIOHandler *handler;
    bool initHandler();

    // image options
    QRect clipRect;
    QSize scaledSize;
    QRect scaledClipRect;
    int quality;
    QMap<QString, QString> text;
    void getText();
    enum {
        UsePluginDefault,
        ApplyTransform,
        DoNotApplyTransform
    } autoTransform;

    // error
    QImageReader::ImageReaderError imageReaderError;
    QString errorString;

    QtImageFormatsImageReader *reader;
};

QtImageFormatsImageReader::Impl::Impl(QtImageFormatsImageReader *reader)
    : autoDetectImageFormat(true)
    , ignoresFormatAndExtension(false)
    , device(Q_NULLPTR)
    , deleteDevice(false)
    , handler(Q_NULLPTR)
    , quality(-1)
    , autoTransform(UsePluginDefault)
    , imageReaderError(QImageReader::UnknownError)
    , reader(reader)
{}

QtImageFormatsImageReader::Impl::~Impl()
{
    delete handler;
    if(deleteDevice)
        delete device;
}

bool QtImageFormatsImageReader::Impl::initHandler()
{
    // check some preconditions
    if(!device || (!deleteDevice && !device->isOpen() && !device->open(QIODevice::ReadOnly)))
    {
        imageReaderError = QImageReader::DeviceError;
        errorString = QCoreApplication::translate("QImageReader", "Invalid device");
        return false;
    }

    // probe the file extension
    if(deleteDevice && !device->isOpen() && !device->open(QIODevice::ReadOnly) && autoDetectImageFormat)
    {
        Q_ASSERT(qobject_cast<QFile*>(device) != 0); // future-proofing; for now this should always be the case, so...
        QFile *file = static_cast<QFile *>(device);
        if(file->error() == QFileDevice::ResourceError)
        {
            // this is bad. we should abort the open attempt and note the failure.
            imageReaderError = QImageReader::DeviceError;
            errorString = file->errorString();
            return false;
        }

        QList<QByteArray> extensions = QtImageFormatsImageReader::supportedImageFormats();
        if(!format.isEmpty())
        {
            // Try the most probable extension first
            int currentFormatIndex = extensions.indexOf(format.toLower());
            if(currentFormatIndex > 0)
#if (QT_VERSION < QT_VERSION_CHECK(5, 13, 0))
                extensions.swap(0, currentFormatIndex);
#else
                extensions.swapItemsAt(0, currentFormatIndex);
#endif
        }

        int currentExtension = 0;

        QString fileName = file->fileName();

        do
        {
            file->setFileName(fileName + QLatin1Char('.') + QLatin1String(extensions.at(currentExtension++).constData()));
            file->open(QIODevice::ReadOnly);
        } while(!file->isOpen() && currentExtension < extensions.size());

        if(!device->isOpen())
        {
            imageReaderError = QImageReader::FileNotFoundError;
            errorString = QCoreApplication::translate("QImageReader", "File not found");
            file->setFileName(fileName); // restore the old file name
            return false;
        }
    }

    // assign a handler
    if(!handler && (handler = createReadHandlerHelper(device, format, autoDetectImageFormat, ignoresFormatAndExtension)) == 0)
    {
        imageReaderError = QImageReader::UnsupportedFormatError;
        errorString = QCoreApplication::translate("QImageReader", "Unsupported image format");
        return false;
    }
    return true;
}

void QtImageFormatsImageReader::Impl::getText()
{
    if(text.isEmpty() && (handler || initHandler()) && handler->supportsOption(QImageIOHandler::Description))
        text = getImageTextFromDescription(handler->option(QImageIOHandler::Description).toString());
}

QtImageFormatsImageReader::QtImageFormatsImageReader()
    : m_impl(new Impl(this))
{}

QtImageFormatsImageReader::QtImageFormatsImageReader(QIODevice *device, const QByteArray &format)
    : m_impl(new Impl(this))
{
    m_impl->device = device;
    m_impl->format = format;
}

QtImageFormatsImageReader::QtImageFormatsImageReader(const QString &fileName, const QByteArray &format)
    : m_impl(new Impl(this))
{
    m_impl->device = new QFile(fileName);
    m_impl->format = format;
    m_impl->deleteDevice = true;
}

QtImageFormatsImageReader::~QtImageFormatsImageReader()
{}

void QtImageFormatsImageReader::setFormat(const QByteArray &format)
{
    m_impl->format = format;
}

QByteArray QtImageFormatsImageReader::format() const
{
    if(!m_impl->format.isEmpty())
        return m_impl->format;
    if(!m_impl->initHandler())
        return QByteArray();
    if(m_impl->handler->canRead())
        return m_impl->handler->format();
    return QByteArray();
}

void QtImageFormatsImageReader::setAutoDetectImageFormat(bool enabled)
{
    m_impl->autoDetectImageFormat = enabled;
}

bool QtImageFormatsImageReader::autoDetectImageFormat() const
{
    return m_impl->autoDetectImageFormat;
}

void QtImageFormatsImageReader::setDecideFormatFromContent(bool ignored)
{
    m_impl->ignoresFormatAndExtension = ignored;
}

bool QtImageFormatsImageReader::decideFormatFromContent() const
{
    return m_impl->ignoresFormatAndExtension;
}

void QtImageFormatsImageReader::setDevice(QIODevice *device)
{
    if(m_impl->device && m_impl->deleteDevice)
        delete m_impl->device;
    m_impl->device = device;
    m_impl->deleteDevice = false;
    delete m_impl->handler;
    m_impl->handler = Q_NULLPTR;
    m_impl->text.clear();
}

QIODevice *QtImageFormatsImageReader::device() const
{
    return m_impl->device;
}

void QtImageFormatsImageReader::setFileName(const QString &fileName)
{
    setDevice(new QFile(fileName));
    m_impl->deleteDevice = true;
}

QString QtImageFormatsImageReader::fileName() const
{
    QFile *file = qobject_cast<QFile*>(m_impl->device);
    return file ? file->fileName() : QString();
}

QSize QtImageFormatsImageReader::size() const
{
    if(!m_impl->initHandler())
        return QSize();
    if(m_impl->handler->supportsOption(QImageIOHandler::Size))
        return m_impl->handler->option(QImageIOHandler::Size).toSize();
    return QSize();
}

QImage::Format QtImageFormatsImageReader::imageFormat() const
{
    if(!m_impl->initHandler())
        return QImage::Format_Invalid;
    if(m_impl->handler->supportsOption(QImageIOHandler::ImageFormat))
        return (QImage::Format)m_impl->handler->option(QImageIOHandler::ImageFormat).toInt();
    return QImage::Format_Invalid;
}

QStringList QtImageFormatsImageReader::textKeys() const
{
    m_impl->getText();
    return m_impl->text.keys();
}

QString QtImageFormatsImageReader::text(const QString &key) const
{
    m_impl->getText();
    return m_impl->text.value(key);
}

void QtImageFormatsImageReader::setClipRect(const QRect &rect)
{
    m_impl->clipRect = rect;
}

QRect QtImageFormatsImageReader::clipRect() const
{
    return m_impl->clipRect;
}

void QtImageFormatsImageReader::setScaledSize(const QSize &size)
{
    m_impl->scaledSize = size;
}

QSize QtImageFormatsImageReader::scaledSize() const
{
    return m_impl->scaledSize;
}

void QtImageFormatsImageReader::setQuality(int quality)
{
    m_impl->quality = quality;
}

int QtImageFormatsImageReader::quality() const
{
    return m_impl->quality;
}

void QtImageFormatsImageReader::setScaledClipRect(const QRect &rect)
{
    m_impl->scaledClipRect = rect;
}

QRect QtImageFormatsImageReader::scaledClipRect() const
{
    return m_impl->scaledClipRect;
}

void QtImageFormatsImageReader::setBackgroundColor(const QColor &color)
{
    if(!m_impl->initHandler())
        return;
    if(m_impl->handler->supportsOption(QImageIOHandler::BackgroundColor))
        m_impl->handler->setOption(QImageIOHandler::BackgroundColor, color);
}

QColor QtImageFormatsImageReader::backgroundColor() const
{
    if(!m_impl->initHandler())
        return QColor();
    if(m_impl->handler->supportsOption(QImageIOHandler::BackgroundColor))
        return qvariant_cast<QColor>(m_impl->handler->option(QImageIOHandler::BackgroundColor));
    return QColor();
}

bool QtImageFormatsImageReader::supportsAnimation() const
{
    if(!m_impl->initHandler())
        return false;
    if(m_impl->handler->supportsOption(QImageIOHandler::Animation))
        return m_impl->handler->option(QImageIOHandler::Animation).toBool();
    return false;
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
QImageIOHandler::Transformations QtImageFormatsImageReader::transformation() const
{
    int option = QImageIOHandler::TransformationNone;
    if(m_impl->initHandler() && m_impl->handler->supportsOption(QImageIOHandler::ImageTransformation))
        option = m_impl->handler->option(QImageIOHandler::ImageTransformation).toInt();
    return QImageIOHandler::Transformations(option);
}

void QtImageFormatsImageReader::setAutoTransform(bool enabled)
{
    m_impl->autoTransform = enabled ? Impl::ApplyTransform : Impl::DoNotApplyTransform;
}

bool QtImageFormatsImageReader::autoTransform() const
{
    switch(m_impl->autoTransform)
    {
    case Impl::ApplyTransform:
        return true;
    case Impl::DoNotApplyTransform:
        return false;
    case Impl::UsePluginDefault:
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        if(m_impl->initHandler())
            return m_impl->handler->supportsOption(QImageIOHandler::TransformedByDefault);
#endif
        break;
    }
    return false;
}
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
void QtImageFormatsImageReader::setGamma(float gamma)
{
    if(m_impl->initHandler() && m_impl->handler->supportsOption(QImageIOHandler::Gamma))
        m_impl->handler->setOption(QImageIOHandler::Gamma, gamma);
}

float QtImageFormatsImageReader::gamma() const
{
    if(m_impl->initHandler() && m_impl->handler->supportsOption(QImageIOHandler::Gamma))
        return m_impl->handler->option(QImageIOHandler::Gamma).toFloat();
    return 0.0f;
}
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
QByteArray QtImageFormatsImageReader::subType() const
{
    if(!m_impl->initHandler())
        return QByteArray();
    if(m_impl->handler->supportsOption(QImageIOHandler::SubType))
        return m_impl->handler->option(QImageIOHandler::SubType).toByteArray();
    return QByteArray();
}

QList<QByteArray> QtImageFormatsImageReader::supportedSubTypes() const
{
    if(!m_impl->initHandler())
        return QList<QByteArray>();
    if(m_impl->handler->supportsOption(QImageIOHandler::SupportedSubTypes))
        return m_impl->handler->option(QImageIOHandler::SupportedSubTypes).value< QList<QByteArray> >();
    return QList<QByteArray>();
}
#endif

bool QtImageFormatsImageReader::canRead() const
{
    if(!m_impl->initHandler())
        return false;
    return m_impl->handler->canRead();
}

QImage QtImageFormatsImageReader::read()
{
    // Because failed image reading might have side effects, we explicitly
    // return a null image instead of the image we've just created.
    QImage image;
    return read(&image) ? image : QImage();
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
extern void qt_imageTransform(QImage &src, QImageIOHandler::Transformations orient);
#endif

bool QtImageFormatsImageReader::read(QImage *image)
{
    if(!image)
    {
        qWarning("QImageReader::read: cannot read into null pointer");
        return false;
    }

    if(!m_impl->handler && !m_impl->initHandler())
        return false;

    // set the handler specific options.
    if(m_impl->handler->supportsOption(QImageIOHandler::ScaledSize) && m_impl->scaledSize.isValid())
    {
        if((m_impl->handler->supportsOption(QImageIOHandler::ClipRect) && !m_impl->clipRect.isNull()) || m_impl->clipRect.isNull())
        {
            // Only enable the ScaledSize option if there is no clip rect, or
            // if the handler also supports ClipRect.
            m_impl->handler->setOption(QImageIOHandler::ScaledSize, m_impl->scaledSize);
        }
    }
    if(m_impl->handler->supportsOption(QImageIOHandler::ClipRect) && !m_impl->clipRect.isNull())
        m_impl->handler->setOption(QImageIOHandler::ClipRect, m_impl->clipRect);
    if(m_impl->handler->supportsOption(QImageIOHandler::ScaledClipRect) && !m_impl->scaledClipRect.isNull())
        m_impl->handler->setOption(QImageIOHandler::ScaledClipRect, m_impl->scaledClipRect);
    if(m_impl->handler->supportsOption(QImageIOHandler::Quality))
        m_impl->handler->setOption(QImageIOHandler::Quality, m_impl->quality);

    // read the image
    if(!m_impl->handler->read(image))
    {
        m_impl->imageReaderError = QImageReader::InvalidDataError;
        m_impl->errorString = QCoreApplication::translate("QImageReader", "Unable to read image data");
        return false;
    }

    // provide default implementations for any unsupported image
    // options
    if(m_impl->handler->supportsOption(QImageIOHandler::ClipRect) && !m_impl->clipRect.isNull())
    {
        if(m_impl->handler->supportsOption(QImageIOHandler::ScaledSize) && m_impl->scaledSize.isValid())
        {
            if(m_impl->handler->supportsOption(QImageIOHandler::ScaledClipRect) && !m_impl->scaledClipRect.isNull())
            {
                // all features are supported by the handler; nothing to do.
            }
            else
            {
                // the image is already scaled, so apply scaled clipping.
                if(!m_impl->scaledClipRect.isNull())
                    *image = image->copy(m_impl->scaledClipRect);
            }
        }
        else
        {
            if(m_impl->handler->supportsOption(QImageIOHandler::ScaledClipRect) && !m_impl->scaledClipRect.isNull())
            {
                // supports scaled clipping but not scaling, most
                // likely a broken handler.
            }
            else
            {
                if(m_impl->scaledSize.isValid())
                    *image = image->scaled(m_impl->scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                if(m_impl->scaledClipRect.isValid())
                    *image = image->copy(m_impl->scaledClipRect);
            }
        }
    }
    else
    {
        if(m_impl->handler->supportsOption(QImageIOHandler::ScaledSize) && m_impl->scaledSize.isValid() && m_impl->clipRect.isNull())
        {
            if(m_impl->handler->supportsOption(QImageIOHandler::ScaledClipRect) && !m_impl->scaledClipRect.isNull())
            {
                // nothing to do (ClipRect is ignored!)
            }
            else
            {
                // provide all workarounds.
                if(m_impl->scaledClipRect.isValid())
                    *image = image->copy(m_impl->scaledClipRect);
            }
        }
        else
        {
            if(m_impl->handler->supportsOption(QImageIOHandler::ScaledClipRect) && !m_impl->scaledClipRect.isNull())
            {
                // this makes no sense; a handler that supports
                // ScaledClipRect but not ScaledSize is broken, and we
                // can't work around it.
            }
            else
            {
                // provide all workarounds.
                if(m_impl->clipRect.isValid())
                    *image = image->copy(m_impl->clipRect);
                if(m_impl->scaledSize.isValid())
                    *image = image->scaled(m_impl->scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                if(m_impl->scaledClipRect.isValid())
                    *image = image->copy(m_impl->scaledClipRect);
            }
        }
    }

    // successful read; check for "@2x" file name suffix and set device pixel ratio.
    static bool disable2xImageLoading = !qEnvironmentVariableIsEmpty("QT_HIGHDPI_DISABLE_2X_IMAGE_LOADING");
    if(!disable2xImageLoading && QFileInfo(fileName()).baseName().endsWith(QLatin1String("@2x")))
        image->setDevicePixelRatio(2.0);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
    if(autoTransform())
        qt_imageTransform(*image, transformation());
#endif

    return true;
}

bool QtImageFormatsImageReader::jumpToNextImage()
{
    if(!m_impl->initHandler())
        return false;
    return m_impl->handler->jumpToNextImage();
}

bool QtImageFormatsImageReader::jumpToImage(int imageNumber)
{
    if(!m_impl->initHandler())
        return false;
    return m_impl->handler->jumpToImage(imageNumber);
}

int QtImageFormatsImageReader::loopCount() const
{
    if(!m_impl->initHandler())
        return -1;
    return m_impl->handler->loopCount();
}

int QtImageFormatsImageReader::imageCount() const
{
    if(!m_impl->initHandler())
        return -1;
    return m_impl->handler->imageCount();
}

int QtImageFormatsImageReader::nextImageDelay() const
{
    if(!m_impl->initHandler())
        return -1;
    return m_impl->handler->nextImageDelay();
}

int QtImageFormatsImageReader::currentImageNumber() const
{
    if(!m_impl->initHandler())
        return -1;
    return m_impl->handler->currentImageNumber();
}

QRect QtImageFormatsImageReader::currentImageRect() const
{
    if(!m_impl->initHandler())
        return QRect();
    return m_impl->handler->currentImageRect();
}

QImageReader::ImageReaderError QtImageFormatsImageReader::error() const
{
    return m_impl->imageReaderError;
}

QString QtImageFormatsImageReader::errorString() const
{
    if(m_impl->errorString.isEmpty())
        return QCoreApplication::translate("QImageReader", "Unknown error");
    return m_impl->errorString;
}

bool QtImageFormatsImageReader::supportsOption(QImageIOHandler::ImageOption option) const
{
    if(!m_impl->initHandler())
        return false;
    return m_impl->handler->supportsOption(option);
}

QByteArray QtImageFormatsImageReader::imageFormat(const QString &fileName)
{
    QFile file(fileName);
    if(!file.open(QFile::ReadOnly))
        return QByteArray();
    return imageFormat(&file);
}

QByteArray QtImageFormatsImageReader::imageFormat(QIODevice *device)
{
    QByteArray format;
    QImageIOHandler *handler = createReadHandlerHelper(device, format, true, false);
    if(handler)
    {
        if(handler->canRead())
            format = handler->format();
        delete handler;
    }
    return format;
}

QList<QByteArray> QtImageFormatsImageReader::supportedImageFormats()
{
    QList<QByteArray> result;
    for(int i = 0; i < NumFormats; i++)
        result.append(BuiltInFormats[i].extensions);
    return result;
}

QList<QByteArray> QtImageFormatsImageReader::supportedMimeTypes()
{
    QList<QByteArray> result;
#if defined (WRAPPER_USE_DDS_HANDLER)
    result.append(QByteArrayLiteral("image/x-dds"));
#endif
#if defined (WRAPPER_USE_ICNS_HANDLER)
    result.append(QByteArrayLiteral("image/x-icns"));
#endif
#if defined (WRAPPER_USE_JP2_HANDLER)
    result.append(QByteArrayLiteral("image/jp2"));
    result.append(QByteArrayLiteral("image/jpx"));
    result.append(QByteArrayLiteral("image/jpm"));
    result.append(QByteArrayLiteral("video/mj2"));
#endif
#if defined (WRAPPER_USE_MNG_HANDLER)
    result.append(QByteArrayLiteral("video/x-mng"));
    result.append(QByteArrayLiteral("image/x-mng"));
#endif
#if defined (WRAPPER_USE_TGA_HANDLER)
    result.append(QByteArrayLiteral("image/x-tga"));
#endif
#if defined (WRAPPER_USE_TIFF_HANDLER)
    result.append(QByteArrayLiteral("image/tiff"));
#endif
#if defined (WRAPPER_USE_WBMP_HANDLER)
    result.append(QByteArrayLiteral("image/vnd.wap.wbmp"));
#endif
#if defined (WRAPPER_USE_WEBP_HANDLER)
    result.append(QByteArrayLiteral("image/webp"));
#endif
    return result;
}

QT_END_NAMESPACE
