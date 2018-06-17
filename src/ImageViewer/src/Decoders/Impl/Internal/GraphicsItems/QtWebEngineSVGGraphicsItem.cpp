/*
   Copyright (C) 2018 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "QtWebEngineSVGGraphicsItem.h"

#include <cmath>
#include <algorithm>

#include <QUrl>
#include <QString>
#include <QByteArray>

#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include <QEventLoop>
#include <QTimer>

#include <QWebEnginePage>
#include <QWebEngineView>
#include <QOpenGLWidget>

#include <QXmlStreamReader>

#include <QDebug>

#include "GraphicsItemUtils.h"

namespace {

const int SVG_RENDERING_FPS = 25;

class SyncExecutor : public QObject
{
public:
    SyncExecutor(QWebEngineView *view)
        : m_view(view)
    {}

    bool setContent(const QByteArray &data, const QString &mimeType = QString(), const QUrl& baseUrl = QUrl())
    {
        m_setContentResult = false;
        QMetaObject::Connection connection = connect(m_view, &QWebEngineView::loadFinished, this, &SyncExecutor::onSetContentResult);
        m_view->setContent(data, mimeType, baseUrl);
        execLoop();
        QObject::disconnect(connection);
        return m_setContentResult;
    }

    QVariant runJavaScript(const QString &scriptSource)
    {
        m_runJavaScriptResult = QVariant();
        QWebEnginePage *page = m_view->page();
        page->runJavaScript(scriptSource, RunJavaScriptResultFunctor(*this));
        execLoop();
        return m_runJavaScriptResult;
    }

private:
    class RunJavaScriptResultFunctor
    {
    public:
        RunJavaScriptResultFunctor(SyncExecutor &executor)
            : m_executor(executor)
        {}

        void operator()(const QVariant &result)
        {
            m_executor.m_runJavaScriptResult = result;
            m_executor.m_loop.exit();
        }

    private:
        SyncExecutor &m_executor;
    };

    void execLoop()
    {
        m_loop.exec(QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers);
    }

    void onSetContentResult(bool result)
    {
        m_setContentResult = result;
        m_loop.exit();
    }

    QWebEngineView* m_view;
    QEventLoop m_loop;

    QVariant m_runJavaScriptResult;
    bool m_setContentResult;
};

} // namespace

struct QtWebEngineSVGGraphicsItem::Impl
{
    QWebEngineView view;
    QRectF svgRect;
    SyncExecutor syncExecutor;
    QTimer timer;
    bool renderProcessTerminated;

    Impl()
        : syncExecutor(&view)
        , renderProcessTerminated(false)
    {}

    void setScaleFactor(qreal scaleFactor)
    {
        view.resize((svgRect.united(QRectF(0, 0, 1, 1)).size() * scaleFactor).toSize());
        view.setZoomFactor(scaleFactor);
    }

    QString detectEncoding(const QByteArray &svgData)
    {
        QXmlStreamReader reader(svgData);
        while(reader.readNext() != QXmlStreamReader::StartDocument && !reader.atEnd());
        return reader.documentEncoding().toString().simplified().toLower();
    }
};

QtWebEngineSVGGraphicsItem::QtWebEngineSVGGraphicsItem(QGraphicsItem *parentItem)
    : QGraphicsObject(parentItem)
    , m_impl(new Impl)
{
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption, true);

    m_impl->view.winId();
    m_impl->view.setContextMenuPolicy(Qt::PreventContextMenu);
    m_impl->view.setAcceptDrops(false);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    m_impl->view.setStyleSheet(QString::fromLatin1("background: transparent; border: none;"));
    m_impl->view.page()->setBackgroundColor(Qt::transparent);
    if(QOpenGLWidget *openGLWidget = m_impl->view.findChild<QOpenGLWidget*>())
        openGLWidget->setAttribute(Qt::WA_AlwaysStackOnTop, false);
#endif

    connect(&m_impl->view, &QWebEngineView::renderProcessTerminated, this, &QtWebEngineSVGGraphicsItem::onRenderProcessTerminated);

    connect(&m_impl->timer, &QTimer::timeout, this, &QtWebEngineSVGGraphicsItem::onUpdateRequested);
    m_impl->timer.setSingleShot(false);
    m_impl->timer.setInterval(1000 / SVG_RENDERING_FPS);
}

QtWebEngineSVGGraphicsItem::~QtWebEngineSVGGraphicsItem()
{
    m_impl->timer.disconnect();
    m_impl->timer.stop();
}

bool QtWebEngineSVGGraphicsItem::load(const QByteArray &svgData, const QUrl &baseUrl)
{
    m_impl->timer.stop();

    m_impl->svgRect = QRectF(QPointF(), defaultSvgSize());
    m_impl->setScaleFactor(1);

    QString mimeType = QString::fromLatin1("image/svg+xml");
    const QString encoding = m_impl->detectEncoding(svgData);
    if(!encoding.isEmpty())
        mimeType.append(QString::fromLatin1(";charset=") + encoding);

    if(!m_impl->syncExecutor.setContent(svgData, mimeType, baseUrl))
    {
        qWarning() << "[QtWebEngineSVGGraphicsItem] Error: can't load content";
        return false;
    }

    if(!rootElementIsSvg())
    {
        qWarning() << "[QtWebEngineSVGGraphicsItem] Error: not an SVG";
        return false;
    }

    removeRootOverflowAttribute();

    m_impl->svgRect = detectSvgRect();
    m_impl->svgRect = QRectF(m_impl->svgRect.topLeft(), m_impl->svgRect.size().expandedTo(QSizeF(1, 1)));
    m_impl->setScaleFactor(1);

//    m_impl->page.mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
//    m_impl->page.mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);

    m_impl->timer.start();

    return true;
}

QRectF QtWebEngineSVGGraphicsItem::boundingRect() const
{
    return m_impl->svgRect;
}

void QtWebEngineSVGGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if(!m_impl->renderProcessTerminated)
    {
        Q_UNUSED(widget);
        const qreal scaleFactor = GraphicsItemUtils::GetDeviceScaleFactor(painter);
        const QRectF exposedRect = boundingRect().intersected(QRectFIntegerized(option->exposedRect));
        const qreal actualScaleFactor = std::max(std::min(scaleFactor, 5.0), 1.0);
        m_impl->setScaleFactor(actualScaleFactor);
        const QRect scaledRect = QRectFIntegerized(QRectF(exposedRect.topLeft() * actualScaleFactor, exposedRect.size() * actualScaleFactor)).toRect();
        const QPixmap pixmap = m_impl->view.grab(scaledRect);
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
    else
    {
        painter->fillRect(boundingRect(), QBrush(Qt::black, Qt::SolidPattern));
        painter->fillRect(boundingRect(), QBrush(Qt::red, Qt::DiagCrossPattern));
    }
}

void QtWebEngineSVGGraphicsItem::onUpdateRequested()
{
    update();
}

void QtWebEngineSVGGraphicsItem::onRenderProcessTerminated(int terminationStatus)
{
    m_impl->renderProcessTerminated = true;
    qWarning() << "[QtWebEngineSVGGraphicsItem] Error: render process is terminated, status =" << terminationStatus;
}

QVariant QtWebEngineSVGGraphicsItem::evalJSImpl(const QString &scriptSource)
{
    return m_impl->syncExecutor.runJavaScript(scriptSource);
}
