/// @note This is modified QImageReader code.

#include "KImageFormatsImageReader.h"

//#define KIMAGEFORMATS_IMAGEREADER_DEBUG

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

#if defined (WRAPPER_USE_ANI_HANDLER)
#include "../kimageformats-master/src/imageformats/ani_p.h"
#endif
#if defined (WRAPPER_USE_AVIF_HANDLER)
#include "../kimageformats-master/src/imageformats/avif_p.h"
#endif
#if defined (WRAPPER_USE_EPS_HANDLER)
#include "../kimageformats-master/src/imageformats/eps_p.h"
#endif
#if defined (WRAPPER_USE_EXR_HANDLER)
#include "../kimageformats-master/src/imageformats/exr_p.h"
#endif
#if defined (WRAPPER_USE_HDR_HANDLER)
#include "../kimageformats-master/src/imageformats/hdr_p.h"
#endif
#if defined (WRAPPER_USE_HEIF_HANDLER)
#include "../kimageformats-master/src/imageformats/heif_p.h"
#endif
#if defined (WRAPPER_USE_JXL_HANDLER)
#include "../kimageformats-master/src/imageformats/jxl_p.h"
#endif
#if defined (WRAPPER_USE_KRA_HANDLER)
#include "../kimageformats-master/src/imageformats/kra.h"
#endif
#if defined (WRAPPER_USE_ORA_HANDLER)
#include "../kimageformats-master/src/imageformats/ora.h"
#endif
#if defined (WRAPPER_USE_PCX_HANDLER)
#include "../kimageformats-master/src/imageformats/pcx_p.h"
#endif
#if defined (WRAPPER_USE_PIC_HANDLER)
#include "../kimageformats-master/src/imageformats/pic_p.h"
#endif
#if defined (WRAPPER_USE_PSD_HANDLER)
#include "../kimageformats-master/src/imageformats/psd_p.h"
#endif
#if defined (WRAPPER_USE_RAS_HANDLER)
#include "../kimageformats-master/src/imageformats/ras_p.h"
#endif
#if defined (WRAPPER_USE_RGB_HANDLER)
#include "../kimageformats-master/src/imageformats/rgb_p.h"
#endif
#if defined (WRAPPER_USE_TGA_HANDLER)
#include "../kimageformats-master/src/imageformats/tga_p.h"
#endif
#if defined (WRAPPER_USE_XCF_HANDLER)
#include "../kimageformats-master/src/imageformats/xcf_p.h"
#endif

QT_BEGIN_NAMESPACE

