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

#include "MacWebKitRasterizerGraphicsItem.h"

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>
#import <AppKit/AppKit.h>
#import <WebKit/WebKit.h>

#include <QtGlobal>
#include <QFileInfo>
#include <QUrl>
#include <QPixmap>
#include <QPainter>
#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QIODevice>
#include <QMacCocoaViewContainer> /// @todo Заменить на нативный контейнер
#include <QDebug>

#include "MacImageUtils.h"

// ====================================================================================================

namespace {

QRectF QRectFromNSRect(const NSRect &rect)
{
    return QRectF(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
}

QRectF getDomActualBoundingRect(DOMNode *node)
{
    QRectF result;
    DOMNodeList *childNodes = [node childNodes];
    for(unsigned int i = 0; i < [childNodes length]; i++)
    {
        DOMNode *childNode = [childNodes item: i];
        QRectF childRect = getDomActualBoundingRect(childNode);
        if(!childRect.isValid())
            childRect = QRectFromNSRect([childNode boundingBox]);

        qDebug() << __FUNCTION__ << QString::fromNSString([childNode localName]) << childRect;

        if(!childRect.isValid())
            continue;
        if(!result.isValid())
            result = childRect;
        else
            result = result.united(childRect);
    }
    return result;
}

} // namespace

// ====================================================================================================

@interface MacWebKitRasterizerViewDelegate : NSObject
-(id) initWithImpl: (MacWebKitRasterizerGraphicsItem::Impl*) impl;
@end

// ====================================================================================================

class MacWebKitRasterizerGraphicsItem::Impl
{
public:
    Impl(const QUrl &url);
    Impl(const QString &htmlString);
    ~Impl();

    MacWebKitRasterizerGraphicsItem::State state() const;
    void setState(MacWebKitRasterizerGraphicsItem::State state);

    void waitForLoad() const;
    QPixmap grapPixmap();

    QRectF rect() const;
    void setRect(const QRectF &rect);

private:
    void init();

    WebView *m_view;
    QMacCocoaViewContainer *m_container;
    MacWebKitRasterizerViewDelegate *m_delegate;
    MacWebKitRasterizerGraphicsItem::State m_state;
    QRectF m_rect;
};

// ====================================================================================================

MacWebKitRasterizerGraphicsItem::Impl::Impl(const QUrl &url)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    init();
    if(!url.isValid())
    {
        m_state = MacWebKitRasterizerGraphicsItem::STATE_FAILED;
    }
    else
    {
        NSURLRequest *request = [NSURLRequest requestWithURL: url.toNSURL()];
        [[m_view mainFrame] loadRequest: request];
        waitForLoad();
    }
    [pool release];
}

MacWebKitRasterizerGraphicsItem::Impl::Impl(const QString &htmlString)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    init();
    if(htmlString.isEmpty())
    {
        m_state = MacWebKitRasterizerGraphicsItem::STATE_FAILED;
    }
    else
    {
        [[m_view mainFrame] loadHTMLString: htmlString.toNSString() baseURL: nil];
        waitForLoad();
    }
    [pool release];
}

MacWebKitRasterizerGraphicsItem::Impl::~Impl()
{
    m_container->setCocoaView(nil);
    [m_view setFrameLoadDelegate    : (id <WebFrameLoadDelegate>)   nil];
    [m_view setPolicyDelegate       : (id <WebPolicyDelegate>)      nil];
    [m_view setUIDelegate           : (id <WebUIDelegate>)          nil];
    [m_view setEditingDelegate      : (id <WebEditingDelegate>)     nil];
    [m_delegate release];
    [m_view release];
    m_container->deleteLater();
}

MacWebKitRasterizerGraphicsItem::State MacWebKitRasterizerGraphicsItem::Impl::state() const
{
    return m_state;
}

void MacWebKitRasterizerGraphicsItem::Impl::setState(MacWebKitRasterizerGraphicsItem::State state)
{
    m_state = state;
}

void MacWebKitRasterizerGraphicsItem::Impl::waitForLoad() const
{
    while(m_state == MacWebKitRasterizerGraphicsItem::STATE_LOADING)
        [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
}

QPixmap MacWebKitRasterizerGraphicsItem::Impl::grapPixmap()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSView *webFrameViewDocView = [[[m_view mainFrame] frameView] documentView];
    NSRect cacheRect = [webFrameViewDocView bounds];
    NSBitmapImageRep *bitmapRep = [[NSBitmapImageRep alloc]
            initWithBitmapDataPlanes: nil
                          pixelsWide: static_cast<NSInteger>(cacheRect.size.width)
                          pixelsHigh: static_cast<NSInteger>(cacheRect.size.height)
                       bitsPerSample: 8
                     samplesPerPixel: 4
                            hasAlpha: YES
                            isPlanar: NO
                      colorSpaceName: NSCalibratedRGBColorSpace
                        bitmapFormat: 0
                         bytesPerRow: static_cast<NSInteger>(4 * cacheRect.size.width)
                        bitsPerPixel: 32];

    [NSGraphicsContext saveGraphicsState];
    NSGraphicsContext *graphicsContext = [NSGraphicsContext graphicsContextWithBitmapImageRep: bitmapRep];
    [NSGraphicsContext setCurrentContext: graphicsContext];
//    CGContextScaleCTM(graphicsContext.graphicsPort, scale, scale);
    [webFrameViewDocView displayRectIgnoringOpacity: cacheRect inContext: graphicsContext];
    [NSGraphicsContext restoreGraphicsState];

    NSImage *webImage = [[NSImage alloc] initWithSize: bitmapRep.size];
    [webImage addRepresentation: bitmapRep];
    QPixmap pixmap = MacImageUtils::QPixmapFromNSImage(webImage);
    [webImage release];
    [pool release];
    return pixmap;
}

