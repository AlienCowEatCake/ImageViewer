/*
   Copyright (C) 2018-2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "ThemeUtils.h"

#import <AppKit/AppKit.h>

namespace ThemeUtils {

/// @brief Функция для определения темная используемая тема системы или нет
bool SystemHasDarkTheme()
{
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_14_AND_LATER)
    if(@available(*, macOS 10.14))
    {
        NSAppearance *appearance = [NSApp effectiveAppearance];
        NSAppearanceName bestMatchedName = [appearance bestMatchFromAppearancesWithNames:@[NSAppearanceNameAqua, NSAppearanceNameDarkAqua]];
        if(bestMatchedName)
            return [bestMatchedName isEqualToString:NSAppearanceNameDarkAqua];
        return [[appearance name] hasSuffix:@"DarkAqua"];
    }
#endif
    return false;
}

} // namespace ThemeUtils

