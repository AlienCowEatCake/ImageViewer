/*
   Copyright (C) 2017-2025 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "MacUtils_mac.h"

#include <cstdlib>
#include <cstring>

#include <AvailabilityMacros.h>

#include <QByteArray>
#include <QImage>
#include <QPixmap>
#include <QString>

#include "Global.h"
#include "InfoUtils.h"

namespace MacUtils {

static void imageDeleter(void *image, const void *, std::size_t)
{
    delete static_cast<QImage*>(image);
}

static CFTypePtr<CGColorSpaceRef> CreateRgbColorSpace()
{
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER) && defined (MAC_OS_X_VERSION_MAX_ALLOWED) && (MAC_OS_X_VERSION_MAX_ALLOWED >= 1050)
#if defined (MAC_OS_X_VERSION_MIN_REQUIRED) && (MAC_OS_X_VERSION_MIN_REQUIRED >= 1050)
    return CFTypePtrFromCreate(CGColorSpaceCreateWithName(kCGColorSpaceSRGB));
#else
    if(InfoUtils::MacVersionGreatOrEqual(10, 5))
        return CFTypePtrFromCreate(CGColorSpaceCreateWithName(kCGColorSpaceSRGB));
    else
        return CFTypePtrFromCreate(CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB));
#endif
#else
    return CFTypePtrFromCreate(CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB));
#endif
}

QPixmap QPixmapFromCGImage(const CFTypePtr<CGImageRef> &image, const QSize &sizeInPixels)
{
    if(!image)
        return QPixmap();

    return QPixmap::fromImage(QImageFromCGImage(image, sizeInPixels));
}

CFTypePtr<CGImageRef> QPixmapToCGImage(const QPixmap &pixmap)
{
    if(pixmap.isNull())
        return Q_NULLPTR;

    return QImageToCGImage(pixmap.toImage());
}

QImage QImageFromCGImage(const CFTypePtr<CGImageRef> &image, const QSize &sizeInPixels)
{
    if(!image)
        return QImage();

    const std::size_t width = sizeInPixels.isEmpty() ? CGImageGetWidth(image) : static_cast<std::size_t>(sizeInPixels.width());
    const std::size_t height = sizeInPixels.isEmpty() ? CGImageGetHeight(image) : static_cast<std::size_t>(sizeInPixels.height());
    QImage result(static_cast<int>(width), static_cast<int>(height), QImage::Format_ARGB32_Premultiplied);
    if(result.isNull())
        return QImage();

    result.fill(Qt::transparent);

    const CFTypePtr<CGColorSpaceRef> colorSpace = CreateRgbColorSpace();
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

CFTypePtr<CGImageRef> QImageToCGImage(const QImage &image)
{
    if(image.isNull())
        return Q_NULLPTR;

    const CFTypePtr<CGColorSpaceRef> colorSpace = CreateRgbColorSpace();
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

QString QStringFromCFString(const CFTypePtr<CFStringRef> &string)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    return QString::fromCFString(string.get());
#else
    const CFIndex strLength = CFStringGetLength(string);
    CFIndex byteLength = strLength * 6;
    CFStringGetBytes(string, CFRangeMake(0, strLength), kCFStringEncodingUTF8, 0, false, Q_NULLPTR, byteLength, &byteLength);
    QByteArray buffer(static_cast<int>(byteLength) + 1, '\0');
    CFStringGetBytes(string, CFRangeMake(0, strLength), kCFStringEncodingUTF8, 0, false, reinterpret_cast<UInt8*>(buffer.data()), buffer.size(), Q_NULLPTR);
    return QString::fromUtf8(buffer.constData());
#endif
}

CFTypePtr<CFStringRef> QStringToCFString(const QString &string)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    return CFTypePtrFromCreate(string.toCFString());
#else
    const QByteArray buffer = string.toUtf8();
    return CFTypePtrFromCreate(CFStringCreateWithCString(kCFAllocatorDefault, buffer.constData(), kCFStringEncodingUTF8));
#endif
}

} // namespace MacUtils

