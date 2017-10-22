/*
   Copyright (C) 2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>

   This file is part of the `QtUtils' library.

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

#include "ObjectiveCUtils_mac.h"

#include <AvailabilityMacros.h>

#include <cstring>
#include <algorithm>

#include <QVariantList>
#include <QVariantMap>
#include <QString>
#include <QByteArray>
#include <QDateTime>
#include <QUrl>
#include <QPixmap>
#include <QImage>
#include <QRectF>
#include <QSizeF>
#include <QPointF>
#include <QSysInfo>
//#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
//#include <QtMac>
//#endif

#include "InfoUtils.h"

namespace ObjCUtils {

namespace {

//#if (QT_VERSION < QT_VERSION_CHECK(5, 2, 0) && QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
void imageDeleter(void *image, const void *, std::size_t)
{
    delete static_cast<QImage*>(image);
}
//#endif

QPixmap QPixmapFromCGImageRef(const CGImageRef image)
{
    if(!image)
        return QPixmap();

//#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
//    return QtMac::fromCGImageRef(image);
//#elif (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
//    return QPixmap::fromMacCGImageRef(image);
//#else
    const std::size_t width = CGImageGetWidth(image);
    const std::size_t height = CGImageGetHeight(image);
    QImage result(static_cast<int>(width), static_cast<int>(height), QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);

    CFRAII<CGColorSpaceRef> colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
    if(!colorSpace)
        return QPixmap();

    CFRAII<CGContextRef> context = CGBitmapContextCreate(static_cast<void*>(result.bits()), width, height,
            8, width * 4, colorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host);
    if(!context)
        return QPixmap();

    CGRect rect = CGRectMake(0, 0, width, height);
    CGContextDrawImage(context, rect, image);
    return QPixmap::fromImage(result);
//#endif
}

CGImageRef QPixmapToCGImageRef(const QPixmap &pixmap)
{
    if(pixmap.isNull())
        return nil;

//#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
//    return QtMac::toCGImageRef(pixmap);
//#elif (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
//    return pixmap.toMacCGImageRef();
//#else
    CFRAII<CGColorSpaceRef> colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
    if(!colorSpace)
        return nil;

    QImage *image = new QImage(pixmap.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied));
    const std::size_t width = static_cast<std::size_t>(image->width());
    const std::size_t height = static_cast<std::size_t>(image->height());

    CFRAII<CGDataProviderRef> provider = CGDataProviderCreateWithData(static_cast<void*>(image),
            static_cast<const void*>(image->bits()), 4 * width * height, imageDeleter);
    if(!provider)
    {
        delete image;
        return nil;
    }

    CGImageRef result = CGImageCreate(width, height, 8, 32, 4 * width, colorSpace,
            kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host, provider, NULL, false,
            kCGRenderingIntentDefault);
    return result;
//#endif
}

} // namespace

AutoReleasePool::AutoReleasePool()
{
    m_pool = [[NSAutoreleasePool alloc] init];
}

AutoReleasePool::~AutoReleasePool()
{
    [m_pool release];
}

QVariant QVariantFromObject(const id obj)
{
    if(!obj)
        return QVariant();
    if([obj isKindOfClass:[NSDictionary class]])
        return QVariantMapFromNSDictionary(obj);
    if([obj isKindOfClass:[NSArray class]])
        return QVariantListFromNSArray(obj);
    if([obj isKindOfClass:[NSString class]])
        return QStringFromNSString(obj);
    if([obj isKindOfClass:[NSData class]])
        return QByteArrayFromNSData(obj);
    if([obj isKindOfClass:[NSDate class]])
        return QDateTimeFromNSDate(obj);
    if([obj isKindOfClass:[NSURL class]])
        return QUrlFromNSURL(obj);
    if([obj isKindOfClass:[NSImage class]])
        return QPixmapFromNSImage(obj);

    if(!([obj isKindOfClass:[NSNumber class]]))
        return QVariant();

    if(!strcmp([(NSNumber*)obj objCType], @encode(BOOL)))
        return QVariant([obj boolValue] ? true : false);
    if(!strcmp([(NSNumber*)obj objCType], @encode(signed char)))
        return QVariant([obj charValue]);
    if(!strcmp([(NSNumber*)obj objCType], @encode(unsigned char)))
        return QVariant([obj unsignedCharValue]);
    if(!strcmp([(NSNumber*)obj objCType], @encode(signed short)))
        return QVariant([obj shortValue]);
    if(!strcmp([(NSNumber*)obj objCType], @encode(unsigned short)))
        return QVariant([obj unsignedShortValue]);
    if(!strcmp([(NSNumber*)obj objCType], @encode(signed int)))
        return QVariant([obj intValue]);
    if(!strcmp([(NSNumber*)obj objCType], @encode(unsigned int)))
        return QVariant([obj unsignedIntValue]);
    if(!strcmp([(NSNumber*)obj objCType], @encode(signed long long)))
        return QVariant([obj longLongValue]);
    if(!strcmp([(NSNumber*)obj objCType], @encode(unsigned long long)))
        return QVariant([obj unsignedLongLongValue]);
    if(!strcmp([(NSNumber*)obj objCType], @encode(float)))
        return QVariant([obj floatValue]);
    if(!strcmp([(NSNumber*)obj objCType], @encode(double)))
        return QVariant([obj doubleValue]);
    return QVariant([obj doubleValue]);
}

id QVariantToObject(const QVariant &variant)
{
    const QVariant::Type type = variant.type();
    if(type == QVariant::Map)
        return QVariantMapToNSDictionary(variant.toMap());
    if(type == QVariant::List)
        return QVariantListToNSArray(variant.toList());
    if(type == QVariant::String)
        return QStringToNSString(variant.toString());
    if(type == QVariant::ByteArray)
        return QByteArrayToNSData(variant.toByteArray());
    if(type == QVariant::Time || type == QVariant::Date || type == QVariant::DateTime)
        return QDateTimeToNSDate(variant.toDateTime());
    if(type == QVariant::Url)
        return QUrlToNSURL(variant.toUrl());
    if(type == QVariant::Pixmap)
        return QPixmapToNSImage(variant.value<QPixmap>());
    if(type == QVariant::Image)
        return QPixmapToNSImage(QPixmap::fromImage(variant.value<QImage>()));
    if(type == QVariant::Bool)
        return [NSNumber numberWithBool:(variant.toBool() ? YES : NO)];
    if(type == QVariant::Char || type == QVariant::Int)
        return [NSNumber numberWithInt:variant.toInt()];
    if(type == QVariant::UInt)
        return [NSNumber numberWithUnsignedInt:variant.toUInt()];
    if(type == QVariant::LongLong)
        return [NSNumber numberWithLongLong:variant.toLongLong()];
    if(type == QVariant::ULongLong)
        return [NSNumber numberWithUnsignedLongLong:variant.toULongLong()];
    if(type == QVariant::Double)
        return [NSNumber numberWithDouble:variant.toDouble()];
    return [NSNull null];
}

QVariantMap QVariantMapFromNSDictionary(const NSDictionary *dict)
{
    QVariantMap map;
    if(!dict)
        return map;
    for(NSString *key in dict)
        map[QStringFromNSString(key)] = QVariantFromObject([dict objectForKey:key]);
    return map;
}

NSDictionary *QVariantMapToNSDictionary(const QVariantMap &map)
{
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    for(QVariantMap::ConstIterator it = map.constBegin(), itEnd = map.constEnd(); it != itEnd; ++it)
        [dict setObject:QVariantToObject(it.value()) forKey:QStringToNSString(it.key())];
    return [NSDictionary dictionaryWithDictionary:dict];
}

QVariantList QVariantListFromNSArray(const NSArray *array)
{
    QVariantList list;
    if(!array)
        return list;
    for(id obj in array)
        list.append(QVariantFromObject(obj));
    return list;
}

NSArray *QVariantListToNSArray(const QVariantList &list)
{
    NSMutableArray *array = [NSMutableArray array];
    for(QVariantList::ConstIterator it = list.constBegin(), itEnd = list.constEnd(); it != itEnd; ++it)
        [array addObject:QVariantToObject(*it)];
    return [NSArray arrayWithArray:array];
}

QString QStringFromNSString(const NSString *string)
{
    if(!string)
        return QString();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    return QString::fromNSString(string);
#else
    return QString::fromUtf8([string UTF8String]);
#endif
}

NSString *QStringToNSString(const QString &string)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    return string.toNSString();
#else
    return [NSString stringWithUTF8String: string.toUtf8().data()];
#endif
}

QByteArray QByteArrayFromNSData(const NSData *data)
{
    if(!data)
        return QByteArray();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    return QByteArray::fromNSData(data);
#else
    return QByteArray(reinterpret_cast<const char*>([data bytes]), static_cast<int>([data length]));
#endif
}

NSData *QByteArrayToNSData(const QByteArray &array)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    return array.toNSData();
#else
    return [NSData dataWithBytes:static_cast<const void*>(array.constData()) length:static_cast<NSUInteger>(array.length())];
#endif
}

QDateTime QDateTimeFromNSDate(const NSDate *date)
{
    if(!date)
        return QDateTime();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
    return QDateTime::fromNSDate(date);
#elif (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
    return QDateTime::fromMSecsSinceEpoch(qRound64([date timeIntervalSince1970] * 1000));
#else
    static const QDateTime epochStart(QDate(1970, 01, 01), QTime(0, 0), Qt::UTC);
    return epochStart.addMSecs(qRound64([date timeIntervalSince1970] * 1000)).toLocalTime();
#endif
}

NSDate *QDateTimeToNSDate(const QDateTime &dateTime)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
    return dateTime.toNSDate();
#elif (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
    return [NSDate dateWithTimeIntervalSince1970:static_cast<NSTimeInterval>(dateTime.toMSecsSinceEpoch()) / 1000];
#else
    static const QDateTime epochStart(QDate(1970, 01, 01), QTime(0, 0), Qt::UTC);
    return [NSDate dateWithTimeIntervalSince1970:static_cast<NSTimeInterval>(epochStart.msecsTo(dateTime)) / 1000];
#endif
}

QUrl QUrlFromNSURL(const NSURL *url)
{
    if(!url)
        return QUrl();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    return QUrl::fromNSURL(url);
#else
    return QUrl(QStringFromNSString([url absoluteString]));
#endif
}

NSURL *QUrlToNSURL(const QUrl &url)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    return url.toNSURL();
#else
    return [NSURL URLWithString:QStringToNSString(url.toString())];
#endif
}

QPixmap QPixmapFromNSImage(const NSImage *image)
{
    QPixmap pixmap;
    if(!image)
        return pixmap;

#if defined (AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER)
    if(InfoUtils::MacVersionGreatOrEqual(10, 6))
    {
        // https://stackoverflow.com/questions/2548059/turning-an-nsimage-into-a-cgimageref
        NSRect imageRect = NSMakeRect(0, 0, image.size.width, image.size.height);
        CGImageRef cgImage = [image CGImageForProposedRect: &imageRect context: NULL hints: nil];
        pixmap = QPixmapFromCGImageRef(cgImage);
    }
    else
#endif
    {
        // https://stackoverflow.com/questions/9264051/nsimage-size-not-real-size-with-some-pictures
        NSArray *imageReps = [image representations];
        NSInteger width = 0;
        NSInteger height = 0;
        for(NSImageRep *imageRep in imageReps)
        {
            width = std::max(width, [imageRep pixelsWide]);
            height = std::max(height, [imageRep pixelsHigh]);
        }
        // https://stackoverflow.com/questions/2468811/load-nsimage-into-qpixmap-or-qimage
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
        if(!bmp)
            return QPixmap();
        [NSGraphicsContext saveGraphicsState];
        [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithBitmapImageRep: bmp]];
        [image drawInRect: NSMakeRect(0, 0, width, height) fromRect: NSZeroRect operation: NSCompositeSourceOver fraction: 1];
        [NSGraphicsContext restoreGraphicsState];
        pixmap = QPixmapFromCGImageRef([bmp CGImage]);
        [bmp release];
    }

    return pixmap;
}

NSImage *QPixmapToNSImage(const QPixmap &pixmap)
{
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER)
    if(InfoUtils::MacVersionGreatOrEqual(10, 6))
    {
        CFRAII<CGImageRef> imageRef = QPixmapToCGImageRef(pixmap);
        if(!imageRef)
            return nil;
        return [[[NSImage alloc] initWithCGImage:imageRef size:QSizeFToNSSize(pixmap.size())] autorelease];
    }
    else
#endif
    {
        // https://stackoverflow.com/questions/10627557/mac-os-x-drawing-into-an-offscreen-nsgraphicscontext-using-cgcontextref-c-funct
        // http://www.cocoabuilder.com/archive/cocoa/133768-converting-cgimagerefs-to-nsimages-how.html
        CFRAII<CGImageRef> imageRef = QPixmapToCGImageRef(pixmap);
        if(!imageRef)
            return nil;
        const NSRect rect = QRectFToNSRect(pixmap.rect());
        NSInteger width = static_cast<NSInteger>(rect.size.width);
        NSInteger height = static_cast<NSInteger>(rect.size.height);
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
        if(!bmp)
            return nil;
        [NSGraphicsContext saveGraphicsState];
        NSGraphicsContext *context = [NSGraphicsContext graphicsContextWithBitmapImageRep: bmp];
        [NSGraphicsContext setCurrentContext: context];
        CGContextDrawImage(reinterpret_cast<CGContextRef>([context graphicsPort]), *reinterpret_cast<const CGRect*>(&rect), imageRef);
        [NSGraphicsContext restoreGraphicsState];
        NSImage *image = [[NSImage alloc] initWithSize:rect.size];
        [image addRepresentation: bmp];
        [bmp release];
        return [image autorelease];
    }
}

QRectF QRectFFromNSRect(const NSRect &rect)
{
    return QRectF(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
}

NSRect QRectFToNSRect(const QRectF &rect)
{
    return NSMakeRect(rect.x(), rect.y(), rect.width(), rect.height());
}

QSizeF QSizeFFromNSSize(const NSSize &size)
{
    return QSizeF(size.width, size.height);
}

NSSize QSizeFToNSSize(const QSizeF &size)
{
    return NSMakeSize(size.width(), size.height());
}

QPointF QPointFFromNSPoint(const NSPoint &point)
{
    return QPointF(point.x, point.y);
}

NSPoint QPointFToNSPoint(const QPointF &point)
{
    return NSMakePoint(point.x(), point.y());
}

} // namespace ObjCUtils

