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

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QToolButton>
#include <QMenuBar>
#include <QAction>
#include <QMenu>
#include <QActionGroup>
#include <QStyleFactory>

#include "Utils/ThemeUtils.h"
#include "ImageViewerWidget.h"

namespace {

const int TOOLBAR_BUTTON_SIZE       = 28;
const int TOOLBAR_BUTTON_ICON_SIZE  = 16;
const int TOOLBAR_LAYOUT_SPACING    = 3;
const int TOOLBAR_HORIZONTAL_MARGIN = 4;
const int TOOLBAR_VERTICAL_MARGIN   = 2;
const int WINDOW_DEFAULT_WIDTH      = 640;
const int WINDOW_DEFAULT_HEIGHT     = 480;

} // namespace

struct MainWindow::UI
{
    MainWindow *mainWindow;

    QFrame *centralWidget;
    ImageViewerWidget *imageViewerWidget;
    QFrame *toolbar;
    QToolButton *navigatePrevious;
    QToolButton *navigateNext;
    QToolButton *zoomOut;
    QToolButton *zoomIn;
    QToolButton *zoomFitToWindow;
    QToolButton *zoomOriginalSize;
    QToolButton *rotateCounterclockwise;
    QToolButton *rotateClockwise;
    QToolButton *openFile;
    QToolButton *saveFileAs;
    QToolButton *deleteFile;
    QToolButton *preferences;
    QToolButton *exit;

    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuLanguage;
    QMenu *menuHelp;

    QAction *actionOpen;
    QAction *actionSaveAs;
    QAction *actionPreferences;
    QAction *actionExit;
    QAction *actionAbout;
    QAction *actionAboutQt;
    QAction *actionEnglish;
    QAction *actionRussian;

