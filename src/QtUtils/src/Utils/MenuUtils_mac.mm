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

#include "MenuUtils.h"

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

namespace MenuUtils {

// https://forum.qt.io/topic/60623/qt-5-4-2-os-x-10-11-el-capitan-how-to-remove-the-enter-full-screen-menu-item

/// @brief Remove (disable) the "Start Dictation..." menu item from the "Edit" menu
void DisableDictationMenuItem()
{
    [[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"NSDisabledDictationMenuItem"];
}

/// @brief Remove (disable) the "Emoji & Symbols" menu item from the "Edit" menu
void DisableCharacterPaletteMenuItem()
{
    [[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"NSDisabledCharacterPaletteMenuItem"];
}

/// @brief Remove (don't allow) the "Show Tab Bar" menu item from the "View" menu, if supported
void DisableShowTabBarMenuItem()
{
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_12_AND_LATER)
    if([NSWindow respondsToSelector:@selector(allowsAutomaticWindowTabbing)])
        NSWindow.allowsAutomaticWindowTabbing = NO;
#endif
}

/// @brief Remove (don't have) the "Enter Full Screen" menu item from the "View" menu
void DisableEnterFullScreenMenuItem()
{
    [[NSUserDefaults standardUserDefaults] setBool:NO forKey:@"NSFullScreenMenuItemEverywhere"];
}

} // namespace MenuUtils
