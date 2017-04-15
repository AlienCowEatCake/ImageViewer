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
#include "Utils/MenuUtils.h"
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
    QToolButton *zoomFullScreen;
    QToolButton *rotateCounterclockwise;
    QToolButton *rotateClockwise;
    QToolButton *openFile;
    QToolButton *saveFileAs;
    QToolButton *deleteFile;
    QToolButton *preferences;
    QToolButton *exit;

    QMenuBar *menubar;
    QMenu *contextMenu;
    QMenu *menuFile;
    QMenu *menuEdit;
    QMenu *menuView;
    QMenu *menuLanguage;
    QMenu *menuHelp;

    QAction *actionOpen;
    QAction *actionSaveAs;
    QAction *actionNavigatePrevious;
    QAction *actionNavigateNext;
    QAction *actionPreferences;
    QAction *actionExit;
    QAction *actionRotateCounterclockwise;
    QAction *actionRotateClockwise;
    QAction *actionDeleteFile;
    QAction *actionZoomOut;
    QAction *actionZoomIn;
    QAction *actionZoomFitToWindow;
    QAction *actionZoomOriginalSize;
    QAction *actionZoomFullScreen;
    QAction *actionEnglish;
    QAction *actionRussian;
    QAction *actionAbout;
    QAction *actionAboutQt;

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
        , zoomFullScreen(createToolbarButton(toolbar))
        , rotateCounterclockwise(createToolbarButton(toolbar))
        , rotateClockwise(createToolbarButton(toolbar))
        , openFile(createToolbarButton(toolbar))
        , saveFileAs(createToolbarButton(toolbar))
        , deleteFile(createToolbarButton(toolbar))
        , preferences(createToolbarButton(toolbar))
        , exit(createToolbarButton(toolbar))
        , menubar(new QMenuBar(mainWindow))
        , contextMenu(new QMenu(mainWindow))
        , menuFile(new QMenu(menubar))
        , menuEdit(new QMenu(menubar))
        , menuView(new QMenu(menubar))
        , menuLanguage(new QMenu(menubar))
        , menuHelp(new QMenu(menubar))
        , actionOpen(createWidgetAction(mainWindow))
        , actionSaveAs(createWidgetAction(mainWindow))
        , actionNavigatePrevious(createWidgetAction(mainWindow))
        , actionNavigateNext(createWidgetAction(mainWindow))
        , actionPreferences(createWidgetAction(mainWindow))
        , actionExit(createWidgetAction(mainWindow))
        , actionRotateCounterclockwise(createWidgetAction(mainWindow))
        , actionRotateClockwise(createWidgetAction(mainWindow))
        , actionDeleteFile(createWidgetAction(mainWindow))
        , actionZoomOut(createWidgetAction(mainWindow))
        , actionZoomIn(createWidgetAction(mainWindow))
        , actionZoomFitToWindow(createWidgetAction(mainWindow))
        , actionZoomOriginalSize(createWidgetAction(mainWindow))
        , actionZoomFullScreen(createWidgetAction(mainWindow))
        , actionEnglish(createWidgetAction(mainWindow))
        , actionRussian(createWidgetAction(mainWindow))
        , actionAbout(createWidgetAction(mainWindow))
        , actionAboutQt(createWidgetAction(mainWindow))
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
        imageViewerWidget->setContextMenuPolicy(Qt::NoContextMenu);

        zoomFitToWindow->setCheckable(true);
        zoomOriginalSize->setCheckable(true);
        zoomFullScreen->setCheckable(true);

        navigatePrevious->setIcon       (ThemeUtils::GetIcon(ThemeUtils::ICON_LEFT                      , ThemeUtils::WidgetHasDarkTheme(navigatePrevious)));
        navigateNext->setIcon           (ThemeUtils::GetIcon(ThemeUtils::ICON_RIGHT                     , ThemeUtils::WidgetHasDarkTheme(navigateNext)));
        zoomOut->setIcon                (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_OUT                  , ThemeUtils::WidgetHasDarkTheme(zoomOut)));
        zoomIn->setIcon                 (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_IN                   , ThemeUtils::WidgetHasDarkTheme(zoomIn)));
        zoomFitToWindow->setIcon        (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_EMPTY                , ThemeUtils::WidgetHasDarkTheme(zoomFitToWindow)));
        zoomOriginalSize->setIcon       (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_IDENTITY             , ThemeUtils::WidgetHasDarkTheme(zoomOriginalSize)));
