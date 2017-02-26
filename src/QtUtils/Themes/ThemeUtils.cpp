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

#include "ThemeUtils.h"
#include <QPixmap>
#include <QDebug>

namespace ThemeUtils {

/// @brief Функция для определения темная используемая тема виджета или нет
/// @param[in] widget - Виджет, для которого выполняется эта проверка
bool WidgetHasDarkTheme(const QWidget *widget)
{
    return widget->palette().color(widget->foregroundRole()).toHsv().value() > 128;
}

/// @brief Создать масштабируемую иконку из нескольких разного размера
/// @param[in] defaultImagePath - Путь к иконке по умолчанию (может быть SVG)
/// @param[in] scaledImagePaths - Список путей иконкам разного размера (растр)
/// @return Масштабируемая иконка
QIcon CreateScalableIcon(const QString &defaultImagePath, const QStringList &scaledImagePaths)
{
    QIcon result(defaultImagePath);
    for(QStringList::ConstIterator it = scaledImagePaths.begin(); it != scaledImagePaths.end(); ++it)
    {
        const QPixmap pixmap(*it);
        if(!pixmap.isNull())
        {
            result.addPixmap(QPixmap(*it), QIcon::Normal);
            result.addPixmap(QPixmap(*it), QIcon::Disabled);
            result.addPixmap(QPixmap(*it), QIcon::Active);
            result.addPixmap(QPixmap(*it), QIcon::Selected);
        }
        else
        {
            qWarning() << "[ThemeUtils::CreateScalableIcon]: Unable to load pixmap" << *it;
        }
    }
    return result;
}

/// @brief Функция для получения иконки
/// @param[in] type - Тип иконки (см. enum IconTypes)
/// @param[in] darkBackground - true, если иконка располагается на темном фоне
/// @param[in] withRasterPixmaps - использовать ли растровые изображения при создании иконки
QIcon GetIcon(IconTypes type, bool darkBackground, bool withRasterPixmaps)
{
    const QString iconNameTemplate = QString::fromLatin1(":/icons/modern/%2_%1.%3")
            .arg(darkBackground ? QString::fromLatin1("white") : QString::fromLatin1("black"));
    const QString defaultExt = QString::fromLatin1("svg");
    const QString pixmapExt = QString::fromLatin1("png");

    switch(type)
    {
#define ADD_NAMED_ICON_CASE(ICON_TYPE, ICON_NAME) \
    case ICON_TYPE: \
    { \
        const QString iconName = QString::fromLatin1(ICON_NAME).toLower(); \
        const QStringList rasterPixmaps = (withRasterPixmaps ? QStringList(iconNameTemplate.arg(iconName).arg(pixmapExt)) : QStringList()); \
        return CreateScalableIcon(iconNameTemplate.arg(iconName).arg(defaultExt), rasterPixmaps); \
    }
#define ADD_ICON_CASE(ICON_TYPE) ADD_NAMED_ICON_CASE(ICON_TYPE, #ICON_TYPE)
    ADD_ICON_CASE(ICON_QT)
    ADD_NAMED_ICON_CASE(ICON_ABOUT, "icon_info")
    ADD_ICON_CASE(ICON_HELP)
    ADD_NAMED_ICON_CASE(ICON_AUTHORS, "icon_people")
    ADD_ICON_CASE(ICON_TEXT)
    ADD_ICON_CASE(ICON_SAVE)
    ADD_ICON_CASE(ICON_SAVE_AS)
    ADD_ICON_CASE(ICON_CLOSE)
    ADD_ICON_CASE(ICON_EXIT)
    ADD_ICON_CASE(ICON_NEW)
    ADD_ICON_CASE(ICON_NEW_WINDOW)
    ADD_ICON_CASE(ICON_OPEN)
    ADD_ICON_CASE(ICON_CUT)
    ADD_ICON_CASE(ICON_COPY)
    ADD_ICON_CASE(ICON_PASTE)
    ADD_ICON_CASE(ICON_DELETE)
    ADD_ICON_CASE(ICON_ZOOM_IN)
    ADD_ICON_CASE(ICON_ZOOM_OUT)
    ADD_ICON_CASE(ICON_ZOOM_IDENTITY)
    ADD_ICON_CASE(ICON_ZOOM_EMPTY)
    ADD_ICON_CASE(ICON_LEFT)
    ADD_ICON_CASE(ICON_RIGHT)
    ADD_ICON_CASE(ICON_ROTATE_CLOCKWISE)
    ADD_ICON_CASE(ICON_ROTATE_COUNTERCLOCKWISE)
    ADD_ICON_CASE(ICON_SETTINGS)
#undef ADD_ICON_CASE
#undef ADD_NAMED_ICON_CASE
    }
    return QIcon();
}

} // namespace ThemeUtils

