/*
   Copyright (C) 2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "ScreenUtils.h"

#include <QtGlobal>
#include <QRect>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#define USE_QSCREEN_IMPL
#endif

#if defined (USE_QSCREEN_IMPL)
#include <QGuiApplication>
#include <QScreen>
#else
#include <QApplication>
#include <QDesktopWidget>
#endif

Screen::Screen()
    : m_underlying(0)
{}

Screen::Screen(quintptr underlying)
    : m_underlying(underlying)
{}

Screen::Screen(const Screen &other)
    : m_underlying(other.m_underlying)
{}

Screen::~Screen()
{}

Screen &Screen::operator = (const Screen &other)
{
    m_underlying = other.m_underlying;
    return *this;
}

QRect Screen::geometry() const
{
#if defined (USE_QSCREEN_IMPL)
    const QScreen *screen = reinterpret_cast<QScreen*>(m_underlying);
    return screen ? screen->geometry() : QRect();
#else
    return qApp->desktop()->screenGeometry(static_cast<int>(m_underlying));
#endif
}

QRect Screen::availableGeometry() const
{
#if defined (USE_QSCREEN_IMPL)
    const QScreen *screen = reinterpret_cast<QScreen*>(m_underlying);
    return screen ? screen->availableGeometry() : QRect();
#else
    return qApp->desktop()->availableGeometry(static_cast<int>(m_underlying));
#endif
}

QList<Screen> ScreenProvider::screens()
{
    QList<Screen> result;
#if defined (USE_QSCREEN_IMPL)
    const QList<QScreen*> appScreens = qApp->screens();
    for(QList<QScreen*>::ConstIterator it = appScreens.constBegin(), itEnd = appScreens.constEnd(); it != itEnd; ++it)
        result.append(Screen(reinterpret_cast<quintptr>(*it)));
#else
    for(int i = 0, iEnd = qApp->desktop()->numScreens(); i < iEnd; ++i)
        result.append(Screen(static_cast<quintptr>(i)));
#endif
    return result;
}

Screen ScreenProvider::primaryScreen()
{
#if defined (USE_QSCREEN_IMPL)
    return Screen(reinterpret_cast<quintptr>(qApp->primaryScreen()));
#else
    return Screen(static_cast<quintptr>(qApp->desktop()->primaryScreen()));
#endif
}

