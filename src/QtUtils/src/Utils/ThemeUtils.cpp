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

#include "ThemeUtils.h"

#include <cassert>

#include <QApplication>
#include <QStyle>
#include <QStyleOption>
#include <QPixmap>
#include <QImage>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

#if defined (Q_OS_WIN)
#include <windows.h>
#endif

#include "InfoUtils.h"

#if defined (DocumentProperties)
#undef DocumentProperties
#endif

namespace ThemeUtils {

namespace {

void AddPixmapToIcon(QIcon &icon, const QPixmap &pixmap)
{
    QStyleOption opt(0);
    opt.palette = QApplication::palette();
#define ADD_PIXMAP(MODE) icon.addPixmap(QApplication::style()->generatedIconPixmap(MODE, pixmap, &opt), MODE)
    ADD_PIXMAP(QIcon::Normal);
    ADD_PIXMAP(QIcon::Disabled);
    ADD_PIXMAP(QIcon::Active);
    ADD_PIXMAP(QIcon::Selected);
#undef ADD_PIXMAP
}

int Lightness(const QColor &color)
{
#if (QT_VERSION >= QT_VERSION_CHECK(4, 6, 0))
    return color.lightness();
#else
    const QColor rgb = color.toRgb();
    const int r = rgb.red();
    const int g = rgb.green();
    const int b = rgb.blue();
    const int cmax = qMax(qMax(r, g), b);
    const int cmin = qMin(qMin(r, g), b);
    const int l = (cmax + cmin) / 2;
    return l;
#endif
}

bool IsDarkTheme(const QPalette &palette, QPalette::ColorRole windowRole, QPalette::ColorRole windowTextRole)
{
    // https://www.qt.io/blog/dark-mode-on-windows-11-with-qt-6.5
    return Lightness(palette.color(windowTextRole)) > Lightness(palette.color(windowRole));
}

QIcon GetQtLogo()
{
    const QStringList knownPaths = QStringList()
            << QString::fromLatin1(":/trolltech/qmessagebox/images/qtlogo-64.png")
            << QString::fromLatin1(":/qt-project.org/qmessagebox/images/qtlogo-64.png");
    for(QStringList::ConstIterator it = knownPaths.constBegin(); it != knownPaths.constEnd(); ++it)
    {
        QFileInfo info(*it);
        if(info.exists())
            return QIcon(info.filePath());
    }
    assert(false);
    return QIcon();
}

QIcon GetIconFromTheme(const QStringList &namesList)
{
#if (QT_VERSION >= QT_VERSION_CHECK(4, 6, 0))
    for(QStringList::ConstIterator it = namesList.begin(); it != namesList.end(); ++it)
    {
        if(QIcon::hasThemeIcon(*it))
        {
            QIcon icon = QIcon::fromTheme(*it, QIcon());
            if(!icon.isNull())
                return icon;
        }
    }
#else
    Q_UNUSED(namesList);
#endif
    return QIcon();
}

} // namespace

/// @brief Считать стилизацию из файла и применить ее к QApplication
/// @param[in] filePath - Путь до QSS файла со стилями
/// @return true если стилизация успешно применена, false - иначе
bool LoadStyleSheet(const QString &filePath)
{
    return LoadStyleSheet(QStringList() << filePath);
}

/// @brief Считать стилизацию из файлов и применить ее к QApplication
/// @param[in] filePaths - Список путей до QSS файлов со стилями
/// @return true если стилизация успешно применена, false - иначе
bool LoadStyleSheet(const QStringList &filePaths)
{
    bool status = true;
    QString allStyles;
    for(QStringList::ConstIterator it = filePaths.constBegin(), itEnd = filePaths.constEnd(); it != itEnd; ++it)
    {
        QFile styleFile(*it);
        if(styleFile.open(QIODevice::ReadOnly))
            allStyles.append(QTextStream(&styleFile).readAll());
        else
            status = false;
    }
    qApp->setStyleSheet(allStyles);
    return status;
}

/// @brief Функция для определения темная используемая тема виджета или нет
/// @param[in] widget - Виджет, для которого выполняется эта проверка
bool WidgetHasDarkTheme(const QWidget *widget)
{
    return IsDarkTheme(widget->palette(), widget->backgroundRole(), widget->foregroundRole());
}

#if !defined (Q_OS_MAC)
/// @brief Функция для определения темная используемая тема системы или нет
bool SystemHasDarkTheme()
{
#if defined (Q_OS_WIN)
    // https://github.com/ysc3839/win32-darkmode/blob/master/win32-darkmode
    HMODULE library = ::LoadLibraryA("uxtheme.dll");
    if(library)
    {
        // https://github.com/mintty/mintty/issues/983
        // https://github.com/mintty/mintty/pull/984
        // https://github.com/mintty/mintty/blob/eeaaed8/src/winmain.c
//        // 1903 18362
//        if(InfoUtils::WinVersionGreatOrEqual(10, 0, 18362))
//        {
//            typedef bool(WINAPI *ShouldSystemUseDarkMode_t)(); // ordinal 138
//            ShouldSystemUseDarkMode_t ShouldSystemUseDarkMode_f = (ShouldSystemUseDarkMode_t)(::GetProcAddress(library, MAKEINTRESOURCEA(138)));
//            if(ShouldSystemUseDarkMode_f)
//            {
//                bool result = !!ShouldSystemUseDarkMode_f();
//                ::FreeLibrary(library);
//                return result;
//            }
//        }

        // 1809 17763
        if(InfoUtils::WinVersionGreatOrEqual(10, 0, 17763))
        {
            typedef bool(WINAPI *ShouldAppsUseDarkMode_t)(); // ordinal 132
            ShouldAppsUseDarkMode_t ShouldAppsUseDarkMode_f = (ShouldAppsUseDarkMode_t)(::GetProcAddress(library, MAKEINTRESOURCEA(132)));
            if(ShouldAppsUseDarkMode_f)
            {
                bool result = !!ShouldAppsUseDarkMode_f();
                ::FreeLibrary(library);
                return result;
            }
        }

        ::FreeLibrary(library);
    }
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
    const QPalette::ColorRole backgroundRole = QPalette::Window;
    const QPalette::ColorRole foregroundRole = QPalette::WindowText;
#else
    const QPalette::ColorRole backgroundRole = QPalette::Background;
    const QPalette::ColorRole foregroundRole = QPalette::Foreground;
#endif
    return qApp ? IsDarkTheme(qApp->palette(), backgroundRole, foregroundRole) : false;
}
#endif

/// @brief Создать масштабируемую иконку из нескольких разного размера
/// @param[in] defaultImagePath - Путь к иконке по умолчанию (может быть SVG)
/// @param[in] scaledImagePaths - Список путей иконкам разного размера (растр)
/// @return Масштабируемая иконка
QIcon CreateScalableIcon(const QString &defaultImagePath, const QStringList &scaledImagePaths)
{
    QIcon result(defaultImagePath);
    for(QStringList::ConstIterator it = scaledImagePaths.begin(), itEnd = scaledImagePaths.end(); it != itEnd; ++it)
    {
        const QPixmap pixmap(*it);
        if(!pixmap.isNull())
            AddPixmapToIcon(result, pixmap);
        else
            qWarning() << "[ThemeUtils::CreateScalableIcon]: Unable to load pixmap" << *it;
    }
    return result;
}

/// @brief Создать масштабируемую иконку из нескольких растровых разного размера
/// @param[in] scaledImagePaths - Список путей иконкам разного размера (только растр)
/// @param[in] invertPixels - Требуется ли инвертированное изображение вместо обычного
/// @return Масштабируемая иконка
QIcon CreateScalableIcon(const QStringList &scaledImagePaths, bool invertPixels)
{
    QIcon result;
    for(QStringList::ConstIterator it = scaledImagePaths.begin(), itEnd = scaledImagePaths.end(); it != itEnd; ++it)
    {
        QImage image(*it);
        if(!image.isNull())
        {
            if(invertPixels)
            {
                image = image.convertToFormat(QImage::Format_ARGB32);
                image.invertPixels(QImage::InvertRgb);
            }
            AddPixmapToIcon(result, QPixmap::fromImage(image));
        }
        else
        {
            qWarning() << "[ThemeUtils::CreateScalableIcon]: Unable to load image" << *it;
        }
    }
    return result;
}

/// @brief Функция для получения иконки
/// @param[in] type - Тип иконки (см. enum IconTypes)
/// @param[in] darkBackground - true, если иконка располагается на темном фоне
QIcon GetIcon(IconTypes type, bool darkBackground)
{
    const QString iconNameTemplate = QString::fromLatin1(":/icons/modern/%1_%2.png");
    switch(type)
    {
#define ADD_NAMED_ICON_CASE(ICON_TYPE, ICON_NAME) \
    case ICON_TYPE: \
    { \
        const QString iconName = QString::fromLatin1(ICON_NAME).toLower(); \
        const QString iconTemplate = iconNameTemplate.arg(iconName); \
        const QStringList rasterPixmaps = QStringList() << iconTemplate.arg(16) << iconTemplate.arg(32); \
        return CreateScalableIcon(rasterPixmaps, darkBackground); \
    }
#define ADD_ICON_CASE(ICON_TYPE) ADD_NAMED_ICON_CASE(ICON_TYPE, #ICON_TYPE)
    ADD_ICON_CASE(ICON_APPLICATION_EXIT)
    ADD_ICON_CASE(ICON_DOCUMENT_NEW)
    ADD_ICON_CASE(ICON_DOCUMENT_OPEN)
    ADD_ICON_CASE(ICON_DOCUMENT_PRINT)
    ADD_NAMED_ICON_CASE(ICON_DOCUMENT_PROPERTIES, "icon_help_about")
    ADD_ICON_CASE(ICON_DOCUMENT_SAVE)
    ADD_ICON_CASE(ICON_DOCUMENT_SAVE_AS)
    ADD_ICON_CASE(ICON_EDIT_COPY)
    ADD_ICON_CASE(ICON_EDIT_CUT)
    ADD_ICON_CASE(ICON_EDIT_DELETE)
    ADD_ICON_CASE(ICON_EDIT_PASTE)
    ADD_ICON_CASE(ICON_EDIT_PREFERENCES)
    ADD_ICON_CASE(ICON_GO_NEXT)
    ADD_ICON_CASE(ICON_GO_PREVIOUS)
    ADD_ICON_CASE(ICON_HELP_ABOUT)
    ADD_ICON_CASE(ICON_HELP_ABOUT_QT)
    ADD_ICON_CASE(ICON_HELP_AUTHORS)
    ADD_ICON_CASE(ICON_HELP_CONTENTS)
    ADD_ICON_CASE(ICON_HELP_LICENSE)
    ADD_ICON_CASE(ICON_MEDIA_PLAYBACK_START)
    ADD_ICON_CASE(ICON_MEDIA_PLAYBACK_STOP)
    ADD_ICON_CASE(ICON_OBJECT_FLIP_HORIZONTAL)
    ADD_ICON_CASE(ICON_OBJECT_FLIP_VERTICAL)
    ADD_ICON_CASE(ICON_OBJECT_ROTATE_LEFT)
    ADD_ICON_CASE(ICON_OBJECT_ROTATE_RIGHT)
    ADD_ICON_CASE(ICON_VIEW_FULLSCREEN)
    ADD_ICON_CASE(ICON_VIEW_REFRESH)
    ADD_ICON_CASE(ICON_WINDOW_CLOSE)
    ADD_ICON_CASE(ICON_WINDOW_NEW)
    ADD_ICON_CASE(ICON_ZOOM_CUSTOM)
    ADD_ICON_CASE(ICON_ZOOM_FIT_BEST)
    ADD_ICON_CASE(ICON_ZOOM_IN)
    ADD_ICON_CASE(ICON_ZOOM_ORIGINAL)
    ADD_ICON_CASE(ICON_ZOOM_OUT)
#undef ADD_ICON_CASE
#undef ADD_NAMED_ICON_CASE
    }
    return QIcon();
}

/// @brief Get icon from current icon theme
/// @param[in] type - Type of icon (see enum IconTypes)
QIcon GetThemeIcon(IconTypes type)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 7, 0))
    switch(type)
    {
#define SKIP_ICON_CASE(ICON_TYPE) case ICON_TYPE: break;
#define ADD_ICON_CASE(ICON_TYPE, ICON_NAME) \
    case ICON_TYPE: \
    { \
        const QIcon::ThemeIcon iconName = QIcon::ThemeIcon::ICON_NAME; \
        if(QIcon::hasThemeIcon(iconName)) \
        { \
            const QIcon icon = QIcon::fromTheme(iconName, QIcon()); \
            if(!icon.isNull()) \
                return icon; \
        } \
        break; \
    }
    ADD_ICON_CASE(ICON_APPLICATION_EXIT, ApplicationExit)
    ADD_ICON_CASE(ICON_DOCUMENT_NEW, DocumentNew)
    ADD_ICON_CASE(ICON_DOCUMENT_OPEN, DocumentOpen)
    ADD_ICON_CASE(ICON_DOCUMENT_PRINT, DocumentPrint)
    ADD_ICON_CASE(ICON_DOCUMENT_PROPERTIES, DocumentProperties)
    ADD_ICON_CASE(ICON_DOCUMENT_SAVE, DocumentSave)
    ADD_ICON_CASE(ICON_DOCUMENT_SAVE_AS, DocumentSaveAs)
    ADD_ICON_CASE(ICON_EDIT_COPY, EditCopy)
    ADD_ICON_CASE(ICON_EDIT_CUT, EditCut)
    ADD_ICON_CASE(ICON_EDIT_DELETE, EditDelete)
    ADD_ICON_CASE(ICON_EDIT_PASTE, EditPaste)
    SKIP_ICON_CASE(ICON_EDIT_PREFERENCES)
    ADD_ICON_CASE(ICON_GO_NEXT, GoNext)
    ADD_ICON_CASE(ICON_GO_PREVIOUS, GoPrevious)
    ADD_ICON_CASE(ICON_HELP_ABOUT, HelpAbout)
    SKIP_ICON_CASE(ICON_HELP_ABOUT_QT)
    SKIP_ICON_CASE(ICON_HELP_AUTHORS)
    SKIP_ICON_CASE(ICON_HELP_CONTENTS)
    SKIP_ICON_CASE(ICON_HELP_LICENSE)
    ADD_ICON_CASE(ICON_MEDIA_PLAYBACK_START, MediaPlaybackStart)
    ADD_ICON_CASE(ICON_MEDIA_PLAYBACK_STOP, MediaPlaybackStop)
    SKIP_ICON_CASE(ICON_OBJECT_FLIP_HORIZONTAL)
    SKIP_ICON_CASE(ICON_OBJECT_FLIP_VERTICAL)
    ADD_ICON_CASE(ICON_OBJECT_ROTATE_RIGHT, ObjectRotateRight)
    ADD_ICON_CASE(ICON_OBJECT_ROTATE_LEFT, ObjectRotateLeft)
    ADD_ICON_CASE(ICON_VIEW_FULLSCREEN, ViewFullscreen)
    ADD_ICON_CASE(ICON_VIEW_REFRESH, ViewRefresh)
    ADD_ICON_CASE(ICON_WINDOW_CLOSE, WindowClose)
    ADD_ICON_CASE(ICON_WINDOW_NEW, WindowNew)
    SKIP_ICON_CASE(ICON_ZOOM_CUSTOM)
    ADD_ICON_CASE(ICON_ZOOM_FIT_BEST, ZoomFitBest)
    ADD_ICON_CASE(ICON_ZOOM_IN, ZoomIn)
    SKIP_ICON_CASE(ICON_ZOOM_ORIGINAL)
    ADD_ICON_CASE(ICON_ZOOM_OUT, ZoomOut)
#undef ADD_ICON_CASE
#undef SKIP_ICON_CASE
    }
