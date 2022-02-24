/*
   Copyright (C) 2022 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "MSEdgeWebView2SVGGraphicsItem.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include <windows.h>
#include "WebView2.h"

#include <QBuffer>
#include <QByteArray>
#include <QDebug>
#include <QEventLoop>
#include <QImageReader>
#include <QJsonDocument>
#include <QPainter>
#include <QString>
#include <QStyleOptionGraphicsItem>
#include <QUrl>
#include <QWidget>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "GraphicsItemUtils.h"

namespace {

template<typename Interface, typename Arg1Type, typename Arg2Type, typename ResultType>
class Handler : public Interface
{
private:
    Handler()
        : m_refCount(1)
        , m_finished(false)
        , m_result(ResultType())
    {}

    virtual ~Handler()
    {}

public:
    static Handler<Interface, Arg1Type, Arg2Type, ResultType> *create()
    {
        return new Handler<Interface, Arg1Type, Arg2Type, ResultType>();
    }

    ResultType result()
    {
        return m_result;
    }

    void waitForFinish()
    {
        if(!m_finished)
            m_loop.exec(QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers);
    }

    HRESULT STDMETHODCALLTYPE Invoke(Arg1Type arg1, Arg2Type arg2) Q_DECL_OVERRIDE;

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) Q_DECL_OVERRIDE
    {
        Q_UNUSED(riid);
        if(!ppvObject)
            return E_INVALIDARG;
        *ppvObject = Q_NULLPTR;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef(void) Q_DECL_OVERRIDE
    {
        ++m_refCount;
        return m_refCount;
    }

    ULONG STDMETHODCALLTYPE Release(void) Q_DECL_OVERRIDE
    {
        const ULONG refCount = --m_refCount;
        if(refCount == 0)
            delete this;
        return refCount;
    }

protected:
    void finish()
    {
        m_loop.exit();
        m_finished = true;
    }

private:
    ULONG m_refCount;
    QEventLoop m_loop;
    bool m_finished;
    ResultType m_result;
};

typedef Handler<ICoreWebView2NavigationCompletedEventHandler, ICoreWebView2*, ICoreWebView2NavigationCompletedEventArgs*, ICoreWebView2NavigationCompletedEventArgs*> CoreWebView2NavigationCompletedEventHandler;
template<>
HRESULT STDMETHODCALLTYPE CoreWebView2NavigationCompletedEventHandler::Invoke(ICoreWebView2 *sender, ICoreWebView2NavigationCompletedEventArgs *args)
{
    Q_UNUSED(sender);
    m_result = args;
    if(m_result)
        m_result->AddRef();
    finish();
    return S_OK;
}

typedef Handler<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler, HRESULT, ICoreWebView2Environment*, ICoreWebView2Environment*> CoreWebView2CreateCoreWebView2EnvironmentCompletedHandler;
template<>
HRESULT STDMETHODCALLTYPE CoreWebView2CreateCoreWebView2EnvironmentCompletedHandler::Invoke(HRESULT errorCode, ICoreWebView2Environment *createdEnvironment)
{
    if(SUCCEEDED(errorCode))
        m_result = createdEnvironment;
    else
        qWarning() << "CoreWebView2CreateCoreWebView2EnvironmentCompletedHandler error:" << QString::number(errorCode, 16);
    if(m_result)
        m_result->AddRef();
    finish();
    return S_OK;
}

typedef Handler<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler, HRESULT, ICoreWebView2Controller*, ICoreWebView2Controller*> CoreWebView2CreateCoreWebView2ControllerCompletedHandler;
template<>
HRESULT STDMETHODCALLTYPE CoreWebView2CreateCoreWebView2ControllerCompletedHandler::Invoke(HRESULT errorCode, ICoreWebView2Controller *createdController)
{
    if(SUCCEEDED(errorCode))
        m_result = createdController;
    else
        qWarning() << "CoreWebView2CreateCoreWebView2ControllerCompletedHandler error:" << QString::number(errorCode, 16);
    if(m_result)
        m_result->AddRef();
    finish();
    return S_OK;
}

typedef Handler<ICoreWebView2CallDevToolsProtocolMethodCompletedHandler, HRESULT, LPCWSTR, QJsonDocument> CoreWebView2CallDevToolsProtocolMethodCompletedHandler;
template<>
HRESULT STDMETHODCALLTYPE CoreWebView2CallDevToolsProtocolMethodCompletedHandler::Invoke(HRESULT errorCode, LPCWSTR returnObjectAsJson)
{
    if(SUCCEEDED(errorCode))
        m_result = QJsonDocument::fromJson(QString::fromStdWString(returnObjectAsJson).toUtf8());
    else
        qWarning() << "CoreWebView2CallDevToolsProtocolMethodCompletedHandler error:" << QString::number(errorCode, 16);
    finish();
    return S_OK;
}

typedef Handler<ICoreWebView2ExecuteScriptCompletedHandler, HRESULT, LPCWSTR, QJsonDocument> CoreWebView2ExecuteScriptCompletedHandler;
template<>
HRESULT STDMETHODCALLTYPE CoreWebView2ExecuteScriptCompletedHandler::Invoke(HRESULT errorCode, LPCWSTR returnObjectAsJson)
{
    if(SUCCEEDED(errorCode))
        m_result = QJsonDocument::fromJson(QString::fromStdWString(returnObjectAsJson).toUtf8());
    else
        qWarning() << "CoreWebView2ExecuteScriptCompletedHandler error:" << QString::number(errorCode, 16);
    finish();
    return S_OK;
}

const int VIEWPORT_MARGINS = 8;
const int SCROLL_FIX_OFFSET = VIEWPORT_MARGINS * 4;

} // namespace

struct MSEdgeWebView2SVGGraphicsItem::Impl
{
    struct RasterizerCache
    {
        QImage image;
        qreal scaleFactor;
        QRect exposedRect;
    };

    QByteArray svgData;
    QRectF svgRect;
    QImage originalImage;
    QWidget *container;
    ICoreWebView2Environment *environment;
    ICoreWebView2Controller *webviewController;
    ICoreWebView2 *webviewWindow;
    RasterizerCache rasterizerCache;

    Impl()
        : container(Q_NULLPTR)
        , environment(Q_NULLPTR)
        , webviewController(Q_NULLPTR)
        , webviewWindow(Q_NULLPTR)
    {
        rasterizerCache.scaleFactor = 0;

        CoreWebView2CreateCoreWebView2EnvironmentCompletedHandler *environmentHandler = CoreWebView2CreateCoreWebView2EnvironmentCompletedHandler::create();
        if(SUCCEEDED(CreateCoreWebView2EnvironmentWithOptions(Q_NULLPTR, Q_NULLPTR, Q_NULLPTR, environmentHandler)))
            environmentHandler->waitForFinish();
        environment = environmentHandler->result();
        environmentHandler->Release();
        if(!environment)
            return;

        container = new QWidget(Q_NULLPTR, Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint | Qt::BypassWindowManagerHint | Qt::Tool | Qt::WindowStaysOnBottomHint | Qt::NoDropShadowWindowHint | Qt::WindowTransparentForInput | Qt::WindowDoesNotAcceptFocus);
        container->setAttribute(Qt::WA_TranslucentBackground, true);
        container->setAutoFillBackground(false);
        container->setFixedSize(1, 1);
        container->move(0, 0);
        container->show();
        CoreWebView2CreateCoreWebView2ControllerCompletedHandler* controllerHandler = CoreWebView2CreateCoreWebView2ControllerCompletedHandler::create();
        if(SUCCEEDED(environment->CreateCoreWebView2Controller(reinterpret_cast<HWND>(container->winId()), controllerHandler)))
            controllerHandler->waitForFinish();
        webviewController = controllerHandler->result();
        controllerHandler->Release();
        if(!webviewController)
            return;

        if(!SUCCEEDED(webviewController->get_CoreWebView2(&webviewWindow)) || !webviewWindow)
            return;

        webviewWindow->AddRef();

        ICoreWebView2Settings *settings = Q_NULLPTR;
        if(!SUCCEEDED(webviewWindow->get_Settings(&settings)))
            return;

        settings->put_AreDefaultContextMenusEnabled(FALSE);
        settings->put_AreDefaultScriptDialogsEnabled(FALSE);
        settings->put_AreDevToolsEnabled(TRUE);
        settings->put_AreHostObjectsAllowed(FALSE);
        settings->put_IsBuiltInErrorPageEnabled(FALSE);
        settings->put_IsScriptEnabled(TRUE);
        settings->put_IsStatusBarEnabled(FALSE);
        settings->put_IsWebMessageEnabled(FALSE);
        settings->put_IsZoomControlEnabled(FALSE);
    }

    ~Impl()
    {
        if(webviewWindow)
            webviewWindow->Release();
        if(webviewController)
            webviewController->Release();
        if(environment)
            environment->Release();
        if(container)
            container->deleteLater();
    }

    QByteArray prepareSvgData(const QByteArray &svgData)
    {
        QString preparedData;
        QXmlStreamWriter writer(&preparedData);
        QXmlStreamReader reader(svgData);
        reader.setNamespaceProcessing(false);
        while(!reader.atEnd())
        {
            reader.readNext();
            if(reader.hasError())
            {
                qWarning() << __FUNCTION__ << reader.errorString();
                return svgData;
            }
            writer.writeCurrentToken(reader);
        }
        return preparedData.toUtf8();
    }

    QImage grabImage(const QRect &rect = QRect(), int scale = 1)
    {
        const QRect targetRect = (rect.isEmpty()
                ? svgRect.toAlignedRect()
                : rect
                ).adjusted(VIEWPORT_MARGINS, VIEWPORT_MARGINS, VIEWPORT_MARGINS, VIEWPORT_MARGINS);
        QByteArray array = QByteArray::fromBase64(evaluateDevToolsProtocolMethod(QString::fromLatin1("Page.captureScreenshot"), QString::fromLatin1("{"
                "\"format\":\"png\","
                "\"clip\":{\"x\":%1,\"y\":%2,\"width\":%3,\"height\":%4,\"scale\":%5}"
            "}").arg(targetRect.left()).arg(targetRect.top()).arg(targetRect.width()).arg(targetRect.height()).arg(scale))[QString::fromLatin1("data")].toString().toLatin1());
        QBuffer buffer(&array);
        QImage image = QImageReader(&buffer, "png").read();

        return image;
    }

    void setViewPort(const QRect &rect)
    {
        if(!webviewController)
            return;

        RECT bounds;
        bounds.left = rect.left();
        bounds.top = rect.top();
        bounds.right = rect.left() + rect.width();
        bounds.bottom = rect.top() + rect.height();
        webviewController->put_Bounds(bounds);
    }

    QRect getViewPort() const
    {
        if(!webviewController)
            return QRect();

        RECT rect;
        if(!SUCCEEDED(webviewController->get_Bounds(&rect)))
            return QRect();
        return QRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
    }

    QRect getContentSize()
    {
        const QJsonDocument metrics = evaluateDevToolsProtocolMethod(QString::fromLatin1("Page.getLayoutMetrics"), QString::fromLatin1("{}"));
        const QJsonValue contentSize = metrics[QString::fromLatin1("contentSize")];
        return QRect(contentSize[QString::fromLatin1("x")].toInt(), contentSize[QString::fromLatin1("y")].toInt(), contentSize[QString::fromLatin1("width")].toInt(), contentSize[QString::fromLatin1("height")].toInt());
    }

    void adjustViewPort()
    {
        const QRect oldViewPort = getViewPort();
        const QRect contentSize = getContentSize();
        if(oldViewPort.united(contentSize) != oldViewPort)
            setViewPort(oldViewPort.united(contentSize).adjusted(0, 0, SCROLL_FIX_OFFSET, SCROLL_FIX_OFFSET));
        else
            setViewPort(oldViewPort.adjusted(0, 0, VIEWPORT_MARGINS * 2, VIEWPORT_MARGINS * 2));
    }

    QVariant evaluateJavaScript(const QString &scriptSource)
    {
        if(!webviewWindow)
            return QVariant();

        CoreWebView2ExecuteScriptCompletedHandler *scriptHandler = CoreWebView2ExecuteScriptCompletedHandler::create();
        if(SUCCEEDED(webviewWindow->ExecuteScript(reinterpret_cast<LPCWSTR>(scriptSource.constData()), scriptHandler)))
            scriptHandler->waitForFinish();
        /// @todo WTF?
        //qWarning() << scriptHandler->result().toJson();
        scriptHandler->Release();
        return QVariant();
    }

    QJsonDocument evaluateDevToolsProtocolMethod(const QString &methodName, const QString &parametersAsJson)
    {
        if(!webviewWindow)
            return QJsonDocument();

        CoreWebView2CallDevToolsProtocolMethodCompletedHandler *devToolsHandler = CoreWebView2CallDevToolsProtocolMethodCompletedHandler::create();
        if(SUCCEEDED(webviewWindow->CallDevToolsProtocolMethod(reinterpret_cast<LPCWSTR>(methodName.constData()), reinterpret_cast<LPCWSTR>(parametersAsJson.constData()), devToolsHandler)))
            devToolsHandler->waitForFinish();
        QJsonDocument result = devToolsHandler->result();
        devToolsHandler->Release();
        return result;
    }
};

MSEdgeWebView2SVGGraphicsItem::MSEdgeWebView2SVGGraphicsItem(QGraphicsItem *parentItem)
    : QGraphicsItem(parentItem)
    , m_impl(new Impl)
{
#if (QT_VERSION >= QT_VERSION_CHECK(4, 6, 0))
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption, true);
#endif
}

MSEdgeWebView2SVGGraphicsItem::~MSEdgeWebView2SVGGraphicsItem()
{}

bool MSEdgeWebView2SVGGraphicsItem::load(const QByteArray &svgData, const QUrl &/*baseUrl*/)
{
    if(!m_impl->webviewWindow)
        return false;

    m_impl->svgData = m_impl->prepareSvgData(svgData);
    m_impl->svgRect = QRectF(QPointF(), defaultSvgSize());
    m_impl->setViewPort(m_impl->svgRect.toAlignedRect());

    CoreWebView2NavigationCompletedEventHandler *navigationHandler = CoreWebView2NavigationCompletedEventHandler::create();
    EventRegistrationToken navigationEventToken;
    m_impl->webviewWindow->add_NavigationCompleted(navigationHandler, &navigationEventToken);
    std::wstring str = QString::fromUtf8(m_impl->svgData).toStdWString();
    m_impl->webviewWindow->NavigateToString(str.c_str());
    navigationHandler->waitForFinish();
    m_impl->webviewWindow->remove_NavigationCompleted(navigationEventToken);
    BOOL navigationSuccess = FALSE;
    navigationHandler->result()->get_IsSuccess(&navigationSuccess);
    navigationHandler->result()->Release();
    navigationHandler->Release();

    if(!navigationSuccess)
    {
        qWarning() << "[MSEdgeWebView2SVGGraphicsItem] Error: navigation failed";
        return false;
    }

    if(!rootElementIsSvg())
    {
        qWarning() << "[MSEdgeWebView2SVGGraphicsItem] Error: not an SVG";
        return false;
    }

    removeRootOverflowAttribute();
    m_impl->evaluateDevToolsProtocolMethod(QString::fromLatin1("Emulation.setDefaultBackgroundColorOverride"), QString::fromLatin1("{\"color\":{\"r\":0,\"g\":0,\"b\":0,\"a\":0}}"));

    m_impl->svgRect = detectSvgRect();
    m_impl->svgRect = QRectF(m_impl->svgRect.topLeft(), m_impl->svgRect.size().expandedTo(QSizeF(1, 1)));
    m_impl->setViewPort(m_impl->svgRect.united(QRectF(0, 0, 1, 1)).toAlignedRect());
    m_impl->adjustViewPort();
    m_impl->originalImage = m_impl->grabImage();

    return true;
}

