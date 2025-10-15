/*
   Copyright (C) 2017-2025 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "DecodersManager.h"

#include <map>
#include <set>

#include <QGraphicsItem>
#include <QFileInfo>
#include <QMap>
#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
#include <QElapsedTimer>
#else
#include <QTime>
typedef QTime QElapsedTimer;
#endif

#include "Utils/Global.h"
#include "Utils/Logging.h"
#include "Utils/SettingsWrapper.h"

#include "Impl/Internal/ImageData.h"

namespace {

const QString DECODERS_SETTINGS_BLACKLIST_KEY   = QString::fromLatin1("Blacklist");

struct DecoderWithPriority
{
    DecoderWithPriority(IDecoder *decoder = Q_NULLPTR, int priority = 0)
        : decoder(decoder)
        , priority(priority)
    {}

    bool operator < (const DecoderWithPriority &other) const
    {
        return priority > other.priority;
    }

    IDecoder *decoder;
    int priority;
};

struct ComplexPriotiry
{
    ComplexPriotiry(int mainPriority = -1, int advancedPriority = -1)
        : mainPriority(mainPriority)
        , advancedPriority(advancedPriority)
    {}

    int mainPriority;
    int advancedPriority;
};

ComplexPriotiry GetDecoderPriority(const IDecoder *decoder)
{
    static QMap<QString, ComplexPriotiry> decoderPriotities;
    if(decoderPriotities.isEmpty())
    {
#define P(NAME, MAIN_PRIORITY, ADVANCED_PRIORITY) decoderPriotities[QString::fromLatin1(NAME)] = ComplexPriotiry(MAIN_PRIORITY, ADVANCED_PRIORITY)
        /// @note Decoders for static raster images
        P("DecoderWIC"                  ,   60, -1); ///< Generic Windows decoder.
        P("DecoderSTB"                  ,  100, 10); ///< Generic STB decoder.
        P("DecoderQImage"               ,  200, 20); ///< Generic Qt decoder for default formats.
        P("DecoderQtImageFormatsImage"  ,  300, -1); ///< Generic QtImageFormats (Qt) decoder for advanced and deprecated formats.
        P("DecoderKImageFormatsImage"   ,  380, -1); ///< Generic KImageFormats (KDE) decoder for various formats.
        P("DecoderNSImage"              ,  400, -1); ///< Generic macOS decoder.
        P("DecoderLibJpeg"              ,  500, 70); ///< For JPEG images.
        P("DecoderLibJasPer"            ,  510, 80); ///< For JPEG 2000 images and some other.
        P("DecoderOpenJPEG"             ,  520, -1); ///< For JPEG 2000 images.
        P("DecoderLibTiff"              ,  530, -1); ///< For TIFF images.
        P("DecoderJbigKit"              ,  540, -1); ///< For JBIG1 images.
        P("DecoderLibRaw"               ,  550, -1); ///< For RAW images.
        P("DecoderLibHEIF"              ,  560, -1); ///< For HEIF images with HEVC and AV1 codecs and some AVIF images.
        P("DecoderOpenEXR"              ,  570, -1); ///< For EXR images.
        P("DecoderLERC"                 ,  590, -1); ///< For LERC images.
        P("DecoderJxrLib"               ,  600, -1); ///< For JPEG XR images.
        /// @note Decoders for animated raster images
        P("DecoderQMovie"               , 1090, -1); ///< Generic Qt decoder for GIF images.
        P("DecoderGifLib"               , 1100, -1); ///< For GIF images.
        P("DecoderLibMng"               , 1110, -1); ///< For MNG and JNG images. MNG support is worse than in QtImageFormatsMovie.
        P("DecoderQtImageFormatsMovie"  , 1200, -1); ///< Generic QtImageFormats (Qt) decoder for MNG images.
        P("DecoderKImageFormatsMovie"   , 1210, -1); ///< Generic KImageFormats (KDE) decoder for various animated formats.
        P("DecoderLibPng"               , 1300, -1); ///< For PNG and APNG images.
        P("DecoderLibWebP"              , 1310, -1); ///< For WEBP images.
        P("DecoderLibBpg"               , 1320, -1); ///< For BPG images.
        P("DecoderFLIF"                 , 1330, -1); ///< For FLIF images.
        P("DecoderLibJxl"               , 1340, -1); ///< For JXL images.
        P("DecoderLibAvif"              , 1360, 90); ///< For AVIF images and HEIF images with AV1 codec.
        /// @note Decoders for vector images
        P("DecoderQtSVG"                , 2100, -1); ///< Generic Qt decoder for SVG images.
        P("DecoderReSVGLt001100"        , 2200, -1); ///< For static SVG images via external library with unstable API/ABI. Rendering is slow.
        P("DecoderReSVGLt001300"        , 2201, -1); ///< For static SVG images via external library with unstable API/ABI. Rendering is slow.
        P("DecoderReSVGLt003300"        , 2202, -1); ///< For static SVG images via external library with unstable API/ABI. Rendering is slow.
        P("DecoderLibRSVG"              , 2300, -1); ///< For static SVG images via external library. Rendering is slow.
        P("DecoderReSVG"                , 2350, -1); ///< For static SVG images via external library with unstable API/ABI.
        P("DecoderLibWmf"               , 2700, -1); ///< For WMF images.
#undef P
    }

    QMap<QString, ComplexPriotiry>::ConstIterator it = decoderPriotities.find(decoder->name());
    if(it != decoderPriotities.constEnd())
        return it.value();

    static ComplexPriotiry unknownDecoderPriority(20000, 10000);
    decoderPriotities[decoder->name()] = unknownDecoderPriority;
    LOG_WARNING() << LOGGING_CTX << "Unknown priority for decoder" << decoder->name();
    unknownDecoderPriority.mainPriority++;
    unknownDecoderPriority.advancedPriority++;
    return unknownDecoderPriority;
}

class StubGraphicsItem : public QGraphicsItem
{
public:
    explicit StubGraphicsItem(const QSize &size)
        : m_size(size)
    {}

    QRectF boundingRect() const Q_DECL_OVERRIDE
    {
        return QRect(QPoint(0, 0), m_size);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) Q_DECL_OVERRIDE
    {
        Q_UNUSED(painter);
        Q_UNUSED(option);
        Q_UNUSED(widget);
    }

private:
    const QSize m_size;
};

} // namespace

struct DecodersManager::Impl
{
    Impl()
        : decodersSettins(QString::fromLatin1("Decoders"))
    {}

    void checkPendingDecoderRegistration()
    {
        if(pendingDecoders.isEmpty())
            return;

        for(QList<IDecoder*>::ConstIterator it = pendingDecoders.constBegin(); it != pendingDecoders.constEnd(); ++it)
        {
            IDecoder *decoder = *it;
            const ComplexPriotiry priority = GetDecoderPriority(decoder);
            if(priority.mainPriority >= 0 && decoder->isAvailable())
            {
                decoders.insert(decoder);
                const QStringList supportedFormats = decoder->supportedFormats();
                for(QStringList::ConstIterator jt = supportedFormats.constBegin(); jt != supportedFormats.constEnd(); ++jt)
                    formats[*jt].insert(DecoderWithPriority(decoder, priority.mainPriority));
                LOG_DEBUG() << LOGGING_CTX << "Decoder" << decoder->name() << "was registered for" << supportedFormats << " with priority =" << priority.mainPriority;

                if(priority.advancedPriority >= 0)
                {
                    const QStringList advancedFormats = decoder->advancedFormats();
                    for(QStringList::ConstIterator jt = advancedFormats.constBegin(); jt != advancedFormats.constEnd(); ++jt)
                        formats[*jt].insert(DecoderWithPriority(decoder, priority.advancedPriority));
                    LOG_DEBUG() << LOGGING_CTX << "Decoder" << decoder->name() << "was registered for" << advancedFormats << " with priority =" << priority.advancedPriority;
                }
            }
            else
            {
                LOG_DEBUG() << LOGGING_CTX << "Decoder" << decoder->name() << "was NOT registered with priority =" << priority.mainPriority;
            }
        }
        pendingDecoders.clear();
        updateBlacklistedDecoders();
    }

    QStringList getBlacklistFromSettings() const
    {
        return decodersSettins.value(DECODERS_SETTINGS_BLACKLIST_KEY, QString()).toString().split(QChar::fromLatin1(','), Qt_SkipEmptyParts);
    }

    QStringList getUnregisteredFromBlacklist() const
    {
        QStringList result;
        const QStringList blacklist = getBlacklistFromSettings();
        for(QStringList::ConstIterator it = blacklist.constBegin(), itEnd = blacklist.constEnd(); it != itEnd; ++it)
        {
            bool found = false;
            for(std::set<IDecoder*>::const_iterator jt = decoders.begin(), jtEnd = decoders.end(); jt != jtEnd && !found; ++jt)
                if((*jt)->name().compare(*it, Qt::CaseInsensitive) == 0)
                    found = true;
            if(!found)
                result.append(*it);
        }
        return result;
    }

    void updateBlacklistedDecoders()
    {
        blacklistedDecoders.clear();
        const QStringList blacklist = getBlacklistFromSettings();
        for(std::set<IDecoder*>::const_iterator it = decoders.begin(), itEnd = decoders.end(); it != itEnd; ++it)
            if(blacklist.contains((*it)->name(), Qt::CaseInsensitive))
                blacklistedDecoders.insert(*it);
    }

    std::set<IDecoder*> decoders;
    std::set<DecoderWithPriority> fallbackDecoders;
    std::map<QString, std::set<DecoderWithPriority> > formats;
    QList<IDecoder*> pendingDecoders;
    SettingsWrapper decodersSettins;
    std::set<IDecoder*> blacklistedDecoders;
};

DecodersManager::~DecodersManager()
{
    LOG_DEBUG() << LOGGING_CTX << "DecodersManager destroyed!";
}

DecodersManager &DecodersManager::getInstance()
{
    static DecodersManager manager;
    return manager;
}

void DecodersManager::registerDecoder(IDecoder *decoder)
{
    m_impl->pendingDecoders.append(decoder);
    LOG_DEBUG() << LOGGING_CTX << "Decoder" << decoder->name() << "was planned for delayed registration";
}

void DecodersManager::registerFallbackDecoder(IDecoder *decoder)
{
    const int fallbackPriority = GetDecoderPriority(decoder).mainPriority;
    m_impl->fallbackDecoders.insert(DecoderWithPriority(decoder, fallbackPriority));
    LOG_DEBUG() << LOGGING_CTX << "Decoder" << decoder->name() << "was registered as FALLBACK with priority =" << fallbackPriority;
}

QStringList DecodersManager::registeredDecoders() const
{
    m_impl->checkPendingDecoderRegistration();
    QStringList result;
    for(std::set<IDecoder*>::const_iterator it = m_impl->decoders.begin(); it != m_impl->decoders.end(); ++it)
        result.append((*it)->name());
    return result;
}

QStringList DecodersManager::supportedFormats() const
{
    m_impl->checkPendingDecoderRegistration();
    QStringList result;
    for(std::map<QString, std::set<DecoderWithPriority> >::const_iterator it = m_impl->formats.begin(); it != m_impl->formats.end(); ++it)
    {
        const std::set<DecoderWithPriority>& decodersWithPriority = it->second;
        bool inBlacklist = true;
        for(std::set<DecoderWithPriority>::const_iterator jt = decodersWithPriority.begin(), jtEnd = decodersWithPriority.end(); jt != jtEnd && inBlacklist; ++jt)
            if(m_impl->blacklistedDecoders.find(jt->decoder) == m_impl->blacklistedDecoders.end())
                inBlacklist = false;
        if(!inBlacklist)
            result.append(it->first);
    }
    return result;
}

QStringList DecodersManager::supportedFormatsWithWildcards() const
{
    QStringList result = supportedFormats();
    for(QStringList::iterator it = result.begin(), itEnd = result.end(); it != itEnd; ++it)
        *it = QString::fromLatin1("*.%1").arg(*it);
    return result;
}

QStringList DecodersManager::blackListedDecoders() const
{
    m_impl->checkPendingDecoderRegistration();
    QStringList result;
    for(std::set<IDecoder*>::const_iterator it = m_impl->blacklistedDecoders.begin(), itEnd = m_impl->blacklistedDecoders.end(); it != itEnd; ++it)
        result.append((*it)->name());
    return result;
}

void DecodersManager::setBlackListedDecoders(const QStringList &blackListedDecoders) const
{
    m_impl->checkPendingDecoderRegistration();
    m_impl->decodersSettins.setValue(DECODERS_SETTINGS_BLACKLIST_KEY, (blackListedDecoders + m_impl->getUnregisteredFromBlacklist()).join(QChar::fromLatin1(',')));
    m_impl->updateBlacklistedDecoders();
}

QSharedPointer<IImageData> DecodersManager::loadImage(const QString &filePath)
{
    m_impl->checkPendingDecoderRegistration();
    const QFileInfo fileInfo(filePath);
    if(!fileInfo.exists() || !fileInfo.isReadable())
    {
        LOG_WARNING() << LOGGING_CTX << "File" << filePath << "is not exist or unreadable!";
        return QSharedPointer<IImageData>();
    }

    std::set<IDecoder*> failedDecodres;

    const QString extension = fileInfo.suffix().toLower();
    std::map<QString, std::set<DecoderWithPriority> >::const_iterator formatData = m_impl->formats.find(extension);
    if(formatData != m_impl->formats.end())
    {
        for(std::set<DecoderWithPriority>::const_iterator decoderData = formatData->second.begin(); decoderData != formatData->second.end(); ++decoderData)
        {
            IDecoder *decoder = decoderData->decoder;
            if(m_impl->blacklistedDecoders.find(decoder) != m_impl->blacklistedDecoders.end())
                continue;
            if(!decoder->isAvailable())
                continue;
            QElapsedTimer timer;
            timer.start();
            QSharedPointer<IImageData> data = decoder->loadImage(filePath);
            const qint64 elapsed = static_cast<qint64>(timer.elapsed());
            if(data && !data->isEmpty())
            {
                LOG_DEBUG() << LOGGING_CTX << "Successfully opened" << filePath << "with decoder" << decoder->name();
                LOG_DEBUG() << LOGGING_CTX << "Elapsed time =" << elapsed << "ms";
                return data;
            }
            LOG_DEBUG() << LOGGING_CTX << "Failed to open" << filePath << "with decoder" << decoder->name();
            LOG_DEBUG() << LOGGING_CTX << "Elapsed time =" << elapsed << "ms";
            failedDecodres.insert(decoder);
        }
    }

    LOG_WARNING() << LOGGING_CTX << "Unknown format for file" << filePath << "with extension" << extension;
    for(std::set<DecoderWithPriority>::const_iterator decoderData = m_impl->fallbackDecoders.begin(); decoderData != m_impl->fallbackDecoders.end(); ++decoderData)
    {
        IDecoder *decoder = decoderData->decoder;
        if(m_impl->blacklistedDecoders.find(decoder) != m_impl->blacklistedDecoders.end())
            continue;
        if(!decoder->isAvailable())
            continue;
        if(failedDecodres.find(decoder) != failedDecodres.end())
            continue;
        QElapsedTimer timer;
        timer.start();
        QSharedPointer<IImageData> data = decoder->loadImage(filePath);
        const qint64 elapsed = static_cast<qint64>(timer.elapsed());
        if(data && !data->isEmpty())
        {
            LOG_DEBUG() << LOGGING_CTX << "Successfully opened" << filePath << "with decoder" << decoder->name() << "(FALLBACK)";
            LOG_DEBUG() << LOGGING_CTX << "Elapsed time =" << elapsed << "ms";
            return data;
        }
        LOG_DEBUG() << LOGGING_CTX << "Failed to open" << filePath << "with decoder" << decoder->name() << "(FALLBACK)";
        LOG_DEBUG() << LOGGING_CTX << "Elapsed time =" << elapsed << "ms";
        failedDecodres.insert(decoder);
    }
    return QSharedPointer<IImageData>();
}

QSharedPointer<IImageData> DecodersManager::loadImage(const QString &filePath, const QString &decoderName)
{
    m_impl->checkPendingDecoderRegistration();
    const QFileInfo fileInfo(filePath);
    if(!fileInfo.exists() || !fileInfo.isReadable())
    {
        LOG_WARNING() << LOGGING_CTX << "File" << filePath << "is not exist or unreadable!";
        return QSharedPointer<IImageData>();
    }

    for(std::set<IDecoder*>::const_iterator it = m_impl->decoders.begin(), itEnd = m_impl->decoders.end(); it != itEnd; ++it)
    {
        IDecoder *decoder = *it;
        if(decoder->name() != decoderName)
            continue;

        QElapsedTimer timer;
        timer.start();
        QSharedPointer<IImageData> data = decoder->loadImage(filePath);
        const qint64 elapsed = static_cast<qint64>(timer.elapsed());
        if(data && !data->isEmpty())
        {
            LOG_DEBUG() << LOGGING_CTX << "Successfully opened" << filePath << "with decoder" << decoder->name();
            LOG_DEBUG() << LOGGING_CTX << "Elapsed time =" << elapsed << "ms";
        }
        else
        {
            data = QSharedPointer<IImageData>();
            LOG_DEBUG() << LOGGING_CTX << "Failed to open" << filePath << "with decoder" << decoder->name();
            LOG_DEBUG() << LOGGING_CTX << "Elapsed time =" << elapsed << "ms";
        }
        return data;
    }

    LOG_WARNING() << LOGGING_CTX << "Decoder with name" << decoderName << "was not found";
    return QSharedPointer<IImageData>();
}

QSharedPointer<IImageData> DecodersManager::generateStub(const QSize &size, const QString &filePath)
{
    return QSharedPointer<IImageData>(new ImageData(new StubGraphicsItem(size), filePath, QString::fromLatin1("DecoderStub")));
}

QSharedPointer<IImageData> DecodersManager::generateStub(const QSharedPointer<IImageData> &base)
{
    return base ? generateStub(base->size(), base->filePath()) : QSharedPointer<IImageData>();
}

DecodersManager::DecodersManager()
    : m_impl(new Impl())
{
    LOG_DEBUG() << LOGGING_CTX << "DecodersManager created!";
}