    UI(MainWindow *mainWindow)
        : mainWindow(mainWindow)
        , centralWidget(new QFrame(mainWindow))
        , imageViewerWidget(new ImageViewerWidget(centralWidget))
        , toolbar(new QFrame(centralWidget))
        , navigatePrevious(createToolbarButton(toolbar))
        , navigateNext(createToolbarButton(toolbar))
        , zoomOut(createToolbarButton(toolbar))
        , zoomIn(createToolbarButton(toolbar))
        , zoomFitToWindow(createToolbarButton(toolbar))
        , zoomOriginalSize(createToolbarButton(toolbar))
        , rotateCounterclockwise(createToolbarButton(toolbar))
        , rotateClockwise(createToolbarButton(toolbar))
        , openFile(createToolbarButton(toolbar))
        , saveFileAs(createToolbarButton(toolbar))
        , deleteFile(createToolbarButton(toolbar))
        , preferences(createToolbarButton(toolbar))
        , exit(createToolbarButton(toolbar))
        , menubar(new QMenuBar(mainWindow))
        , menuFile(new QMenu(menubar))
        , menuLanguage(new QMenu(menubar))
        , menuHelp(new QMenu(menubar))
        , actionOpen(new QAction(menuFile))
        , actionSaveAs(new QAction(menuFile))
        , actionPreferences(new QAction(menuFile))
        , actionExit(new QAction(menuFile))
        , actionAbout(new QAction(menuHelp))
        , actionAboutQt(new QAction(menuHelp))
        , actionEnglish(new QAction(menuLanguage))
        , actionRussian(new QAction(menuLanguage))
    {
#if defined (Q_OS_MAC)
        QStyle *style = NULL;
        if(QStyleFactory::keys().contains(QString::fromLatin1("Fusion"), Qt::CaseInsensitive))
             style = QStyleFactory::create(QString::fromLatin1("Fusion"));
        else if(QStyleFactory::keys().contains(QString::fromLatin1("Windows"), Qt::CaseInsensitive))
            style = QStyleFactory::create(QString::fromLatin1("Windows"));
        if(style)
        {
            toolbar->setStyle(style);
            const QList<QWidget*> toolbarChildren = toolbar->findChildren<QWidget*>();
            for(QList<QWidget*>::ConstIterator it = toolbarChildren.constBegin(); it != toolbarChildren.constEnd(); ++it)
                (*it)->setStyle(style);
        }
#endif

        const QList<QWidget*> mainWindowChildren = mainWindow->findChildren<QWidget*>();
        for(QList<QWidget*>::ConstIterator it = mainWindowChildren.constBegin(); it != mainWindowChildren.constEnd(); ++it)
            (*it)->setFocusPolicy(Qt::NoFocus);

        imageViewerWidget->setAcceptDrops(false);

        zoomFitToWindow->setCheckable(true);
        zoomOriginalSize->setCheckable(true);

        navigatePrevious->setIcon       (ThemeUtils::GetIcon(ThemeUtils::ICON_LEFT                      , ThemeUtils::WidgetHasDarkTheme(navigatePrevious)));
        navigateNext->setIcon           (ThemeUtils::GetIcon(ThemeUtils::ICON_RIGHT                     , ThemeUtils::WidgetHasDarkTheme(navigateNext)));
        zoomOut->setIcon                (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_OUT                  , ThemeUtils::WidgetHasDarkTheme(zoomOut)));
        zoomIn->setIcon                 (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_IN                   , ThemeUtils::WidgetHasDarkTheme(zoomIn)));
        zoomFitToWindow->setIcon        (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_EMPTY                , ThemeUtils::WidgetHasDarkTheme(zoomFitToWindow)));
        zoomOriginalSize->setIcon       (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_IDENTITY             , ThemeUtils::WidgetHasDarkTheme(zoomOriginalSize)));
        rotateCounterclockwise->setIcon (ThemeUtils::GetIcon(ThemeUtils::ICON_ROTATE_COUNTERCLOCKWISE   , ThemeUtils::WidgetHasDarkTheme(rotateCounterclockwise)));
        rotateClockwise->setIcon        (ThemeUtils::GetIcon(ThemeUtils::ICON_ROTATE_CLOCKWISE          , ThemeUtils::WidgetHasDarkTheme(rotateClockwise)));
        openFile->setIcon               (ThemeUtils::GetIcon(ThemeUtils::ICON_OPEN                      , ThemeUtils::WidgetHasDarkTheme(openFile)));
        saveFileAs->setIcon             (ThemeUtils::GetIcon(ThemeUtils::ICON_SAVE_AS                   , ThemeUtils::WidgetHasDarkTheme(saveFileAs)));
        deleteFile->setIcon             (ThemeUtils::GetIcon(ThemeUtils::ICON_DELETE                    , ThemeUtils::WidgetHasDarkTheme(deleteFile)));
        preferences->setIcon            (ThemeUtils::GetIcon(ThemeUtils::ICON_SETTINGS                  , ThemeUtils::WidgetHasDarkTheme(preferences)));
        exit->setIcon                   (ThemeUtils::GetIcon(ThemeUtils::ICON_EXIT                      , ThemeUtils::WidgetHasDarkTheme(exit)));

        QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbar);
        toolbarLayout->setContentsMargins(TOOLBAR_HORIZONTAL_MARGIN, TOOLBAR_VERTICAL_MARGIN, TOOLBAR_HORIZONTAL_MARGIN, TOOLBAR_VERTICAL_MARGIN);
        toolbarLayout->setSpacing(TOOLBAR_LAYOUT_SPACING);
        toolbarLayout->addStretch();
        toolbarLayout->addWidget(navigatePrevious);
        toolbarLayout->addWidget(navigateNext);
        toolbarLayout->addWidget(createVerticalSeparator(toolbar));
        toolbarLayout->addWidget(zoomOut);
        toolbarLayout->addWidget(zoomIn);
        toolbarLayout->addWidget(zoomFitToWindow);
        toolbarLayout->addWidget(zoomOriginalSize);
        toolbarLayout->addWidget(createVerticalSeparator(toolbar));
        toolbarLayout->addWidget(rotateCounterclockwise);
        toolbarLayout->addWidget(rotateClockwise);
        toolbarLayout->addWidget(createVerticalSeparator(toolbar));
        toolbarLayout->addWidget(openFile);
        toolbarLayout->addWidget(saveFileAs);
        toolbarLayout->addWidget(deleteFile);
        toolbarLayout->addWidget(createVerticalSeparator(toolbar));
        toolbarLayout->addWidget(preferences);
        toolbarLayout->addWidget(exit);
        toolbarLayout->addStretch();

        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);
        mainLayout->addWidget(imageViewerWidget);
        mainLayout->addWidget(toolbar);

        menuFile->addAction(actionOpen);
        actionOpen->setShortcut(QKeySequence::Open);
        actionOpen->setMenuRole(QAction::NoRole);
        menuFile->addAction(actionSaveAs);
        actionSaveAs->setShortcut(QKeySequence::Save);
        actionSaveAs->setMenuRole(QAction::NoRole);
        menuFile->addSeparator();
        menuFile->addAction(actionPreferences);
#if defined(Q_OS_MAC)
        actionPreferences->setShortcut(Qt::CTRL + Qt::Key_Comma);
#endif
        actionPreferences->setMenuRole(QAction::PreferencesRole);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);
#if defined(Q_OS_WIN)
        actionExit->setShortcut(Qt::ALT + Qt::Key_F4);
#else
        actionExit->setShortcut(Qt::CTRL + Qt::Key_Q);
