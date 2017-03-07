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

#if !defined (QTUTILS_WORKAROUNDS_H_INCLUDED)
#define QTUTILS_WORKAROUNDS_H_INCLUDED

#include <QString>

namespace Workarounds {

/// @brief Исправить отображение локализованных шрифтов под Windows
/// @param[in] language - язык, для которого будет проводиться исправление
/// @note Функцию следует вызывать при смене языка
void FontsFix(const QString &language);

/// @brief Автоматически увеличить масштаб отображения для высоких DPI
/// @note Функцию следует вызывать перед созданием QApplication
/// @attention Актуально только для Qt 5.4+
void HighDPIFix();

/// @brief Определить, запущено ли приложение удаленно
/// @return true - удаленный запуск, false - иначе
bool IsRemoteSession();

/// @brief Переопределить неподдерживаемые QT_QPA_PLATFORMTHEME и QT_STYLE_OVERRIDE
void StyleFix();

} // Workarounds

#endif // QTUTILS_WORKAROUNDS_H_INCLUDED

