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

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>
#import <WebKit/WebKit.h>

#include "MacWebKitRasterizerGraphicsItem.h"

#include <cmath>
#include <algorithm>
#include <limits>

#include <QtGlobal>
#include <QFileInfo>
#include <QUrl>
#include <QImage>
#include <QPainter>
#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#include <QSysInfo>
#include <QTime>
#include <QDebug>

#include "MacImageUtils.h"

//#define MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_DEBUG

#if defined (MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_DEBUG)
#define DLOG qDebug() << QString::fromLatin1("[%1]").arg(QString::fromLatin1(__FUNCTION__)).toLatin1().data()
#define WHEREAMI qDebug() << QString::fromLatin1("[%1]").arg(QString::fromLatin1(__PRETTY_FUNCTION__)).toLatin1().data()
#endif

// ====================================================================================================

namespace {

const qreal MAX_IMAGE_DIMENSION = 16384;
const qreal MIN_IMAGE_DIMENSION = 1;
const qreal DEFAULT_WIDTH = 640;
const qreal DEFAULT_HEIGHT = 480;
const qreal DIMENSION_DELTA = 0.5;

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

QRectF WebFrameDocumentBounds(WebFrame *frame)
{
    return QRectFFromNSRect([[[frame frameView] documentView] bounds]);
}

QRectF SVGViewBoxAttribute(WebView *webView)
{
    const NSString *str = [webView stringByEvaluatingJavaScriptFromString: @"document.querySelector('svg').getAttribute('viewBox');"];
    const QStringList vb = QString::fromUtf8([str UTF8String]).split(QRegExp(QString::fromLatin1("\\s")));
    return (vb.size() == 4 ? QRectF(vb.at(0).toDouble(), vb.at(1).toDouble(), vb.at(2).toDouble(), vb.at(3).toDouble()) : QRectF());
}

QSizeF SVGSizeAttribute(WebView *webView)
{
    return QSizeF(
        static_cast<qreal>([[webView stringByEvaluatingJavaScriptFromString: @"document.querySelector('svg').getAttribute('width');"] doubleValue]),
        static_cast<qreal>([[webView stringByEvaluatingJavaScriptFromString: @"document.querySelector('svg').getAttribute('height');"] doubleValue])
    );
}

QRectF SVGBBox(WebView *webView)
{
    return QRectF(
        static_cast<qreal>([[webView stringByEvaluatingJavaScriptFromString: @"document.querySelector('svg').getBBox().x;"] doubleValue]),
        static_cast<qreal>([[webView stringByEvaluatingJavaScriptFromString: @"document.querySelector('svg').getBBox().y;"] doubleValue]),
        static_cast<qreal>([[webView stringByEvaluatingJavaScriptFromString: @"document.querySelector('svg').getBBox().width;"] doubleValue]),
        static_cast<qreal>([[webView stringByEvaluatingJavaScriptFromString: @"document.querySelector('svg').getBBox().height;"] doubleValue])
    );
}

QRectF SVGBoundingClientRect(WebView *webView)
{
    if(QSysInfo::MacintoshVersion >= QSysInfo::MV_10_9) /// @todo Работает в 10.8.5, нужно проверить ранние версии 10.8
    {
        return QRectF(
            static_cast<qreal>([[webView stringByEvaluatingJavaScriptFromString: @"document.querySelector('svg').getBoundingClientRect().x;"] doubleValue]),
            static_cast<qreal>([[webView stringByEvaluatingJavaScriptFromString: @"document.querySelector('svg').getBoundingClientRect().y;"] doubleValue]),
            static_cast<qreal>([[webView stringByEvaluatingJavaScriptFromString: @"document.querySelector('svg').getBoundingClientRect().width;"] doubleValue]),
            static_cast<qreal>([[webView stringByEvaluatingJavaScriptFromString: @"document.querySelector('svg').getBoundingClientRect().height;"] doubleValue])
        );
    }
    else
    {
        return WebFrameDocumentBounds([webView mainFrame]);
    }
}

QRectF SVGActualBoundingBox(WebView *webView)
{
    QRectF rect;
    if(QSysInfo::MacintoshVersion >= QSysInfo::MV_10_9) /// @todo Работает в 10.8.5, нужно проверить ранние версии 10.8
        rect = DOMNodeActualBoundingBox([webView mainFrameDocument]);
    else
        rect = QRectFIntegerized(SVGBBox(webView));
    if(rect.isValid())
        rect = rect.intersected(QRectF(0, 0, std::numeric_limits<qreal>::max(), std::numeric_limits<qreal>::max()));
    return rect;
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
        QImage image;
        QRectF exposedRect;
        qreal scaleFactor;
    };

