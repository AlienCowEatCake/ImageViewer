/*
   Copyright (C) 2017-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#import <CoreServices/CoreServices.h>
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>
#import <WebKit/WebKit.h>

#include "MacWebViewRasterizerGraphicsItem.h"

#include <cmath>
#include <algorithm>

#include <QtGlobal>
#include <QUrl>
#include <QImage>
#include <QPainter>
#include <QString>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
#include <QElapsedTimer>
#else
#include <QTime>
typedef QTime QElapsedTimer;
#endif

#include "Utils/InfoUtils.h"
#include "Utils/Logging.h"
#include "Utils/ObjectiveCUtils.h"

#include "GraphicsItemUtils.h"

#include "Utils/MacNSInteger.h"

//#define MAC_WEBVIEW_RASTERIZER_GRAPHICS_ITEM_DEBUG

#if defined (MAC_WEBVIEW_RASTERIZER_GRAPHICS_ITEM_DEBUG)
#define DLOG LOG_INFO() << LOGGING_CTX
#endif

#include "Workarounds/BeginIgnoreDeprecated.h"

// ====================================================================================================

namespace {

const qreal MAX_IMAGE_DIMENSION = 16384;
const qreal MIN_IMAGE_DIMENSION = 1;
const qreal DIMENSION_DELTA = 0.5;

enum State
{
    STATE_LOADING,
    STATE_FAILED,
    STATE_SUCCEED
};

} // namespace

@interface MacWebViewRasterizerViewDelegate : NSObject
{
    @private
    State m_state;
}

-(id)init;

- (State)state;
- (void)setState:(State)state;

@end

@implementation MacWebViewRasterizerViewDelegate

-(id)init
{
    self = [super init];
    if(self)
        m_state = STATE_LOADING;
    return self;
}

- (State)state
{
    return m_state;
}

- (void)setState:(State)state
{
    m_state = state;
}

-(void)webView:(WebView*)view didFinishLoadForFrame:(WebFrame*)frame
{
    AUTORELEASE_POOL;
    if(frame == [view mainFrame])
        m_state = STATE_SUCCEED;
}

- (void)webView:(WebView*)view didFailProvisionalLoadWithError:(NSError*)error forFrame:(WebFrame*)frame
{
    AUTORELEASE_POOL;
    if(frame == [view mainFrame])
    {
        LOG_WARNING() << LOGGING_CTX << ObjCUtils::QStringFromNSString([error description]);
        m_state = STATE_FAILED;
    }
}

@end

// ====================================================================================================

struct MacWebViewRasterizerGraphicsItem::Impl
{
public:
    enum ScaleMethod
    {
        SCALE_BY_RESIZE_FRAME,
        SCALE_BY_RESIZE_FRAME_AND_SCALE_CONTENT
    };

    struct RasterizerCache
    {
        QImage image;
        QRectF exposedRect;
        qreal scaleFactor;
    };

    MacWebViewRasterizerGraphicsItem *item;
    WebView *view;
    MacWebViewRasterizerViewDelegate *delegate;
    QWidget *container;
    RasterizerCache rasterizerCache;
    QRectF rect;
    qreal minScaleFactor;
    qreal maxScaleFactor;
    ScaleMethod scaleMethod;

    Impl(MacWebViewRasterizerGraphicsItem *item)
        : item(item)
    {
        AUTORELEASE_POOL;
        view = [[WebView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        container = new QWidget;
        delegate = [[MacWebViewRasterizerViewDelegate alloc] init];
        [view setFrameLoadDelegate:reinterpret_cast<id>(delegate)];
        @try
        {
            [reinterpret_cast<NSView*>(container->winId()) addSubview:view];
        }
        @catch(NSException *exception)
        {
            LOG_WARNING() << LOGGING_CTX << ObjCUtils::QStringFromNSString([exception reason]);
        }
        if([view respondsToSelector:@selector(setDrawsBackground:)])
        {
            BOOL drawBackground = NO;
            NSMethodSignature *signature = [[view class] instanceMethodSignatureForSelector:@selector(setDrawsBackground:)];
            NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:signature];
            [invocation setTarget:view];
            [invocation setSelector:@selector(setDrawsBackground:)];
            [invocation setArgument:&drawBackground atIndex:2];
            [invocation invoke];
        }
        Reset();
    }

    ~Impl()
    {
        AUTORELEASE_POOL;
        [view setFrameLoadDelegate: nil];
        [delegate release];
        [view removeFromSuperview];
        [view release];
        container->deleteLater();
    }

    void Reset()
    {
        AUTORELEASE_POOL;
        minScaleFactor = 1;
        maxScaleFactor = 1;
        scaleMethod = SCALE_BY_RESIZE_FRAME;
        rect = QRectF();
        [delegate setState:STATE_LOADING];
        rasterizerCache.scaleFactor = 0;
    }

    QImage grabImage(qreal scaleFactor, const QRectF &targetArea)
    {
        AUTORELEASE_POOL;
        const qreal actualScaleFactor = std::max(std::min(scaleFactor, maxScaleFactor), minScaleFactor);
#if defined (MAC_WEBVIEW_RASTERIZER_GRAPHICS_ITEM_DEBUG)
        DLOG << "actualScaleFactor:" << actualScaleFactor;
#endif
        if(actualScaleFactor < scaleFactor)
        {
            LOG_WARNING() << LOGGING_CTX << QString::fromLatin1("Extremely large scale factor requested! Requested: %1; Max: %2; Will be used: %3.")
                    .arg(scaleFactor)
                    .arg(maxScaleFactor)
                    .arg(actualScaleFactor)
                    .toLatin1().data();
        }
        const QRectF scaledRect = QRectFIntegerized(QRectF(rect.topLeft() * actualScaleFactor, rect.size() * actualScaleFactor));
        const QRectF scaledPage = QRectFIntegerized(scaledRect.united(QRectF(0, 0, 1, 1)));

        switch(scaleMethod)
        {
        case SCALE_BY_RESIZE_FRAME_AND_SCALE_CONTENT:
        {
            const QString zoomScript = QString::fromLatin1("document.documentElement.style.zoom = '%1'");
            const double oldScaleFactor = item->evalJS("document.documentElement.style.zoom;").toDouble();
            if(oldScaleFactor > actualScaleFactor)
            {
                item->evalJS(zoomScript.arg(actualScaleFactor));
                [view setFrameSize:ObjCUtils::QSizeFToNSSize(scaledPage.size())];
            }
            else
            {
                [view setFrameSize: ObjCUtils::QSizeFToNSSize(scaledPage.size())];
                item->evalJS(zoomScript.arg(actualScaleFactor));
            }
            break;
        }
        case SCALE_BY_RESIZE_FRAME:
        {
            [view setFrameSize: ObjCUtils::QSizeFToNSSize(scaledPage.size())];
            break;
        }
        }

#if defined (MAC_WEBVIEW_RASTERIZER_GRAPHICS_ITEM_DEBUG)
        QElapsedTimer time;
        time.start();
#endif

        NSView *webFrameViewDocView = [[[view mainFrame] frameView] documentView];
        const NSRect cacheRect = ObjCUtils::QRectFToNSRect(scaledPage);
        const NSInteger one = static_cast<NSInteger>(1);
        NSBitmapImageRep *bitmapRep = [[NSBitmapImageRep alloc]
                initWithBitmapDataPlanes:nil
                              pixelsWide:std::max(static_cast<NSInteger>(cacheRect.size.width), one)
                              pixelsHigh:std::max(static_cast<NSInteger>(cacheRect.size.height), one)
                           bitsPerSample:8
                         samplesPerPixel:4
                                hasAlpha:YES
                                isPlanar:NO
                          colorSpaceName:NSCalibratedRGBColorSpace
                             bytesPerRow:4 * std::max(static_cast<NSInteger>(cacheRect.size.width), one)
                            bitsPerPixel:32
        ];
        NSDictionary *contextAttr = [NSDictionary dictionaryWithObject:bitmapRep forKey:NSGraphicsContextDestinationAttributeName];
        NSGraphicsContext *graphicsContext = [NSGraphicsContext graphicsContextWithAttributes:contextAttr];
        [NSGraphicsContext saveGraphicsState];
        [NSGraphicsContext setCurrentContext:graphicsContext];
        [webFrameViewDocView displayRectIgnoringOpacity:cacheRect inContext:graphicsContext];
        [NSGraphicsContext restoreGraphicsState];

#if defined (MAC_WEBVIEW_RASTERIZER_GRAPHICS_ITEM_DEBUG)
        DLOG << "Offscreen rendering time =" << time.elapsed() << "ms";
        time.restart();
#endif

        const QRect outputRect = (targetArea.isValid() ? QRectF(targetArea.topLeft() * actualScaleFactor, targetArea.size() * actualScaleFactor) : scaledRect).toRect();
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

#if defined (MAC_WEBVIEW_RASTERIZER_GRAPHICS_ITEM_DEBUG)
        DLOG << "Image converting time =" << time.elapsed() << "ms";
#endif

        [bitmapRep release];
        return image;
    }
};

// ====================================================================================================

MacWebViewRasterizerGraphicsItem::MacWebViewRasterizerGraphicsItem(QGraphicsItem *parentItem)
    : QGraphicsItem(parentItem)
    , m_impl(new Impl(this))
{
#if (QT_VERSION >= QT_VERSION_CHECK(4, 6, 0))
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption, true);
#endif
}

MacWebViewRasterizerGraphicsItem::~MacWebViewRasterizerGraphicsItem()
{}

bool MacWebViewRasterizerGraphicsItem::isAvailable()
{
    /// @note WebView on 10.3 has no SVG support
    if(InfoUtils::MacVersionGreatOrEqual(10, 4))
       return true;
    return false;
}

bool MacWebViewRasterizerGraphicsItem::load(const QByteArray &svgData, const QUrl &baseUrl)
{
    AUTORELEASE_POOL;
    m_impl->Reset();

    [[m_impl->view mainFrame]
                loadData: ObjCUtils::QByteArrayToNSData(svgData)
                MIMEType: @"image/svg+xml"
        textEncodingName: nil
                 baseURL: ObjCUtils::QUrlToNSURL(baseUrl)
    ];
    while([m_impl->delegate state] == STATE_LOADING)
        [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];

    if([m_impl->delegate state] != STATE_SUCCEED)
    {
        LOG_WARNING() << LOGGING_CTX << "Error: can't load content";
        return false;
    }

    if(!rootElementIsSvg())
    {
        LOG_WARNING() << LOGGING_CTX << "Error: not an SVG";
        return false;
    }

    removeRootOverflowAttribute();

    const bool emptySVGViewBoxSupported = InfoUtils::MacVersionGreatOrEqual(10, 8);
    const QRectF viewBox = svgViewBoxAttribute();
    const QSizeF size = svgSizeAttribute();
    m_impl->rect = detectSvgRect();

#if defined (MAC_WEBVIEW_RASTERIZER_GRAPHICS_ITEM_DEBUG)
    LOG_INFO() << LOGGING_CTX << "***** ----------------------------------------";
    LOG_INFO() << LOGGING_CTX << "***** Detected SVG document";
    LOG_INFO() << LOGGING_CTX << "***** ----------------------------------------";
    LOG_INFO() << LOGGING_CTX << "***** viewBox attribute:" << svgViewBoxAttribute();
    LOG_INFO() << LOGGING_CTX << "***** size attribute:" << svgSizeAttribute();
    LOG_INFO() << LOGGING_CTX << "***** getBBox() value:" << svgBoundingBoxRect();
    LOG_INFO() << LOGGING_CTX << "***** getBoundingClientRect() value:" << svgBoundingClientRect();
    LOG_INFO() << LOGGING_CTX << "***** ----------------------------------------";
    LOG_INFO() << LOGGING_CTX << "***** actual rect:" << m_impl->rect;
    LOG_INFO() << LOGGING_CTX << "***** ----------------------------------------";
#endif

    if(size.isEmpty() && (viewBox.isValid() || !emptySVGViewBoxSupported))
        m_impl->scaleMethod = Impl::SCALE_BY_RESIZE_FRAME;
    else
        m_impl->scaleMethod = Impl::SCALE_BY_RESIZE_FRAME_AND_SCALE_CONTENT;

    if(!emptySVGViewBoxSupported && !viewBox.isValid())
    {
        const QRectF fakeViewBoxRect = QRectFIntegerized(QRectF(QPointF(), m_impl->rect.size()));
        const qreal x = -DIMENSION_DELTA;
        const qreal y = -DIMENSION_DELTA;
        const qreal w = fakeViewBoxRect.width() + 2 * DIMENSION_DELTA;
        const qreal h = fakeViewBoxRect.height() + 2 * DIMENSION_DELTA;
        evalJS(QString::fromLatin1("document.rootElement.setAttribute('viewBox', '%1 %2 %3 %4');").arg(x).arg(y).arg(w).arg(h));
#if defined (MAC_WEBVIEW_RASTERIZER_GRAPHICS_ITEM_DEBUG)
        DLOG << "Set fake ViewBox:" << x << y << w << h;
#endif
    }

    m_impl->minScaleFactor = std::max(MIN_IMAGE_DIMENSION / m_impl->rect.width(), MIN_IMAGE_DIMENSION / m_impl->rect.height());
    m_impl->maxScaleFactor = std::min(MAX_IMAGE_DIMENSION / m_impl->rect.width(), MAX_IMAGE_DIMENSION / m_impl->rect.height());

    return true;
}

QImage MacWebViewRasterizerGraphicsItem::grabImage()
{
    return grabImage(static_cast<qreal>(1.0));
}

QImage MacWebViewRasterizerGraphicsItem::grabImage(qreal scaleFactor)
{
    return m_impl->grabImage(scaleFactor, boundingRect());
}

QRectF MacWebViewRasterizerGraphicsItem::boundingRect() const
{
    return m_impl->rect;
}

void MacWebViewRasterizerGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    const qreal newScaleFactor = GraphicsItemUtils::GetDeviceScaleFactor(painter);
    const QRectF newExposedRect = boundingRect().intersected(option->exposedRect);
    QImage &image = m_impl->rasterizerCache.image;
    QRectF &exposedRect = m_impl->rasterizerCache.exposedRect;
    qreal &scaleFactor = m_impl->rasterizerCache.scaleFactor;
    if(!GraphicsItemUtils::IsFuzzyEqualScaleFactors(newScaleFactor, scaleFactor) || !exposedRect.contains(newExposedRect))
    {
        image = m_impl->grabImage(newScaleFactor, newExposedRect);
        scaleFactor = newScaleFactor;
        exposedRect = newExposedRect;
    }
    painter->drawImage(exposedRect, image, QRectF(image.rect()));
}

QVariant MacWebViewRasterizerGraphicsItem::evalJSImpl(const QString &scriptSource)
{
    AUTORELEASE_POOL;
    return ObjCUtils::QStringFromNSString([m_impl->view stringByEvaluatingJavaScriptFromString: ObjCUtils::QStringToNSString(scriptSource)]);
}

// ====================================================================================================

#include "Workarounds/EndIgnoreDeprecated.h"