//        zoomFullScreen->setIcon         (ThemeUtils::GetIcon(ThemeUtils::ICON_FULLSCREEN                , ThemeUtils::WidgetHasDarkTheme(zoomFullScreen)));
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
        toolbarLayout->addWidget(zoomFullScreen);
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
        actionOpen->setShortcuts(QList<QKeySequence>() << QKeySequence::Open << Qt::Key_O);
        actionOpen->setMenuRole(QAction::NoRole);
        menuFile->addAction(actionSaveAs);
        actionSaveAs->setShortcuts(QList<QKeySequence>() << QKeySequence::Save << Qt::Key_S);
        actionSaveAs->setMenuRole(QAction::NoRole);
        menuFile->addSeparator();
        menuFile->addAction(actionNavigatePrevious);
        actionNavigatePrevious->setShortcuts(QList<QKeySequence>() << Qt::Key_Left << Qt::Key_Up);
        actionNavigatePrevious->setMenuRole(QAction::NoRole);
        menuFile->addAction(actionNavigateNext);
        actionNavigateNext->setShortcuts(QList<QKeySequence>() << Qt::Key_Right << Qt::Key_Down << Qt::Key_Space << Qt::Key_Return << Qt::Key_Enter);
        actionNavigateNext->setMenuRole(QAction::NoRole);
        menuFile->addSeparator();
        menuFile->addAction(actionPreferences);
#if defined(Q_OS_MAC)
        actionPreferences->setShortcuts(QList<QKeySequence>() << Qt::CTRL + Qt::Key_Comma << Qt::CTRL + Qt::Key_P << Qt::Key_P);
#else
        actionPreferences->setShortcuts(QList<QKeySequence>() << Qt::CTRL + Qt::Key_P << Qt::Key_P << Qt::CTRL + Qt::Key_Comma);
#endif
        actionPreferences->setMenuRole(QAction::PreferencesRole);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);
#if defined(Q_OS_WIN)
        actionExit->setShortcuts(QList<QKeySequence>() << Qt::ALT + Qt::Key_F4 << Qt::CTRL + Qt::Key_Q);
#else
        actionExit->setShortcuts(QList<QKeySequence>() << Qt::CTRL + Qt::Key_Q << Qt::ALT + Qt::Key_F4);
#endif
        actionExit->setMenuRole(QAction::QuitRole);

        menuEdit->addAction(actionRotateCounterclockwise);
        actionRotateCounterclockwise->setShortcuts(QList<QKeySequence>() << Qt::CTRL + Qt::Key_L << Qt::Key_L);
        actionRotateCounterclockwise->setMenuRole(QAction::NoRole);
        menuEdit->addAction(actionRotateClockwise);
        actionRotateClockwise->setShortcuts(QList<QKeySequence>() << Qt::CTRL + Qt::Key_R << Qt::Key_R);
        actionRotateClockwise->setMenuRole(QAction::NoRole);
        menuEdit->addSeparator();
        menuEdit->addAction(actionDeleteFile);
        actionDeleteFile->setShortcuts(QKeySequence::Delete);
        actionDeleteFile->setMenuRole(QAction::NoRole);

        menuView->addAction(actionZoomOut);
        actionZoomOut->setShortcuts(QList<QKeySequence>() << Qt::Key_Minus << Qt::Key_Underscore);
        actionZoomOut->setMenuRole(QAction::NoRole);
        menuView->addAction(actionZoomIn);
        actionZoomIn->setShortcuts(QList<QKeySequence>() << Qt::Key_Plus << Qt::Key_Equal);
        actionZoomIn->setMenuRole(QAction::NoRole);
        menuView->addAction(actionZoomFitToWindow);
        actionZoomFitToWindow->setShortcuts(QList<QKeySequence>() << Qt::CTRL + Qt::Key_F << Qt::Key_F);
        actionZoomFitToWindow->setMenuRole(QAction::NoRole);
        menuView->addAction(actionZoomOriginalSize);
        actionZoomOriginalSize->setShortcuts(QList<QKeySequence>() << Qt::CTRL + Qt::Key_G << Qt::Key_G);
        actionZoomOriginalSize->setMenuRole(QAction::NoRole);
        menuView->addSeparator();
        menuView->addAction(actionZoomFullScreen);