    enum ScaleMethod
    {
        SCALE_BY_RESIZE_FRAME,
        SCALE_BY_RESIZE_FRAME_AND_SCALE_CONTENT
    };

    Impl(const QUrl &url);
    Impl(const QByteArray &htmlData, MacWebKitRasterizerGraphicsItem::DataType dataType);
    ~Impl();

    MacWebKitRasterizerGraphicsItem::State state() const;
    void setState(MacWebKitRasterizerGraphicsItem::State state);

    void waitForLoad() const;
    QImage grabImage(qreal scaleFactor = 1, const QRectF &targetArea = QRectF());

    QRectF rect() const;
    void setRect(const QRectF &rect);

    qreal minScaleFactor() const;
    void setMinScaleFactor(qreal minScaleFactor);

    qreal maxScaleFactor() const;
    void setMaxScaleFactor(qreal maxScaleFactor);

    ScaleMethod scaleMethod() const;
    void setScaleMethod(ScaleMethod scaleMethod);

    RasterizerCache &rasterizerCache();
    const RasterizerCache &rasterizerCache() const;

private:
    void init();

    WebView *m_view;
    QWidget *m_container;
    MacWebKitRasterizerViewDelegate *m_delegate;
    MacWebKitRasterizerGraphicsItem::State m_state;
    QRectF m_rect;
    qreal m_minScaleFactor;
    qreal m_maxScaleFactor;
    ScaleMethod m_scaleMethod;
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
        NSURLRequest *request = [NSURLRequest requestWithURL: [NSURL URLWithString: [NSString stringWithUTF8String: url.toString().toUtf8().data()]]];
        [[m_view mainFrame] loadRequest: request];
        waitForLoad();
    }
    [pool release];
}

MacWebKitRasterizerGraphicsItem::Impl::Impl(const QByteArray &htmlData, MacWebKitRasterizerGraphicsItem::DataType dataType)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    init();
    if(htmlData.isEmpty())
    {
        m_state = MacWebKitRasterizerGraphicsItem::STATE_FAILED;
    }
    else
    {
        NSString *mimeType = nil;
        switch(dataType)
        {
        case DATA_TYPE_HTML:
            mimeType = @"text/html";
            break;
        case DATA_TYPE_XHTML:
            mimeType = @"application/xhtml+xml";
            break;
        case DATA_TYPE_XML:
            mimeType = @"application/xml";
            break;
        case DATA_TYPE_SVG:
            mimeType = @"image/svg+xml";
            break;
        default:
            break;
        }
        NSData *data = [NSData dataWithBytes: const_cast<void*>(static_cast<const void*>(htmlData.constData())) length: static_cast<NSUInteger>(htmlData.size())];
        [[m_view mainFrame] loadData: data MIMEType: mimeType textEncodingName: nil baseURL: nil];
        waitForLoad();
    }
    [pool release];
}

MacWebKitRasterizerGraphicsItem::Impl::~Impl()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [m_view setFrameLoadDelegate: nil];
    [m_delegate release];
    [m_view removeFromSuperview];
    [m_view release];
    m_container->deleteLater();
    [pool release];
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
        [[NSRunLoop currentRunLoop] runMode: NSDefaultRunLoopMode beforeDate: [NSDate distantFuture]];
}

QImage MacWebKitRasterizerGraphicsItem::Impl::grabImage(qreal scaleFactor, const QRectF &targetArea)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    const qreal actualScaleFactor = std::max(std::min(scaleFactor, maxScaleFactor()), minScaleFactor());
#if defined (MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_DEBUG)
    DLOG << "actualScaleFactor:" << actualScaleFactor;