enum BuiltInFormatType
{
#if defined (WRAPPER_USE_ANI_HANDLER)
    AniFormat,
#endif
#if defined (WRAPPER_USE_AVIF_HANDLER)
    AvifFormat,
#endif
#if defined (WRAPPER_USE_EPS_HANDLER)
    EpsFormat,
#endif
#if defined (WRAPPER_USE_EXR_HANDLER)
    ExrFormat,
#endif
#if defined (WRAPPER_USE_HDR_HANDLER)
    HdrFormat,
#endif
#if defined (WRAPPER_USE_HEIF_HANDLER)
    HeifFormat,
#endif
#if defined (WRAPPER_USE_JXL_HANDLER)
    JxlFormat,
#endif
#if defined (WRAPPER_USE_KRA_HANDLER)
    KraFormat,
#endif
#if defined (WRAPPER_USE_ORA_HANDLER)
    OraFormat,
#endif
#if defined (WRAPPER_USE_PCX_HANDLER)
    PcxFormat,
#endif
#if defined (WRAPPER_USE_PIC_HANDLER)
    PicFormat,
#endif
#if defined (WRAPPER_USE_PSD_HANDLER)
    PsdFormat,
#endif
#if defined (WRAPPER_USE_RAS_HANDLER)
    RasFormat,
#endif
#if defined (WRAPPER_USE_RGB_HANDLER)
    RgbFormat,
#endif
#if defined (WRAPPER_USE_TGA_HANDLER)
    TgaFormat,
#endif
#if defined (WRAPPER_USE_XCF_HANDLER)
    XcfFormat,
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
#if defined (WRAPPER_USE_ANI_HANDLER)
    { AniFormat , QList<QByteArray>() << "ani" },
#endif
#if defined (WRAPPER_USE_AVIF_HANDLER)
    { AvifFormat , QList<QByteArray>() << "avif" << "avifs" },
#endif
#if defined (WRAPPER_USE_EPS_HANDLER)
    { EpsFormat , QList<QByteArray>() << "eps" << "epsi" << "epsf" },
#endif
#if defined (WRAPPER_USE_EXR_HANDLER)
    { ExrFormat , QList<QByteArray>() << "exr" },
#endif
#if defined (WRAPPER_USE_HDR_HANDLER)
    { HdrFormat , QList<QByteArray>() << "hdr" },
#endif
#if defined (WRAPPER_USE_HEIF_HANDLER)
    { HeifFormat , QList<QByteArray>() << "heif" << "heic" },
#endif
#if defined (WRAPPER_USE_JXL_HANDLER)
    { JxlFormat , QList<QByteArray>() << "jxl" },
#endif
#if defined (WRAPPER_USE_KRA_HANDLER)
    { KraFormat , QList<QByteArray>() << "kra" },
#endif
#if defined (WRAPPER_USE_ORA_HANDLER)
    { OraFormat , QList<QByteArray>() << "ora" },
#endif
#if defined (WRAPPER_USE_PCX_HANDLER)
    { PcxFormat , QList<QByteArray>() << "pcx" },
#endif
#if defined (WRAPPER_USE_PIC_HANDLER)
    { PicFormat , QList<QByteArray>() << "pic" },
#endif
#if defined (WRAPPER_USE_PSD_HANDLER)
    { PsdFormat , QList<QByteArray>() << "psd" << "psb" << "pdd" << "psdt" },
#endif
#if defined (WRAPPER_USE_RAS_HANDLER)
    { RasFormat , QList<QByteArray>() << "ras" },
#endif
#if defined (WRAPPER_USE_RGB_HANDLER)
    { RgbFormat , QList<QByteArray>() << "rgb" << "rgba" << "bw" << "sgi" },
#endif
#if defined (WRAPPER_USE_TGA_HANDLER)
    { TgaFormat , QList<QByteArray>() << "tga" },
#endif
#if defined (WRAPPER_USE_XCF_HANDLER)
    { XcfFormat , QList<QByteArray>() << "xcf" },
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
#if defined (KIMAGEFORMATS_IMAGEREADER_DEBUG)
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
#if defined (WRAPPER_USE_ANI_HANDLER)
        case AniFormat:
            handler = new ANIHandler;
            break;
#endif
#if defined (WRAPPER_USE_AVIF_HANDLER)
        case AvifFormat:
            handler = new QAVIFHandler;
            break;
#endif
#if defined (WRAPPER_USE_EPS_HANDLER)
        case EpsFormat:
            handler = new EPSHandler;
            break;
#endif
#if defined (WRAPPER_USE_EXR_HANDLER)
        case ExrFormat:
            handler = new EXRHandler;
            break;
#endif
#if defined (WRAPPER_USE_HDR_HANDLER)
        case HdrFormat:
            handler = new HDRHandler;
            break;
#endif
#if defined (WRAPPER_USE_HEIF_HANDLER)
        case HeifFormat:
            handler = new HEIFHandler;
            break;
#endif
#if defined (WRAPPER_USE_JXL_HANDLER)
        case JxlFormat:
            handler = new QJpegXLHandler;
            break;
#endif
#if defined (WRAPPER_USE_KRA_HANDLER)
        case KraFormat:
            handler = new KraHandler;
            break;
#endif
#if defined (WRAPPER_USE_ORA_HANDLER)
        case OraFormat:
            handler = new OraHandler;
            break;
#endif
#if defined (WRAPPER_USE_PCX_HANDLER)
        case PcxFormat:
            handler = new PCXHandler;
            break;
#endif
#if defined (WRAPPER_USE_PIC_HANDLER)
        case PicFormat:
            handler = new SoftimagePICHandler;
            break;
#endif
#if defined (WRAPPER_USE_PSD_HANDLER)
        case PsdFormat:
            handler = new PSDHandler;
            break;
#endif
#if defined (WRAPPER_USE_RAS_HANDLER)
        case RasFormat:
            handler = new RASHandler;
            break;
#endif
#if defined (WRAPPER_USE_RGB_HANDLER)
        case RgbFormat:
            handler = new RGBHandler;
            break;
#endif
#if defined (WRAPPER_USE_TGA_HANDLER)
        case TgaFormat:
            handler = new TGAHandler;
            break;
#endif
#if defined (WRAPPER_USE_XCF_HANDLER)
        case XcfFormat:
            handler = new XCFHandler;
            break;
#endif
        default:
            break;
        }
