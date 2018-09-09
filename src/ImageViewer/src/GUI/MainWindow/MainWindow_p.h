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

#if !defined(MAINWINDOW_P_H_INCLUDED)
#define MAINWINDOW_P_H_INCLUDED

#include "MainWindow.h"

#include <QVBoxLayout>
#include <QFrame>
#include <QList>
#include <QStyle>

#include "Utils/ObjectsUtils.h"
#include "Utils/ThemeUtils.h"

#include "ImageViewerWidget.h"
#if defined (HAS_MAC_TOOLBAR)
#include "MacToolBar.h"
#else
#include "ToolBar.h"
#endif
#include "MenuBar.h"

namespace {

const int WINDOW_DEFAULT_WIDTH  = 640;
const int WINDOW_DEFAULT_HEIGHT = 480;

} // namespace

struct MainWindow::UI
{
    MainWindow *mainWindow;
    QFrame *centralWidget;
    ImageViewerWidget *imageViewerWidget;
#if defined (HAS_MAC_TOOLBAR)
    MacToolBar *toolbar;
#else
    ToolBar *toolbar;
#endif
    MenuBar *menubar;
    QList<IControlsContainer*> controlsContainers;

    UI(MainWindow *mainWindow)
        : mainWindow(mainWindow)
        , CONSTRUCT_OBJECT(centralWidget, QFrame, (mainWindow))
        , CONSTRUCT_OBJECT(imageViewerWidget, ImageViewerWidget, (centralWidget))
#if defined (HAS_MAC_TOOLBAR)
        , toolbar(new MacToolBar(mainWindow))
#else
        , CONSTRUCT_OBJECT(toolbar, ToolBar, (centralWidget))
#endif
        , CONSTRUCT_OBJECT(menubar, MenuBar, (mainWindow))
        , controlsContainers(QList<IControlsContainer*>() << toolbar << menubar)
    {
        const QList<QWidget*> mainWindowChildren = mainWindow->findChildren<QWidget*>();
        for(QList<QWidget*>::ConstIterator it = mainWindowChildren.constBegin(); it != mainWindowChildren.constEnd(); ++it)
            (*it)->setFocusPolicy(Qt::NoFocus);
        menubar->setFocusProxy(mainWindow);

        imageViewerWidget->setAcceptDrops(false);
        imageViewerWidget->setContextMenuPolicy(Qt::NoContextMenu);

        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);
        mainLayout->addWidget(imageViewerWidget);
#if !defined (HAS_MAC_TOOLBAR)
        mainLayout->addWidget(toolbar);
#endif

        mainWindow->setCentralWidget(centralWidget);
        mainWindow->setMenuBar(menubar);
        mainWindow->resize(WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT);

        mainWindow->ensurePolished();

#if defined (HAS_MAC_TOOLBAR)
        toolbar->attachToWindow(mainWindow);
#endif

        onThemeChanged();
    }

    void onThemeChanged()
    {
        imageViewerWidget->setProperty("DarkMode", ThemeUtils::SystemHasDarkTheme());
        imageViewerWidget->style()->unpolish(imageViewerWidget);
        imageViewerWidget->style()->polish(imageViewerWidget);
    }
};

#endif // MAINWINDOW_P_H_INCLUDED
