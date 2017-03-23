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

#include "DecoderNSImage.h"

#include <set>

#include <QtGlobal>
#include <QRegExp>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QFileInfo>

#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/MacImageUtils.h"

#if defined (QT_DEBUG)
#define DECODER_NSIMAGE_PRIORITY -1
#else
#define DECODER_NSIMAGE_PRIORITY 120
#endif

namespace {

DecoderAutoRegistrator registrator(new DecoderNSImage, DECODER_NSIMAGE_PRIORITY);

} // namespace

QString DecoderNSImage::name() const
{
    return QString::fromLatin1("DecoderNSImage");
}

QList<DecoderFormatInfo> DecoderNSImage::supportedFormats() const
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    std::set<QString> fileTypes;
    for(NSString *fileType in [NSImage imageFileTypes])
    {
        QString simplifiedFileType = QString::fromUtf8([fileType UTF8String]).toLower();
        simplifiedFileType.replace(QRegExp(QString::fromLatin1("[^\\w]")), QString::fromLatin1(""));
        fileTypes.insert(simplifiedFileType.simplified());
    }
    QList<DecoderFormatInfo> result;
    for(std::set<QString>::const_iterator it = fileTypes.begin(); it != fileTypes.end(); ++it)
    {
        DecoderFormatInfo info;
        info.decoderPriority = DECODER_NSIMAGE_PRIORITY;
        info.format = *it;
        result.append(info);
    }
    [pool release];
    return result;
}

QGraphicsItem *DecoderNSImage::loadImage(const QString &filePath)
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
