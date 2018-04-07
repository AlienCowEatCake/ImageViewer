/*
   Copyright (C) 2018 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <QFileInfo>
#include <QFile>
#include <QUrl>
#include <QString>
#include <QByteArray>
#include <QDebug>

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItems/QtWebEngineSVGGraphicsItem.h"
#if defined (HAS_ZLIB)
#include "Internal/Utils/ZLibUtils.h"
#endif

namespace {

class DecoderQtWebEngine : public IDecoder
{
public:
    QString name() const
    {
        return QString::fromLatin1("DecoderQtWebEngine");
    }

    QStringList supportedFormats() const
    {
        return QStringList()
                << QString::fromLatin1("svg")
#if defined (HAS_ZLIB)
                << QString::fromLatin1("svgz")
#endif
                   ;
    }

    QStringList advancedFormats() const
    {
        return QStringList();
    }

    QGraphicsItem *loadImage(const QString &filePath)
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return NULL;

        QByteArray svgData;
#if defined (HAS_ZLIB)
        if(fileInfo.suffix().toLower() == QString::fromLatin1("svgz"))
        {
            svgData = ZLibUtils::InflateFile(fileInfo.absoluteFilePath());
        }
        else
#endif
        {
            QFile inFile(filePath);
            if(!inFile.open(QIODevice::ReadOnly))
            {
                qWarning() << "Can't open" << filePath;
                return NULL;
            }
            svgData = inFile.readAll();
            inFile.close();
        }

        if(svgData.isEmpty())
        {
            qWarning() << "Can't read content of" << filePath;
            return NULL;
        }

        QtWebEngineSVGGraphicsItem *result = new QtWebEngineSVGGraphicsItem();
        if(result->load(svgData, QUrl::fromLocalFile(fileInfo.absolutePath())))
            return result;

        qWarning() << "Can't load content of" << filePath;
        delete result;
        return NULL;
    }
};

DecoderAutoRegistrator registrator(new DecoderQtWebEngine);

} // namespace
