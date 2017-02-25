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

#if !defined (QTUTILS_THEMEUTILS_H_INCLUDED)
#define QTUTILS_THEMEUTILS_H_INCLUDED

#include <QIcon>
#include <QMenu>
#include <QString>
#include <QStringList>

namespace ThemeUtils {

/// @brief Функция для определения темная используемая тема виджета или нет
/// @param[in] widget - Виджет, для которого выполняется эта проверка
bool WidgetHasDarkTheme(const QWidget *widget);

/// @brief Создать масштабируемую иконку из нескольких разного размера
/// @param[in] defaultImagePath - Путь к иконке по умолчанию (может быть SVG)
/// @param[in] scaledImagePaths - Список путей иконкам разного размера (растр)
/// @return Масштабируемая иконка
QIcon CreateScalableIcon(const QString &defaultImagePath, const QStringList &scaledImagePaths);

/// @brief Типы иконок
enum IconTypes
{
    ICON_QT,
    ICON_ABOUT,
    ICON_HELP,
    ICON_AUTHORS,
    ICON_TEXT,
    ICON_SAVE,
    ICON_SAVE_AS,
    ICON_CLOSE,
    ICON_EXIT,
    ICON_NEW,
    ICON_NEW_WINDOW,
    ICON_OPEN,
    ICON_CUT,
    ICON_COPY,
    ICON_PASTE
};

/// @brief Функция для получения иконки
/// @param[in] type - Тип иконки (см. enum IconTypes)
/// @param[in] darkBackground - true, если иконка располагается на темном фоне
QIcon GetIcon(IconTypes type, bool darkBackground = false);

} // namespace ThemeUtils

#endif // QTUTILS_THEMEUTILS_H_INCLUDED

