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

#include "Workarounds.h"

#include <cstdlib>
#include <cstring>

#include <QtGlobal>
#include <QSysInfo>
#include <QApplication>
#include <QList>
#include <QPair>
#include <QFont>
#include <QFontDatabase>
#include <QStringList>

#if defined (Q_OS_WIN)
#include <windows.h>
#endif

#include "ThemeUtils.h"

/// @brief Initialize resources of static QtUtils library
/// @attention This function should be in global namespace
static void InitQtUtilsResources_internal()
{
    static bool isInitialized = false;
    if(isInitialized)
        return;

    isInitialized = true;
    Q_INIT_RESOURCE(qtutils_icons);
    Q_INIT_RESOURCE(qtutils_style);
#if !defined (QTUTILS_DISABLE_EMBED_TRANSLATIONS)
    Q_INIT_RESOURCE(qtutils_translations);
#endif
}

namespace Workarounds {

/// @brief Initialize resources of static QtUtils library
void InitQtUtilsResources()
{
    InitQtUtilsResources_internal();
}

/// @brief Apply locale specific font hacks for Windows and Wine
/// @param[in] language - current language
/// @note Function should be called on GUI language changes
void FontsFix(const QString &language)
{
#if defined (Q_OS_WIN)
    /// @todo This is very old and legacy hack. It was designed to support
    /// Cyrillic characters in some Windows 9x and older versions of Wine.
    /// I don't know if this code is needed now. I don't know if Tahoma is
    /// suitable as fallback font for other WritingSystems.

    // Known languages and corresponding WritingSystems
    static const QList<QPair<QString, QFontDatabase::WritingSystem> > writingSystemMap =
            QList<QPair<QString, QFontDatabase::WritingSystem> >()
            << qMakePair(QString::fromLatin1("en"), QFontDatabase::Latin)
            << qMakePair(QString::fromLatin1("ru"), QFontDatabase::Cyrillic);

    // Find WritingSystem for current language
    QFontDatabase::WritingSystem currentWritingSystem = QFontDatabase::Any;
    for(QList<QPair<QString, QFontDatabase::WritingSystem> >::ConstIterator it = writingSystemMap.begin(); it != writingSystemMap.end(); ++it)
    {
        if(it->first == language)
        {
            currentWritingSystem = it->second;
            break;
        }
    }

    // Default font is saved for future use
    static const QFont defaultFont = qApp->font();

    // We need to restore default font for any unspecified languages
    if(currentWritingSystem == QFontDatabase::Any)
    {
        qApp->setFont(defaultFont);
        return;
    }

    // Fallback Tahoma font is used when default font is not support current language
    QFont fallbackFont = defaultFont;
    fallbackFont.setFamily(QString::fromLatin1("Tahoma"));

    // Check default font for support of WritingSystem for current language
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QStringList currentWritingSystemFamilies = QFontDatabase::families(currentWritingSystem);
#else
    static const QFontDatabase fontDatabase;
    const QStringList currentWritingSystemFamilies = fontDatabase.families(currentWritingSystem);
#endif
    if(!currentWritingSystemFamilies.contains(defaultFont.family(), Qt::CaseInsensitive))
        qApp->setFont(fallbackFont);   // Replace font to Tahoma if check is FAIL
    else
        qApp->setFont(defaultFont);    // Restore default font if check is OK

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    // Replace font to Tahoma for all languages except English on Windows 9x
    if(QSysInfo::windowsVersion() <= QSysInfo::WV_Me)
    {
        if(language != QString::fromLatin1("en"))
            qApp->setFont(fallbackFont);
        else
            qApp->setFont(defaultFont);
    }
#endif

#else

    Q_UNUSED(language);

#endif
}

/// @brief Automatically scale GUI on high DPI screens
/// @note Function should be called before creating QApplication instance
/// @attention Does nothing before Qt 5.4 because scaling is not supported
/// @attention Does nothing since Qt 6.0 because scaling is already active
void HighDPIFix()
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))

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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
    }

    // Qt::AA_UseHighDpiPixmaps is available in earlier versions too,
    // but it is not useful without support of HighDPI scaling
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

