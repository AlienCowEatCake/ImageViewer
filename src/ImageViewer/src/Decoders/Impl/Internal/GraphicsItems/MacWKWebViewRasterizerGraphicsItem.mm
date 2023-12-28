/*
   Copyright (C) 2017-2023 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "MacWKWebViewRasterizerGraphicsItem.h"

#include <cmath>
#include <algorithm>

#include <QtGlobal>
#include <QUrl>
#include <QImage>
#include <QPainter>
#include <QString>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#include <QDebug>
#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
#include <QElapsedTimer>
#else
#include <QTime>
typedef QTime QElapsedTimer;
#endif

#include "Utils/ObjectiveCUtils.h"

#include "../Utils/XmlStreamReader.h"

#include "GraphicsItemUtils.h"

//#define MAC_WKWEBVIEW_RASTERIZER_GRAPHICS_ITEM_DEBUG

#if defined (MAC_WKWEBVIEW_RASTERIZER_GRAPHICS_ITEM_DEBUG)
#define DLOG qDebug() << QString::fromLatin1("[%1]").arg(QString::fromLatin1(__FUNCTION__)).toLatin1().data()
#endif

// ====================================================================================================

#if defined (AVAILABLE_MAC_OS_X_VERSION_10_13_AND_LATER)

namespace {

const qreal MAX_IMAGE_DIMENSION = 16384;
const qreal MIN_IMAGE_DIMENSION = 1;

enum State
{
    STATE_LOADING,
    STATE_FAILED,
    STATE_SUCCEED
};

} // namespace

@interface MacWKWebViewRasterizerWKNavigationDelegate : NSObject
{
    @private
    State m_state;
}

-(id)init;

- (State)state;
- (void)setState:(State)state;

@end

@implementation MacWKWebViewRasterizerWKNavigationDelegate

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

- (void)webView:(WKWebView*)webView didFinishNavigation:(WKNavigation*)navigation
{
    AUTORELEASE_POOL;
    m_state = STATE_SUCCEED;
}

- (void)webView:(WKWebView*)webView didFailProvisionalNavigation:(WKNavigation*)navigation withError:(NSError*)error
{
    AUTORELEASE_POOL;
    qWarning() << ObjCUtils::QStringFromNSString([error description]);
    m_state = STATE_FAILED;
}

- (void)webView:(WKWebView*)webView didFailNavigation:(WKNavigation*)navigation withError:(NSError*)error
{
    AUTORELEASE_POOL;
    qWarning() << ObjCUtils::QStringFromNSString([error description]);
    m_state = STATE_FAILED;
}

- (void)webViewWebContentProcessDidTerminate:(WKWebView*)webView
{
    AUTORELEASE_POOL;
    qWarning() << "webViewWebContentProcessDidTerminate";
    m_state = STATE_FAILED;
}

@end

#endif

// ====================================================================================================

struct MacWKWebViewRasterizerGraphicsItem::Impl
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
        qreal scaleFactor;
    };

    MacWKWebViewRasterizerGraphicsItem *item;
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_13_AND_LATER)
    WKWebView *view;
    MacWKWebViewRasterizerWKNavigationDelegate *delegate;
#endif
    QWidget *container;
    QImage originalImage;
    RasterizerCache rasterizerCache;
    QRectF rect;
    qreal minScaleFactor;
    qreal maxScaleFactor;
    ScaleMethod scaleMethod;

    Impl(MacWKWebViewRasterizerGraphicsItem *item)
        : item(item)
    {
        AUTORELEASE_POOL;
        container = new QWidget;
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_13_AND_LATER)
        if(@available(macOS 10.13, *))
        {
            view = [[WKWebView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
            delegate = [[MacWKWebViewRasterizerWKNavigationDelegate alloc] init];
            [view setNavigationDelegate:reinterpret_cast<id>(delegate)];
            [reinterpret_cast<NSView*>(container->winId()) addSubview:view];
            [view setValue:@NO forKey:@"drawsBackground"];
        }
#endif
        Reset();
    }

    ~Impl()
    {
        AUTORELEASE_POOL;
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_13_AND_LATER)
        if(@available(macOS 10.13, *))
        {
            [view setNavigationDelegate:nil];
            [delegate release];
            [view removeFromSuperview];
            [view release];
        }
#endif
        container->deleteLater();
    }

    void Reset()
    {
        AUTORELEASE_POOL;
        minScaleFactor = 1;
        maxScaleFactor = 1;
        scaleMethod = SCALE_BY_RESIZE_FRAME;
        rect = QRectF();
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_13_AND_LATER)
        if(@available(macOS 10.13, *))
        {
            [delegate setState:STATE_LOADING];
        }
#endif
        rasterizerCache.scaleFactor = 0;
    }

    QString detectEncoding(const QByteArray &svgData)
    {
        QString encoding = XmlStreamReader::getEncoding(svgData);
        if(encoding.isEmpty())
            encoding = QString::fromLatin1("UTF-8");
#if defined (MAC_WKWEBVIEW_RASTERIZER_GRAPHICS_ITEM_DEBUG)
        DLOG << "encoding:" << encoding;
#endif
        return encoding;
    }

    QImage grabImage(qreal scaleFactor)
    {
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_13_AND_LATER)
        AUTORELEASE_POOL;
        if(@available(macOS 10.13, *))
        {
            const qreal actualScaleFactor = std::max(std::min(scaleFactor, maxScaleFactor), minScaleFactor);
#if defined (MAC_WKWEBVIEW_RASTERIZER_GRAPHICS_ITEM_DEBUG)
            DLOG << "actualScaleFactor:" << actualScaleFactor;
#endif
            if(actualScaleFactor < scaleFactor)
            {
                qWarning() << QString::fromLatin1("%1 Extremely large scale factor requested! Requested: %2; Max: %3; Will be used: %4.")
                              .arg(QString::fromLatin1("[MacWKWebViewRasterizerGraphicsItem]"))
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

#if defined (MAC_WKWEBVIEW_RASTERIZER_GRAPHICS_ITEM_DEBUG)
            QElapsedTimer time;
            time.start();
#endif

            WKSnapshotConfiguration *wkSnapshotConfig = [[WKSnapshotConfiguration alloc] init];
            __block bool inProgress = true;
            __block QImage result;
            [view takeSnapshotWithConfiguration:wkSnapshotConfig completionHandler:^(NSImage *snapshotImage, NSError *error){
                inProgress = false;
                if(error)
                    qWarning() << "[MacWKWebViewRasterizerGraphicsItem] Error in grabImage:" << ObjCUtils::QStringFromNSString([NSString stringWithFormat:@"%@", [error localizedDescription]]);
                else if(snapshotImage)
                    result = ObjCUtils::QImageFromNSImage(snapshotImage);
            }];
            while(inProgress)
                [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
            [wkSnapshotConfig release];

#if defined (MAC_WKWEBVIEW_RASTERIZER_GRAPHICS_ITEM_DEBUG)
            DLOG << "Offscreen rendering time =" << time.elapsed() << "ms";
#endif

            return result;
        }
#else
        Q_UNUSED(scaleFactor);
#endif
        qWarning() << "[MacWKWebViewRasterizerGraphicsItem] Error in grabImage: WKWebView is not supported";
        return QImage();
    }
};

// ====================================================================================================

MacWKWebViewRasterizerGraphicsItem::MacWKWebViewRasterizerGraphicsItem(QGraphicsItem *parentItem)
    : QGraphicsItem(parentItem)
    , m_impl(new Impl(this))
{
#if (QT_VERSION >= QT_VERSION_CHECK(4, 6, 0))
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption, true);
#endif
}

MacWKWebViewRasterizerGraphicsItem::~MacWKWebViewRasterizerGraphicsItem()
{}

bool MacWKWebViewRasterizerGraphicsItem::isAvailable()
{
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_13_AND_LATER)
    if(@available(macOS 10.13, *))
    {
        return true;
    }
#endif
    return false;
}

bool MacWKWebViewRasterizerGraphicsItem::load(const QByteArray &svgData, const QUrl &baseUrl)
{
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_13_AND_LATER)
    AUTORELEASE_POOL;
    if(@available(macOS 10.13, *))
    {
        m_impl->Reset();

        [m_impl->view
                        loadData:ObjCUtils::QByteArrayToNSData(svgData)
                        MIMEType:@"image/svg+xml"
           characterEncodingName:ObjCUtils::QStringToNSString(m_impl->detectEncoding(svgData))
                         baseURL:ObjCUtils::QUrlToNSURL(baseUrl)
        ];
        while([m_impl->delegate state] == STATE_LOADING)
            [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];

        if([m_impl->delegate state] != STATE_SUCCEED)
        {
            qWarning() << "[MacWKWebViewRasterizerGraphicsItem] Error: can't load content";
            return false;
        }

        if(!rootElementIsSvg())
        {
            qWarning() << "[MacWKWebViewRasterizerGraphicsItem] Error: not an SVG";
            return false;
        }

        removeRootOverflowAttribute();

        const QRectF viewBox = svgViewBoxAttribute();
        const QSizeF size = svgSizeAttribute();
        m_impl->rect = detectSvgRect();

#if defined (MAC_WKWEBVIEW_RASTERIZER_GRAPHICS_ITEM_DEBUG)
        qDebug() << "***** ----------------------------------------";
        qDebug() << "***** Detected SVG document";
        qDebug() << "***** ----------------------------------------";
        qDebug() << "***** viewBox attribute:" << svgViewBoxAttribute();
        qDebug() << "***** size attribute:" << svgSizeAttribute();
        qDebug() << "***** getBBox() value:" << svgBoundingBoxRect();
        qDebug() << "***** getBoundingClientRect() value:" << svgBoundingClientRect();
        qDebug() << "***** ----------------------------------------";
        qDebug() << "***** actual rect:" << m_impl->rect;
        qDebug() << "***** ----------------------------------------";
#endif

        if(size.isEmpty() && viewBox.isValid())
            m_impl->scaleMethod = Impl::SCALE_BY_RESIZE_FRAME;
        else
            m_impl->scaleMethod = Impl::SCALE_BY_RESIZE_FRAME_AND_SCALE_CONTENT;

        m_impl->minScaleFactor = std::max(MIN_IMAGE_DIMENSION / m_impl->rect.width(), MIN_IMAGE_DIMENSION / m_impl->rect.height());
        m_impl->maxScaleFactor = std::min(MAX_IMAGE_DIMENSION / m_impl->rect.width(), MAX_IMAGE_DIMENSION / m_impl->rect.height());

        m_impl->originalImage = m_impl->grabImage(1.0);
        m_impl->rasterizerCache.image = m_impl->originalImage;
        m_impl->rasterizerCache.scaleFactor = 1.0;

        return true;
    }
#else
    Q_UNUSED(svgData);
    Q_UNUSED(baseUrl);
#endif
    qWarning() << "[MacWKWebViewRasterizerGraphicsItem] Error: WKWebView is not supported";
    return false;
}

QImage MacWKWebViewRasterizerGraphicsItem::grabImage()
{
    return m_impl->originalImage;
}

QRectF MacWKWebViewRasterizerGraphicsItem::boundingRect() const
{
    return m_impl->rect;
}

void MacWKWebViewRasterizerGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    const qreal newScaleFactor = GraphicsItemUtils::GetDeviceScaleFactor(painter);
    QImage &image = m_impl->rasterizerCache.image;
    qreal &scaleFactor = m_impl->rasterizerCache.scaleFactor;
    if(!GraphicsItemUtils::IsFuzzyEqualScaleFactors(newScaleFactor, scaleFactor))
        QMetaObject::invokeMethod(this, "onUpdateCacheRequested", Qt::QueuedConnection, Q_ARG(qreal, newScaleFactor));
    painter->drawImage(m_impl->rect, image, QRectF(image.rect()));
}

void MacWKWebViewRasterizerGraphicsItem::onUpdateCacheRequested(qreal scaleFactor)
{
    m_impl->rasterizerCache.image = m_impl->grabImage(scaleFactor);
    m_impl->rasterizerCache.scaleFactor = scaleFactor;
    update();
}

QVariant MacWKWebViewRasterizerGraphicsItem::evalJSImpl(const QString &scriptSource)
{
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_13_AND_LATER)
    AUTORELEASE_POOL;
    if(@available(macOS 10.13, *))
    {
        __block QString output;
        __block bool inProgress = true;
        [m_impl->view evaluateJavaScript:ObjCUtils::QStringToNSString(scriptSource) completionHandler:^(id result, NSError *error){
            inProgress = false;
            if(error)
                qWarning() << "[MacWKWebViewRasterizerGraphicsItem] Error in evalJSImpl:" << ObjCUtils::QStringFromNSString([NSString stringWithFormat:@"%@", [error localizedDescription]]);
            else if(result)
                output = ObjCUtils::QStringFromNSString([NSString stringWithFormat:@"%@", result]);
        }];
        while(inProgress)
            [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
        return output;
    }
#else
    Q_UNUSED(scriptSource);
#endif
    qWarning() << "[MacWKWebViewRasterizerGraphicsItem] Error in evalJSImpl: WKWebView is not supported";
    return QVariant();
}

// ====================================================================================================

