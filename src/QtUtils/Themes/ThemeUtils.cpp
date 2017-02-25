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
QIcon GetIcon(IconTypes type, bool darkBackground)
{
    const QString iconNameTemplate = QString::fromLatin1(":/icons/modern/%2_%1.%3")
            .arg(darkBackground ? QString::fromLatin1("white") : QString::fromLatin1("black"));
    const QString defaultExt = QString::fromLatin1("svg");
    const QString pixmapExt = QString::fromLatin1("png");

    switch(type)
    {
    case ICON_QT:
        return CreateScalableIcon(iconNameTemplate.arg(QString::fromLatin1("icon_qt")).arg(defaultExt),
                QStringList(iconNameTemplate.arg(QString::fromLatin1("icon_qt")).arg(pixmapExt)));
    case ICON_ABOUT:
        return CreateScalableIcon(iconNameTemplate.arg(QString::fromLatin1("icon_info")).arg(defaultExt),
                QStringList(iconNameTemplate.arg(QString::fromLatin1("icon_info")).arg(pixmapExt)));
    case ICON_HELP:
        return CreateScalableIcon(iconNameTemplate.arg(QString::fromLatin1("icon_help")).arg(defaultExt),
                QStringList(iconNameTemplate.arg(QString::fromLatin1("icon_help")).arg(pixmapExt)));
    case ICON_AUTHORS:
        return CreateScalableIcon(iconNameTemplate.arg(QString::fromLatin1("icon_people")).arg(defaultExt),
                QStringList(iconNameTemplate.arg(QString::fromLatin1("icon_people")).arg(pixmapExt)));
    case ICON_TEXT:
        return CreateScalableIcon(iconNameTemplate.arg(QString::fromLatin1("icon_text")).arg(defaultExt),
                QStringList(iconNameTemplate.arg(QString::fromLatin1("icon_text")).arg(pixmapExt)));
    case ICON_SAVE:
        return CreateScalableIcon(iconNameTemplate.arg(QString::fromLatin1("icon_save")).arg(defaultExt),
                QStringList(iconNameTemplate.arg(QString::fromLatin1("icon_save")).arg(pixmapExt)));
    case ICON_SAVE_AS:
        return CreateScalableIcon(iconNameTemplate.arg(QString::fromLatin1("icon_save_as")).arg(defaultExt),
                QStringList(iconNameTemplate.arg(QString::fromLatin1("icon_save_as")).arg(pixmapExt)));
    case ICON_CLOSE:
        return CreateScalableIcon(iconNameTemplate.arg(QString::fromLatin1("icon_close")).arg(defaultExt),
                QStringList(iconNameTemplate.arg(QString::fromLatin1("icon_close")).arg(pixmapExt)));
    case ICON_EXIT:
        return CreateScalableIcon(iconNameTemplate.arg(QString::fromLatin1("icon_exit")).arg(defaultExt),
                QStringList(iconNameTemplate.arg(QString::fromLatin1("icon_exit")).arg(pixmapExt)));
    case ICON_NEW:
        return CreateScalableIcon(iconNameTemplate.arg(QString::fromLatin1("icon_new")).arg(defaultExt),
                QStringList(iconNameTemplate.arg(QString::fromLatin1("icon_new")).arg(pixmapExt)));
    case ICON_NEW_WINDOW:
        return CreateScalableIcon(iconNameTemplate.arg(QString::fromLatin1("icon_new_window")).arg(defaultExt),
                QStringList(iconNameTemplate.arg(QString::fromLatin1("icon_new_window")).arg(pixmapExt)));
    case ICON_OPEN:
        return CreateScalableIcon(iconNameTemplate.arg(QString::fromLatin1("icon_open")).arg(defaultExt),
                QStringList(iconNameTemplate.arg(QString::fromLatin1("icon_open")).arg(pixmapExt)));
    case ICON_CUT:
        return CreateScalableIcon(iconNameTemplate.arg(QString::fromLatin1("icon_cut")).arg(defaultExt),
                QStringList(iconNameTemplate.arg(QString::fromLatin1("icon_cut")).arg(pixmapExt)));
    case ICON_COPY:
        return CreateScalableIcon(iconNameTemplate.arg(QString::fromLatin1("icon_copy")).arg(defaultExt),
                QStringList(iconNameTemplate.arg(QString::fromLatin1("icon_copy")).arg(pixmapExt)));
    case ICON_PASTE:
        return CreateScalableIcon(iconNameTemplate.arg(QString::fromLatin1("icon_paste")).arg(defaultExt),
                QStringList(iconNameTemplate.arg(QString::fromLatin1("icon_paste")).arg(pixmapExt)));

    }
    return QIcon();
}

} // namespace ThemeUtils

