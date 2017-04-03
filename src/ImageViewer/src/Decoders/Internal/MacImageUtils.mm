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

#include "MacImageUtils.h"

#include <QtGlobal>
#include <QSysInfo>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtMac>
#endif

namespace MacImageUtils {

QPixmap QPixmapFromCGImageRef(CGImageRef image)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    return QtMac::fromCGImageRef(image);
#else
    return QPixmap::fromMacCGImageRef(image);
#endif
}

QPixmap QPixmapFromNSImage(NSImage *image)
{
    QPixmap pixmap;

#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6)
    if(QSysInfo::MacintoshVersion >= QSysInfo::MV_SNOWLEOPARD)
    {
        // https://stackoverflow.com/questions/2548059/turning-an-nsimage-into-a-cgimageref
        NSRect imageRect = NSMakeRect(0, 0, image.size.width, image.size.height);
        CGImageRef cgImage = [image CGImageForProposedRect: &imageRect context: NULL hints: nil];
        pixmap = MacImageUtils::QPixmapFromCGImageRef(cgImage);
    }
    else
#endif
    {
        // https://stackoverflow.com/questions/2468811/load-nsimage-into-qpixmap-or-qimage
        NSInteger width = static_cast<NSInteger>(image.size.width);
        NSInteger height = static_cast<NSInteger>(image.size.height);
        NSBitmapImageRep *bmp = [[NSBitmapImageRep alloc]
                initWithBitmapDataPlanes: NULL
                              pixelsWide: width
                              pixelsHigh: height
                           bitsPerSample: 8
                         samplesPerPixel: 4
                                hasAlpha: YES
                                isPlanar: NO
                          colorSpaceName: NSCalibratedRGBColorSpace
                            bitmapFormat: NSAlphaFirstBitmapFormat
                             bytesPerRow: 0
                            bitsPerPixel: 0
        ];
        [NSGraphicsContext saveGraphicsState];
        [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithBitmapImageRep: bmp]];
        [image drawInRect: NSMakeRect(0, 0, width, height) fromRect: NSZeroRect operation: NSCompositeSourceOver fraction: 1];
        [NSGraphicsContext restoreGraphicsState];
        pixmap = MacImageUtils::QPixmapFromCGImageRef([bmp CGImage]);
        [bmp release];
    }

    return pixmap;
}

} // namespace MacImageUtils