QRectF MacWebKitRasterizerGraphicsItem::Impl::rect() const
{
    return m_rect;
}

void MacWebKitRasterizerGraphicsItem::Impl::setRect(const QRectF &rect)
{
    qDebug() << "rect:" << rect;
    m_rect = rect;
}

void MacWebKitRasterizerGraphicsItem::Impl::init()
{
    m_view = [[WebView alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
    m_container = new QMacCocoaViewContainer(m_view);
    m_delegate = [[MacWebKitRasterizerViewDelegate alloc] initWithImpl: this];
    m_state = MacWebKitRasterizerGraphicsItem::STATE_LOADING;

    [m_view setFrameLoadDelegate    : (id <WebFrameLoadDelegate>)   m_delegate];
    [m_view setPolicyDelegate       : (id <WebPolicyDelegate>)      m_delegate];
    [m_view setUIDelegate           : (id <WebUIDelegate>)          m_delegate];
    [m_view setEditingDelegate      : (id <WebEditingDelegate>)     m_delegate];

    [m_view setDrawsBackground: NO];
}

// ====================================================================================================

@implementation MacWebKitRasterizerViewDelegate
{
    MacWebKitRasterizerGraphicsItem::Impl *m_impl;
}

-(id)                 initWithImpl: (MacWebKitRasterizerGraphicsItem::Impl*) impl
{
    self = [super init];
    if(self)
        m_impl = impl;
    return self;
}

-(void)                    webView: (WebView*)  view
             didFinishLoadForFrame: (WebFrame*) frame
{
    qDebug() << __FUNCTION__;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    if(frame == [view mainFrame])
    {
        NSString *viewBoxStr = [view stringByEvaluatingJavaScriptFromString: @"document.querySelector('svg').getAttribute('viewBox');"];
        NSString *heightStr  = [view stringByEvaluatingJavaScriptFromString: @"document.querySelector('svg').getAttribute('height');"];
        NSString *widthStr   = [view stringByEvaluatingJavaScriptFromString: @"document.querySelector('svg').getAttribute('width');"];
        const QStringList vb = QString::fromNSString(viewBoxStr).split(QRegExp(QString::fromLatin1("\\s")));
        const QRectF viewBox = (vb.size() == 4 ? QRectF(vb.at(0).toDouble(), vb.at(1).toDouble(), vb.at(2).toDouble(), vb.at(3).toDouble()) : QRectF());
        const QSizeF size = QSizeF(static_cast<qreal>([widthStr doubleValue]), static_cast<qreal>([heightStr doubleValue]));

        QRectF actualRect;
        if(viewBox.isValid())
            actualRect = QRectF(0, 0, viewBox.width(), viewBox.height()); /// @note WebKit рендерит с учетом смещения, нужен только размер
        else if(!size.isEmpty())
            actualRect = QRectF(0, 0, size.width(), size.height());
        else
            actualRect = getDomActualBoundingRect([view mainFrameDocument]);
        if(!actualRect.isValid())
            actualRect = QRectFromNSRect([[[frame frameView] documentView] bounds]);
        m_impl->setRect(actualRect);

        m_impl->setState(MacWebKitRasterizerGraphicsItem::STATE_SUCCEED);
    }
    [pool release];
}

- (void)                   webView: (WebView*)  view
   didFailProvisionalLoadWithError: (NSError*)  error
                          forFrame: (WebFrame*) frame
{
    qDebug() << __FUNCTION__;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    if(frame == [view mainFrame])
    {
        qWarning() << QString::fromNSString([error description]);
        m_impl->setState(MacWebKitRasterizerGraphicsItem::STATE_FAILED);
    }
    [pool release];
}

@end

// ====================================================================================================

MacWebKitRasterizerGraphicsItem::MacWebKitRasterizerGraphicsItem(const QUrl &url, QGraphicsItem *parentItem)
    : QGraphicsObject(parentItem)
    , m_impl(new Impl(url))
{}

MacWebKitRasterizerGraphicsItem::MacWebKitRasterizerGraphicsItem(const QString &htmlString, QGraphicsItem *parentItem)
    : QGraphicsObject(parentItem)
    , m_impl(new Impl(htmlString))
{}

MacWebKitRasterizerGraphicsItem::~MacWebKitRasterizerGraphicsItem()
{}

QRectF MacWebKitRasterizerGraphicsItem::boundingRect() const
{
    return m_impl->rect();
}

void MacWebKitRasterizerGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->drawPixmap(boundingRect(), m_impl->grapPixmap(), boundingRect());
}

MacWebKitRasterizerGraphicsItem::State MacWebKitRasterizerGraphicsItem::state() const
{
    return m_impl->state();
}

// ====================================================================================================

