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

#include "DecoderMacWebKit.h"

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>

#include <QtGlobal>
#include <QGraphicsProxyWidget>
#include <QFileInfo>
#include <QUrl>
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
#include <QMacCocoaViewContainer>
#endif
#include <QPixmap>
#include <QLabel>

#include <QDebug>

#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/MacImageUtils.h"

//#if defined (QT_DEBUG)
//#define DECODER_MAC_WEBKIT_PRIORITY -1
//#else
#define DECODER_MAC_WEBKIT_PRIORITY 200
//#endif

namespace {

DecoderAutoRegistrator registrator(new DecoderMacWebKit);

class MacWebKitWidget;

} // namespace

@interface MacWebKitWidgetViewDelegate : NSObject
-(id) initWithWidget: (MacWebKitWidget*) widget;
@end

namespace {

class MacWebKitWidget : public QLabel
{
public:
    MacWebKitWidget(const QString &filePath, QWidget *parent = NULL)
        : QLabel(parent)
        , m_view([[WebView alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)])
        , m_delegate([[MacWebKitWidgetViewDelegate alloc] initWithWidget: this])
        , m_loading(true)
    {
        setDelegate(m_delegate);
        [m_view setDrawsBackground:NO];
        NSURLRequest *request = [NSURLRequest requestWithURL: QUrl::fromLocalFile(QFileInfo(filePath).absoluteFilePath()).toNSURL()];
        [[m_view mainFrame] loadRequest: request];

        QMacCocoaViewContainer *container = new QMacCocoaViewContainer(m_view);
//        container->setWindowFlags(Qt::Window);
//        container->show();

        while(m_loading)
            [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
    }

    ~MacWebKitWidget()
    {
        setDelegate(nil);
        [m_delegate release];
        [m_view release];
    }

    void setLoading(bool loading)
    {
        m_loading = loading;
    }

private:
    void setDelegate(MacWebKitWidgetViewDelegate *delegate)
    {
        [m_view setFrameLoadDelegate:(id <WebFrameLoadDelegate>)delegate];
        [m_view setPolicyDelegate:(id <WebPolicyDelegate>)delegate];
        [m_view setUIDelegate:(id <WebUIDelegate>)delegate];
        [m_view setEditingDelegate:(id <WebEditingDelegate>)delegate];
    }

    WebView *m_view;
    MacWebKitWidgetViewDelegate *m_delegate;
    bool m_loading;
};

} // namespace

@implementation MacWebKitWidgetViewDelegate
{
    MacWebKitWidget *m_widget;
}

-(id) initWithWidget: (MacWebKitWidget*) widget
{
    self = [super init];
    if(self)
        m_widget = widget;
    return self;
}

-(void)                    webView: (WebView *)sender
             didFinishLoadForFrame: (WebFrame *)frame
{
    qDebug() << __PRETTY_FUNCTION__;

    if(frame != [sender mainFrame])
        return;

    NSView *webFrameViewDocView = [[[sender mainFrame] frameView] documentView];
    NSRect cacheRect = [webFrameViewDocView bounds];

//    NSBitmapImageRep *bitmapRep = [webFrameViewDocView bitmapImageRepForCachingDisplayInRect:cacheRect];
//    [webFrameViewDocView cacheDisplayInRect:cacheRect toBitmapImageRep:bitmapRep];

//    NSSize imgSize = cacheRect.size;
//    if (imgSize.height > imgSize.width)
//    {
//        imgSize.height = imgSize.width;
//    }

//    NSRect srcRect = NSZeroRect;
//    srcRect.size = imgSize;
//    srcRect.origin.y = cacheRect.size.height - imgSize.height;

//    NSRect destRect = NSZeroRect;
//    destRect.size = imgSize;

//    NSImage *webImage = [[[NSImage alloc] initWithSize:imgSize] autorelease];
//    [webImage lockFocus];
//    [bitmapRep drawInRect:destRect
//                          fromRect:srcRect
//                          operation:NSCompositeCopy
//                          fraction:1.0
//                          respectFlipped:YES
//                          hints:nil];
//    [webImage unlockFocus];

//    NSSize defaultDisplaySize;
//    defaultDisplaySize.height = 64.0 * (imgSize.height / imgSize.width);
//    defaultDisplaySize.width = 64.0;
//    [webImage setSize:defaultDisplaySize];

    NSBitmapImageRep *bitmapRep;

    bitmapRep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:nil
                                                        pixelsWide:cacheRect.size.width
                                                        pixelsHigh:cacheRect.size.height
                                                     bitsPerSample:8
                                                   samplesPerPixel:4
                                                          hasAlpha:YES
                                                          isPlanar:NO
                                                    colorSpaceName:NSCalibratedRGBColorSpace
                                                      bitmapFormat:0
                                                       bytesPerRow:(4 * cacheRect.size.width)
                                                      bitsPerPixel:32];

    [NSGraphicsContext saveGraphicsState];

    NSGraphicsContext *graphicsContext = [NSGraphicsContext graphicsContextWithBitmapImageRep:bitmapRep];
    [NSGraphicsContext setCurrentContext:graphicsContext];
//    CGContextScaleCTM(graphicsContext.graphicsPort, scale, scale);

    [webFrameViewDocView displayRectIgnoringOpacity: cacheRect inContext:graphicsContext];

    [NSGraphicsContext restoreGraphicsState];

    NSImage *webImage = [[NSImage alloc] initWithSize:bitmapRep.size];
    [webImage addRepresentation:bitmapRep];

    m_widget->setPixmap(MacImageUtils::QPixmapFromNSImage(webImage));
    m_widget->setLoading(false);
}

- (void)                   webView: (WebView *)sender
   didFailProvisionalLoadWithError: (NSError *)error
                          forFrame: (WebFrame *)frame
{
    m_widget->setLoading(false);
    qDebug() << __PRETTY_FUNCTION__;
}

@end

QString DecoderMacWebKit::name() const
{
    return QString::fromLatin1("DecoderMacWebKit");
}

QList<DecoderFormatInfo> DecoderMacWebKit::supportedFormats() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
    const QList<QByteArray> svgFormats = QList<QByteArray>()
            << "svg"
            << "svgz";
    QList<DecoderFormatInfo> result;
    for(QList<QByteArray>::ConstIterator it = svgFormats.constBegin(); it != svgFormats.constEnd(); ++it)
    {
        DecoderFormatInfo info;
        info.decoderPriority = DECODER_MAC_WEBKIT_PRIORITY;
        info.format = QString::fromLatin1(*it).toLower();
        result.append(info);
    }
    return result;
#else
    return QList<DecoderFormatInfo>();
#endif
}

QGraphicsItem *DecoderMacWebKit::loadImage(const QString &filePath)
{
    const QFileInfo fileInfo(filePath);
    if(!fileInfo.exists() || !fileInfo.isReadable())
        return NULL;

    QGraphicsProxyWidget *result = NULL;

//    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    MacWebKitWidget *container = new MacWebKitWidget(filePath);
    container->resize(1000, 1000);
    container->setAttribute(Qt::WA_NoSystemBackground, true);

    result = new QGraphicsProxyWidget();
    result->setWidget(container);

//    [pool release];

    return result;
}
