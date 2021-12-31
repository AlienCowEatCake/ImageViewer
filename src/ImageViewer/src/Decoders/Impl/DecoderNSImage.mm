/*
   Copyright (C) 2017-2021 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#include <set>

#include <QtGlobal>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QRegularExpression>
#else
#include <QRegExp>
#endif
#include <QPixmap>
#include <QImage>
#include <QFileInfo>
#include <QDebug>

#include "Utils/Global.h"
#include "Utils/InfoUtils.h"
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
        AUTORELEASE_POOL;
        std::set<QString> fileTypes;
        if([NSImage respondsToSelector:@selector(imageFileTypes)])
        {
            for(NSString *fileType in [NSImage performSelector:@selector(imageFileTypes)])
            {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                typedef QRegularExpression QRE;
#else
                typedef QRegExp QRE;
#endif
                QString simplifiedFileType = ObjCUtils::QStringFromNSString(fileType).toLower();
                simplifiedFileType.replace(QRE(QString::fromLatin1("[^\\w]")), QString::fromLatin1(""));
                fileTypes.insert(simplifiedFileType.simplified());
            }
        }
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_10_AND_LATER)
        if(InfoUtils::MacVersionGreatOrEqual(10, 10))
        {
            for(NSString *uti in [NSImage imageTypes])
            {
                if(!uti)
                    continue;
                CFArrayRef tags = UTTypeCopyAllTagsWithClass((__bridge CFStringRef)uti, kUTTagClassFilenameExtension);
                if(!tags)
                    continue;
                for(CFIndex i = 0, count = CFArrayGetCount(tags); i < count; ++i)
                {
                    CFStringRef tag = (CFStringRef)CFArrayGetValueAtIndex(tags, i);
                    fileTypes.insert(ObjCUtils::QStringFromNSString((__bridge NSString*)tag).toLower());
                }
                CFRelease(tags);
            }
        }
#endif
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
        NSImage *picture = [[[NSImage alloc] initWithContentsOfFile: ObjCUtils::QStringToNSString(filePath)] autorelease];
        if(picture == nil)
            return QSharedPointer<IImageData>();
        QGraphicsItem *result = GraphicsItemsFactory::instance().createPixmapItem(ObjCUtils::QPixmapFromNSImage(picture));
        IImageMetaData *metaData = result ? ImageMetaData::createMetaData(filePath) : Q_NULLPTR;
        return QSharedPointer<IImageData>(new ImageData(result, filePath, name(), metaData));
    }
};

DecoderAutoRegistrator registrator(new DecoderNSImage, true);

} // namespace
