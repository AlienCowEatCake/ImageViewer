/*
   Copyright (C) 2017-2018 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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
#include <QFileInfo>
#include <QStyleFactory>
#include "GUI/MainController.h"
#include "Utils/Application.h"
#include "Utils/LocalizationManager.h"
#include "Utils/ThemeManager.h"
#include "Utils/Workarounds.h"

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
    Workarounds::HighDPIFix();
    Workarounds::StyleFix();
    Application app(argc, argv);
    app.setOrganizationDomain(QString::fromLatin1("fami.codefreak.ru"));
    app.setOrganizationName(QString::fromLatin1("FAMI Net Team"));
    app.setApplicationName(QString::fromLatin1("Image Viewer"));
    app.setApplicationVersion(QString::fromLatin1("0.99.14"));
#if !defined (Q_OS_MAC)
    app.setWindowIcon(QIcon(QString::fromLatin1(":/icon/icon.ico")));
    app.setAttribute(Qt::AA_DontShowIconsInMenus, false);
#else
    app.setAttribute(Qt::AA_DontShowIconsInMenus);
#endif
    Workarounds::InitQtUtilsResources();

    ThemeManager *themeManager = ThemeManager::instance();
    themeManager->registerTheme(QString::fromLatin1(QT_TRANSLATE_NOOP("Themes", "System")),
                                QStringList() << QString::fromLatin1(":/style/style.qss"),
                                QString::fromLatin1("Themes"), true);
    themeManager->registerTheme(QString::fromLatin1(QT_TRANSLATE_NOOP("Themes", "Dark")),
                                QStringList() << QString::fromLatin1(":/style/style.qss")
                                    << QString::fromLatin1(":/style/theme-dark.qss"),
                                QString::fromLatin1("Themes"));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    themeManager->registerTheme(QString::fromLatin1(QT_TRANSLATE_NOOP("Themes", "Fusion_Light")),
                                QStringList() << QString::fromLatin1(":/style/style.qss")
                                    << QString::fromLatin1(":/style/theme-fusion-light.qss"),
                                QString::fromLatin1("Themes"));
    themeManager->registerTheme(QString::fromLatin1(QT_TRANSLATE_NOOP("Themes", "Fusion_Dark")),
                                QStringList() << QString::fromLatin1(":/style/style.qss")
                                    << QString::fromLatin1(":/style/theme-fusion-dark.qss"),
                                QString::fromLatin1("Themes"));
#endif
    themeManager->applyCurrentTheme();

    LocalizationManager::instance()->initializeResources(QStringList()
            << QString::fromLatin1(":/translations/imageviewer_%1")
            << QString::fromLatin1(":/translations/qtutils_%1")
    );

    MainController controller;
    QStringList filePaths;
    for(int i = 1; i < argc; i++)
    {
        const QString filePath = QString::fromLocal8Bit(argv[i]);
        if(QFileInfo(filePath).exists())
            filePaths.append(filePath);
    }
    if(filePaths.empty() && app.hasLastOpenFilePath())
        filePaths.append(app.getLastOpenFilePath());
    if(filePaths.size() == 1)
        controller.openPath(filePaths.first());
    else if(!filePaths.empty())
        controller.openPaths(filePaths);
    QObject::connect(&app, SIGNAL(openFileEvent(const QString&)), &controller, SLOT(openPath(const QString&)));
    controller.showMainWindow();
    return app.exec();
}
