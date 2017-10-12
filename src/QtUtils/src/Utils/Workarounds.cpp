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

#include "Workarounds.h"
#include <cstdlib>
#include <QtGlobal>
#include <QSysInfo>
#include <QApplication>
#include <QList>
#include <QPair>
#include <QFont>
#include <QFontDatabase>
#include <QStringList>

/// @brief Инициализация ресурсов статической библиотеки QtUtils
/// @attention Функция должна быть в глобальном namespace
static void InitQtUtilsResources_internal()
{
    Q_INIT_RESOURCE(qtutils_icons);
    Q_INIT_RESOURCE(qtutils_translations);
}

namespace Workarounds {

/// @brief Инициализация ресурсов статической библиотеки QtUtils
void InitQtUtilsResources()
{
    InitQtUtilsResources_internal();
}

/// @brief Исправить отображение локализованных шрифтов под Windows
/// @param[in] language - язык, для которого будет проводиться исправление
/// @note Функцию следует вызывать при смене языка
void FontsFix(const QString &language)
{
#if defined (Q_OS_WIN)

    // Отображение название языка -> соответствующая ему WritingSystem
    static const QList<QPair<QString, QFontDatabase::WritingSystem> > writingSystemMap =
            QList<QPair<QString, QFontDatabase::WritingSystem> >()
            << qMakePair(QString::fromLatin1("en"), QFontDatabase::Latin)
            << qMakePair(QString::fromLatin1("ru"), QFontDatabase::Cyrillic);

    // Найдем WritingSystem для текущего языка
    QFontDatabase::WritingSystem currentWritingSystem = QFontDatabase::Any;
    for(QList<QPair<QString, QFontDatabase::WritingSystem> >::ConstIterator it = writingSystemMap.begin(); it != writingSystemMap.end(); ++it)
    {
        if(it->first == language)
        {
            currentWritingSystem = it->second;
            break;
        }
    }

    // Шрифт стандартный, по умолчанию
    static const QFont defaultFont = qApp->font();
    // Шрифт Tahoma, если стандартный не поддерживает выбранный язык
    QFont fallbackFont = defaultFont;
    fallbackFont.setFamily(QString::fromLatin1("Tahoma"));

    // Проверим, умеет ли стандартный шрифт писать на новом языке
    static const QFontDatabase fontDatabase;
    if(!fontDatabase.families(currentWritingSystem).contains(defaultFont.family(), Qt::CaseInsensitive))
        qApp->setFont(fallbackFont);   // Если не умеет - заменим на Tahoma
    else
        qApp->setFont(defaultFont);    // Если умеет, то вернем его обратно

    // Для Win98 форсированно заменяем шрифты на Tahoma для всех не-английских локалей
    if(QSysInfo::windowsVersion() <= QSysInfo::WV_Me)
    {
        if(language != QString::fromLatin1("en"))
            qApp->setFont(fallbackFont);
        else
            qApp->setFont(defaultFont);
    }

#else

    Q_UNUSED(language);

#endif
}

/// @brief Автоматически увеличить масштаб отображения для высоких DPI
/// @note Функцию следует вызывать перед созданием QApplication
/// @attention Актуально только для Qt 5.4+
void HighDPIFix()
{
    if(!IsRemoteSession())
    {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
        static char newEnv[] = "QT_AUTO_SCREEN_SCALE_FACTOR=1";
        if(!getenv("QT_AUTO_SCREEN_SCALE_FACTOR") && !getenv("QT_DEVICE_PIXEL_RATIO"))
            putenv(newEnv);
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
        static char newEnv[] = "QT_DEVICE_PIXEL_RATIO=auto";
        if(!getenv("QT_DEVICE_PIXEL_RATIO"))
            putenv(newEnv);
#endif
    }

    // Qt::AA_UseHighDpiPixmaps доступен и в более ранних версиях,
    // однако без поддержки HighDPI его применение нецелесообразно
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
}

/// @brief Определить, запущено ли приложение удаленно
/// @return true - удаленный запуск, false - иначе
bool IsRemoteSession()
{
#if defined (Q_OS_UNIX) && !defined (Q_OS_MAC)
    if(getenv("SSH_CLIENT") || getenv("SSH_TTY") || getenv("SSH_CONNECTION"))
        return true;
#endif
    return false;
}

/// @brief Переопределить неподдерживаемые QT_QPA_PLATFORMTHEME и QT_STYLE_OVERRIDE
void StyleFix()
{
#if (defined (Q_OS_UNIX) && !defined (Q_OS_MAC) && QT_VERSION >= QT_VERSION_CHECK(5, 0, 0) && QT_VERSION < QT_VERSION_CHECK(5, 7, 0))
    // До 5.6 включительно использовался стиль gtk, с 5.7 он называется gtk2
    // Если это статическая сборка, например, с 5.6 и в системе определены
    // QT_QPA_PLATFORMTHEME и/или QT_STYLE_OVERRIDE, то стили не подхватятся.
    static char newPlatformThemeEnv[] = "QT_QPA_PLATFORMTHEME=gtk";
    const char *platformThemeEnv = getenv("QT_QPA_PLATFORMTHEME");
    if(platformThemeEnv && !QString::fromLatin1(platformThemeEnv).compare(QString::fromLatin1("gtk2"), Qt::CaseInsensitive))
        putenv(newPlatformThemeEnv);
    static char newStyleOverrideEnv[] = "QT_STYLE_OVERRIDE=gtk";
    const char *styleOverrideEnv = getenv("QT_STYLE_OVERRIDE");
    if(styleOverrideEnv && !QString::fromLatin1(styleOverrideEnv).compare(QString::fromLatin1("gtk2"), Qt::CaseInsensitive))
        putenv(newStyleOverrideEnv);
#endif
}

} // Workarounds

