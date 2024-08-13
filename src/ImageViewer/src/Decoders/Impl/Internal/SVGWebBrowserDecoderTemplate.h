/*
   Copyright (C) 2017-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (SVG_WEB_BROWSER_DECODER_TEMPLATE_H_INCLUDED)
#define SVG_WEB_BROWSER_DECODER_TEMPLATE_H_INCLUDED

#include <QFileInfo>
#include <QFile>
#include <QUrl>
#include <QString>
#include <QByteArray>

#include "Utils/Global.h"
#include "Utils/Logging.h"

#include "../../IDecoder.h"
#include "ImageData.h"
#include "ImageMetaData.h"
#include "Utils/MappedBuffer.h"

template<typename T>
class SVGWebBrowserDecoderTemplate : public IDecoder
{
public:
    explicit SVGWebBrowserDecoderTemplate(const char * const name)
        : m_name(name)
    {}

    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1(m_name);
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
                << QString::fromLatin1("svg")
#if defined (HAS_ZLIB)
                << QString::fromLatin1("svgz")
#endif
                   ;
    }

    QStringList advancedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList();
    }

    bool isAvailable() const Q_DECL_OVERRIDE
    {
        return T::isAvailable();
    }

    QSharedPointer<IImageData> loadImage(const QString &filePath) Q_DECL_OVERRIDE
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return QSharedPointer<IImageData>();

        QByteArray svgData = MappedBuffer(filePath, MappedBuffer::AutoInflate).byteArray();
        if(svgData.isEmpty())
        {
            LOG_WARNING() << LOGGING_CTX << "Can't read content of" << filePath;
            return QSharedPointer<IImageData>();
        }

        T *result = new T();
        if(result->load(svgData, QUrl::fromLocalFile(fileInfo.absolutePath())))
        {
            IImageMetaData *metaData = ImageMetaData::createMetaData(filePath);
            return QSharedPointer<IImageData>(new ImageData(result, filePath, name(), metaData));
        }

        LOG_WARNING() << LOGGING_CTX << "Can't load content of" << filePath;
        delete result;
        return QSharedPointer<IImageData>();
    }

private:
    const char * const m_name;
};

#endif // SVG_WEB_BROWSER_DECODER_TEMPLATE_H_INCLUDED
