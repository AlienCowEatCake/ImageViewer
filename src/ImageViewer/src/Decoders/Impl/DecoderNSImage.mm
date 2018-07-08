/*
   Copyright (C) 2017-2018 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#include <set>

#include <QtGlobal>
#include <QRegExp>
#include <QPixmap>
#include <QImage>
#include <QFileInfo>

#include "Utils/ObjectiveCUtils.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"

namespace {

class DecoderNSImage : public IDecoder
{
public:
    QString name() const
    {
        return QString::fromLatin1("DecoderNSImage");
    }

    QStringList supportedFormats() const
    {
        AUTORELEASE_POOL;
        std::set<QString> fileTypes;
        for(NSString *fileType in [NSImage imageFileTypes])
        {
            QString simplifiedFileType = ObjCUtils::QStringFromNSString(fileType).toLower();
            simplifiedFileType.replace(QRegExp(QString::fromLatin1("[^\\w]")), QString::fromLatin1(""));
            fileTypes.insert(simplifiedFileType.simplified());
        }
        QStringList result;
        for(std::set<QString>::const_iterator it = fileTypes.begin(); it != fileTypes.end(); ++it)
            result.append(*it);
        return result;
    }

    QStringList advancedFormats() const
    {
        return QStringList();
    }

    bool isAvailable() const
    {
        return true;
    }

    QSharedPointer<IImageData> loadImage(const QString &filePath)
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return QSharedPointer<IImageData>();

        AUTORELEASE_POOL;
        NSImage *picture = [[[NSImage alloc] initWithContentsOfFile: ObjCUtils::QStringToNSString(filePath)] autorelease];
        if(picture == nil)
            return QSharedPointer<IImageData>();
        QGraphicsItem *result = GraphicsItemsFactory::instance().createPixmapItem(ObjCUtils::QPixmapFromNSImage(picture));
        return QSharedPointer<IImageData>(new ImageData(result, name()));
    }
};

DecoderAutoRegistrator registrator(new DecoderNSImage, true);

} // namespace
