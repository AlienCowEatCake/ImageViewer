/*
   Copyright (C) 2011-2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (QTUTILS_MENUUTILS_H_INCLUDED)
#define QTUTILS_MENUUTILS_H_INCLUDED

#include <QtGlobal>

namespace MenuUtils {

#if defined (Q_OS_MAC)

/// @brief Remove (disable) the "Start Dictation..." menu item from the "Edit" menu
void DisableDictationMenuItem();

/// @brief Remove (disable) the "Emoji & Symbols" menu item from the "Edit" menu
void DisableCharacterPaletteMenuItem();

/// @brief Remove (don't allow) the "Show Tab Bar" menu item from the "View" menu, if supported
void DisableShowTabBarMenuItem();

/// @brief Remove (don't have) the "Enter Full Screen" menu item from the "View" menu
void DisableEnterFullScreenMenuItem();

#endif

} // namespace MenuUtils

#endif // QTUTILS_MENUUTILS_H_INCLUDED

