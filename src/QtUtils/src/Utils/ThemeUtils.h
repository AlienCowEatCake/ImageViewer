/*
   Copyright (C) 2011-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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
#include <QImage>
#include <QMenu>
#include <QString>
#include <QStringList>

namespace ThemeUtils {

/// @brief Считать стилизацию из файла и применить ее к QApplication
/// @param[in] filePath - Путь до QSS файла со стилями
/// @return true если стилизация успешно применена, false - иначе
bool LoadStyleSheet(const QString &filePath);

/// @brief Считать стилизацию из файлов и применить ее к QApplication
/// @param[in] filePaths - Список путей до QSS файлов со стилями
/// @return true если стилизация успешно применена, false - иначе
bool LoadStyleSheet(const QStringList &filePaths);

/// @brief Функция для определения темная используемая тема виджета или нет
/// @param[in] widget - Виджет, для которого выполняется эта проверка
bool WidgetHasDarkTheme(const QWidget *widget);

/// @brief Функция для определения темная используемая тема системы или нет
bool SystemHasDarkTheme();

/// @brief Создать масштабируемую иконку из нескольких разного размера
/// @param[in] defaultImagePath - Путь к иконке по умолчанию (может быть SVG)
/// @param[in] scaledImagePaths - Список путей иконкам разного размера (растр)
/// @return Масштабируемая иконка
QIcon CreateScalableIcon(const QString &defaultImagePath, const QStringList &scaledImagePaths = QStringList());

/// @brief Создать масштабируемую иконку из нескольких растровых разного размера
/// @param[in] scaledImagePaths - Список путей иконкам разного размера (только растр)
/// @param[in] invertPixels - Требуется ли инвертированное изображение вместо обычного
/// @return Масштабируемая иконка
QIcon CreateScalableIcon(const QStringList &scaledImagePaths, bool invertPixels = false);

/// @brief Create scalable icon from several images with different sizes
/// @param[in] scaledImages - List of several images with different sizes
/// @param[in] invertPixels - true if images should be inverted
/// @return Scalable icon
QIcon CreateScalableIcon(const QList<QImage> &scaledImages, bool invertPixels = false);

/// @brief Типы иконок
enum IconTypes
{
    ICON_APPLICATION_EXIT,
    ICON_DOCUMENT_NEW,
    ICON_DOCUMENT_OPEN,
    ICON_DOCUMENT_PRINT,
    ICON_DOCUMENT_PROPERTIES,
    ICON_DOCUMENT_SAVE,
    ICON_DOCUMENT_SAVE_AS,
    ICON_EDIT_COPY,
    ICON_EDIT_CUT,
    ICON_EDIT_DELETE,
    ICON_EDIT_PASTE,
    ICON_EDIT_PREFERENCES,
    ICON_GO_NEXT,
    ICON_GO_PREVIOUS,
    ICON_HELP_ABOUT,
    ICON_HELP_ABOUT_QT,
    ICON_HELP_AUTHORS,
    ICON_HELP_CONTENTS,
    ICON_HELP_LICENSE,
    ICON_MEDIA_PLAYBACK_START,
    ICON_MEDIA_PLAYBACK_STOP,
    ICON_MEDIA_SLIDESHOW,
    ICON_OBJECT_FLIP_HORIZONTAL,
    ICON_OBJECT_FLIP_VERTICAL,
    ICON_OBJECT_ROTATE_LEFT,
    ICON_OBJECT_ROTATE_RIGHT,
    ICON_VIEW_FULLSCREEN,
    ICON_VIEW_REFRESH,
    ICON_WINDOW_CLOSE,
    ICON_WINDOW_NEW,
    ICON_ZOOM_CUSTOM,
    ICON_ZOOM_FIT_BEST,
    ICON_ZOOM_IN,
    ICON_ZOOM_ORIGINAL,
    ICON_ZOOM_OUT
};

/// @brief Функция для получения иконки
/// @param[in] type - Тип иконки (см. enum IconTypes)
/// @param[in] darkBackground - true, если иконка располагается на темном фоне
QIcon GetIcon(IconTypes type, bool darkBackground = false);

/// @brief Get icon from current icon theme
/// @param[in] type - Type of icon (see enum IconTypes)
QIcon GetThemeIcon(IconTypes type);

} // namespace ThemeUtils

#endif // QTUTILS_THEMEUTILS_H_INCLUDED