#if defined (KIMAGEFORMATS_IMAGEREADER_DEBUG)
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
#if defined (WRAPPER_USE_ANI_HANDLER)
            case AniFormat:
                if(ANIHandler::canRead(device))
                    handler = new ANIHandler;
                break;
#endif
#if defined (WRAPPER_USE_AVIF_HANDLER)
            case AvifFormat:
                if(QAVIFHandler::canRead(device))
                    handler = new QAVIFHandler;
                break;
#endif
#if defined (WRAPPER_USE_EPS_HANDLER)
            case EpsFormat:
                if(EPSHandler::canRead(device))
                    handler = new EPSHandler;
                break;
#endif
#if defined (WRAPPER_USE_EXR_HANDLER)
            case ExrFormat:
                if(EXRHandler::canRead(device))
                    handler = new EXRHandler;
                break;
#endif
#if defined (WRAPPER_USE_HDR_HANDLER)
            case HdrFormat:
                if(HDRHandler::canRead(device))
                    handler = new HDRHandler;
                break;
#endif
#if defined (WRAPPER_USE_HEIF_HANDLER)
            case HeifFormat:
                if(HEIFHandler::canRead(device))
                    handler = new HEIFHandler;
                break;
#endif
#if defined (WRAPPER_USE_JXL_HANDLER)
            case JxlFormat:
                if(QJpegXLHandler::canRead(device))
                    handler = new QJpegXLHandler;
                break;
#endif
#if defined (WRAPPER_USE_KRA_HANDLER)
            case KraFormat:
                if(KraHandler::canRead(device))
                    handler = new KraHandler;
                break;
#endif
#if defined (WRAPPER_USE_ORA_HANDLER)
            case OraFormat:
                if(OraHandler::canRead(device))
                    handler = new OraHandler;
                break;
#endif
#if defined (WRAPPER_USE_PCX_HANDLER)
            case PcxFormat:
                if(PCXHandler::canRead(device))
                    handler = new PCXHandler;
                break;
#endif
#if defined (WRAPPER_USE_PIC_HANDLER)
            case PicFormat:
                if(SoftimagePICHandler::canRead(device))
                    handler = new SoftimagePICHandler;
                break;
#endif
#if defined (WRAPPER_USE_PSD_HANDLER)
            case PsdFormat:
                if(PSDHandler::canRead(device))
                    handler = new PSDHandler;
                break;
#endif
#if defined (WRAPPER_USE_RAS_HANDLER)
            case RasFormat:
                if(RASHandler::canRead(device))
                    handler = new RASHandler;
                break;
#endif
#if defined (WRAPPER_USE_RGB_HANDLER)
            case RgbFormat:
                if(RGBHandler::canRead(device))
                    handler = new RGBHandler;
                break;
#endif
#if defined (WRAPPER_USE_TGA_HANDLER)
            case TgaFormat:
                if(TGAHandler::canRead(device))
                    handler = new TGAHandler;
                break;
#endif
#if defined (WRAPPER_USE_XCF_HANDLER)
            case XcfFormat:
                if(XCFHandler::canRead(device))
                    handler = new XCFHandler;
                break;
#endif
            default:
                break;
            }
            if(!device->isSequential())
                device->seek(pos);

            if(handler)
            {
#if defined (KIMAGEFORMATS_IMAGEREADER_DEBUG)
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
#if defined (KIMAGEFORMATS_IMAGEREADER_DEBUG)
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

struct KImageFormatsImageReader::Impl
{
    Impl(KImageFormatsImageReader *reader);
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

    KImageFormatsImageReader *reader;
};

KImageFormatsImageReader::Impl::Impl(KImageFormatsImageReader *reader)
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

KImageFormatsImageReader::Impl::~Impl()
{
    if(deleteDevice)
        delete device;
    delete handler;
}

bool KImageFormatsImageReader::Impl::initHandler()
{
    // check some preconditions
    if(!device || (!deleteDevice && !device->isOpen() && !device->open(QIODevice::ReadOnly)))
    {
        imageReaderError = QImageReader::DeviceError;
        errorString = qApp->translate("QImageReader", "Invalid device");
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

        QList<QByteArray> extensions = KImageFormatsImageReader::supportedImageFormats();
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
            errorString = qApp->translate("QImageReader", "File not found");
            file->setFileName(fileName); // restore the old file name
            return false;
        }
    }

    // assign a handler
    if(!handler && (handler = createReadHandlerHelper(device, format, autoDetectImageFormat, ignoresFormatAndExtension)) == 0)
    {
        imageReaderError = QImageReader::UnsupportedFormatError;
        errorString = qApp->translate("QImageReader", "Unsupported image format");
        return false;
    }
    return true;
}

