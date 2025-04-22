/*
   Copyright (C) 2017-2025 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (QTUTILS_WINDOWUTILS_MAC_H_INCLUDED)
#define QTUTILS_WINDOWUTILS_MAC_H_INCLUDED

#include "ObjectiveCFwd.h"

class QWidget;

OBJC_CLASS(NSWindow);

namespace WindowUtils {

/// @brief Get NSWindow for widget
NSWindow *GetNativeWindow(QWidget* widget);

/// @brief Switch to native FullScreen mode
void ToggleFullScreenMode(QWidget* window);

} // namespace WindowUtils

#endif // QTUTILS_WINDOWUTILS_MAC_H_INCLUDED

