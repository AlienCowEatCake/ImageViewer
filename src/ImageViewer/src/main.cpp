/*
   Copyright (C) 2017-2025 Peter S. Zhigalov <peter.zhigalov@gmail.com>

   This file is part of the `ImageViewer' program.

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

#include <QtPlugin>
#include <QIcon>
#include <QDir>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QStyleFactory>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QTextCodec>
#endif

#include "Utils/Application.h"
#include "Utils/Global.h"
#include "Utils/IconThemeManager.h"
#include "Utils/InfoUtils.h"
#include "Utils/LocalizationManager.h"
#include "Utils/ThemeManager.h"
#include "Utils/Workarounds.h"

#include "GUI/MainController.h"

#if defined (USE_STATIC_QJPEG)
Q_IMPORT_PLUGIN(qjpeg)
#endif
#if defined (USE_STATIC_QTIFF)
Q_IMPORT_PLUGIN(qtiff)
#endif
#if defined (USE_STATIC_QICO)
Q_IMPORT_PLUGIN(qico)
#endif
#if defined (USE_STATIC_QGIF)
Q_IMPORT_PLUGIN(qgif)
#endif
#if defined (USE_STATIC_QMNG)
Q_IMPORT_PLUGIN(qmng)
#endif
#if defined (USE_STATIC_QSVG)
Q_IMPORT_PLUGIN(qsvg)
#endif

int main(int argc, char *argv[])
{
    Workarounds::ApplyEnvVarOverrides("ACECIV_");
    Workarounds::HighDPIFix();
    Workarounds::StyleFix();
    Application app(argc, argv);
    app.setOrganizationDomain(QString::fromLatin1("aliencoweatcake.github.com"));
    app.setOrganizationName(QString::fromLatin1("AlienCowEatCake"));
    app.setApplicationName(QString::fromLatin1("Image Viewer"));
    app.setApplicationVersion(QString::fromLatin1("1.8.2"));
#if !defined (Q_OS_MAC)
#if !defined (Q_OS_WIN) && (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
    app.setDesktopFileName(QString::fromLatin1("com.github.aliencoweatcake.imageviewer"));
#endif
    app.setWindowIcon(QIcon(QString::fromLatin1(":/icon/icon.ico")));
    app.setAttribute(Qt::AA_DontShowIconsInMenus, false);
#else
    app.setAttribute(Qt::AA_DontShowIconsInMenus, !InfoUtils::MacVersionGreatOrEqual(16, 0));
#endif
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif
    Workarounds::InitQtUtilsResources();

    ThemeManager *themeManager = ThemeManager::instance();
    themeManager->registerTheme(QString::fromLatin1(QT_TRANSLATE_NOOP("Themes", "System")),
                                QStringList() << QString::fromLatin1(":/style/style.qss"),
                                QString::fromLatin1("Themes"), true);
    themeManager->registerTheme(QString::fromLatin1(QT_TRANSLATE_NOOP("Themes", "Light")),
                                QStringList() << QString::fromLatin1(":/style/style.qss")
                                              << QString::fromLatin1(":/style/theme-light.qss"),
                                QString::fromLatin1("Themes"));
    themeManager->registerTheme(QString::fromLatin1(QT_TRANSLATE_NOOP("Themes", "Dark")),
                                QStringList() << QString::fromLatin1(":/style/style.qss")
                                              << QString::fromLatin1(":/style/theme-dark.qss"),
                                QString::fromLatin1("Themes"));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    themeManager->registerTheme(QString::fromLatin1(QT_TRANSLATE_NOOP("Themes", "Fusion")),
                                QStringList() << QString::fromLatin1(":/style/style.qss")
                                              << QString::fromLatin1(":/style/theme-fusion.qss"),
                                QString::fromLatin1("Themes"));
    themeManager->registerTheme(QString::fromLatin1(QT_TRANSLATE_NOOP("Themes", "Fusion Light")),
                                QStringList() << QString::fromLatin1(":/style/style.qss")
                                              << QString::fromLatin1(":/style/theme-fusion-light.qss"),
                                QString::fromLatin1("Themes"));
    themeManager->registerTheme(QString::fromLatin1(QT_TRANSLATE_NOOP("Themes", "Fusion Dark")),
                                QStringList() << QString::fromLatin1(":/style/style.qss")
                                              << QString::fromLatin1(":/style/theme-fusion-dark.qss"),
                                QString::fromLatin1("Themes"));
#endif
    themeManager->registerTheme(QString::fromLatin1(QT_TRANSLATE_NOOP("Themes", "Windows")),
                                QStringList() << QString::fromLatin1(":/style/style.qss")
                                              << QString::fromLatin1(":/style/theme-windows.qss"),
                                QString::fromLatin1("Themes"));
    themeManager->registerTheme(QString::fromLatin1(QT_TRANSLATE_NOOP("Themes", "Windows 95")),
                                QStringList() << QString::fromLatin1(":/style/style.qss")
                                              << QString::fromLatin1(":/style/theme-windows-95.qss"),
                                QString::fromLatin1("Themes"));
    themeManager->registerTheme(QString::fromLatin1(QT_TRANSLATE_NOOP("Themes", "Windows 2000")),
                                QStringList() << QString::fromLatin1(":/style/style.qss")
                                              << QString::fromLatin1(":/style/theme-windows-2000.qss"),
                                QString::fromLatin1("Themes"));
    themeManager->applyCurrentTheme();

#if defined (QTUTILS_ICONTHEMEMANAGER_SUPPORTS_SYSTEM_THEME)
    const bool systemIconsAreDefault = true;
#else
    const bool systemIconsAreDefault = false;
#endif
    IconThemeManager *iconThemeManager = IconThemeManager::instance();
    iconThemeManager->registerTheme(IconThemeManager::BUILTIN_THEME_ID,
                                    QStringList(),
                                    QString(),
                                    !systemIconsAreDefault);
#if defined (QTUTILS_ICONTHEMEMANAGER_SUPPORTS_SYSTEM_THEME)
    iconThemeManager->registerTheme(IconThemeManager::SYSTEM_THEME_ID,
                                    QStringList(),
                                    QString(),
                                    systemIconsAreDefault);
#endif

#if !defined (DISABLE_EMBED_TRANSLATIONS)
    const QString translationsPath = QString::fromLatin1(":/translations");
#elif (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QString translationsPath = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#else
    const QString translationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif
    LocalizationManager::instance()->initializeResources(QStringList()
            << QDir(translationsPath).filePath(QString::fromLatin1("imageviewer_%1"))
    );

    MainController controller;
    QStringList filePaths;
    {
        const QStringList arguments = app.arguments();
        QStringList::ConstIterator it = arguments.constBegin();
        if(it != arguments.constEnd())
        {
            for(++it; it != arguments.constEnd(); ++it)
            {
                const QString &filePath = *it;
                if(QFileInfo_exists(filePath))
                    filePaths.append(filePath);
            }
        }
    }
    if(filePaths.empty() && app.hasLastOpenFilePath())
        filePaths.append(app.getLastOpenFilePath());
    if(filePaths.size() == 1)
        controller.openPath(filePaths.first());
    else if(!filePaths.empty())
        controller.openPaths(filePaths);
    QObject::connect(&app, SIGNAL(openFileEvent(QString)), &controller, SLOT(openPath(QString)));
    controller.showMainWindow();
    return app.exec();
}