void KImageFormatsImageReader::Impl::getText()
{
    if(text.isEmpty() && (handler || initHandler()) && handler->supportsOption(QImageIOHandler::Description))
        text = getImageTextFromDescription(handler->option(QImageIOHandler::Description).toString());
}

KImageFormatsImageReader::KImageFormatsImageReader()
    : m_impl(new Impl(this))
{}

KImageFormatsImageReader::KImageFormatsImageReader(QIODevice *device, const QByteArray &format)
    : m_impl(new Impl(this))
{
    m_impl->device = device;
    m_impl->format = format;
}

KImageFormatsImageReader::KImageFormatsImageReader(const QString &fileName, const QByteArray &format)
    : m_impl(new Impl(this))
{
    m_impl->device = new QFile(fileName);
    m_impl->format = format;
    m_impl->deleteDevice = true;
}

KImageFormatsImageReader::~KImageFormatsImageReader()
{}

void KImageFormatsImageReader::setFormat(const QByteArray &format)
{
    m_impl->format = format;
}

QByteArray KImageFormatsImageReader::format() const
{
    if(!m_impl->format.isEmpty())
        return m_impl->format;
    if(!m_impl->initHandler())
        return QByteArray();
    if(m_impl->handler->canRead())
        return m_impl->handler->format();
    return QByteArray();
}

void KImageFormatsImageReader::setAutoDetectImageFormat(bool enabled)
{
    m_impl->autoDetectImageFormat = enabled;
}

bool KImageFormatsImageReader::autoDetectImageFormat() const
{
    return m_impl->autoDetectImageFormat;
}

void KImageFormatsImageReader::setDecideFormatFromContent(bool ignored)
{
    m_impl->ignoresFormatAndExtension = ignored;
}

bool KImageFormatsImageReader::decideFormatFromContent() const
{
    return m_impl->ignoresFormatAndExtension;
}

void KImageFormatsImageReader::setDevice(QIODevice *device)
{
    if(m_impl->device && m_impl->deleteDevice)
        delete m_impl->device;
    m_impl->device = device;
    m_impl->deleteDevice = false;
    delete m_impl->handler;
    m_impl->handler = Q_NULLPTR;
    m_impl->text.clear();
}

QIODevice *KImageFormatsImageReader::device() const
{
    return m_impl->device;
}

void KImageFormatsImageReader::setFileName(const QString &fileName)
{
    setDevice(new QFile(fileName));
    m_impl->deleteDevice = true;
}

QString KImageFormatsImageReader::fileName() const
{
    QFile *file = qobject_cast<QFile*>(m_impl->device);
    return file ? file->fileName() : QString();
}

QSize KImageFormatsImageReader::size() const
{
    if(!m_impl->initHandler())
        return QSize();
    if(m_impl->handler->supportsOption(QImageIOHandler::Size))
        return m_impl->handler->option(QImageIOHandler::Size).toSize();
    return QSize();
}

QImage::Format KImageFormatsImageReader::imageFormat() const
{
    if(!m_impl->initHandler())
        return QImage::Format_Invalid;
    if(m_impl->handler->supportsOption(QImageIOHandler::ImageFormat))
        return (QImage::Format)m_impl->handler->option(QImageIOHandler::ImageFormat).toInt();
    return QImage::Format_Invalid;
}

QStringList KImageFormatsImageReader::textKeys() const
{
    m_impl->getText();
    return m_impl->text.keys();
}

QString KImageFormatsImageReader::text(const QString &key) const
{
    m_impl->getText();
    return m_impl->text.value(key);
}

void KImageFormatsImageReader::setClipRect(const QRect &rect)
{
    m_impl->clipRect = rect;
}

QRect KImageFormatsImageReader::clipRect() const
{
    return m_impl->clipRect;
}

void KImageFormatsImageReader::setScaledSize(const QSize &size)
{
    m_impl->scaledSize = size;
}