#endif
}

/// @brief Check if app is running with X11 forwarding over SSH
/// @return true - X11 forwarding over SSH is active, false - otherwise
bool IsRemoteSession()
{
#if defined (Q_OS_UNIX) && !defined (Q_OS_MAC)
    if(getenv("SSH_CLIENT") || getenv("SSH_TTY") || getenv("SSH_CONNECTION"))
        return true;
#endif
    return false;
}

/// @brief Override unsupported QT_QPA_PLATFORMTHEME and QT_STYLE_OVERRIDE
void StyleFix()
{
#if (defined (Q_OS_UNIX) && !defined (Q_OS_MAC) && QT_VERSION >= QT_VERSION_CHECK(5, 0, 0) && QT_VERSION < QT_VERSION_CHECK(5, 7, 0))
    // Well, "gtk" style was renamed to "gtk2" in Qt 5.7. This will result in
    // broken styles under following conditions:
    //  1. Application was built with Qt 5.6 or earlier
    //  2. System has Qt 5.7 or later
    //  3. System has QT_QPA_PLATFORMTHEME and/or QT_STYLE_OVERRIDE env vars
    //     with new "gtk2" values
    // So we should replace new "gtk2" value to old "gtk" value for Qt < 5.7
    static char newPlatformThemeEnv[] = "QT_QPA_PLATFORMTHEME=gtk";
    const char *platformThemeEnv = getenv("QT_QPA_PLATFORMTHEME");
    if(platformThemeEnv && !QString::fromLatin1(platformThemeEnv).compare(QString::fromLatin1("gtk2"), Qt::CaseInsensitive))
        putenv(newPlatformThemeEnv);
    static char newStyleOverrideEnv[] = "QT_STYLE_OVERRIDE=gtk";
    const char *styleOverrideEnv = getenv("QT_STYLE_OVERRIDE");
    if(styleOverrideEnv && !QString::fromLatin1(styleOverrideEnv).compare(QString::fromLatin1("gtk2"), Qt::CaseInsensitive))
        putenv(newStyleOverrideEnv);
#endif

#if (defined (Q_OS_WIN) && QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    HIGHCONTRASTW highContrast;
    memset(&highContrast, 0, sizeof(highContrast));
    highContrast.cbSize = sizeof(highContrast);
    bool isHighContrast = false;
    if(SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(highContrast), &highContrast, FALSE))
        isHighContrast = highContrast.dwFlags & HCF_HIGHCONTRASTON;

    if(!isHighContrast && ThemeUtils::SystemHasDarkTheme())
    {
        // https://www.qt.io/blog/dark-mode-on-windows-11-with-qt-6.5
        // https://doc.qt.io/qt-6/whatsnew67.html#qt-widgets-module
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
        static char newPlatformEnv[] = "QT_QPA_PLATFORM=windows:darkmode=1";
        const char *platformEnv = getenv("QT_QPA_PLATFORM");
        if(!platformEnv)
            putenv(newPlatformEnv);
#elif QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
        static char newPlatformEnv[] = "QT_QPA_PLATFORM=windows:darkmode=2";
        const char *platformEnv = getenv("QT_QPA_PLATFORM");
        static char newStyleOverrideEnv[] = "QT_STYLE_OVERRIDE=fusion";
        const char *styleOverrideEnv = getenv("QT_STYLE_OVERRIDE");
        if(!platformEnv && !styleOverrideEnv)
        {
            putenv(newPlatformEnv);
            putenv(newStyleOverrideEnv);
        }
#elif QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
        static char newStyleOverrideEnv[] = "QT_STYLE_OVERRIDE=fusion";
        const char *styleOverrideEnv = getenv("QT_STYLE_OVERRIDE");
        if(!styleOverrideEnv)
            putenv(newStyleOverrideEnv);
#endif
    }
#endif
}

} // Workarounds

