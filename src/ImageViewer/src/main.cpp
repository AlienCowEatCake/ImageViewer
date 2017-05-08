/*
   Copyright (C) 2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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
#include <QStyleFactory>
#include "GUI/MainWindow.h"
#include "Utils/Application.h"
#include "Utils/ThemeUtils.h"
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
    app.setApplicationVersion(QString::fromLatin1("0.99.5"));
#if !defined (Q_OS_MAC)
    app.setWindowIcon(QIcon(QString::fromLatin1(":/icon/icon.ico")));
    app.setAttribute(Qt::AA_DontShowIconsInMenus, false);
#else
    app.setAttribute(Qt::AA_DontShowIconsInMenus);
#endif
    Workarounds::InitQtUtilsResources();
    ThemeUtils::LoadStyleSheet(QString::fromLatin1(":/style/style.qss"));
    MainWindow * window = new MainWindow;
    if(argc > 1)
    {
        std::string filename;
        for(int i = 1; i < argc; i++)
        {
            filename.append(argv[i]);
            if(i + 1 < argc)
                filename.append(" ");
        }
        window->onOpenPathRequested(QString::fromLocal8Bit(filename.c_str()));
    }
    else if(app.hasLastOpenFilePath())
    {
        window->onOpenPathRequested(app.getLastOpenFilePath());
    }
    QObject::connect(&app, SIGNAL(openFileEvent(const QString &)), window, SLOT(onOpenPathRequested(const QString &)));
    window->show();
    return app.exec();
}
