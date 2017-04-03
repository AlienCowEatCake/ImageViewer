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

} // namespace

struct DecodersManager::Impl
{
    Impl()
    {}

    void checkPendingDecoderRegistration()
    {
        if(!pendingDecoders.isEmpty())
        {
            for(QList<IDecoder*>::ConstIterator it = pendingDecoders.constBegin(); it != pendingDecoders.constEnd(); ++it)
            {
                IDecoder *decoder = *it;
                decoders.insert(decoder);
                const QList<DecoderFormatInfo> info = decoder->supportedFormats();
                QStringList debugFormatList;
                for(QList<DecoderFormatInfo>::ConstIterator jt = info.constBegin(); jt != info.constEnd(); ++jt)
                {
                    const int priority = jt->decoderPriority;
                    if(priority >= 0)
                    {
                        formats[jt->format].insert(DecoderWithPriority(decoder, priority));
                        debugFormatList.append(jt->format);
                    }
                }
                qDebug() << "Decoder" << decoder->name() << "registered for" << debugFormatList;
            }
            pendingDecoders.clear();
        }
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

void DecodersManager::registerFallbackDecoder(IDecoder *decoder, int fallbackPriority)
{
    m_impl->fallbackDecoders.insert(DecoderWithPriority(decoder, fallbackPriority));
    qDebug() << "Decoder" << decoder->name() << "registered as FALLBACK with priority =" << fallbackPriority;
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