#endif
    const QRectF scaledRect = QRectFIntegerized(QRectF(m_rect.topLeft() * actualScaleFactor, m_rect.size() * actualScaleFactor));
    const QRectF scaledPage = QRectFIntegerized(scaledRect.united(QRectF(0, 0, 1, 1)));

    switch(scaleMethod())
    {
    case SCALE_BY_RESIZE_FRAME_AND_SCALE_CONTENT:
    {
        const QString zoomScript = QString::fromLatin1("document.documentElement.style.zoom = '%1'");
        const double oldScaleFactor = QString::fromUtf8([[m_view stringByEvaluatingJavaScriptFromString: @"document.documentElement.style.zoom;"] UTF8String]).toDouble();
        if(oldScaleFactor > actualScaleFactor)
        {
            [m_view stringByEvaluatingJavaScriptFromString: [NSString stringWithUTF8String: zoomScript.arg(actualScaleFactor).toUtf8().data()]];
            [m_view setFrameSize: NSMakeSize(scaledPage.width(), scaledPage.height())];
        }
        else
        {
            [m_view setFrameSize: NSMakeSize(scaledPage.width(), scaledPage.height())];
            [m_view stringByEvaluatingJavaScriptFromString: [NSString stringWithUTF8String: zoomScript.arg(actualScaleFactor).toUtf8().data()]];
        }
        break;
    }
    case SCALE_BY_RESIZE_FRAME:
    {
        [m_view setFrameSize: NSMakeSize(scaledPage.width(), scaledPage.height())];
        break;
    }
    }

#if defined (MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_DEBUG)
    QTime time;
    time.start();
#endif

    NSView *webFrameViewDocView = [[[m_view mainFrame] frameView] documentView];
    const NSRect cacheRect = QRectFToNSRect(scaledPage);
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

#if defined (MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_DEBUG)
    DLOG << "Offscreen rendering time =" << time.elapsed() << "ms";
    time.restart();
#endif

    QRect outputRect = (targetArea.isValid() ? QRectF(targetArea.topLeft() * actualScaleFactor, targetArea.size() * actualScaleFactor) : scaledRect).toRect();
    QImage image(outputRect.size(), QImage::Format_ARGB32);
    for(int i = 0; i < outputRect.height(); i++)
    {
        QRgb *dstLine = reinterpret_cast<QRgb*>(image.scanLine(i));
        unsigned char *srcLine = [bitmapRep bitmapData] + (i + outputRect.y()) * [bitmapRep bytesPerRow] + outputRect.x() * [bitmapRep samplesPerPixel];
        for(int j = 0; j < outputRect.width(); j++)
        {
            unsigned char *source = srcLine + j * [bitmapRep samplesPerPixel];
            dstLine[j] = qRgba(source[0], source[1], source[2], source[3]);
        }
    }

#if defined (MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_DEBUG)
    DLOG << "Image converting time =" << time.elapsed() << "ms";
#endif

    [bitmapRep release];
    [pool release];
    return image;
}

QRectF MacWebKitRasterizerGraphicsItem::Impl::rect() const
{
    return m_rect;
}

void MacWebKitRasterizerGraphicsItem::Impl::setRect(const QRectF &rect)
{
#if defined (MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_DEBUG)
    DLOG << "rect:" << rect;
#endif
    m_rect = rect;
}

qreal MacWebKitRasterizerGraphicsItem::Impl::minScaleFactor() const
{
    return m_minScaleFactor;
}

void MacWebKitRasterizerGraphicsItem::Impl::setMinScaleFactor(qreal minScaleFactor)
{
#if defined (MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_DEBUG)
    DLOG << "minScaleFactor:" << minScaleFactor;
#endif
    m_minScaleFactor = minScaleFactor;
}

qreal MacWebKitRasterizerGraphicsItem::Impl::maxScaleFactor() const
{
    return m_maxScaleFactor;
}

void MacWebKitRasterizerGraphicsItem::Impl::setMaxScaleFactor(qreal maxScaleFactor)
{
#if defined (MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_DEBUG)
    DLOG << "maxScaleFactor:" << maxScaleFactor;
#endif
    m_maxScaleFactor = maxScaleFactor;
}

MacWebKitRasterizerGraphicsItem::Impl::ScaleMethod MacWebKitRasterizerGraphicsItem::Impl::scaleMethod() const
{
    return m_scaleMethod;
}

