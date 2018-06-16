/*
   Copyright (C) 2017-2018 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "WindowUtils_mac.h"

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#include <AvailabilityMacros.h>

#include <QWidget>

#include "InfoUtils.h"
#include "ObjectiveCUtils.h"

namespace WindowUtils {

/// @brief Получить NSWindow для виджета
NSWindow *GetNativeWindow(QWidget *widget)
{
    if(!widget)
        return nil;
    QWidget *qtWindow = widget->window();
    if(!qtWindow)
        return nil;
    NSView *windowView = reinterpret_cast<NSView*>(qtWindow->winId());
    if(!windowView)
        return nil;
    return [windowView window];
}

/// @brief По возможности нативно переключить режим FullScreen
void ToggleFullScreenMode(QWidget* window)
{
    AUTORELEASE_POOL;

#if defined (AVAILABLE_MAC_OS_X_VERSION_10_7_AND_LATER)
    if(InfoUtils::MacVersionGreatOrEqual(10, 7))
    {
        NSWindow *nsWindow = GetNativeWindow(window);
        [nsWindow toggleFullScreen:nil];
        return;
    }
#endif

    QWidget *qtWindow = window->window();
    if(qtWindow->isFullScreen())
    {
        qtWindow->setWindowState(qtWindow->windowState() & ~Qt::WindowFullScreen);
        qtWindow->showNormal();
    }
    else
    {
        qtWindow->showFullScreen();
        qtWindow->setWindowState(qtWindow->windowState() | Qt::WindowFullScreen);
    }
}

} // namespace WindowUtils

