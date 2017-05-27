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

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#include <set>

#include <QtGlobal>
#include <QRegExp>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QFileInfo>

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/Utils/MacImageUtils.h"

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
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        std::set<QString> fileTypes;
        for(NSString *fileType in [NSImage imageFileTypes])
        {
            QString simplifiedFileType = QString::fromUtf8([fileType UTF8String]).toLower();
            simplifiedFileType.replace(QRegExp(QString::fromLatin1("[^\\w]")), QString::fromLatin1(""));
            fileTypes.insert(simplifiedFileType.simplified());
        }
        QStringList result;
        for(std::set<QString>::const_iterator it = fileTypes.begin(); it != fileTypes.end(); ++it)
            result.append(*it);
        [pool release];
        return result;
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

        QGraphicsPixmapItem *result = NULL;
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

        NSString *pathNSString = [NSString stringWithUTF8String: filePath.toUtf8().data()];
        NSImage *picture = [[NSImage alloc] initWithContentsOfFile: pathNSString];
        if(picture == nil)
            return NULL;

        QPixmap pixmap = MacImageUtils::QPixmapFromNSImage(picture);
        if(!pixmap.isNull())
            result = new QGraphicsPixmapItem(pixmap);

        [picture release];
        [pool release];
        return result;
    }
};

DecoderAutoRegistrator registrator(new DecoderNSImage, true);

} // namespace