#endif

    switch(type)
    {
#define ADD_ICON_CASE(ICON_TYPE, ICON) \
    case ICON_TYPE: \
    { \
        QIcon icon = ICON; \
        if(!icon.isNull()) \
            return icon; \
        break; \
    }
#define ADD_THEMED_ICON_CASE(ICON_TYPE, ICON_NAMES_LIST) ADD_ICON_CASE(ICON_TYPE, GetIconFromTheme(ICON_NAMES_LIST))
    ADD_THEMED_ICON_CASE(ICON_APPLICATION_EXIT, QStringList()
            << QString::fromLatin1("application-exit")
            << QString::fromLatin1("gtk-quit")
            << QString::fromLatin1("stock_exit")
            << QString::fromLatin1("exit")
            )
    ADD_THEMED_ICON_CASE(ICON_DOCUMENT_NEW, QStringList()
            << QString::fromLatin1("document-new")
            << QString::fromLatin1("gtk-new")
            )
    ADD_THEMED_ICON_CASE(ICON_DOCUMENT_OPEN, QStringList()
            << QString::fromLatin1("document-open")
            << QString::fromLatin1("gtk-open")
            )
    ADD_THEMED_ICON_CASE(ICON_DOCUMENT_PRINT, QStringList()
            << QString::fromLatin1("document-print")
            << QString::fromLatin1("gtk-print")
            )
    ADD_THEMED_ICON_CASE(ICON_DOCUMENT_PROPERTIES, QStringList()
            << QString::fromLatin1("document-properties")
            << QString::fromLatin1("gtk-properties")
            )
    ADD_THEMED_ICON_CASE(ICON_DOCUMENT_SAVE, QStringList()
            << QString::fromLatin1("document-save")
            << QString::fromLatin1("gtk-save")
            << QString::fromLatin1("stock_save")
            )
    ADD_THEMED_ICON_CASE(ICON_DOCUMENT_SAVE_AS, QStringList()
            << QString::fromLatin1("document-save-as")
            << QString::fromLatin1("gtk-save-as")
            << QString::fromLatin1("stock_save-as")
            )
    ADD_THEMED_ICON_CASE(ICON_EDIT_COPY, QStringList()
            << QString::fromLatin1("edit-copy")
            << QString::fromLatin1("gtk-copy")
            )
    ADD_THEMED_ICON_CASE(ICON_EDIT_CUT, QStringList()
            << QString::fromLatin1("edit-cut")
            << QString::fromLatin1("gtk-cut")
            )
    ADD_THEMED_ICON_CASE(ICON_EDIT_DELETE, QStringList()
            << QString::fromLatin1("edit-delete")
            << QString::fromLatin1("gtk-delete")
            )
    ADD_THEMED_ICON_CASE(ICON_EDIT_PASTE, QStringList()
            << QString::fromLatin1("edit-paste")
            << QString::fromLatin1("gtk-paste")
            )
    ADD_THEMED_ICON_CASE(ICON_EDIT_PREFERENCES, QStringList()
            << QString::fromLatin1("edit-preferences")
            << QString::fromLatin1("gtk-preferences")
            << QString::fromLatin1("preferences-system")
            )
    ADD_THEMED_ICON_CASE(ICON_GO_NEXT, QStringList()
            << QString::fromLatin1("go-next")
            << QString::fromLatin1("gtk-go-forward")
            )
    ADD_THEMED_ICON_CASE(ICON_GO_PREVIOUS, QStringList()
            << QString::fromLatin1("go-previous")
            << QString::fromLatin1("gtk-go-back")
            )
    ADD_THEMED_ICON_CASE(ICON_HELP_ABOUT, QStringList()
            << QString::fromLatin1("help-about")
            << QString::fromLatin1("gtk-about")
            << QString::fromLatin1("stock_about")
            << QString::fromLatin1("gnome-about-logo")
            )
    ADD_ICON_CASE(ICON_HELP_ABOUT_QT, GetQtLogo())
    ADD_THEMED_ICON_CASE(ICON_HELP_AUTHORS, QStringList()
            << QString::fromLatin1("stock_people")
            << QString::fromLatin1("emblem-people")
            << QString::fromLatin1("authors")
            << QString::fromLatin1("text-x-authors")
            << QString::fromLatin1("gnome-mime-text-x-authors")
            )
    ADD_THEMED_ICON_CASE(ICON_HELP_CONTENTS, QStringList()
            << QString::fromLatin1("help-contents")
            << QString::fromLatin1("gtk-help")
            << QString::fromLatin1("stock_help")
            << QString::fromLatin1("help-browser")
            << QString::fromLatin1("stock_help-book")
            << QString::fromLatin1("gnome-help")
            << QString::fromLatin1("browser-help")
            << QString::fromLatin1("help")
            )
    ADD_THEMED_ICON_CASE(ICON_HELP_LICENSE, QStringList()
            << QString::fromLatin1("text-x-generic")
            << QString::fromLatin1("x-office-document")
            << QString::fromLatin1("gnome-mime-application-x-applix-word")
            << QString::fromLatin1("gnome-mime-application-msword")
            << QString::fromLatin1("application-msword")
            )
    ADD_THEMED_ICON_CASE(ICON_MEDIA_PLAYBACK_START, QStringList()
            << QString::fromLatin1("media-playback-start")
            << QString::fromLatin1("gtk-media-play")
            )
    ADD_THEMED_ICON_CASE(ICON_MEDIA_PLAYBACK_STOP, QStringList()
            << QString::fromLatin1("media-playback-stop")
            << QString::fromLatin1("gtk-media-stop")
            )
    ADD_THEMED_ICON_CASE(ICON_VIEW_FULLSCREEN, QStringList()
            << QString::fromLatin1("view-fullscreen")
            << QString::fromLatin1("gtk-fullscreen")
            )
    ADD_THEMED_ICON_CASE(ICON_VIEW_REFRESH, QStringList()
            << QString::fromLatin1("view-refresh")
            << QString::fromLatin1("gtk-refresh")
            )
    ADD_THEMED_ICON_CASE(ICON_OBJECT_ROTATE_LEFT, QStringList()
            << QString::fromLatin1("object-rotate-left")
            )
    ADD_THEMED_ICON_CASE(ICON_OBJECT_ROTATE_RIGHT, QStringList()
            << QString::fromLatin1("object-rotate-right")
            )
    ADD_THEMED_ICON_CASE(ICON_OBJECT_FLIP_HORIZONTAL, QStringList()
            << QString::fromLatin1("object-flip-horizontal")
            )
    ADD_THEMED_ICON_CASE(ICON_OBJECT_FLIP_VERTICAL, QStringList()
            << QString::fromLatin1("object-flip-vertical")
            )
    ADD_THEMED_ICON_CASE(ICON_WINDOW_CLOSE, QStringList()
            << QString::fromLatin1("window-close")
            << QString::fromLatin1("gtk-close")
            << QString::fromLatin1("stock_close")
            )
    ADD_THEMED_ICON_CASE(ICON_WINDOW_NEW, QStringList()
            << QString::fromLatin1("window-new")
            )
    ADD_ICON_CASE(ICON_ZOOM_CUSTOM, QIcon())
    ADD_THEMED_ICON_CASE(ICON_ZOOM_FIT_BEST, QStringList()
            << QString::fromLatin1("zoom-fit-best")
            << QString::fromLatin1("gtk-zoom-fit")
            )
    ADD_THEMED_ICON_CASE(ICON_ZOOM_IN, QStringList()
            << QString::fromLatin1("zoom-in")
            << QString::fromLatin1("gtk-zoom-in")
            )
    ADD_THEMED_ICON_CASE(ICON_ZOOM_ORIGINAL, QStringList()
            << QString::fromLatin1("zoom-original")
            << QString::fromLatin1("gtk-zoom-100")
            )
    ADD_THEMED_ICON_CASE(ICON_ZOOM_OUT, QStringList()
            << QString::fromLatin1("zoom-out")
            << QString::fromLatin1("gtk-zoom-out")
            )
#undef ADD_THEMED_ICON_CASE
#undef ADD_ICON_CASE
    }

    return QIcon();
}

} // namespace ThemeUtils