#endif
        actionExit->setMenuRole(QAction::QuitRole);

        menuLanguage->addAction(actionEnglish);
        actionEnglish->setMenuRole(QAction::NoRole);
        actionEnglish->setCheckable(true);
        menuLanguage->addAction(actionRussian);
        actionRussian->setMenuRole(QAction::NoRole);
        actionRussian->setCheckable(true);

        menuHelp->addAction(actionAbout);
        actionAbout->setMenuRole(QAction::AboutRole);
        menuHelp->addAction(actionAboutQt);
        actionAboutQt->setMenuRole(QAction::AboutQtRole);

        const bool menuHasDarkTheme = ThemeUtils::WidgetHasDarkTheme(menuFile);
        actionOpen->setIcon         (ThemeUtils::GetIcon(ThemeUtils::ICON_OPEN      , menuHasDarkTheme));
        actionSaveAs->setIcon       (ThemeUtils::GetIcon(ThemeUtils::ICON_SAVE_AS   , menuHasDarkTheme));
        actionPreferences->setIcon  (ThemeUtils::GetIcon(ThemeUtils::ICON_SETTINGS  , menuHasDarkTheme));
        actionExit->setIcon         (ThemeUtils::GetIcon(ThemeUtils::ICON_EXIT      , menuHasDarkTheme));
        actionAbout->setIcon        (ThemeUtils::GetIcon(ThemeUtils::ICON_ABOUT     , menuHasDarkTheme));
        actionAboutQt->setIcon      (ThemeUtils::GetIcon(ThemeUtils::ICON_QT        , menuHasDarkTheme));

        QActionGroup *langActions = new QActionGroup(menuLanguage);
        langActions->addAction(actionEnglish);
        langActions->addAction(actionRussian);
        langActions->setExclusive(true);

        menubar->addMenu(menuFile);
        menubar->addMenu(menuLanguage);
        menubar->addMenu(menuHelp);

        setImageControlsEnabled(false);
        mainWindow->setCentralWidget(centralWidget);
        mainWindow->setMenuBar(menubar);
        mainWindow->resize(WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT);
    }

    ~UI()
    {}

    void retranslate()
    {
        navigatePrevious->setToolTip(qApp->translate("MainWindow", "Previous"));
        navigateNext->setToolTip(qApp->translate("MainWindow", "Next"));
        zoomOut->setToolTip(qApp->translate("MainWindow", "Zoom Out"));
        zoomIn->setToolTip(qApp->translate("MainWindow", "Zoom In"));
        zoomFitToWindow->setToolTip(qApp->translate("MainWindow", "Fit Image To Window Size"));
        zoomOriginalSize->setToolTip(qApp->translate("MainWindow", "Original Size"));
        rotateCounterclockwise->setToolTip(qApp->translate("MainWindow", "Rotate Counterclockwise"));
        rotateClockwise->setToolTip(qApp->translate("MainWindow", "Rotate Clockwise"));
        openFile->setToolTip(qApp->translate("MainWindow", "Open File"));
        saveFileAs->setToolTip(qApp->translate("MainWindow", "Save File As"));
        deleteFile->setToolTip(qApp->translate("MainWindow", "Delete File"));
        preferences->setToolTip(qApp->translate("MainWindow", "Preferences"));
        exit->setToolTip(qApp->translate("MainWindow", "Exit"));

        menuFile->setTitle(QApplication::translate("MainWindow", "&File"));
        menuHelp->setTitle(QApplication::translate("MainWindow", "&Help"));
        menuLanguage->setTitle(QApplication::translate("MainWindow", "&Language"));

        actionOpen->setText(QApplication::translate("MainWindow", "&Open"));
        actionSaveAs->setText(QApplication::translate("MainWindow", "&Save As"));
        actionPreferences->setText(QApplication::translate("MainWindow", "&Preferences"));
        actionExit->setText(QApplication::translate("MainWindow", "&Exit"));
        actionAbout->setText(QApplication::translate("MainWindow", "&About"));
        actionAboutQt->setText(QApplication::translate("MainWindow", "About &Qt"));
        actionEnglish->setText(QApplication::translate("MainWindow", "&English"));
        actionRussian->setText(QApplication::translate("MainWindow", "&Russian"));

        updateDockMenu();
    }

    void setImageControlsEnabled(bool isEnabled)
    {
        zoomOut->setEnabled(isEnabled);
        zoomIn->setEnabled(isEnabled);
        zoomFitToWindow->setEnabled(isEnabled);
        zoomOriginalSize->setEnabled(isEnabled);
        rotateCounterclockwise->setEnabled(isEnabled);
        rotateClockwise->setEnabled(isEnabled);
        saveFileAs->setEnabled(isEnabled);
        actionSaveAs->setEnabled(isEnabled);
    }

private:

    QToolButton *createToolbarButton(QWidget *parent) const
    {
        QToolButton *button = new QToolButton(parent);
        button->setFixedSize(QSize(TOOLBAR_BUTTON_SIZE, TOOLBAR_BUTTON_SIZE));
        button->setIconSize(QSize(TOOLBAR_BUTTON_ICON_SIZE, TOOLBAR_BUTTON_ICON_SIZE));
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        return button;
    }

    QWidget *createVerticalSeparator(QWidget *parent) const
    {
        QFrame *separator = new QFrame(parent);
        separator->setFrameShape(QFrame::VLine);
        separator->setFrameShadow(QFrame::Sunken);
        return separator;
    }

    void updateDockMenu()
    {
#if defined (Q_OS_MAC)
        void qt_mac_set_dock_menu(QMenu *menu);
        static QMenu dock_menu;
        dock_menu.clear();
        dock_menu.addAction(qApp->translate("Dock", "New Window"), mainWindow, SLOT(openNewWindow()));
        qt_mac_set_dock_menu(&dock_menu);
#endif
    }
};

#endif // MAINWINDOW_P_H_INCLUDED