QImage MSEdgeWebView2SVGGraphicsItem::grabImage()
{
    return m_impl->originalImage;
}

QRectF MSEdgeWebView2SVGGraphicsItem::boundingRect() const
{
    return m_impl->svgRect;
}

void MSEdgeWebView2SVGGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    const qreal scaleFactor = GraphicsItemUtils::GetDeviceScaleFactor(painter);
    const QRect exposedRect = boundingRect().intersected(QRectFIntegerized(option->exposedRect)).toAlignedRect();
    if(!GraphicsItemUtils::IsFuzzyEqualScaleFactors(scaleFactor, m_impl->rasterizerCache.scaleFactor) || !m_impl->rasterizerCache.exposedRect.contains(exposedRect))
    {
        QMetaObject::invokeMethod(this, "onUpdateCacheRequested", Qt::QueuedConnection, Q_ARG(QRect, exposedRect), Q_ARG(qreal, scaleFactor));
        painter->drawImage(boundingRect(), m_impl->originalImage, m_impl->originalImage.rect());
    }
    else
    {
        painter->drawImage(m_impl->rasterizerCache.exposedRect, m_impl->rasterizerCache.image, m_impl->rasterizerCache.image.rect());
    }
}

void MSEdgeWebView2SVGGraphicsItem::onUpdateCacheRequested(QRect exposedRect, qreal scaleFactor)
{
    if(scaleFactor > 1)
    {
        m_impl->rasterizerCache.image = m_impl->grabImage(exposedRect, scaleFactor);
        m_impl->rasterizerCache.exposedRect = exposedRect;
    }
    else
    {
        m_impl->rasterizerCache.image = m_impl->originalImage.scaled(m_impl->originalImage.size() * scaleFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_impl->rasterizerCache.exposedRect = boundingRect().toAlignedRect();
    }
    m_impl->rasterizerCache.scaleFactor = scaleFactor;
    update();
}

QVariant MSEdgeWebView2SVGGraphicsItem::evalJSImpl(const QString &scriptSource)
{
    return m_impl->evaluateJavaScript(scriptSource);
}

QByteArray MSEdgeWebView2SVGGraphicsItem::getSvgData()
{
    return m_impl->svgData;
}
