/*
   Copyright (C) 2018-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "QMLWebEngineSVGGraphicsItem.h"

#include <cmath>
#include <algorithm>

#include <QUrl>
#include <QString>
#include <QByteArray>
#include <QVector>
#include <QPair>

#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include <QEventLoop>
#include <QTimer>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QtWebEngineQuick>
#else
#include <QtWebEngine>
#endif
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QQmlProperty>

#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLFunctions>

#include "Utils/Logging.h"

#include "../Utils/XmlStreamReader.h"

#include "GraphicsItemUtils.h"

class QQuickWebEngineLoadRequest;
class QWebEngineLoadingInfo;

namespace {

const int SVG_RENDERING_FPS = 25;
const qreal MAX_IMAGE_DIMENSION = 16384;
const qreal MIN_IMAGE_DIMENSION = 1;

int getMaxTextureSize()
{
    static int maxTextureSize = 0;
    static bool isInitialized = false;
    if(!isInitialized)
    {
        isInitialized = true;

        QOpenGLContext context;
        if(!context.create())
            return maxTextureSize;

        QOffscreenSurface surface;
        surface.setFormat(context.format());
        surface.create();
        if(!surface.isValid())
            return maxTextureSize;

        context.makeCurrent(&surface);

        QOpenGLFunctions *functions = context.functions();
        if(!functions)
            return maxTextureSize;

        functions->glEnable(GL_TEXTURE_2D);
        functions->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

        LOG_INFO() << LOGGING_CTX << "GL_MAX_TEXTURE_SIZE =" << maxTextureSize;
    }
    return maxTextureSize;
}

QByteArray getWebEngineComponentQml()
{
    static QByteArray qml;
    static bool isInitialized = false;
    if(isInitialized)
        return qml;

    isInitialized = true;
    QVector<QPair<QByteArray, QByteArray>> qmls;
    qmls.push_back(qMakePair<QByteArray, QByteArray>(
                       "import QtQuick 2.0\n"
                       "import QtWebEngine 1.6\n"
                       "\n"
                       "WebEngineView {\n"
                       "    anchors.fill: parent\n"
                       "    backgroundColor: \"transparent\"\n"
                       "    settings.showScrollBars: false\n"
                       "}\n"
                       , "QtWebEngine 1.6+"));
    qmls.push_back(qMakePair<QByteArray, QByteArray>(
                       "import QtQuick 2.0\n"
                       "import QtWebEngine 1.2\n"
                       "\n"
                       "WebEngineView {\n"
                       "    anchors.fill: parent\n"
                       "    backgroundColor: \"transparent\"\n"
                       "}\n"
                       , "QtWebEngine 1.2+"));
    qmls.push_back(qMakePair<QByteArray, QByteArray>(
                       "import QtQuick 2.0\n"
                       "import QtWebEngine 1.1\n"
                       "\n"
                       "WebEngineView {\n"
                       "    anchors.fill: parent\n"
                       "}\n"
                       , "QtWebEngine 1.1"));
    qmls.push_back(qMakePair<QByteArray, QByteArray>(
                       "import QtQuick 2.0\n"
                       "import QtWebEngine 1.0\n"
                       "\n"
                       "WebEngineView {\n"
                       "    anchors.fill: parent\n"
                       "}\n"
                       , "QtWebEngine 1.0"));

    QQmlEngine engine;
    for(typename QVector<QPair<QByteArray, QByteArray>>::ConstIterator it = qmls.constBegin(), itEnd = qmls.constEnd(); it != itEnd; ++it)
    {
        QQmlComponent component(&engine);
        qml = it->first;
        component.setData(qml, QUrl());
        if(component.status() == QQmlComponent::Ready)
        {
            LOG_INFO() << LOGGING_CTX << it->second.data() << "detected";
            return qml;
        }
        else
        {
            LOG_INFO() << LOGGING_CTX << it->second.data() << "not detected:" << component.errorString();
        }
    }
    return qml;
}

struct WebEngineInitializer
{
    WebEngineInitializer()
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        QtWebEngineQuick::initialize();
#else
        QtWebEngine::initialize();
#endif
    }
};

WebEngineInitializer webEngineInitializer;

} // namespace

struct QMLWebEngineSVGGraphicsItem::Impl
{
    QQuickView quickView;
    QQuickItem *webEngineView;
    QByteArray svgData;
    QRectF svgRect;
    qreal minScaleFactor;
    qreal maxScaleFactor;
    QTimer timer;

    Impl()
        : webEngineView(createWebEngineView(&quickView))
        , minScaleFactor(1)
        , maxScaleFactor(1)
    {}

    void setScaleFactor(qreal scaleFactor)
    {
        quickView.resize((svgRect.united(QRectF(0, 0, 1, 1)).size() * scaleFactor).toSize());
        QQmlProperty(webEngineView, QString::fromLatin1("zoomFactor")).write(scaleFactor / quickView.devicePixelRatio());
    }

    QQuickItem *createWebEngineView(QQuickView *quickView) const
    {
        QQmlEngine *engine = quickView->engine();
        if(!engine)
        {
            LOG_WARNING() << LOGGING_CTX << "Error: can't get QQmlEngine";
            return Q_NULLPTR;
        }

        QQmlComponent *component = new QQmlComponent(engine);
        component->setData(getWebEngineComponentQml(), QUrl());
        if(component->status() != QQmlComponent::Ready)
        {
            LOG_INFO() << LOGGING_CTX << "Error:" << component->errorString();
            return Q_NULLPTR;
        }

        QQuickItem *webEngineView = qobject_cast<QQuickItem*>(component->create(engine->rootContext()));
        if(!webEngineView)
        {
            LOG_WARNING() << LOGGING_CTX << "Error: can't create WebEngineView component";
            return Q_NULLPTR;
        }

        quickView->setContent(QUrl(), component, webEngineView);
        return webEngineView;
    }
};

QMLWebEngineSVGGraphicsItem::QMLWebEngineSVGGraphicsItem(QGraphicsItem *parentItem)
    : QGraphicsObject(parentItem)
    , m_impl(new Impl)
{
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption, true);

    m_impl->quickView.setFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    m_impl->quickView.setColor(Qt::transparent);
    m_impl->quickView.winId();

    connect(&m_impl->timer, SIGNAL(timeout()), this, SLOT(onUpdateRequested()));
    m_impl->timer.setSingleShot(false);
    m_impl->timer.setInterval(1000 / SVG_RENDERING_FPS);
}

QMLWebEngineSVGGraphicsItem::~QMLWebEngineSVGGraphicsItem()
{
    m_impl->timer.disconnect();
    m_impl->timer.stop();
}

bool QMLWebEngineSVGGraphicsItem::isAvailable()
{
    return true;
}

bool QMLWebEngineSVGGraphicsItem::load(const QByteArray &svgData, const QUrl &baseUrl)
{
    if(!m_impl->webEngineView)
    {
        LOG_WARNING() << LOGGING_CTX << "Error: WebEngineView component is null";
        return false;
    }

    m_impl->timer.stop();

    m_impl->svgRect = QRectF(QPointF(), defaultSvgSize());
    m_impl->setScaleFactor(1);

    m_impl->svgData = svgData;
    const QString svgDataString = XmlStreamReader::getDecodedString(svgData);

    QEventLoop loop;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QMetaObject::Connection connection = connect(m_impl->webEngineView, SIGNAL(loadingChanged(QWebEngineLoadingInfo)), &loop, SLOT(quit()));
#else
    QMetaObject::Connection connection = connect(m_impl->webEngineView, SIGNAL(loadingChanged(QQuickWebEngineLoadRequest*)), &loop, SLOT(quit()));
#endif
    QMetaObject::invokeMethod(m_impl->webEngineView, "loadHtml", Q_ARG(QString, svgDataString), Q_ARG(QUrl, baseUrl));
    loop.exec(QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers);
    disconnect(connection);
    if(!QQmlProperty(m_impl->webEngineView, QString::fromLatin1("loading")).read().toBool())
    {
        LOG_WARNING() << LOGGING_CTX << "Error: can't load content";
        return false;
    }

    if(!rootElementIsSvg())
    {
        LOG_WARNING() << LOGGING_CTX << "Error: not an SVG";
        return false;
    }

    m_impl->svgRect = detectSvgRect();
    m_impl->svgRect = QRectF(m_impl->svgRect.topLeft(), m_impl->svgRect.size().expandedTo(QSizeF(1, 1)));
    m_impl->setScaleFactor(1);

    const qreal maxImageDimension = std::min(MAX_IMAGE_DIMENSION, static_cast<qreal>(getMaxTextureSize()));
    m_impl->minScaleFactor = std::max(0.25, std::max(std::max(MIN_IMAGE_DIMENSION / m_impl->svgRect.width(), MIN_IMAGE_DIMENSION / m_impl->svgRect.height()), static_cast<qreal>(1)));
    m_impl->maxScaleFactor = std::min(5.0, std::min(maxImageDimension / m_impl->svgRect.width(), maxImageDimension / m_impl->svgRect.height()));
    if(m_impl->maxScaleFactor < m_impl->minScaleFactor)
    {
        LOG_WARNING() << LOGGING_CTX << "Error: too large SVG size, max =" << maxImageDimension << "x" << maxImageDimension;
        return false;
    }

    m_impl->timer.start();

    return true;
}

QRectF QMLWebEngineSVGGraphicsItem::boundingRect() const
{
    return m_impl->svgRect;
}

void QMLWebEngineSVGGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    const qreal scaleFactor = GraphicsItemUtils::GetDeviceScaleFactor(painter);
    const QRectF exposedRect = boundingRect().intersected(QRectFIntegerized(option->exposedRect));
    const qreal actualScaleFactor = std::max(std::min(scaleFactor, m_impl->maxScaleFactor), m_impl->minScaleFactor);
    m_impl->setScaleFactor(actualScaleFactor);
    const QRect scaledRect = QRectFIntegerized(QRectF(exposedRect.topLeft() * actualScaleFactor, exposedRect.size() * actualScaleFactor)).toRect();
    const QPixmap pixmap = QPixmap::fromImage(m_impl->quickView.grabWindow().copy(scaledRect));
    if(!pixmap.isNull())
    {
        painter->drawPixmap(exposedRect, pixmap, pixmap.rect());
    }
    else
    {
        painter->fillRect(boundingRect(), QBrush(Qt::black, Qt::SolidPattern));
        painter->fillRect(boundingRect(), QBrush(Qt::red, Qt::CrossPattern));
    }
}

QByteArray QMLWebEngineSVGGraphicsItem::getSvgData()
{
    return m_impl->svgData;
}

void QMLWebEngineSVGGraphicsItem::onUpdateRequested()
{
    update();
}
