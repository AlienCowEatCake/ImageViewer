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
#include <QStringList>
#include <QRegExp>
#include <QEventLoop>
#include <QTimer>

#include <QPainter>
#include <QRegion>
#include <QStyleOptionGraphicsItem>
#include <QOpenGLWidget>

#include <QWebEnginePage>
#include <QWebEngineView>

#include <QDebug>

namespace {

const int DEFAULT_SVG_SIZE = 512;
const int SVG_RENDERING_FPS = 30;

const qreal POINTS_PER_INCH = 72.0;
const qreal CM_PER_INCH     = 2.54;
const qreal MM_PER_INCH     = 25.4;
const qreal PICA_PER_INCH   = 6.0;
const qreal PIXELS_PER_INCH = 96.0;

QRectF QRectFIntegerized(const QRectF &rect)
{
    const qreal left = std::floor(rect.left());
    const qreal top = std::floor(rect.top());
    const qreal width = std::ceil(rect.width() + std::abs(rect.left() - left));
    const qreal height = std::ceil(rect.height() + std::abs(rect.top() - top));
    return QRectF(left, top, width, height);
}

qreal parseLength(const QString &str)
{
    const QString data = str.simplified().toLower();
    if(data.endsWith(QString::fromLatin1("px")))
        return static_cast<qreal>(data.left(data.length() - 2).toDouble());
    if(data.endsWith(QString::fromLatin1("pt")))
        return static_cast<qreal>(data.left(data.length() - 2).toDouble()) / POINTS_PER_INCH * PIXELS_PER_INCH;
    if(data.endsWith(QString::fromLatin1("in")))
        return static_cast<qreal>(data.left(data.length() - 2).toDouble()) * PIXELS_PER_INCH;
    if(data.endsWith(QString::fromLatin1("cm")))
        return static_cast<qreal>(data.left(data.length() - 2).toDouble()) / CM_PER_INCH * PIXELS_PER_INCH;
    if(data.endsWith(QString::fromLatin1("mm")))
        return static_cast<qreal>(data.left(data.length() - 2).toDouble()) / MM_PER_INCH * PIXELS_PER_INCH;
    if(data.endsWith(QString::fromLatin1("pc")))
        return static_cast<qreal>(data.left(data.length() - 2).toDouble()) / PICA_PER_INCH * PIXELS_PER_INCH;
    if(data.endsWith(QString::fromLatin1("em")))
        return 0; /// @todo
    if(data.endsWith(QString::fromLatin1("ex")))
        return 0; /// @todo
    if(data.endsWith(QString::fromLatin1("%")))
        return 0; /// @todo
    return static_cast<qreal>(data.toDouble());
}

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

    QRectF detectRect()
    {
#if 0
        const QRectF rect = QRectF(static_cast<qreal>(evalJS("document.rootElement.x.baseVal.value").toDouble()),
                                   static_cast<qreal>(evalJS("document.rootElement.y.baseVal.value").toDouble()),
                                   static_cast<qreal>(evalJS("document.rootElement.width.baseVal.value").toDouble()),
                                   static_cast<qreal>(evalJS("document.rootElement.height.baseVal.value").toDouble()));
        if(!rect.isEmpty())
            return QRectF(QPointF(), rect.size());

        const QRectF viewBox = QRectF(static_cast<qreal>(evalJS("document.rootElement.viewBox.baseVal.x").toDouble()),
                                      static_cast<qreal>(evalJS("document.rootElement.viewBox.baseVal.y").toDouble()),
                                      static_cast<qreal>(evalJS("document.rootElement.viewBox.baseVal.width").toDouble()),
                                      static_cast<qreal>(evalJS("document.rootElement.viewBox.baseVal.height").toDouble()));
        if(!viewBox.isEmpty())
            return QRectF(QPointF(), viewBox.size());
#else
        const QSizeF size = QSizeF(parseLength(evalJS("document.rootElement.getAttribute('width');").toString()),
                                   parseLength(evalJS("document.rootElement.getAttribute('height');").toString()));
        if(!size.isEmpty())
            return QRectF(QPointF(), size);

        const QStringList viewBoxData = evalJS("document.rootElement.getAttribute('viewBox');").toString()
                .split(QRegExp(QString::fromLatin1("\\s")), QString::SkipEmptyParts);
        const QRectF viewBox = (viewBoxData.size() == 4)
                ? QRectF(parseLength(viewBoxData.at(0)),
                         parseLength(viewBoxData.at(1)),
                         parseLength(viewBoxData.at(2)),
                         parseLength(viewBoxData.at(3)))
                : QRectF();
        if(!viewBox.isEmpty())
        {
            if(!size.width() && size.height())
            {
                const qreal aspectRatio = viewBox.width() / viewBox.height();
                return QRectF(0, 0, aspectRatio * size.height(), size.height());
            }
            if(size.width() && !size.height())
            {
                const qreal aspectRatio = viewBox.width() / viewBox.height();
                return QRectF(0, 0, size.width(), size.height() / aspectRatio);
            }
            return QRectF(QPointF(), viewBox.size());
        }
#endif

        const QRectF bBox(static_cast<qreal>(evalJS("document.rootElement.getBBox().x;").toDouble()),
                          static_cast<qreal>(evalJS("document.rootElement.getBBox().y;").toDouble()),
                          static_cast<qreal>(evalJS("document.rootElement.getBBox().width;").toDouble()),
                          static_cast<qreal>(evalJS("document.rootElement.getBBox().height;").toDouble()));
        if(!bBox.isEmpty())
            return QRectFIntegerized(bBox);

        return QRectF(0, 0, DEFAULT_SVG_SIZE, DEFAULT_SVG_SIZE);
    }

    QVariant evalJS(const char *scriptSource)
    {
        return syncExecutor.runJavaScript(QString::fromLatin1(scriptSource));
    }
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

    m_impl->svgRect = QRectF(0, 0, DEFAULT_SVG_SIZE, DEFAULT_SVG_SIZE);
    m_impl->view.resize(m_impl->svgRect.size().toSize());

    if(!m_impl->syncExecutor.setContent(svgData, QString::fromLatin1("image/svg+xml"), baseUrl))
    {
        qWarning() << "[QtWebEngineSVGGraphicsItem] Error: can't load content";
        return false;
    }

    const QString root = m_impl->evalJS("document.rootElement.nodeName").toString();
    if(root.isEmpty() || root.compare(QString::fromLatin1("svg"), Qt::CaseInsensitive))
    {
        qWarning() << "[QtWebEngineSVGGraphicsItem] Error: not an SVG";
        return false;
    }

    m_impl->svgRect = m_impl->detectRect();
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
