/*
   Copyright (C) 2017-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

   This file is part of the `QtUtils' library.

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

#include "RestorableGeometryHelper.h"

//#define RESTORABLE_GEOMETRY_HELPER_DEBUG

#include <utility>

#include <QWidget>
#include <QByteArray>
#include <QString>
#include <QStringList>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#else
#include <QRegExp>
#endif
#include <QRect>
#include <QRegion>
#include <QEvent>
#include <QList>
#include <QDateTime>
#if defined (RESTORABLE_GEOMETRY_HELPER_DEBUG)
#include <QDebug>
#endif

#include "ScreenUtils.h"

namespace {

const int MILLISECONDS_NUMBER_FOR_DROP_AFTER_WINDOW_STATE_CHANGE = 1500;
const int MILLISECONDS_NUMBER_FOR_DROP_RECENT_EVENTS = 500;
const int MAX_RECORDS_NUMBER_FOR_AUTO_SAVING_GEOMETRY = 100;
const int MAX_TRY_TO_CORRECTING_INVALID_GEOMETRY = 5;
const int SERIALIZER_FORMAT_VERSION = 1;

} // namespace

struct RestorableGeometryHelper::Impl : public QObject
{
    QWidget *window;
    bool isLastMaximized;
    bool isLastMinimized;
    bool isLastFullScreen;

    bool blocked;
    bool temporaryBlockedBySetGeometry;

    QRect normalGeometry;
    QList<std::pair<QDateTime, QRect> > autoSavedGeometry;

    explicit Impl(QWidget *window)
        : window(window)
        , isLastMaximized(window->isMaximized())
        , isLastMinimized(window->isMinimized())
        , isLastFullScreen(window->isFullScreen())
        , blocked(false)
        , temporaryBlockedBySetGeometry(false)
        , normalGeometry(window->normalGeometry())
    {
        window->installEventFilter(this);
    }

    bool isRealMaximized() const
    {
        return window->isMaximized() || isLastMaximized;
    }

    bool isRealMinimized() const
    {
        return window->isMinimized() || isLastMinimized;
    }

    bool isRealFullScreen() const
    {
        return window->isFullScreen() || isLastFullScreen;
    }

    void saveGeometry()
    {
        if(!blocked && !isRealMaximized() && !isRealMinimized() && !isRealFullScreen())
        {
            autoSavedGeometry.clear();
            normalGeometry = window->normalGeometry();
#if defined (RESTORABLE_GEOMETRY_HELPER_DEBUG)
            qDebug() << "[RGH] normalGeometry = " << normalGeometry;
#endif
        }
    }

    void restoreGeometry()
    {
        if(!blocked && !isRealMaximized() && !isRealMinimized() && !isRealFullScreen())
        {
            if(!autoSavedGeometry.isEmpty())
            {
                forceSetGeometry(autoSavedGeometry.last().second);
#if defined (RESTORABLE_GEOMETRY_HELPER_DEBUG)
                qDebug() << "[RGH] window->setGeometry (auto)" << autoSavedGeometry.last().second;
#endif
            }
            else
            {
                forceSetGeometry(normalGeometry);
#if defined (RESTORABLE_GEOMETRY_HELPER_DEBUG)
                qDebug() << "[RGH] window->setGeometry (normal)" << normalGeometry;
#endif
            }
        }
    }

    void autoSaveGeometry()
    {
        if(!blocked && !temporaryBlockedBySetGeometry && !isRealMaximized() && !isRealMinimized() && !isRealFullScreen())
        {
            autoSavedGeometry.append(std::make_pair(QDateTime::currentDateTime(), window->normalGeometry()));
#if defined (RESTORABLE_GEOMETRY_HELPER_DEBUG)
            qDebug() << "[RGH] autoSavedGeometry.append" << autoSavedGeometry.last().second;
#endif
        }

        while(autoSavedGeometry.size() > MAX_RECORDS_NUMBER_FOR_AUTO_SAVING_GEOMETRY)
            autoSavedGeometry.removeFirst();
    }

    void dropAutoSavedDataForMsec(int msec)
    {
        const QDateTime targetKey = QDateTime::currentDateTime().addMSecs(-msec);
        while(!autoSavedGeometry.empty())
        {
            if(autoSavedGeometry.last().first > targetKey)
                autoSavedGeometry.removeLast();
            else
                break;
        }
    }

    void forceSetGeometry(const QRect& geometry)
    {
        temporaryBlockedBySetGeometry = true;
        window->setGeometry(geometry);
        for(int i = 0; i < MAX_TRY_TO_CORRECTING_INVALID_GEOMETRY && window->normalGeometry() != geometry; i++)
        {
            // Qt 4.8.7 + OS X 10.9.5
#if defined (RESTORABLE_GEOMETRY_HELPER_DEBUG)
            qWarning() << "[RGH] Failed to set geometry! Trying to correct, attempt =" << i + 1;
#endif
            const QRect actualGeometry = window->normalGeometry();
            const QRect correctedGeometry(
                        geometry.topLeft() + (geometry.topLeft() - actualGeometry.topLeft()),
                        geometry.bottomRight() + (geometry.bottomRight() - actualGeometry.bottomRight())
                        );
            window->setGeometry(correctedGeometry);
        }
        temporaryBlockedBySetGeometry = false;
    }

    void overrideSavedGeometry(const QRect &newGeometry)
    {
        if(newGeometry.isEmpty())
            return;
        normalGeometry = newGeometry;
        autoSavedGeometry.clear();
#if defined (RESTORABLE_GEOMETRY_HELPER_DEBUG)
        qDebug() << "[RGH] overrideSavedGeometry" << newGeometry;
#endif
        restoreGeometry();
    }

    QRect getFallbackGeometry() const
    {
        const QRect primaryScreenGeometry = ScreenProvider::primaryScreen().availableGeometry();
        const QPoint newPos = primaryScreenGeometry.center() - QPoint(normalGeometry.width() / 2, normalGeometry.height() / 2);
        return QRect(newPos, normalGeometry.size());
    }

    bool eventFilter(QObject *object, QEvent *event)
    {
        switch(event->type())
        {
        case QEvent::Resize:
        case QEvent::Move:
            autoSaveGeometry();
            break;
        case QEvent::WindowStateChange:
            isLastMinimized = window->isMinimized();
            isLastMaximized = window->isMaximized();
            isLastFullScreen = window->isFullScreen();
            if(window->isMaximized() || window->isFullScreen() || window->isMinimized())
                dropAutoSavedDataForMsec(MILLISECONDS_NUMBER_FOR_DROP_AFTER_WINDOW_STATE_CHANGE);
            break;
        default:
            break;
        }
        return QObject::eventFilter(object, event);
    }
};


RestorableGeometryHelper::RestorableGeometryHelper(QWidget *window)
    : m_impl(new Impl(window->window()))
{}

RestorableGeometryHelper::~RestorableGeometryHelper()
{}

QByteArray RestorableGeometryHelper::serialize() const
{
    const QRect &normalGeometry = m_impl->normalGeometry;
    if(normalGeometry.isEmpty())
        return QByteArray();
#if defined (RESTORABLE_GEOMETRY_HELPER_DEBUG)
    qDebug() << "[RGH] serialize" << m_impl->normalGeometry;
#endif
    return QString::fromLatin1("FormatVersion:%1;NormalGeometry:(%2,%3 %4x%5)")
            .arg(SERIALIZER_FORMAT_VERSION)
            .arg(normalGeometry.x())
            .arg(normalGeometry.y())
            .arg(normalGeometry.width())
            .arg(normalGeometry.height())
            .toLatin1();
}

void RestorableGeometryHelper::deserialize(const QByteArray &data)
{
    const QString regExpString = QString::fromLatin1("FormatVersion:%1;NormalGeometry:\\((\\d+),(\\d+) (\\d+)x(\\d+)\\)").arg(SERIALIZER_FORMAT_VERSION);
    QRect newGeometry;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QRegularExpression regExp(regExpString);
    const QRegularExpressionMatch match = regExp.match(QString::fromLatin1(data));
    if(match.hasMatch())
        newGeometry = QRect(match.captured(1).toInt(), match.captured(2).toInt(), match.captured(3).toInt(), match.captured(4).toInt());
#else
    QRegExp regExp(regExpString);
    if(regExp.indexIn(QString::fromLatin1(data)) != -1)
        newGeometry = QRect(regExp.cap(1).toInt(), regExp.cap(2).toInt(), regExp.cap(3).toInt(), regExp.cap(4).toInt());
#endif
    QRegion availableRegion;
    const QList<Screen> screens = ScreenProvider::screens();
    for(QList<Screen>::ConstIterator it = screens.constBegin(), itEnd = screens.constEnd(); it != itEnd; ++it)
        availableRegion += it->availableGeometry();
    if(newGeometry.isEmpty() || availableRegion.intersected(newGeometry) != newGeometry || newGeometry.topLeft().isNull())
        newGeometry = m_impl->getFallbackGeometry();
#if defined (RESTORABLE_GEOMETRY_HELPER_DEBUG)
    qDebug() << "[RGH] deserialize" << newGeometry;
#endif
    m_impl->overrideSavedGeometry(newGeometry);
}

void RestorableGeometryHelper::saveGeometry()
{
    m_impl->saveGeometry();
}

void RestorableGeometryHelper::restoreGeometry()
{
    m_impl->restoreGeometry();
}

void RestorableGeometryHelper::block()
{
    m_impl->blocked = true;
}

void RestorableGeometryHelper::unblock()
{
    m_impl->blocked = false;
}

void RestorableGeometryHelper::skipRecentEvents()
{
    m_impl->dropAutoSavedDataForMsec(MILLISECONDS_NUMBER_FOR_DROP_RECENT_EVENTS);
}