#if defined(Q_OS_MAC)
        actionZoomFullScreen->setShortcuts(QList<QKeySequence>() << Qt::CTRL + Qt::META + Qt::Key_F << Qt::Key_F11);
#else
        actionZoomFullScreen->setShortcuts(QList<QKeySequence>() << Qt::Key_F11 << Qt::CTRL + Qt::META + Qt::Key_F);
#endif
        actionZoomFullScreen->setMenuRole(QAction::NoRole);

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
        actionOpen->setIcon                     (ThemeUtils::GetIcon(ThemeUtils::ICON_OPEN                      , menuHasDarkTheme));
        actionSaveAs->setIcon                   (ThemeUtils::GetIcon(ThemeUtils::ICON_SAVE_AS                   , menuHasDarkTheme));
        actionNavigatePrevious->setIcon         (ThemeUtils::GetIcon(ThemeUtils::ICON_LEFT                      , menuHasDarkTheme));
        actionNavigateNext->setIcon             (ThemeUtils::GetIcon(ThemeUtils::ICON_RIGHT                     , menuHasDarkTheme));
        actionPreferences->setIcon              (ThemeUtils::GetIcon(ThemeUtils::ICON_SETTINGS                  , menuHasDarkTheme));
        actionExit->setIcon                     (ThemeUtils::GetIcon(ThemeUtils::ICON_EXIT                      , menuHasDarkTheme));
        actionRotateCounterclockwise->setIcon   (ThemeUtils::GetIcon(ThemeUtils::ICON_ROTATE_COUNTERCLOCKWISE   , menuHasDarkTheme));
        actionRotateClockwise->setIcon          (ThemeUtils::GetIcon(ThemeUtils::ICON_ROTATE_CLOCKWISE          , menuHasDarkTheme));
        actionDeleteFile->setIcon               (ThemeUtils::GetIcon(ThemeUtils::ICON_DELETE                    , menuHasDarkTheme));
        actionZoomOut->setIcon                  (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_OUT                  , menuHasDarkTheme));
        actionZoomIn->setIcon                   (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_IN                   , menuHasDarkTheme));
        actionZoomFitToWindow->setIcon          (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_EMPTY                , menuHasDarkTheme));
        actionZoomOriginalSize->setIcon         (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_IDENTITY             , menuHasDarkTheme));
//        actionZoomFullScreen->setIcon           (ThemeUtils::GetIcon(ThemeUtils::ICON_FULLSCREEN                , menuHasDarkTheme));
        actionAbout->setIcon                    (ThemeUtils::GetIcon(ThemeUtils::ICON_ABOUT                     , menuHasDarkTheme));
        actionAboutQt->setIcon                  (ThemeUtils::GetIcon(ThemeUtils::ICON_QT                        , menuHasDarkTheme));

        QActionGroup *langActions = new QActionGroup(menuLanguage);
        langActions->addAction(actionEnglish);
        langActions->addAction(actionRussian);
        langActions->setExclusive(true);

        menubar->addMenu(menuFile);
        menubar->addMenu(menuEdit);
        menubar->addMenu(menuView);
        menubar->addMenu(menuLanguage);
        menubar->addMenu(menuHelp);

        contextMenu->addMenu(menuFile);
        contextMenu->addMenu(menuEdit);
        contextMenu->addMenu(menuView);
        contextMenu->addMenu(menuLanguage);
        contextMenu->addMenu(menuHelp);

        setImageControlsEnabled(false);
        mainWindow->setCentralWidget(centralWidget);
        mainWindow->setMenuBar(menubar);
        mainWindow->resize(WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT);