void MacWebKitRasterizerGraphicsItem::Impl::setScaleMethod(MacWebKitRasterizerGraphicsItem::Impl::ScaleMethod scaleMethod)
{
#if defined (MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_DEBUG)
    QString methodStr;
    switch(scaleMethod)
    {
#define METHOD_CASE(METHOD) \
    case METHOD: \
        methodStr = QString::fromLatin1(#METHOD); \
        break;
    METHOD_CASE(SCALE_BY_RESIZE_FRAME)
    METHOD_CASE(SCALE_BY_RESIZE_FRAME_AND_SCALE_CONTENT)
#undef METHOD_CASE
    }
    DLOG << "scaleMethod:" << methodStr;
#endif
    m_scaleMethod = scaleMethod;
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
    m_minScaleFactor = 1;
    m_maxScaleFactor = 1;
    m_scaleMethod = SCALE_BY_RESIZE_FRAME;
    [m_view setFrameLoadDelegate: reinterpret_cast<id>(m_delegate)];
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
    WHEREAMI;
#endif
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    if(frame == [view mainFrame])
    {
        QRectF actualRect;
        const bool isSvg = QString::fromUtf8([[view stringByEvaluatingJavaScriptFromString: @"document.documentElement instanceof SVGElement;"] UTF8String]) == QString::fromLatin1("true");
        const bool emptySVGViewBoxSupported = QSysInfo::MacintoshVersion >= QSysInfo::MV_10_9; /// @todo Работает в 10.8.5, нужно проверить ранние версии 10.8
        if(isSvg)
        {
            const QRectF viewBox = SVGViewBoxAttribute(view);
            const QRectF boundClientRect = SVGBoundingClientRect(view);

            /// @note WebKit рендерит с учетом смещения, нужен только размер
            if(boundClientRect.isValid())
                actualRect = QRectF(0, 0, boundClientRect.width(), boundClientRect.height());
            else if(viewBox.isValid())
                actualRect = QRectF(0, 0, viewBox.width(), viewBox.height());
            else
                actualRect = SVGActualBoundingBox(view);
            if(!actualRect.isValid())
                actualRect = WebFrameDocumentBounds(frame);

#if defined (MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_DEBUG)
            qDebug() << "***** ----------------------------------------";
            qDebug() << "***** Detected SVG document";
            qDebug() << "***** ----------------------------------------";
            qDebug() << "***** viewBox attribute:" << SVGViewBoxAttribute(view);
            qDebug() << "***** size attribute:" << SVGSizeAttribute(view);
            qDebug() << "***** getBBox() value:" << SVGBBox(view);
            qDebug() << "***** getBoundingClientRect() value:" << SVGBoundingClientRect(view);
            qDebug() << "***** DOM boundingBox:" << DOMNodeActualBoundingBox([view mainFrameDocument]);
            qDebug() << "***** documentView bounds:" << WebFrameDocumentBounds(frame);
            qDebug() << "***** ----------------------------------------";
            qDebug() << "***** actual rect:" << actualRect;
            qDebug() << "***** ----------------------------------------";
#endif

            MacWebKitRasterizerGraphicsItem::Impl::ScaleMethod scaleMethod;
            if(SVGSizeAttribute(view).isEmpty() && (viewBox.isValid() || !emptySVGViewBoxSupported))
                scaleMethod = MacWebKitRasterizerGraphicsItem::Impl::SCALE_BY_RESIZE_FRAME;
            else
                scaleMethod = MacWebKitRasterizerGraphicsItem::Impl::SCALE_BY_RESIZE_FRAME_AND_SCALE_CONTENT;
            m_impl->setScaleMethod(scaleMethod);
        }
        else
        {
            /// @todo Тут, возможно, следует сделать более гибкий алгоритм
            actualRect = WebFrameDocumentBounds(frame);
            actualRect = actualRect.united(QRectF(0, 0, DEFAULT_WIDTH, 1));

#if defined (MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_DEBUG)
            qDebug() << "***** ----------------------------------------";
            qDebug() << "***** Detected HTML document";
            qDebug() << "***** ----------------------------------------";
            qDebug() << "***** DOM boundingBox:" << DOMNodeActualBoundingBox([view mainFrameDocument]);
            qDebug() << "***** documentView bounds:" << WebFrameDocumentBounds(frame);
            qDebug() << "***** ----------------------------------------";
            qDebug() << "***** actual rect:" << actualRect;
            qDebug() << "***** ----------------------------------------";
#endif
            m_impl->setScaleMethod(MacWebKitRasterizerGraphicsItem::Impl::SCALE_BY_RESIZE_FRAME_AND_SCALE_CONTENT);
        }

        if(!actualRect.isValid())
        {
            const qreal width = std::max(actualRect.width(), DEFAULT_WIDTH);
            const qreal height = std::max(actualRect.height(), DEFAULT_HEIGHT);
            const qreal top = (actualRect.top() + actualRect.height() > height ? actualRect.top() : 0);
            const qreal left = (actualRect.left() + actualRect.width() > width ? actualRect.left() : 0);
            actualRect = QRectF(left, top, width, height);
        }

        if(isSvg && !emptySVGViewBoxSupported && !SVGViewBoxAttribute(view).isValid())
        {
            const QRectF fakeViewBoxRect = QRectFIntegerized(QRectF(0, 0, actualRect.width(), actualRect.height()));
            const qreal x = -DIMENSION_DELTA;
            const qreal y = -DIMENSION_DELTA;
            const qreal w = fakeViewBoxRect.width() + 2 * DIMENSION_DELTA;
            const qreal h = fakeViewBoxRect.height() + 2 * DIMENSION_DELTA;
            [view stringByEvaluatingJavaScriptFromString: [NSString stringWithFormat: @"document.querySelector('svg').setAttribute('viewBox', '%f %f %f %f');", x, y, w, h]];
#if defined (MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_DEBUG)
            DLOG << "Set fake ViewBox:" << x << y << w << h;
#endif
        }

        m_impl->setRect(actualRect);
        m_impl->setMinScaleFactor(std::max(MIN_IMAGE_DIMENSION / actualRect.width(), MIN_IMAGE_DIMENSION / actualRect.height()));
        m_impl->setMaxScaleFactor(std::min(MAX_IMAGE_DIMENSION / actualRect.width(), MAX_IMAGE_DIMENSION / actualRect.height()));
        m_impl->setState(MacWebKitRasterizerGraphicsItem::STATE_SUCCEED);
    }
    [pool release];
}

- (void)                   webView: (WebView*)  view
   didFailProvisionalLoadWithError: (NSError*)  error
                          forFrame: (WebFrame*) frame
{
#if defined (MAC_WEBKIT_RASTERIZER_GRAPHICS_ITEM_DEBUG)
    WHEREAMI;
#endif
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    if(frame == [view mainFrame])
    {
        qWarning() << QString::fromUtf8([[error description] UTF8String]);
        m_impl->setState(MacWebKitRasterizerGraphicsItem::STATE_FAILED);
    }
    [pool release];
}

@end

// ====================================================================================================

MacWebKitRasterizerGraphicsItem::MacWebKitRasterizerGraphicsItem(const QUrl &url, QGraphicsItem *parentItem)
    : QGraphicsObject(parentItem)
    , m_impl(new Impl(url))
{
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption, true);
}

