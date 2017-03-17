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

#include "DecoderNSImage.h"

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#include <set>

#include <QtGlobal>
#include <QRegExp>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QFileInfo>
#include <QSysInfo>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtMac>
#endif

#include "DecoderAutoRegistrator.h"

#if defined (QT_DEBUG)
#define DECODER_NSIMAGE_PRIORITY -1
#else
#define DECODER_NSIMAGE_PRIORITY 120
#endif

namespace {

DecoderAutoRegistrator registrator(new DecoderNSImage, DECODER_NSIMAGE_PRIORITY);

QPixmap fromCGImageRef(CGImageRef image)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    return QtMac::fromCGImageRef(image);
#else
    return QPixmap::fromMacCGImageRef(image);
#endif
}

} // namespace

QString DecoderNSImage::name() const
{
    return QString::fromLatin1("DecoderNSImage");
}

QList<DecoderFormatInfo> DecoderNSImage::supportedFormats() const
{
    @autoreleasepool
    {
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
        return result;
    }
}

QGraphicsItem *DecoderNSImage::loadImage(const QString &filePath)
{
    const QFileInfo fileInfo(filePath);
    if(!fileInfo.exists() || !fileInfo.isReadable())
        return NULL;

    QGraphicsPixmapItem *result = NULL;

    @autoreleasepool
    {
        NSString *pathNSString = [NSString stringWithUTF8String: filePath.toUtf8().data()];
        NSImage *picture = [[NSImage alloc] initWithContentsOfFile: pathNSString];
        if(picture == nil)
            return NULL;

        QPixmap pixmap;

#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6)
        if(QSysInfo::MacintoshVersion >= QSysInfo::MV_SNOWLEOPARD)
        {
            // https://stackoverflow.com/questions/2548059/turning-an-nsimage-into-a-cgimageref
            NSRect imageRect = NSMakeRect(0, 0, picture.size.width, picture.size.height);
            CGImageRef cgImage = [picture CGImageForProposedRect: &imageRect context: NULL hints: nil];
            pixmap = fromCGImageRef(cgImage);
        }
        else
#endif
        {
            // https://stackoverflow.com/questions/2468811/load-nsimage-into-qpixmap-or-qimage
            NSInteger width = static_cast<NSInteger>(picture.size.width);
            NSInteger height = static_cast<NSInteger>(picture.size.height);
            NSBitmapImageRep *bmp = [[NSBitmapImageRep alloc]
                    initWithBitmapDataPlanes: NULL
                                  pixelsWide: width
                                  pixelsHigh: height
                               bitsPerSample: 8
                             samplesPerPixel: 4
                                    hasAlpha: YES
                                    isPlanar: NO
                              colorSpaceName: NSDeviceRGBColorSpace
                                bitmapFormat: NSAlphaFirstBitmapFormat
                                 bytesPerRow: 0
                                bitsPerPixel: 0
            ];
            [NSGraphicsContext saveGraphicsState];
            [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithBitmapImageRep: bmp]];
            [picture drawInRect:NSMakeRect(0, 0, width, height) fromRect: NSZeroRect operation: NSCompositeSourceOver fraction: 1];
            [NSGraphicsContext restoreGraphicsState];
            pixmap = fromCGImageRef([bmp CGImage]);
            [bmp release];
        }

        if(!pixmap.isNull())
            result = new QGraphicsPixmapItem(pixmap);

        [picture release];
    }

    return result;
}