QSize KImageFormatsImageReader::scaledSize() const
{
    return m_impl->scaledSize;
}

void KImageFormatsImageReader::setQuality(int quality)
{
    m_impl->quality = quality;
}

int KImageFormatsImageReader::quality() const
{
    return m_impl->quality;
}

void KImageFormatsImageReader::setScaledClipRect(const QRect &rect)
{
    m_impl->scaledClipRect = rect;
}

QRect KImageFormatsImageReader::scaledClipRect() const
{
    return m_impl->scaledClipRect;
}

void KImageFormatsImageReader::setBackgroundColor(const QColor &color)
{
    if(!m_impl->initHandler())
        return;
    if(m_impl->handler->supportsOption(QImageIOHandler::BackgroundColor))
        m_impl->handler->setOption(QImageIOHandler::BackgroundColor, color);
}

QColor KImageFormatsImageReader::backgroundColor() const
{
    if(!m_impl->initHandler())
        return QColor();
    if(m_impl->handler->supportsOption(QImageIOHandler::BackgroundColor))
        return qvariant_cast<QColor>(m_impl->handler->option(QImageIOHandler::BackgroundColor));
    return QColor();
}

bool KImageFormatsImageReader::supportsAnimation() const
{
    if(!m_impl->initHandler())
        return false;
    if(m_impl->handler->supportsOption(QImageIOHandler::Animation))
        return m_impl->handler->option(QImageIOHandler::Animation).toBool();
    return false;
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
QImageIOHandler::Transformations KImageFormatsImageReader::transformation() const
{
    int option = QImageIOHandler::TransformationNone;
    if(m_impl->initHandler() && m_impl->handler->supportsOption(QImageIOHandler::ImageTransformation))
        option = m_impl->handler->option(QImageIOHandler::ImageTransformation).toInt();
    return QImageIOHandler::Transformations(option);
}

void KImageFormatsImageReader::setAutoTransform(bool enabled)
{
    m_impl->autoTransform = enabled ? Impl::ApplyTransform : Impl::DoNotApplyTransform;
}

bool KImageFormatsImageReader::autoTransform() const
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
void KImageFormatsImageReader::setGamma(float gamma)
{
    if(m_impl->initHandler() && m_impl->handler->supportsOption(QImageIOHandler::Gamma))
        m_impl->handler->setOption(QImageIOHandler::Gamma, gamma);
}

float KImageFormatsImageReader::gamma() const
{
    if(m_impl->initHandler() && m_impl->handler->supportsOption(QImageIOHandler::Gamma))
        return m_impl->handler->option(QImageIOHandler::Gamma).toFloat();
    return 0.0f;
}
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
QByteArray KImageFormatsImageReader::subType() const
{
    if(!m_impl->initHandler())
        return QByteArray();
    if(m_impl->handler->supportsOption(QImageIOHandler::SubType))
        return m_impl->handler->option(QImageIOHandler::SubType).toByteArray();
    return QByteArray();
}

QList<QByteArray> KImageFormatsImageReader::supportedSubTypes() const
{
    if(!m_impl->initHandler())
        return QList<QByteArray>();
    if(m_impl->handler->supportsOption(QImageIOHandler::SupportedSubTypes))
        return m_impl->handler->option(QImageIOHandler::SupportedSubTypes).value< QList<QByteArray> >();
    return QList<QByteArray>();
}
#endif

bool KImageFormatsImageReader::canRead() const
{
    if(!m_impl->initHandler())
        return false;
    return m_impl->handler->canRead();
}

QImage KImageFormatsImageReader::read()
{
    // Because failed image reading might have side effects, we explicitly
    // return a null image instead of the image we've just created.
    QImage image;
    return read(&image) ? image : QImage();
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
extern void qt_imageTransform(QImage &src, QImageIOHandler::Transformations orient);
#endif

bool KImageFormatsImageReader::read(QImage *image)
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
        m_impl->errorString = qApp->translate("QImageReader", "Unable to read image data");
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

bool KImageFormatsImageReader::jumpToNextImage()
{
    if(!m_impl->initHandler())
        return false;
    return m_impl->handler->jumpToNextImage();
}

bool KImageFormatsImageReader::jumpToImage(int imageNumber)
{
    if(!m_impl->initHandler())
        return false;
    return m_impl->handler->jumpToImage(imageNumber);
}

int KImageFormatsImageReader::loopCount() const
{
    if(!m_impl->initHandler())
        return -1;
    return m_impl->handler->loopCount();
}

int KImageFormatsImageReader::imageCount() const
{
    if(!m_impl->initHandler())
        return -1;
    return m_impl->handler->imageCount();
}

int KImageFormatsImageReader::nextImageDelay() const
{
    if(!m_impl->initHandler())
        return -1;
    return m_impl->handler->nextImageDelay();
}

int KImageFormatsImageReader::currentImageNumber() const
{
    if(!m_impl->initHandler())
        return -1;
    return m_impl->handler->currentImageNumber();
}

QRect KImageFormatsImageReader::currentImageRect() const
{
    if(!m_impl->initHandler())
        return QRect();
    return m_impl->handler->currentImageRect();
}

QImageReader::ImageReaderError KImageFormatsImageReader::error() const
{
    return m_impl->imageReaderError;
}

QString KImageFormatsImageReader::errorString() const
{
    if(m_impl->errorString.isEmpty())
        return qApp->translate("QImageReader", "Unknown error");
    return m_impl->errorString;
}

bool KImageFormatsImageReader::supportsOption(QImageIOHandler::ImageOption option) const
{
    if(!m_impl->initHandler())
        return false;
    return m_impl->handler->supportsOption(option);
}

QByteArray KImageFormatsImageReader::imageFormat(const QString &fileName)
{
    QFile file(fileName);
    if(!file.open(QFile::ReadOnly))
        return QByteArray();
    return imageFormat(&file);
}

QByteArray KImageFormatsImageReader::imageFormat(QIODevice *device)
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

QList<QByteArray> KImageFormatsImageReader::supportedImageFormats()
{
    QList<QByteArray> result;
    for(int i = 0; i < NumFormats; i++)
        result.append(BuiltInFormats[i].extensions);
    return result;
}

QList<QByteArray> KImageFormatsImageReader::supportedMimeTypes()
{
    QList<QByteArray> result;
#if defined (WRAPPER_USE_ANI_HANDLER)
    result.append(QByteArrayLiteral("application/x-navi-animation"));
#endif
#if defined (WRAPPER_USE_AVIF_HANDLER)
    result.append(QByteArrayLiteral("image/avif"));
#endif
#if defined (WRAPPER_USE_EPS_HANDLER)
    result.append(QByteArrayLiteral("image/x-eps"));
#endif
#if defined (WRAPPER_USE_EXR_HANDLER)
    result.append(QByteArrayLiteral("image/x-exr"));
#endif
#if defined (WRAPPER_USE_HDR_HANDLER)
    result.append(QByteArrayLiteral("image/x-hdr"));
    result.append(QByteArrayLiteral("image/vnd.radiance"));
#endif
#if defined (WRAPPER_USE_HEIF_HANDLER)
    result.append(QByteArrayLiteral("image/heif"));
#endif
#if defined (WRAPPER_USE_JXL_HANDLER)
    result.append(QByteArrayLiteral("image/jxl"));
#endif
#if defined (WRAPPER_USE_KRA_HANDLER)
    result.append(QByteArrayLiteral("application/x-krita"));
#endif
#if defined (WRAPPER_USE_ORA_HANDLER)
    result.append(QByteArrayLiteral("image/openraster"));
#endif
#if defined (WRAPPER_USE_PCX_HANDLER)
    result.append(QByteArrayLiteral("image/x-pcx"));
#endif
#if defined (WRAPPER_USE_PIC_HANDLER)
    result.append(QByteArrayLiteral("image/x-pic"));
#endif
#if defined (WRAPPER_USE_PSD_HANDLER)
    result.append(QByteArrayLiteral("image/vnd.adobe.photoshop"));
#endif
#if defined (WRAPPER_USE_RAS_HANDLER)
    result.append(QByteArrayLiteral("image/x-sun-raster"));
#endif
#if defined (WRAPPER_USE_RGB_HANDLER)
    result.append(QByteArrayLiteral("image/x-rgb"));
#endif
#if defined (WRAPPER_USE_TGA_HANDLER)
    result.append(QByteArrayLiteral("image/x-tga"));
#endif
#if defined (WRAPPER_USE_XCF_HANDLER)
    result.append(QByteArrayLiteral("image/x-xcf"));
#endif
    return result;
}

QT_END_NAMESPACE