MacWebKitRasterizerGraphicsItem::MacWebKitRasterizerGraphicsItem(const QByteArray &htmlData, DataType dataType, QGraphicsItem *parentItem)
    : QGraphicsObject(parentItem)
    , m_impl(new Impl(htmlData, dataType))
{
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption, true);
}

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
    const QRectF identityRect = QRectF(0, 0, 1, 1);
    const QRectF deviceTransformedRect = painter->deviceTransform().mapRect(identityRect);
    const qreal newScaleFactor = std::max(deviceTransformedRect.width(), deviceTransformedRect.height());
    const QRectF newExposedRect = boundingRect().intersected(option->exposedRect);
    QImage &image = m_impl->rasterizerCache().image;
    QRectF &exposedRect = m_impl->rasterizerCache().exposedRect;
    qreal &scaleFactor = m_impl->rasterizerCache().scaleFactor;
    if(std::abs(newScaleFactor - scaleFactor) / std::max(newScaleFactor, scaleFactor) > 1e-2 || !exposedRect.contains(newExposedRect))
    {
        image = m_impl->grabImage(newScaleFactor, newExposedRect);
        scaleFactor = newScaleFactor;
        exposedRect = newExposedRect;
    }
    painter->drawImage(newExposedRect, image, QRectF(image.rect()));
}

MacWebKitRasterizerGraphicsItem::State MacWebKitRasterizerGraphicsItem::state() const
{
    return m_impl->state();
}

// ====================================================================================================

