/*
   Copyright (C) 2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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
#include <QRect>
#include <QEvent>
#include <QList>
#include <QDateTime>
#if defined (RESTORABLE_GEOMETRY_HELPER_DEBUG)
#include <QDebug>
#endif

namespace {

const int MILLISECONDS_NUMBER_FOR_DROP_AFTER_WINDOW_STATE_CHANGE = 1500;
const int MAX_RECORDS_NUMBER_FOR_AUTO_SAVING_GEOMETRY = 100;
const int MAX_TRY_TO_CORRECTING_INVALID_GEOMETRY = 5;

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

    Impl(QWidget *window)
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
            {
                const QDateTime targetKey = QDateTime::currentDateTime().addMSecs(-MILLISECONDS_NUMBER_FOR_DROP_AFTER_WINDOW_STATE_CHANGE);
                while(!autoSavedGeometry.empty())
                {
                    if(autoSavedGeometry.last().first > targetKey)
                        autoSavedGeometry.removeLast();
                    else
                        break;
                }
            }
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

