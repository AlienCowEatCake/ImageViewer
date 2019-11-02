/*
   Copyright (C) 2018-2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include <QEventLoop>
#include <QTimer>

#include <QQuickView>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QQmlProperty>

#include <QTextCodec>
#include <QXmlStreamReader>

#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLFunctions>

#include <QDebug>

#include "GraphicsItemUtils.h"

class QQuickWebEngineLoadRequest;

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

        qDebug() << "GL_MAX_TEXTURE_SIZE =" << maxTextureSize;
    }
    return maxTextureSize;
}

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
        QQmlProperty(webEngineView, QString::fromLatin1("zoomFactor")).write(scaleFactor);
    }

    QString detectEncoding(const QByteArray &svgData)
    {
        QXmlStreamReader reader(svgData);
        while(reader.readNext() != QXmlStreamReader::StartDocument && !reader.atEnd());
        return reader.documentEncoding().toString().simplified().toLower();
    }

    QQuickItem *createWebEngineView(QQuickView *quickView) const
    {
        QQmlEngine *engine = quickView->engine();
        if(!engine)
        {
            qWarning() << "[QMLWebEngineSVGGraphicsItem] Error: can't get QQmlEngine";
            return Q_NULLPTR;
        }

        QQmlComponent *component = new QQmlComponent(engine);
        component->setData(
                    "import QtQuick 2.0\n"
                    "import QtWebEngine 1.6\n"
                    "\n"
                    "WebEngineView {\n"
                    "    anchors.fill: parent\n"
                    "    backgroundColor: \"transparent\"\n"
                    "    settings.showScrollBars: false\n"
                    "}\n"
                    , QUrl());
        QQuickItem *webEngineView = qobject_cast<QQuickItem*>(component->create(engine->rootContext()));
        if(!webEngineView)
        {
            qWarning() << "[QMLWebEngineSVGGraphicsItem] Error: can't create WebEngineView component";
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

bool QMLWebEngineSVGGraphicsItem::load(const QByteArray &svgData, const QUrl &baseUrl)
{
    if(!m_impl->webEngineView)
    {
        qWarning() << "[QMLWebEngineSVGGraphicsItem] Error: WebEngineView component is null";
        return false;
    }

    m_impl->timer.stop();

    m_impl->svgRect = QRectF(QPointF(), defaultSvgSize());
    m_impl->setScaleFactor(1);

    m_impl->svgData = svgData;
    QString svgDataString;
    const QString encoding = m_impl->detectEncoding(svgData);
    if(!encoding.isEmpty())
        svgDataString = QTextCodec::codecForName(encoding.toLatin1())->toUnicode(svgData);
    else
        svgDataString = QString::fromUtf8(svgData);

    QEventLoop loop;
    QMetaObject::Connection connection = connect(m_impl->webEngineView, SIGNAL(loadingChanged(QQuickWebEngineLoadRequest*)), &loop, SLOT(quit()));
    QMetaObject::invokeMethod(m_impl->webEngineView, "loadHtml", Q_ARG(QString, svgDataString), Q_ARG(QUrl, baseUrl));
    loop.exec(QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers);
    disconnect(connection);
    if(!QQmlProperty(m_impl->webEngineView, QString::fromLatin1("loading")).read().toBool())
    {
        qWarning() << "[QMLWebEngineSVGGraphicsItem] Error: can't load content";
        return false;
    }

    if(!rootElementIsSvg())
    {
        qWarning() << "[QMLWebEngineSVGGraphicsItem] Error: not an SVG";
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
        qWarning() << "[QMLWebEngineSVGGraphicsItem] Error: too large SVG size, max =" << maxImageDimension << "x" << maxImageDimension;
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
