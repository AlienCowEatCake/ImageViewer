/*
   Copyright (C) 2011-2025 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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
#include <QList>
#include <QMenu>
#include <QString>
#include <QStringList>

namespace ThemeUtils {

/// @brief Read style sheet from QSS file and apply to QApplication
/// @param[in] filePath - Path to QSS file
/// @return true if style sheet was read and applied, false - otherwise
bool LoadStyleSheet(const QString &filePath);

/// @brief Read style sheets from QSS files and apply to QApplication
/// @param[in] filePaths - List of paths to QSS files
/// @return true if style sheets was read and applied, false - otherwise
bool LoadStyleSheet(const QStringList &filePaths);

/// @brief Reinitializes appearance of given widget
/// @param[in] widget - widget which appearance should be reinitialized
void RepolishWidget(QWidget *widget);

/// @brief Reinitializes appearance of given widget with all its children (recursive search)
/// @param[in] widget - widget which appearance should be reinitialized
void RepolishWidgetRecursively(QWidget *widget);

/// @brief Check if widget theme is dark or not
/// @param[in] widget - Widget which should be checked
bool WidgetHasDarkTheme(const QWidget *widget);

/// @brief Check if system theme is dark or not
bool SystemHasDarkTheme();

/// @brief Get list of default image sizes for rasterized icons
/// @return List of default image sizes for rasterized icons
QList<int> GetDefaultIconSizes();

/// @brief Create scalable icon from several images with different sizes
/// @param[in] defaultImagePath - Path to default icon (raster or SVG)
/// @param[in] scaledImagePaths - List of paths to images with different sizes (raster only)
/// @return Scalable icon
QIcon CreateScalableIcon(const QString &defaultImagePath, const QStringList &scaledImagePaths = QStringList());

/// @brief Create scalable icon from several images with different sizes
/// @param[in] scaledImagePaths - List of paths to images with different sizes (raster only)
/// @param[in] invertPixels - true if images should be inverted
/// @return Scalable icon
QIcon CreateScalableIcon(const QStringList &scaledImagePaths, bool invertPixels = false);

/// @brief Create scalable icon from several images with different sizes
/// @param[in] scaledImages - List of several images with different sizes
/// @param[in] invertPixels - true if images should be inverted
/// @return Scalable icon
QIcon CreateScalableIcon(const QList<QImage> &scaledImages, bool invertPixels = false);

/// @brief Icon types
enum IconTypes
{
    ICON_APPLICATION_EXIT,
    ICON_DOCUMENT_NEW,
    ICON_DOCUMENT_OPEN,
    ICON_DOCUMENT_OPEN_WITH,
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
    ICON_SYNC_SYNCHRONIZING,
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

/// @brief Get icon from default QtUtils icon theme
/// @param[in] type - Type of icon (see enum IconTypes)
/// @param[in] darkBackground - true if image should be placed on dark background
QIcon GetIcon(IconTypes type, bool darkBackground = false);

/// @brief Get icon from current icon theme
/// @param[in] type - Type of icon (see enum IconTypes)
QIcon GetThemeIcon(IconTypes type);

#if defined (Q_OS_WIN)
QImage GetWinSystemImage(IconTypes type, const QSize &size);
#endif

} // namespace ThemeUtils

#endif // QTUTILS_THEMEUTILS_H_INCLUDED

