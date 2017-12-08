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

#if !defined(MAINWINDOW_P_H_INCLUDED)
#define MAINWINDOW_P_H_INCLUDED

#include "MainWindow.h"

#include <QVBoxLayout>
#include <QFrame>
#include <QList>

#include "Utils/ObjectsUtils.h"

#include "ImageViewerWidget.h"
#include "ToolBar.h"
#include "MenuBar.h"
#if defined (HAS_MAC_TOOLBAR)
#include "MacToolBar.h"
#endif

namespace {

const int WINDOW_DEFAULT_WIDTH  = 640;
const int WINDOW_DEFAULT_HEIGHT = 480;

} // namespace

struct MainWindow::UI
{
    MainWindow *mainWindow;
    QFrame *centralWidget;
    ImageViewerWidget *imageViewerWidget;
    ToolBar *toolbar;
    MenuBar *menubar;
    QList<IControlsContainer*> controlsContainers;

    UI(MainWindow *mainWindow)
        : mainWindow(mainWindow)
        , CONSTRUCT_OBJECT(centralWidget, QFrame, (mainWindow))
        , CONSTRUCT_OBJECT(imageViewerWidget, ImageViewerWidget, (centralWidget))
        , CONSTRUCT_OBJECT(toolbar, ToolBar, (centralWidget))
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
        mainLayout->addWidget(toolbar);

        mainWindow->setCentralWidget(centralWidget);
        mainWindow->setMenuBar(menubar);
        mainWindow->resize(WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT);

        mainWindow->ensurePolished();

#if defined (HAS_MAC_TOOLBAR)
        MacToolBar *macToolbar = new MacToolBar(mainWindow);
        controlsContainers.append(macToolbar);
        macToolbar->attachToWindow(mainWindow->windowHandle());
#endif
    }
};

#endif // MAINWINDOW_P_H_INCLUDED