#if defined (Q_OS_MAC)
        MenuUtils::DisableDictationMenuItem();
        MenuUtils::DisableCharacterPaletteMenuItem();
        MenuUtils::DisableShowTabBarMenuItem();
        MenuUtils::DisableEnterFullScreenMenuItem();
#endif
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
        zoomFullScreen->setToolTip(qApp->translate("MainWindow", "Full Screen"));
        rotateCounterclockwise->setToolTip(qApp->translate("MainWindow", "Rotate Counterclockwise"));
        rotateClockwise->setToolTip(qApp->translate("MainWindow", "Rotate Clockwise"));
        openFile->setToolTip(qApp->translate("MainWindow", "Open File"));
        saveFileAs->setToolTip(qApp->translate("MainWindow", "Save File As"));
        deleteFile->setToolTip(qApp->translate("MainWindow", "Delete File"));
        preferences->setToolTip(qApp->translate("MainWindow", "Preferences"));
        exit->setToolTip(qApp->translate("MainWindow", "Exit"));

        menuFile->setTitle(QApplication::translate("MainWindow", "&File"));
        menuEdit->setTitle(QApplication::translate("MainWindow", "&Edit"));
        menuView->setTitle(QApplication::translate("MainWindow", "&View"));
        menuLanguage->setTitle(QApplication::translate("MainWindow", "&Language"));
        menuHelp->setTitle(QApplication::translate("MainWindow", "&Help"));

        actionOpen->setText(QApplication::translate("MainWindow", "&Open"));
        actionSaveAs->setText(QApplication::translate("MainWindow", "&Save As"));
        actionNavigatePrevious->setText(qApp->translate("MainWindow", "P&revious"));
        actionNavigateNext->setText(qApp->translate("MainWindow", "&Next"));
        actionPreferences->setText(QApplication::translate("MainWindow", "&Preferences"));
        actionExit->setText(QApplication::translate("MainWindow", "&Exit"));
        actionRotateCounterclockwise->setText(QApplication::translate("MainWindow", "Rotate &Counterclockwise"));
        actionRotateClockwise->setText(QApplication::translate("MainWindow", "&Rotate Clockwise"));
        actionDeleteFile->setText(QApplication::translate("MainWindow", "&Delete File"));
        actionZoomOut->setText(QApplication::translate("MainWindow", "Zoom &Out"));
        actionZoomIn->setText(QApplication::translate("MainWindow", "Zoom &In"));
        actionZoomFitToWindow->setText(QApplication::translate("MainWindow", "&Fit Image To Window Size"));
        actionZoomOriginalSize->setText(QApplication::translate("MainWindow", "Ori&ginal Size"));
        actionZoomFullScreen->setText(QApplication::translate("MainWindow", "Full &Screen"));
        actionEnglish->setText(QApplication::translate("MainWindow", "&English"));
        actionRussian->setText(QApplication::translate("MainWindow", "&Russian"));
        actionAbout->setText(QApplication::translate("MainWindow", "&About"));
        actionAboutQt->setText(QApplication::translate("MainWindow", "About &Qt"));

        updateDockMenu();
    }

    void setImageControlsEnabled(bool isEnabled)
    {
        zoomOut->setEnabled(isEnabled);
        actionZoomOut->setEnabled(isEnabled);
        zoomIn->setEnabled(isEnabled);
        actionZoomIn->setEnabled(isEnabled);
        zoomFitToWindow->setEnabled(isEnabled);
        actionZoomFitToWindow->setEnabled(isEnabled);
        zoomOriginalSize->setEnabled(isEnabled);
        actionZoomOriginalSize->setEnabled(isEnabled);
        rotateCounterclockwise->setEnabled(isEnabled);
        actionRotateCounterclockwise->setEnabled(isEnabled);
        rotateClockwise->setEnabled(isEnabled);
        actionRotateClockwise->setEnabled(isEnabled);
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

    QAction *createWidgetAction(QWidget *widget)
    {
        QAction *action = new QAction(widget);
        widget->addAction(action);
        return action;
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
