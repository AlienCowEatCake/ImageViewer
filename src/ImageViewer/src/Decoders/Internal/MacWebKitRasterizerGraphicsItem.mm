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

#include <cmath>
#include <algorithm>

#include <QtGlobal>
#include <QFileInfo>
#include <QUrl>
#include <QPixmap>
#include <QPainter>
#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>
#include <QWidget>
#include <QPolygonF>
#include <QDebug>

#include "MacImageUtils.h"

//#define MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_DEBUG

// ====================================================================================================

namespace {

const qreal MAX_PIXMAP_DIMENSION = 16384;

QRectF QRectFIntegerized(const QRectF rect)
{
    const qreal left = std::floor(rect.left());
    const qreal top = std::floor(rect.top());
    const qreal width = std::ceil(rect.width() + std::abs(rect.left() - left));
    const qreal height = std::ceil(rect.height() + std::abs(rect.top() - top));
    return QRectF(left, top, width, height);
}

QRectF QRectFFromNSRect(const NSRect &rect)
{
    return QRectF(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
}

NSRect QRectFToNSRect(const QRectF &rect)
{
    return NSMakeRect(rect.x(), rect.y(), rect.width(), rect.height());
}

QRectF DOMNodeActualBoundingBox(DOMNode *node)
{
    QRectF result;
    DOMNodeList *childNodes = [node childNodes];
    for(unsigned int i = 0; i < [childNodes length]; i++)
    {
        DOMNode *childNode = [childNodes item: i];
        QRectF childRect = QRectFFromNSRect([childNode boundingBox]);
        if(!childRect.isValid())
            childRect = DOMNodeActualBoundingBox(childNode);
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
    struct RasterizerCache
    {
        QPixmap pixmap;
        qreal scaleFactor;
    };

    Impl(const QUrl &url);
    Impl(const QString &htmlString);
    ~Impl();

    MacWebKitRasterizerGraphicsItem::State state() const;
    void setState(MacWebKitRasterizerGraphicsItem::State state);

    void waitForLoad() const;
    QPixmap grapPixmap(qreal scaleFactor = 1);

    QRectF rect() const;
    void setRect(const QRectF &rect);

    qreal maxScaleFactor() const;
    void setMaxScaleFactor(qreal maxScaleFactor);

    RasterizerCache &rasterizerCache();
    const RasterizerCache &rasterizerCache() const;

private:
    void init();

    WebView *m_view;
    QWidget *m_container;
    MacWebKitRasterizerViewDelegate *m_delegate;
    MacWebKitRasterizerGraphicsItem::State m_state;
    QRectF m_rect;
    qreal m_maxScaleFactor;
    RasterizerCache m_rasterizerCache;
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
    [m_view setFrameLoadDelegate    : (id <WebFrameLoadDelegate>)   nil];
    [m_view setPolicyDelegate       : (id <WebPolicyDelegate>)      nil];
    [m_view setUIDelegate           : (id <WebUIDelegate>)          nil];
    [m_view setEditingDelegate      : (id <WebEditingDelegate>)     nil];
    [m_delegate release];
    [m_view removeFromSuperview];
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

QPixmap MacWebKitRasterizerGraphicsItem::Impl::grapPixmap(qreal scaleFactor)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    const QString zoomScript = QString::fromLatin1("document.documentElement.style.zoom = '%1'");
    const QRectF scaledRect = QRectF(m_rect.topLeft() * scaleFactor, m_rect.size() * scaleFactor);
    const QSizeF scaledPageSize = scaledRect.united(QRectF(0, 0, 1, 1)).size();
    const double oldScaleFactor = QString::fromNSString([m_view stringByEvaluatingJavaScriptFromString: @"document.documentElement.style.zoom;"]).toDouble();
    if(oldScaleFactor > scaleFactor)
    {
        [m_view stringByEvaluatingJavaScriptFromString: zoomScript.arg(scaleFactor).toNSString()];
        [m_view setFrameSize: NSMakeSize(scaledPageSize.width(), scaledPageSize.height())];
    }
    else
    {
        [m_view setFrameSize: NSMakeSize(scaledPageSize.width(), scaledPageSize.height())];
        [m_view stringByEvaluatingJavaScriptFromString: zoomScript.arg(scaleFactor).toNSString()];
    }

    NSView *webFrameViewDocView = [[[m_view mainFrame] frameView] documentView];
    const NSRect cacheRect = QRectFToNSRect(QRectFIntegerized(scaledRect));
    const NSInteger one = static_cast<NSInteger>(1);
    NSBitmapImageRep *bitmapRep = [[NSBitmapImageRep alloc]
            initWithBitmapDataPlanes: nil
                          pixelsWide: std::max(static_cast<NSInteger>(cacheRect.size.width), one)
                          pixelsHigh: std::max(static_cast<NSInteger>(cacheRect.size.height), one)
                       bitsPerSample: 8
                     samplesPerPixel: 4
                            hasAlpha: YES
                            isPlanar: NO
                      colorSpaceName: NSCalibratedRGBColorSpace
                        bitmapFormat: 0
                         bytesPerRow: 4 * std::max(static_cast<NSInteger>(cacheRect.size.width), one)
                        bitsPerPixel: 32];

    [NSGraphicsContext saveGraphicsState];
    NSGraphicsContext *graphicsContext = [NSGraphicsContext graphicsContextWithBitmapImageRep: bitmapRep];
    [NSGraphicsContext setCurrentContext: graphicsContext];
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
#if defined (MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_DEBUG)
    qDebug() << __FUNCTION__ << "rect:" << rect;
#endif
    m_rect = rect;
}

qreal MacWebKitRasterizerGraphicsItem::Impl::maxScaleFactor() const
{
    return m_maxScaleFactor;
}

void MacWebKitRasterizerGraphicsItem::Impl::setMaxScaleFactor(qreal maxScaleFactor)
{
    m_maxScaleFactor = maxScaleFactor;
}

MacWebKitRasterizerGraphicsItem::Impl::RasterizerCache &MacWebKitRasterizerGraphicsItem::Impl::rasterizerCache()
{
    return m_rasterizerCache;
}

const MacWebKitRasterizerGraphicsItem::Impl::RasterizerCache &MacWebKitRasterizerGraphicsItem::Impl::rasterizerCache() const
{
    return m_rasterizerCache;
}

void MacWebKitRasterizerGraphicsItem::Impl::init()
{
    m_view = [[WebView alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
    m_container = new QWidget;
    m_delegate = [[MacWebKitRasterizerViewDelegate alloc] initWithImpl: this];
    m_state = MacWebKitRasterizerGraphicsItem::STATE_LOADING;
    m_rasterizerCache.scaleFactor = 0;
    m_maxScaleFactor = 1;

    [m_view setFrameLoadDelegate    : (id <WebFrameLoadDelegate>)   m_delegate];
    [m_view setPolicyDelegate       : (id <WebPolicyDelegate>)      m_delegate];
    [m_view setUIDelegate           : (id <WebUIDelegate>)          m_delegate];
    [m_view setEditingDelegate      : (id <WebEditingDelegate>)     m_delegate];

    [reinterpret_cast<NSView*>(m_container->winId()) addSubview: m_view];
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
#if defined (MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_DEBUG)
    qDebug() << __FUNCTION__;
#endif
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    if(frame == [view mainFrame])
    {
        QRectF actualRect;
        if(QString::fromNSString([view stringByEvaluatingJavaScriptFromString: @"document.documentElement instanceof SVGElement;"]) == QString::fromLatin1("true"))
        {
            const NSString *viewBoxStr = [view stringByEvaluatingJavaScriptFromString: @"document.querySelector('svg').getAttribute('viewBox');"];
            const QStringList vb = QString::fromNSString(viewBoxStr).split(QRegExp(QString::fromLatin1("\\s")));
            const QRectF viewBox = (vb.size() == 4 ? QRectF(vb.at(0).toDouble(), vb.at(1).toDouble(), vb.at(2).toDouble(), vb.at(3).toDouble()) : QRectF());

            const NSString *heightStr  = [view stringByEvaluatingJavaScriptFromString: @"document.querySelector('svg').getAttribute('height');"];
            const NSString *widthStr   = [view stringByEvaluatingJavaScriptFromString: @"document.querySelector('svg').getAttribute('width');"];
            const QSizeF size = QSizeF(static_cast<qreal>([widthStr doubleValue]), static_cast<qreal>([heightStr doubleValue]));

            if(!size.isEmpty())
                actualRect = QRectF(0, 0, size.width(), size.height());
            else if(viewBox.isValid())
                /// @note WebKit рендерит с viewBox с учетом смещения, нужен только размер
                actualRect = QRectF(0, 0, viewBox.width(), viewBox.height());
            else
                actualRect = DOMNodeActualBoundingBox([view mainFrameDocument]);
            if(!actualRect.isValid())
                actualRect = QRectFFromNSRect([[[frame frameView] documentView] bounds]);

#if defined (MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_DEBUG)
            qDebug() << "***** ----------------------------------------";
            qDebug() << "***** Detected SVG document";
            qDebug() << "***** ----------------------------------------";
            qDebug() << "***** viewBox:" << viewBox;
            qDebug() << "***** size:" << size;
            qDebug() << "***** DOM boundingBox:" << DOMNodeActualBoundingBox([view mainFrameDocument]);
            qDebug() << "***** documentView bounds:" << QRectFFromNSRect([[[frame frameView] documentView] bounds]);
            qDebug() << "***** ----------------------------------------";
#endif
        }
        else
        {
            /// @todo Тут, возможно, следует сделать более гибкий алгоритм
            actualRect = QRectFFromNSRect([[[frame frameView] documentView] bounds]);
            actualRect = actualRect.united(QRectF(0, 0, 640, 1));

#if defined (MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_DEBUG)
            qDebug() << "***** ----------------------------------------";
            qDebug() << "***** Detected HTML document";
            qDebug() << "***** ----------------------------------------";
            qDebug() << "***** DOM boundingBox:" << DOMNodeActualBoundingBox([view mainFrameDocument]);
            qDebug() << "***** documentView bounds:" << QRectFFromNSRect([[[frame frameView] documentView] bounds]);
            qDebug() << "***** ----------------------------------------";
#endif
        }
        m_impl->setRect(actualRect);
        m_impl->setMaxScaleFactor(std::min(MAX_PIXMAP_DIMENSION / actualRect.width(), MAX_PIXMAP_DIMENSION / actualRect.height()));
        m_impl->setState(MacWebKitRasterizerGraphicsItem::STATE_SUCCEED);
    }
    [pool release];
}

- (void)                   webView: (WebView*)  view
   didFailProvisionalLoadWithError: (NSError*)  error
                          forFrame: (WebFrame*) frame
{
#if defined (MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_DEBUG)
    qDebug() << __FUNCTION__;
#endif
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
    Q_UNUSED(option);
    Q_UNUSED(widget);
#if defined (MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_DEBUG)
    painter->setBrush(Qt::transparent);
    painter->setPen(Qt::red);
    painter->drawRect(boundingRect());
#endif
    QPixmap &pixmap = m_impl->rasterizerCache().pixmap;
    const QRectF identityRect = QRectF(0, 0, 1, 1);
    const QRectF deviceTransformedRect = painter->deviceTransform().mapRect(identityRect);
    const qreal newScaleFactor = std::min(std::max(deviceTransformedRect.width(), deviceTransformedRect.height()), m_impl->maxScaleFactor());
    qreal &scaleFactor = m_impl->rasterizerCache().scaleFactor;
    if(std::abs(newScaleFactor - scaleFactor) / std::max(newScaleFactor, scaleFactor) > 1e-2)
    {
        scaleFactor = newScaleFactor;
        pixmap = m_impl->grapPixmap(newScaleFactor);
    }
    painter->drawPixmap(boundingRect(), pixmap, QRectF(pixmap.rect()));
}

MacWebKitRasterizerGraphicsItem::State MacWebKitRasterizerGraphicsItem::state() const
{
    return m_impl->state();
}

// ====================================================================================================

