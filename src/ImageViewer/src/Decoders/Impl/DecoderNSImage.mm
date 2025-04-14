/*
   Copyright (C) 2017-2025 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#import <CoreFoundation/CoreFoundation.h>
#import <CoreServices/CoreServices.h>
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#include <set>

#include <QtGlobal>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QRegularExpression>
#else
#include <QRegExp>
#endif
#include <QImage>
#include <QFileInfo>

#include "Utils/Global.h"
#include "Utils/InfoUtils.h"
#include "Utils/Logging.h"
#include "Utils/MacUtils.h"
#include "Utils/ObjectiveCUtils.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"

namespace {

class DecoderNSImage : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderNSImage");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        std::set<QString> fileTypes;
        fillTypesFor1100(fileTypes) || fillTypesFor1010(fileTypes) || fillTypesFor1000(fileTypes);
        QStringList result;
        for(std::set<QString>::const_iterator it = fileTypes.begin(); it != fileTypes.end(); ++it)
            result.append(*it);
        return result;
    }

    QStringList advancedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList();
    }

    bool isAvailable() const Q_DECL_OVERRIDE
    {
        return true;
    }

    QSharedPointer<IImageData> loadImage(const QString &filePath) Q_DECL_OVERRIDE
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return QSharedPointer<IImageData>();

        AUTORELEASE_POOL;
        @try
        {
            NSImage *picture = [[[NSImage alloc] initWithContentsOfFile: ObjCUtils::QStringToNSString(filePath)] autorelease];
            if(picture == nil)
                return QSharedPointer<IImageData>();
            QGraphicsItem *result = GraphicsItemsFactory::instance().createImageItem(ObjCUtils::QImageFromNSImage(picture));
            IImageMetaData *metaData = result ? ImageMetaData::createMetaData(filePath) : Q_NULLPTR;
            return QSharedPointer<IImageData>(new ImageData(result, filePath, name(), metaData));
        }
        @catch(NSException *exception)
        {
            LOG_WARNING() << LOGGING_CTX << ObjCUtils::QStringFromNSString([exception reason]);
        }
        return QSharedPointer<IImageData>();
    }

private:

    /// @note macOS 11.0+
    static bool fillTypesFor1100(std::set<QString>& fileTypes)
    {
        if(InfoUtils::MacVersionGreatOrEqual(11, 0))
        {
            CFBundleRef utiBundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.UniformTypeIdentifiers"));
            if(!utiBundle)
                return false;
            void *UTType_d = CFBundleGetDataPointerForName(utiBundle, CFSTR("OBJC_CLASS_$_UTType"));
            if(!UTType_d)
                return false;
            void *UTTagClassFilenameExtension_d = CFBundleGetDataPointerForName(utiBundle, CFSTR("UTTagClassFilenameExtension"));
            if(!UTTagClassFilenameExtension_d)
                return false;
            Class UTType = reinterpret_cast<Class>(UTType_d);
            const NSString *UTTagClassFilenameExtension = *reinterpret_cast<const NSString**>(UTTagClassFilenameExtension_d);

            AUTORELEASE_POOL;
            @try
            {
                if(![NSImage respondsToSelector:@selector(imageTypes)])
                    return false;
                NSArray *imageTypes = (NSArray*)[NSImage performSelector:@selector(imageTypes)];
                NSEnumerator *enumerator = [imageTypes objectEnumerator];
                NSString *uti = nil;
                while((uti = [enumerator nextObject]) != nil)
                {
                    if(!uti)
                        continue;
                    if(![UTType respondsToSelector:@selector(typeWithIdentifier:)])
                        return false;
                    NSObject *type = (NSObject*)[UTType performSelector:@selector(typeWithIdentifier:) withObject:uti];
                    if(!type)
                        continue;
                    if(![type respondsToSelector:@selector(tags)])
                        return false;
                    NSDictionary *tags = (NSDictionary*)[type performSelector:@selector(tags)];
                    if(!tags)
                        continue;
                    NSArray *exts = [tags objectForKey:UTTagClassFilenameExtension];
                    if(!exts)
                        continue;
                    NSEnumerator *extsEnumerator = [exts objectEnumerator];
                    NSString *ext = nil;
                    while((ext = [extsEnumerator nextObject]) != nil)
                        fileTypes.insert(ObjCUtils::QStringFromNSString(ext).toLower());
                }
                return true;
            }
            @catch(NSException *exception)
            {
                LOG_WARNING() << LOGGING_CTX << ObjCUtils::QStringFromNSString([exception reason]);
            }
        }
        return false;
    }

    /// @note macOS 10.10-12.0
    static bool fillTypesFor1010(std::set<QString>& fileTypes)
    {
        if(InfoUtils::MacVersionGreatOrEqual(10, 10))
        {
            CFBundleRef appKitBundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.AppKit"));
            if(!appKitBundle)
                return false;
            void *UTTypeCopyAllTagsWithClass_f = CFBundleGetFunctionPointerForName(appKitBundle, CFSTR("UTTypeCopyAllTagsWithClass"));
            if(!UTTypeCopyAllTagsWithClass_f)
                return false;
            void *kUTTagClassFilenameExtension_d = CFBundleGetDataPointerForName(appKitBundle, CFSTR("kUTTagClassFilenameExtension"));
            if(!kUTTagClassFilenameExtension_d)
                return false;
            typedef CFArrayRef (*UTTypeCopyAllTagsWithClass_t)(CFStringRef inUTI, CFStringRef inTagClass);
            UTTypeCopyAllTagsWithClass_t UTTypeCopyAllTagsWithClass = reinterpret_cast<UTTypeCopyAllTagsWithClass_t>(UTTypeCopyAllTagsWithClass_f);
            const CFStringRef kUTTagClassFilenameExtension = *reinterpret_cast<const CFStringRef*>(kUTTagClassFilenameExtension_d);

            AUTORELEASE_POOL;
            @try
            {
                if(![NSImage respondsToSelector:@selector(imageTypes)])
                    return false;
                NSArray *imageTypes = (NSArray*)[NSImage performSelector:@selector(imageTypes)];
                NSEnumerator *enumerator = [imageTypes objectEnumerator];
                NSString *uti = nil;
                while((uti = [enumerator nextObject]) != nil)
                {
                    if(!uti)
                        continue;
                    CFArrayRef tags = UTTypeCopyAllTagsWithClass((CFStringRef)uti, kUTTagClassFilenameExtension);
                    if(!tags)
                        continue;
                    for(CFIndex i = 0, count = CFArrayGetCount(tags); i < count; ++i)
                    {
                        CFStringRef tag = (CFStringRef)CFArrayGetValueAtIndex(tags, i);
                        fileTypes.insert(MacUtils::QStringFromCFString(CFTypePtrFromGet(tag)).toLower());
                    }
                    CFRelease(tags);
                }
                return true;
            }
            @catch(NSException *exception)
            {
                LOG_WARNING() << LOGGING_CTX << ObjCUtils::QStringFromNSString([exception reason]);
            }
        }
        return false;
    }

    /// @note macOS 10.0-10.10
    static bool fillTypesFor1000(std::set<QString>& fileTypes)
    {
        AUTORELEASE_POOL;
        if([NSImage respondsToSelector:@selector(imageFileTypes)])
        {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            typedef QRegularExpression QRE;
#else
            typedef QRegExp QRE;
#endif
            const QRE nonWordRE = QRE(QString::fromLatin1("[^\\w]"));
            @try
            {
                NSArray *imageFileTypes = [NSImage performSelector:@selector(imageFileTypes)];
                NSEnumerator *enumerator = [imageFileTypes objectEnumerator];
                NSString *fileType = nil;
                while((fileType = [enumerator nextObject]) != nil)
                {
                    QString simplifiedFileType = ObjCUtils::QStringFromNSString(fileType).toLower();
                    simplifiedFileType.replace(nonWordRE, QString::fromLatin1(""));
                    fileTypes.insert(simplifiedFileType.simplified());
                }
                return true;
            }
            @catch(NSException *exception)
            {
                LOG_WARNING() << LOGGING_CTX << ObjCUtils::QStringFromNSString([exception reason]);
            }
        }
        return false;
    }
};

DecoderAutoRegistrator registrator(new DecoderNSImage, true);

} // namespace
