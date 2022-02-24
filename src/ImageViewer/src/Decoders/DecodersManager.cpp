/*
   Copyright (C) 2017-2022 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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
#include <cassert>

#include <QGraphicsItem>
#include <QFileInfo>
#include <QDebug>
#include <QMap>
#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
#include <QElapsedTimer>
#else
#include <QTime>
typedef QTime QElapsedTimer;
#endif

#include "Utils/Global.h"
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
        /// @note Декодеры статических изображений
        P("DecoderWIC"                  ,   60, -1); ///< Умеет много разных форматов, но плохо. Годится только в качестве резервного.
        P("DecoderSTB"                  ,  100, 10); ///< Резервный декодер, так как мало что умеет.
        P("DecoderQImage"               ,  200, 20); ///< Умеет все, что умеет Qt. Не поддерживает EXIF и ICCP.
        P("DecoderQtImageFormatsImage"  ,  300, -1); ///< Экзотические и deprecated декодеры Qt. Должен быть выше QImage.
        P("DecoderGraphicsMagick"       ,  330, -1); ///< Умеет очень много разных форматов, в том числе анимированные.
        P("DecoderGraphicsMagickWand"   ,  340, -1); ///< Умеет очень много разных форматов, в том числе анимированные. Более высокоуровневое API, чем GraphicsMagick.
        P("DecoderMagickCore"           ,  350, -1); ///< Умеет очень много разных форматов, в том числе анимированные.
        P("DecoderMagickWand"           ,  360, -1); ///< Умеет очень много разных форматов, в том числе анимированные. Более высокоуровневое API, чем MagickCore.
        P("DecoderNSImage"              ,  400, -1); ///< Умеет очень много разных форматов. Должен быть выше декодеров общего назначения, но ниже специализированных декодеров.
        P("DecoderLibJpeg"              ,  500, 70); ///< Умеет jpeg форматы. Поддерживает EXIF и ICCP. Должен быть выше QImage.
        P("DecoderLibJasPer"            ,  510, 80); ///< Умеет формат JPEG 2000 и несколько побочных. Поддержка хуже, чем в QtImageFormatsImage, но имеет ряд дополнительных проверок от крашей.
        P("DecoderOpenJPEG"             ,  520, -1); ///< Умеет формат JPEG 2000. Поддерживает ICCP. Поддержка в чем-то лучше LibJasPer, в чем-то хуже.
        P("DecoderLibTiff"              ,  530, -1); ///< Умеет формат tiff. Поддерживает EXIF(?) и ICCP. Должен быть выше QImage и QtImageFormatsImage.
        P("DecoderJbigKit"              ,  540, -1); ///< Умеет формат JBIG1.
        P("DecoderLibRaw"               ,  550, -1); ///< Умеет форматы RAW.
        P("DecoderLibHEIF"              ,  560, -1); ///< Умеет формат HEIF с кодеками HEVC и AV1, а также некоторые AVIF.
        P("DecoderOpenEXR"              ,  570, -1); ///< Умеет формат EXR.
        P("DecoderLibAvif"              ,  580, 90); ///< Умеет формат AVIF, некоторые HEIF с кодеком AV1.
        P("DecoderLERC"                 ,  590, -1); ///< Умеет формат LERC.
        P("DecoderJxrLib"               ,  600, -1); ///< Умеет формат JPEG XR.
        /// @note Декодеры анимированных изображений
        P("DecoderQMovie"               , 1090, -1); ///< Умеет анимированные gif.
        P("DecoderGifLib"               , 1100, -1); ///< Умеет анимированные gif, но медленнее, чем QMovie, зато поддерживает ICCP и более всеяден.
        P("DecoderLibMng"               , 1110, -1); ///< Умеет анимированные mng и jng. Поддержка mng хуже, чем в QtImageFormatsMovie.
        P("DecoderQtImageFormatsMovie"  , 1200, -1); ///< Умеет анимированные mng.
        P("DecoderLibPng"               , 1300, -1); ///< Умеет анимированные png. Поддерживает EXIF и ICCP.
        P("DecoderLibWebP"              , 1310, -1); ///< Умеет анимированные webp. Поддержка лучше, чем в QtImageFormatsMovie.
        P("DecoderLibBpg"               , 1320, -1); ///< Умеет анимированные bpg. Поддерживает EXIF и ICCP.
        P("DecoderFLIF"                 , 1330, -1); ///< Умеет анимированные flif.
        P("DecoderLibJxl"               , 1340, -1); ///< Умеет анимированные jxl.
        /// @note Декодеры векторных изображений
        P("DecoderNanoSVG"              , 2000, -1); ///< Умеет только самые простые неинтерактивные svg без текста.
        P("DecoderMSHTML"               , 2010, -1); ///< Умеет неинтерактивные svg без прозрачности, требует IE9+. Может некорректно определять геометрию.
        P("DecoderMSEdgeWebView2"       , 2011, -1); ///< Умеет неинтерактивные svg, требует Edge. Может некорректно определять геометрию.
        P("DecoderQtSVG"                , 2100, -1); ///< Умеет svg, но очень плохо.
        P("DecoderReSVGLt001100"        , 2200, -1); ///< Умеет неинтерактивные svg, подгружает внешние библиотеки. Нестабилен, слишком строг к стандартам. Нестабильное API/ABI. Весьма медленная отрисовка.
        P("DecoderReSVGLt001300"        , 2201, -1); ///< Умеет неинтерактивные svg, подгружает внешние библиотеки. Нестабильное API/ABI. Весьма медленная отрисовка.
        P("DecoderReSVG"                , 2202, -1); ///< Умеет неинтерактивные svg, подгружает внешние библиотеки. Нестабильное API/ABI. Весьма медленная отрисовка.
        P("DecoderLibRSVG"              , 2300, -1); ///< Умеет неинтерактивные svg, подгружает внешние библиотеки. Весьма медленная отрисовка.
        P("DecoderMacWKWebView"         , 2400, -1); ///< Умеет неинтерактивные svg, только под macOS. Весьма медленная отрисовка. Медленнее, чем MacWebView, использует приватное недокументированное API.
        P("DecoderMacWebView"           , 2401, -1); ///< Умеет неинтерактивные svg, только под macOS. Весьма медленная отрисовка. Deprecated начиная с macOS 10.14.
        P("DecoderQMLWebEngine"         , 2500, -1); ///< Умеет чуть меньше, чем QtWebEngine, к тому же еще медленнее.
        P("DecoderQtWebEngine"          , 2510, -1); ///< Умеет анимированные svg, в том числе и с JavaScript. Весьма медленная отрисовка.
        P("DecoderQtWebKit"             , 2600, -1); ///< Умеет анимированные svg, в том числе и с JavaScript. Очень быстр, но deprecated.
        P("DecoderLibWmf"               , 2700, -1); ///< Умеет wmf.
#undef P
    }

    QMap<QString, ComplexPriotiry>::ConstIterator it = decoderPriotities.find(decoder->name());
    if(it != decoderPriotities.end())
        return it.value();

    static ComplexPriotiry unknownDecoderPriority(20000, 10000);
    decoderPriotities[decoder->name()] = unknownDecoderPriority;
    qWarning() << "Unknown priority for decoder" << decoder->name();
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
                qDebug() << "Decoder" << decoder->name() << "was registered for" << supportedFormats << " with priority =" << priority.mainPriority;

                if(priority.advancedPriority >= 0)
                {
                    const QStringList advancedFormats = decoder->advancedFormats();
                    for(QStringList::ConstIterator jt = advancedFormats.constBegin(); jt != advancedFormats.constEnd(); ++jt)
                        formats[*jt].insert(DecoderWithPriority(decoder, priority.advancedPriority));
                    qDebug() << "Decoder" << decoder->name() << "was registered for" << advancedFormats << " with priority =" << priority.advancedPriority;
                }
            }
            else
            {
                qDebug() << "Decoder" << decoder->name() << "was NOT registered with priority =" << priority.mainPriority;
            }
        }
        pendingDecoders.clear();
        updateBlacklistedDecoders();
    }

    QStringList getBlacklistFromSettings() const
    {
        return decodersSettins.value(DECODERS_SETTINGS_BLACKLIST_KEY, QString()).toString().split(QChar::fromLatin1(','),
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
                                                                                                  Qt::SkipEmptyParts);
#else
                                                                                                  QString::SkipEmptyParts);
#endif
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
    qDebug() << "DecodersManager destroyed!";
}

DecodersManager &DecodersManager::getInstance()
{
    static DecodersManager manager;
    return manager;
}

void DecodersManager::registerDecoder(IDecoder *decoder)
{
    m_impl->pendingDecoders.append(decoder);
    qDebug() << "Decoder" << decoder->name() << "was planned for delayed registration";
}

void DecodersManager::registerFallbackDecoder(IDecoder *decoder)
{
    const int fallbackPriority = GetDecoderPriority(decoder).mainPriority;
    m_impl->fallbackDecoders.insert(DecoderWithPriority(decoder, fallbackPriority));
    qDebug() << "Decoder" << decoder->name() << "was registered as FALLBACK with priority =" << fallbackPriority;
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
        qDebug() << "File" << filePath << "is not exist or unreadable!";
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
            QElapsedTimer timer;
            timer.start();
            QSharedPointer<IImageData> data = decoder->loadImage(filePath);
            const qint64 elapsed = static_cast<qint64>(timer.elapsed());
            if(data && !data->isEmpty())
            {
                qDebug() << "Successfully opened" << filePath << "with decoder" << decoder->name();
                qDebug() << "Elapsed time =" << elapsed << "ms";
                return data;
            }
            qDebug() << "Failed to open" << filePath << "with decoder" << decoder->name();
            qDebug() << "Elapsed time =" << elapsed << "ms";
            failedDecodres.insert(decoder);
        }
    }

    qDebug() << "Unknown format for file" << filePath << "with extension" << extension;
    for(std::set<DecoderWithPriority>::const_iterator decoderData = m_impl->fallbackDecoders.begin(); decoderData != m_impl->fallbackDecoders.end(); ++decoderData)
    {
        IDecoder *decoder = decoderData->decoder;
        if(m_impl->blacklistedDecoders.find(decoder) != m_impl->blacklistedDecoders.end())
            continue;
        if(failedDecodres.find(decoder) != failedDecodres.end())
            continue;
        QElapsedTimer timer;
        timer.start();
        QSharedPointer<IImageData> data = decoder->loadImage(filePath);
        const qint64 elapsed = static_cast<qint64>(timer.elapsed());
        if(data && !data->isEmpty())
        {
            qDebug() << "Successfully opened" << filePath << "with decoder" << decoder->name() << "(FALLBACK)";
            qDebug() << "Elapsed time =" << elapsed << "ms";
            return data;
        }
        qDebug() << "Failed to open" << filePath << "with decoder" << decoder->name() << "(FALLBACK)";
        qDebug() << "Elapsed time =" << elapsed << "ms";
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
        qDebug() << "File" << filePath << "is not exist or unreadable!";
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
            qDebug() << "Successfully opened" << filePath << "with decoder" << decoder->name();
            qDebug() << "Elapsed time =" << elapsed << "ms";
        }
        else
        {
            data = QSharedPointer<IImageData>();
            qDebug() << "Failed to open" << filePath << "with decoder" << decoder->name();
            qDebug() << "Elapsed time =" << elapsed << "ms";
        }
        return data;
    }

    qDebug() << "Decoder with name" << decoderName << "was not found";
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
    qDebug() << "DecodersManager created!";
}
