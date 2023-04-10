/*
   Copyright (C) 2017-2023 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (QTUTILS_INFOUTILS_H_INCLUDED)
#define QTUTILS_INFOUTILS_H_INCLUDED

class QString;

namespace InfoUtils {

/// @brief Проверить текущую версию macOS
bool MacVersionGreatOrEqual(const int major, const int minor, const int patch = -1);

/// @brief Проверить текущую версию Windows
bool WinVersionGreatOrEqual(const int major, const int minor, const int build = -1);

/// @brief Получить человеко-читаемую информацию о системе
QString GetSystemDescription();

/// @brief Получить человеко-читаемую информацию о компиляторе
QString GetCompilerDescription();

} // namespace InfoUtils

#endif // QTUTILS_INFOUTILS_H_INCLUDED

