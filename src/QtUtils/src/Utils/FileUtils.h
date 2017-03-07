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

#if !defined (QTUTILS_FILEUTILS_H_INCLUDED)
#define QTUTILS_FILEUTILS_H_INCLUDED

#include <cstddef>

class QString;

namespace FileUtils {

/// @brief Удаление указанного файла или директории в корзину
/// @attention Используется удаление без запроса. В случае отсутствия на целевой системе
///         корзины, либо если корзина программно отключена поведение этой функции строго
///         не специфицируется. Предполагаемое поведение - удаление файла мимо корзины.
/// @param[in] path - путь к файлу или директории
/// @param[out] errorDescription - текстовое описание ошибки в случае ее возникновения
/// @return - true в случае успешного удаления, false в случае ошибки
bool MoveToTrash(const QString &path, QString* errorDescription = NULL);

} // namespace FileUtils

#endif // QTUTILS_FILEUTILS_H_INCLUDED

