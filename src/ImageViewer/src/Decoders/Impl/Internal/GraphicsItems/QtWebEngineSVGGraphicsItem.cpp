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

#include <QDebug>

namespace {

const int SVG_RENDERING_FPS = 30;

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

    Impl()
        : syncExecutor(&view)
    {}
};

QtWebEngineSVGGraphicsItem::QtWebEngineSVGGraphicsItem(QGraphicsItem *parentItem)
    : QGraphicsObject(parentItem)
    , m_impl(new Impl)
{
    m_impl->view.winId();
    m_impl->view.setContextMenuPolicy(Qt::PreventContextMenu);
    m_impl->view.setAcceptDrops(false);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    m_impl->view.setStyleSheet(QString::fromLatin1("background: transparent; border: none;"));
    m_impl->view.page()->setBackgroundColor(Qt::transparent);
    if(QOpenGLWidget *openGLWidget = m_impl->view.findChild<QOpenGLWidget*>())
        openGLWidget->setAttribute(Qt::WA_AlwaysStackOnTop, false);
#endif

    connect(&m_impl->timer, SIGNAL(timeout()), this, SLOT(onUpdateRequested()));
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
    m_impl->view.resize(m_impl->svgRect.size().toSize());

    if(!m_impl->syncExecutor.setContent(svgData, QString::fromLatin1("image/svg+xml"), baseUrl))
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
    m_impl->view.resize(m_impl->svgRect.united(QRectF(0, 0, 1, 1)).size().toSize());

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
    Q_UNUSED(widget);
    const QRect exposedRect = QRectFIntegerized(option->exposedRect).toRect();
    const QPixmap pixmap = m_impl->view.grab(exposedRect);
    painter->drawPixmap(exposedRect, pixmap, pixmap.rect());
}

void QtWebEngineSVGGraphicsItem::onUpdateRequested()
{
    update();
}

QVariant QtWebEngineSVGGraphicsItem::evalJSImpl(const QString &scriptSource)
{
    return m_impl->syncExecutor.runJavaScript(scriptSource);
}
