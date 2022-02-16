/*
   Copyright (C) 2017-2022 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "Global.h"
#include "InfoUtils.h"

namespace ObjCUtils {

AutoReleasePool::AutoReleasePool()
{
    m_pool = [[NSAutoreleasePool alloc] init];
}

AutoReleasePool::~AutoReleasePool()
{
    [m_pool release];
}

//#if (QT_VERSION < QT_VERSION_CHECK(5, 2, 0) && QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
namespace {
void imageDeleter(void *image, const void *, std::size_t)
{
    delete static_cast<QImage*>(image);
}
} // namespace
//#endif

QPixmap QPixmapFromCGImageRef(const CFTypePtr<CGImageRef> &image)
{
    if(!image)
        return QPixmap();

//#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
//    return QtMac::fromCGImageRef(image);
//#elif (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
//    return QPixmap::fromMacCGImageRef(image);
//#else
    return QPixmap::fromImage(QImageFromCGImageRef(image));
//#endif
}

CFTypePtr<CGImageRef> QPixmapToCGImageRef(const QPixmap &pixmap)
{
    if(pixmap.isNull())
        return Q_NULLPTR;

//#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
//    return CFTypePtrFromCreate(QtMac::toCGImageRef(pixmap));
//#elif (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
//    return CFTypePtrFromCreate(pixmap.toMacCGImageRef());
//#else
    return QImageToCGImageRef(pixmap.toImage());
//#endif
}

QImage QImageFromCGImageRef(const CFTypePtr<CGImageRef> &image)
{
    if(!image)
        return QImage();

    const std::size_t width = CGImageGetWidth(image);
    const std::size_t height = CGImageGetHeight(image);
    QImage result(static_cast<int>(width), static_cast<int>(height), QImage::Format_ARGB32_Premultiplied);
    if(result.isNull())
        return QImage();

    result.fill(Qt::transparent);

    const CFTypePtr<CGColorSpaceRef> colorSpace = CFTypePtrFromCreate(CGColorSpaceCreateWithName(kCGColorSpaceSRGB));
    if(!colorSpace)
        return QImage();

    const CFTypePtr<CGContextRef> context = CFTypePtrFromCreate(CGBitmapContextCreate(static_cast<void*>(result.bits()),
            width, height, 8, width * 4, colorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host));
    if(!context)
        return QImage();

    CGRect rect = CGRectMake(0, 0, width, height);
    CGContextDrawImage(context, rect, image);
    return result;
}

CFTypePtr<CGImageRef> QImageToCGImageRef(const QImage &image)
{
    if(image.isNull())
        return Q_NULLPTR;

    const CFTypePtr<CGColorSpaceRef> colorSpace = CFTypePtrFromCreate(CGColorSpaceCreateWithName(kCGColorSpaceSRGB));
    if(!colorSpace)
        return Q_NULLPTR;

    QImage *imageDP = new QImage(image.convertToFormat(QImage::Format_ARGB32_Premultiplied));
    const std::size_t width = static_cast<std::size_t>(imageDP->width());
    const std::size_t height = static_cast<std::size_t>(imageDP->height());

    const CFTypePtr<CGDataProviderRef> provider = CFTypePtrFromCreate(CGDataProviderCreateWithData(
            static_cast<void*>(imageDP), static_cast<const void*>(imageDP->bits()), 4 * width * height, imageDeleter));
    if(!provider)
    {
        delete imageDP;
        return Q_NULLPTR;
    }

    return CFTypePtrFromCreate(CGImageCreate(width, height, 8, 32, 4 * width, colorSpace,
            kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host, provider, Q_NULLPTR, false,
            kCGRenderingIntentDefault));
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
        return QImageFromNSImage(obj);

    if(!([obj isKindOfClass:[NSNumber class]]))
        return QVariant();

    const char *numberType = [(NSNumber*)obj objCType];
    if(!strcmp(numberType, @encode(BOOL)))
        return QVariant([obj boolValue] ? true : false);
    if(!strcmp(numberType, @encode(signed char)) || !strcmp(numberType, @encode(char)))
        return QVariant([obj charValue]);
    if(!strcmp(numberType, @encode(unsigned char)))
        return QVariant([obj unsignedCharValue]);
    if(!strcmp(numberType, @encode(signed short)))
        return QVariant([obj shortValue]);
    if(!strcmp(numberType, @encode(unsigned short)))
        return QVariant([obj unsignedShortValue]);
    if(!strcmp(numberType, @encode(signed int)))
        return QVariant([obj intValue]);
    if(!strcmp(numberType, @encode(unsigned int)))
        return QVariant([obj unsignedIntValue]);
    if(!strcmp(numberType, @encode(signed long)))
        return QVariant(static_cast<signed long long>([obj longValue]));
    if(!strcmp(numberType, @encode(unsigned long)))
        return QVariant(static_cast<unsigned long long>([obj unsignedLongValue]));
    if(!strcmp(numberType, @encode(signed long long)))
        return QVariant([obj longLongValue]);
    if(!strcmp(numberType, @encode(unsigned long long)))
        return QVariant([obj unsignedLongLongValue]);
    if(!strcmp(numberType, @encode(float)))
        return QVariant([obj floatValue]);
    if(!strcmp(numberType, @encode(double)))
        return QVariant([obj doubleValue]);
    return QVariant([obj doubleValue]);
}

id QVariantToObject(const QVariant &variant)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    enum Types
    {
        QMetaType_QVariantMap   = QMetaType::QVariantMap,
        QMetaType_QVariantList  = QMetaType::QVariantList,
        QMetaType_QString       = QMetaType::QString,
        QMetaType_QByteArray    = QMetaType::QByteArray,
        QMetaType_QTime         = QMetaType::QTime,
        QMetaType_QDate         = QMetaType::QDate,
        QMetaType_QDateTime     = QMetaType::QDateTime,
        QMetaType_QUrl          = QMetaType::QUrl,
        QMetaType_QPixmap       = QMetaType::QPixmap,
        QMetaType_QImage        = QMetaType::QImage,
        QMetaType_Bool          = QMetaType::Bool,
        QMetaType_QChar         = QMetaType::QChar,
        QMetaType_Char          = QMetaType::Char,
        QMetaType_UChar         = QMetaType::UChar,
        QMetaType_Short         = QMetaType::Short,
        QMetaType_UShort        = QMetaType::UShort,
        QMetaType_Int           = QMetaType::Int,
        QMetaType_UInt          = QMetaType::UInt,
        QMetaType_Long          = QMetaType::Long,
        QMetaType_ULong         = QMetaType::ULong,
        QMetaType_LongLong      = QMetaType::LongLong,
        QMetaType_ULongLong     = QMetaType::ULongLong,
        QMetaType_Float         = QMetaType::Float,
        QMetaType_Double        = QMetaType::Double
    };
    const int type = variant.typeId();
#else
    enum Types
    {
        QMetaType_QVariantMap   = QVariant::Map,
        QMetaType_QVariantList  = QVariant::List,
        QMetaType_QString       = QVariant::String,
        QMetaType_QByteArray    = QVariant::ByteArray,
        QMetaType_QTime         = QVariant::Time,
        QMetaType_QDate         = QVariant::Date,
        QMetaType_QDateTime     = QVariant::DateTime,
        QMetaType_QUrl          = QVariant::Url,
        QMetaType_QPixmap       = QVariant::Pixmap,
        QMetaType_QImage        = QVariant::Image,
        QMetaType_Bool          = QVariant::Bool,
        QMetaType_QChar         = QVariant::Char,
        QMetaType_Char          = QVariant::Int,        ///< !
        QMetaType_UChar         = QVariant::UInt,       ///< !
        QMetaType_Short         = QVariant::Int,        ///< !
        QMetaType_UShort        = QVariant::UInt,       ///< !
        QMetaType_Int           = QVariant::Int,
        QMetaType_UInt          = QVariant::UInt,
        QMetaType_Long          = QVariant::LongLong,   ///< !
        QMetaType_ULong         = QVariant::ULongLong,  ///< !
        QMetaType_LongLong      = QVariant::LongLong,
        QMetaType_ULongLong     = QVariant::ULongLong,
        QMetaType_Float         = QVariant::Double,     ///< !
        QMetaType_Double        = QVariant::Double,
    };
    const int type = static_cast<int>(variant.type());
#endif
    if(type == QMetaType_QVariantMap)
        return QVariantMapToNSDictionary(variant.toMap());
    if(type == QMetaType_QVariantList)
        return QVariantListToNSArray(variant.toList());
    if(type == QMetaType_QString)
        return QStringToNSString(variant.toString());
    if(type == QMetaType_QByteArray)
        return QByteArrayToNSData(variant.toByteArray());
    if(type == QMetaType_QTime || type == QMetaType_QDate || type == QMetaType_QDateTime)
        return QDateTimeToNSDate(variant.toDateTime());
    if(type == QMetaType_QUrl)
        return QUrlToNSURL(variant.toUrl());
    if(type == QMetaType_QPixmap)
        return QPixmapToNSImage(variant.value<QPixmap>());
    if(type == QMetaType_QImage)
        return QImageToNSImage(variant.value<QImage>());
    if(type == QMetaType_Bool)
        return [NSNumber numberWithBool:(variant.toBool() ? YES : NO)];
    if(type == QMetaType_QChar || type == QMetaType_Char || type == QMetaType_Short || type == QMetaType_Int)
        return [NSNumber numberWithInt:variant.toInt()];
    if(type == QMetaType_UChar || type == QMetaType_UShort || type == QMetaType_UInt)
        return [NSNumber numberWithUnsignedInt:variant.toUInt()];
    if(type == QMetaType_Long || type == QMetaType_LongLong)
        return [NSNumber numberWithLongLong:variant.toLongLong()];
    if(type == QMetaType_ULong || type == QMetaType_ULongLong)
        return [NSNumber numberWithUnsignedLongLong:variant.toULongLong()];
    if(type == QMetaType_Float || type == QMetaType_Double)
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
    return [NSString stringWithUTF8String:string.toUtf8().data()];
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
    return QPixmap::fromImage(QImageFromNSImage(image));
}

NSImage *QPixmapToNSImage(const QPixmap &pixmap, const QSize &sizeInPoints)
{
    return QImageToNSImage(pixmap.toImage(), sizeInPoints);
}

QImage QImageFromNSImage(const NSImage *image)
{
    QImage result;
    if(!image)
        return result;

#if defined (AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER)
    if(InfoUtils::MacVersionGreatOrEqual(10, 6))
    {
        // https://stackoverflow.com/questions/2548059/turning-an-nsimage-into-a-cgimageref
        NSRect imageRect = NSMakeRect(0, 0, image.size.width, image.size.height);
        result = QImageFromCGImageRef(CFTypePtrFromGet([image CGImageForProposedRect:&imageRect context:nil hints:nil]));
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
                initWithBitmapDataPlanes:nil
                              pixelsWide:width
                              pixelsHigh:height
                           bitsPerSample:8
                         samplesPerPixel:4
                                hasAlpha:YES
                                isPlanar:NO
                          colorSpaceName:NSCalibratedRGBColorSpace
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_14_AND_LATER)
                            bitmapFormat:NSBitmapFormatAlphaFirst
#else
                            bitmapFormat:NSAlphaFirstBitmapFormat
#endif
                             bytesPerRow:0
                            bitsPerPixel:0
        ];
        if(!bmp)
            return result;
        [NSGraphicsContext saveGraphicsState];
        [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithBitmapImageRep:bmp]];
        [image drawInRect:NSMakeRect(0, 0, width, height) fromRect:NSZeroRect operation:
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_12_AND_LATER)
                NSCompositingOperationSourceOver
#else
                NSCompositeSourceOver
#endif
                fraction:1];
        [NSGraphicsContext restoreGraphicsState];
        result = QImageFromCGImageRef(CFTypePtrFromGet([bmp CGImage]));
        [bmp release];
    }

    return result;
}

NSImage *QImageToNSImage(const QImage &image, const QSize &sizeInPoints)
{
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER)
    if(InfoUtils::MacVersionGreatOrEqual(10, 6))
    {
        const CFTypePtr<CGImageRef> imageRef = QImageToCGImageRef(image);
        if(!imageRef)
            return nil;
        return [[[NSImage alloc] initWithCGImage:imageRef size:QSizeFToNSSize(sizeInPoints.isValid() ? sizeInPoints : image.size())] autorelease];
    }
    else
#endif
    {
        // https://stackoverflow.com/questions/10627557/mac-os-x-drawing-into-an-offscreen-nsgraphicscontext-using-cgcontextref-c-funct
        // http://www.cocoabuilder.com/archive/cocoa/133768-converting-cgimagerefs-to-nsimages-how.html
        const CFTypePtr<CGImageRef> imageRef = QImageToCGImageRef(image);
        if(!imageRef)
            return nil;
        const NSRect rect = QRectFToNSRect(image.rect());
        NSInteger width = static_cast<NSInteger>(rect.size.width);
        NSInteger height = static_cast<NSInteger>(rect.size.height);
        NSBitmapImageRep *bmp = [[NSBitmapImageRep alloc]
                initWithBitmapDataPlanes:nil
                              pixelsWide:width
                              pixelsHigh:height
                           bitsPerSample:8
                         samplesPerPixel:4
                                hasAlpha:YES
                                isPlanar:NO
                          colorSpaceName:NSCalibratedRGBColorSpace
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_14_AND_LATER)
                            bitmapFormat:NSBitmapFormatAlphaFirst
#else
                            bitmapFormat:NSAlphaFirstBitmapFormat
#endif
                             bytesPerRow:0
                            bitsPerPixel:0
        ];
        if(!bmp)
            return nil;
        [NSGraphicsContext saveGraphicsState];
        NSGraphicsContext *context = [NSGraphicsContext graphicsContextWithBitmapImageRep:bmp];
        [NSGraphicsContext setCurrentContext:context];
#include "Workarounds/BeginIgnoreDeprecated.h"
        CGContextDrawImage(reinterpret_cast<CGContextRef>([context graphicsPort]), *reinterpret_cast<const CGRect*>(&rect), imageRef);
#include "Workarounds/EndIgnoreDeprecated.h"
        [NSGraphicsContext restoreGraphicsState];
        NSImage *result = [[NSImage alloc] initWithSize:rect.size];
        [result addRepresentation:bmp];
        [bmp release];
        return [result autorelease];
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

