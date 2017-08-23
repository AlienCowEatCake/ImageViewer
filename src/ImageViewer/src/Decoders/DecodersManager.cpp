/*
   Copyright (C) 2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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
#include <QTime>
#include <QMap>

namespace {

struct DecoderWithPriority
{
    DecoderWithPriority(IDecoder *decoder = NULL, int priority = 0)
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
        P("DecoderSTB"                  ,  100, -1); ///< Резервный декодер, так как мало что умеет.
        P("DecoderQImage"               ,  200, -1); ///< Умеет все, что умеет Qt. Не поддерживает EXIF и ICCP.
        P("DecoderQtImageFormatsImage"  ,  300, -1); ///< Экзотические и deprecated декодеры Qt. Должен быть выше QImage.
        P("DecoderNSImage"              ,  400, -1); ///< Умеет очень много разных форматов. Должен быть выше декодеров общего назначения, но ниже специализированных декодеров.
        P("DecoderLibJpeg"              ,  500, -1); ///< Умеет jpeg форматы. Поддерживает EXIF и ICCP. Должен быть выше QImage.
        P("DecoderLibJasPer"            ,  510, 90); ///< Умеет формат JPEG 2000 и несколько побочных. Поддержка хуже, чем в QtImageFormatsImage, но имеет ряд дополнительных проверок от крашей.
        P("DecoderLibTiff"              ,  520, -1); ///< Умеет формат tiff. Поддерживает EXIF(?) и ICCP. Должен быть выше QImage и QtImageFormatsImage.
        P("DecoderJbigKit"              ,  530, -1); ///< Умеет формат JBIG1.
        /// @note Декодеры анимированных изображений
        P("DecoderQMovie"               , 1100, -1); ///< Умеет анимированные gif.
        P("DecoderLibMng"               , 1110, -1); ///< Умеет анимированные mng и jng. Поддержка mng хуже, чем в QtImageFormatsMovie.
        P("DecoderQtImageFormatsMovie"  , 1200, -1); ///< Умеет анимированные mng.
        P("DecoderLibPng"               , 1300, -1); ///< Умеет анимированные png. Поддерживает EXIF и ICCP.
        P("DecoderLibWebP"              , 1310, -1); ///< Умеет анимированные webp. Поддержка лучше, чем в QtImageFormatsMovie.
        P("DecoderLibBpg"               , 1320, -1); ///< Умеет анимированные bpg. Поддерживает EXIF и ICCP.
        /// @note Декодеры векторных изображений
        P("DecoderQtSVG"                , 2100, -1); ///< Умеет svg, но очень плохо.
        P("DecoderMacWebKit"            , 2200, -1); ///< Умеет неинтерактивные svg.
        P("DecoderLibWmf"               , 2300, -1); ///< Умеет wmf.
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

} // namespace

struct DecodersManager::Impl
{
    Impl()
    {}

    void checkPendingDecoderRegistration()
    {
        if(pendingDecoders.isEmpty())
            return;

        for(QList<IDecoder*>::ConstIterator it = pendingDecoders.constBegin(); it != pendingDecoders.constEnd(); ++it)
        {
            IDecoder *decoder = *it;
            const ComplexPriotiry priority = GetDecoderPriority(decoder);
            if(priority.mainPriority >= 0)
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
    }

    std::set<IDecoder*> decoders;
    std::set<DecoderWithPriority> fallbackDecoders;
    std::map<QString, std::set<DecoderWithPriority> > formats;
    QList<IDecoder*> pendingDecoders;
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
        result.append(it->first);
    return result;
}

QStringList DecodersManager::supportedFormatsWithWildcards() const
{
    m_impl->checkPendingDecoderRegistration();
    QStringList result;
    for(std::map<QString, std::set<DecoderWithPriority> >::const_iterator it = m_impl->formats.begin(); it != m_impl->formats.end(); ++it)
        result.append(QString::fromLatin1("*.%1").arg(it->first));
    return result;
}

QGraphicsItem *DecodersManager::loadImage(const QString &filePath)
{
    m_impl->checkPendingDecoderRegistration();
    const QFileInfo fileInfo(filePath);
    if(!fileInfo.exists() || !fileInfo.isReadable())
    {
        qDebug() << "File" << filePath << "is not exist or unreadable!";
        return NULL;
    }

    std::set<IDecoder*> failedDecodres;

    const QString extension = fileInfo.suffix().toLower();
    std::map<QString, std::set<DecoderWithPriority> >::const_iterator formatData = m_impl->formats.find(extension);
    if(formatData != m_impl->formats.end())
    {
        for(std::set<DecoderWithPriority>::const_iterator decoderData = formatData->second.begin(); decoderData != formatData->second.end(); ++decoderData)
        {
            IDecoder *decoder = decoderData->decoder;
            QTime time;
            time.start();
            QGraphicsItem *item = decoder->loadImage(filePath);
            const int elapsed = time.elapsed();
            if(item)
            {
                qDebug() << "Successfully opened" << filePath << "with decoder" << decoder->name();
                qDebug() << "Elapsed time =" << elapsed << "ms";
                return item;
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
        if(failedDecodres.find(decoder) != failedDecodres.end())
            continue;
        QTime time;
        time.start();
        QGraphicsItem *item = decoder->loadImage(filePath);
        const int elapsed = time.elapsed();
        if(item)
        {
            qDebug() << "Successfully opened" << filePath << "with decoder" << decoder->name() << "(FALLBACK)";
            qDebug() << "Elapsed time =" << elapsed << "ms";
            return item;
        }
        qDebug() << "Failed to open" << filePath << "with decoder" << decoder->name() << "(FALLBACK)";
        qDebug() << "Elapsed time =" << elapsed << "ms";
        failedDecodres.insert(decoder);
    }
    return NULL;
}

DecodersManager::DecodersManager()
    : m_impl(new Impl())
{
    qDebug() << "DecodersManager created!";
}
