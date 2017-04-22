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

#include "Utils/MenuUtils.h"
#include "Utils/ObjectsUtils.h"
#include "Utils/ThemeUtils.h"
#include "Widgets/AdjustableFrame.h"

#include "ImageViewerWidget.h"

namespace {

const int WINDOW_DEFAULT_WIDTH  = 640;
const int WINDOW_DEFAULT_HEIGHT = 480;

} // namespace

struct MainWindow::UI
{
    MainWindow *mainWindow;
    bool isSlideShowMode;

    QFrame *centralWidget;
    ImageViewerWidget *imageViewerWidget;
    AdjustableFrame *toolbar;
    QToolButton *navigatePrevious;
    QToolButton *navigateNext;
    QToolButton *startSlideShow;
    QToolButton *zoomOut;
    QToolButton *zoomIn;
    QToolButton *zoomFitToWindow;
    QToolButton *zoomOriginalSize;
    QToolButton *zoomFullScreen;
    QToolButton *rotateCounterclockwise;
    QToolButton *rotateClockwise;
    QToolButton *flipHorizontal;
    QToolButton *flipVertical;
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
    QAction *actionStartSlideShow;
    QAction *actionPreferences;
    QAction *actionExit;
    QAction *actionRotateCounterclockwise;
    QAction *actionRotateClockwise;
    QAction *actionFlipHorizontal;
    QAction *actionFlipVertical;
    QAction *actionDeleteFile;
    QAction *actionZoomOut;
    QAction *actionZoomIn;
    QAction *actionZoomFitToWindow;
    QAction *actionZoomOriginalSize;
    QAction *actionZoomFullScreen;
    QAction *actionShowMenuBar;
    QAction *actionShowToolBar;
    QAction *actionEnglish;
    QAction *actionRussian;
    QAction *actionAbout;
    QAction *actionAboutQt;

    UI(MainWindow *mainWindow)
        : mainWindow(mainWindow)
        , isSlideShowMode(false)
        , CONSTRUCT_OBJECT(centralWidget, QFrame, (mainWindow))
        , CONSTRUCT_OBJECT(imageViewerWidget, ImageViewerWidget, (centralWidget))
        , CONSTRUCT_OBJECT(toolbar, AdjustableFrame, (centralWidget))
        , CONSTRUCT_OBJECT(navigatePrevious, QToolButton, (toolbar))
        , CONSTRUCT_OBJECT(navigateNext, QToolButton, (toolbar))
        , CONSTRUCT_OBJECT(startSlideShow, QToolButton, (toolbar))
        , CONSTRUCT_OBJECT(zoomOut, QToolButton, (toolbar))
        , CONSTRUCT_OBJECT(zoomIn, QToolButton, (toolbar))
        , CONSTRUCT_OBJECT(zoomFitToWindow, QToolButton, (toolbar))
        , CONSTRUCT_OBJECT(zoomOriginalSize, QToolButton, (toolbar))
        , CONSTRUCT_OBJECT(zoomFullScreen, QToolButton, (toolbar))
        , CONSTRUCT_OBJECT(rotateCounterclockwise, QToolButton, (toolbar))
        , CONSTRUCT_OBJECT(rotateClockwise, QToolButton, (toolbar))
        , CONSTRUCT_OBJECT(flipHorizontal, QToolButton, (toolbar))
        , CONSTRUCT_OBJECT(flipVertical, QToolButton, (toolbar))
        , CONSTRUCT_OBJECT(openFile, QToolButton, (toolbar))
        , CONSTRUCT_OBJECT(saveFileAs, QToolButton, (toolbar))
        , CONSTRUCT_OBJECT(deleteFile, QToolButton, (toolbar))
        , CONSTRUCT_OBJECT(preferences, QToolButton, (toolbar))
        , CONSTRUCT_OBJECT(exit, QToolButton, (toolbar))
        , CONSTRUCT_OBJECT(menubar, QMenuBar, (mainWindow))
        , CONSTRUCT_OBJECT(contextMenu, QMenu, (mainWindow))
        , CONSTRUCT_OBJECT(menuFile, QMenu, (menubar))
        , CONSTRUCT_OBJECT(menuEdit, QMenu, (menubar))
        , CONSTRUCT_OBJECT(menuView, QMenu, (menubar))
        , CONSTRUCT_OBJECT(menuLanguage, QMenu, (menubar))
        , CONSTRUCT_OBJECT(menuHelp, QMenu, (menubar))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionOpen, createWidgetAction(mainWindow))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionSaveAs, createWidgetAction(mainWindow))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionNavigatePrevious, createWidgetAction(mainWindow))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionNavigateNext, createWidgetAction(mainWindow))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionStartSlideShow, createWidgetAction(mainWindow))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionPreferences, createWidgetAction(mainWindow))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionExit, createWidgetAction(mainWindow))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionRotateCounterclockwise, createWidgetAction(mainWindow))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionRotateClockwise, createWidgetAction(mainWindow))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionFlipHorizontal, createWidgetAction(mainWindow))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionFlipVertical, createWidgetAction(mainWindow))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionDeleteFile, createWidgetAction(mainWindow))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionZoomOut, createWidgetAction(mainWindow))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionZoomIn, createWidgetAction(mainWindow))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionZoomFitToWindow, createWidgetAction(mainWindow))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionZoomOriginalSize, createWidgetAction(mainWindow))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionZoomFullScreen, createWidgetAction(mainWindow))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionShowMenuBar, createWidgetAction(mainWindow))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionShowToolBar, createWidgetAction(mainWindow))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionEnglish, createWidgetAction(mainWindow))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionRussian, createWidgetAction(mainWindow))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionAbout, createWidgetAction(mainWindow))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionAboutQt, createWidgetAction(mainWindow))
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
        menubar->setFocusProxy(mainWindow);

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
        zoomFullScreen->setIcon         (ThemeUtils::GetIcon(ThemeUtils::ICON_FULLSCREEN                , ThemeUtils::WidgetHasDarkTheme(zoomFullScreen)));
        rotateCounterclockwise->setIcon (ThemeUtils::GetIcon(ThemeUtils::ICON_ROTATE_COUNTERCLOCKWISE   , ThemeUtils::WidgetHasDarkTheme(rotateCounterclockwise)));
        rotateClockwise->setIcon        (ThemeUtils::GetIcon(ThemeUtils::ICON_ROTATE_CLOCKWISE          , ThemeUtils::WidgetHasDarkTheme(rotateClockwise)));
        flipHorizontal->setIcon         (ThemeUtils::GetIcon(ThemeUtils::ICON_FLIP_HORIZONTAL           , ThemeUtils::WidgetHasDarkTheme(flipHorizontal)));
        flipVertical->setIcon           (ThemeUtils::GetIcon(ThemeUtils::ICON_FLIP_VERTICAL             , ThemeUtils::WidgetHasDarkTheme(flipVertical)));
        openFile->setIcon               (ThemeUtils::GetIcon(ThemeUtils::ICON_OPEN                      , ThemeUtils::WidgetHasDarkTheme(openFile)));
        saveFileAs->setIcon             (ThemeUtils::GetIcon(ThemeUtils::ICON_SAVE_AS                   , ThemeUtils::WidgetHasDarkTheme(saveFileAs)));
        deleteFile->setIcon             (ThemeUtils::GetIcon(ThemeUtils::ICON_DELETE                    , ThemeUtils::WidgetHasDarkTheme(deleteFile)));
        preferences->setIcon            (ThemeUtils::GetIcon(ThemeUtils::ICON_SETTINGS                  , ThemeUtils::WidgetHasDarkTheme(preferences)));
        exit->setIcon                   (ThemeUtils::GetIcon(ThemeUtils::ICON_EXIT                      , ThemeUtils::WidgetHasDarkTheme(exit)));

        QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbar);
        toolbarLayout->addStretch();
        toolbarLayout->addWidget(navigatePrevious);
        toolbarLayout->addWidget(navigateNext);
        toolbarLayout->addWidget(startSlideShow);
        toolbarLayout->addWidget(createVerticalSeparator(toolbar));
        toolbarLayout->addWidget(zoomOut);
        toolbarLayout->addWidget(zoomIn);
        toolbarLayout->addWidget(zoomFitToWindow);
        toolbarLayout->addWidget(zoomOriginalSize);
        toolbarLayout->addWidget(zoomFullScreen);
        toolbarLayout->addWidget(createVerticalSeparator(toolbar));
        toolbarLayout->addWidget(rotateCounterclockwise);
        toolbarLayout->addWidget(rotateClockwise);
        toolbarLayout->addWidget(flipHorizontal);
        toolbarLayout->addWidget(flipVertical);
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
        menuFile->addAction(actionNavigatePrevious);
        actionNavigatePrevious->setShortcuts(QList<QKeySequence>() << Qt::Key_Left << Qt::Key_Up);
        actionNavigatePrevious->setMenuRole(QAction::NoRole);
        menuFile->addAction(actionNavigateNext);
        actionNavigateNext->setShortcuts(QList<QKeySequence>() << Qt::Key_Right << Qt::Key_Down << Qt::Key_Space << Qt::Key_Return << Qt::Key_Enter);
        actionNavigateNext->setMenuRole(QAction::NoRole);
        menuFile->addAction(actionStartSlideShow);
        actionStartSlideShow->setShortcuts(createAnyModifierShortcuts(Qt::Key_W));
        actionStartSlideShow->setMenuRole(QAction::NoRole);
        menuFile->addSeparator();
        menuFile->addAction(actionPreferences);
#if defined (Q_OS_MAC)
        actionPreferences->setShortcuts(QList<QKeySequence>() << Qt::CTRL + Qt::Key_Comma << createAnyModifierShortcuts(Qt::Key_P));
#else
        actionPreferences->setShortcuts(QList<QKeySequence>() << createAnyModifierShortcuts(Qt::Key_P) << Qt::CTRL + Qt::Key_Comma);
#endif
        actionPreferences->setMenuRole(QAction::PreferencesRole);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);
#if defined (Q_OS_WIN)
        actionExit->setShortcuts(QList<QKeySequence>() << Qt::ALT + Qt::Key_F4 << Qt::CTRL + Qt::Key_Q);
#else
        actionExit->setShortcuts(QList<QKeySequence>() << Qt::CTRL + Qt::Key_Q << Qt::ALT + Qt::Key_F4);
#endif
        actionExit->setMenuRole(QAction::QuitRole);

        menuEdit->addAction(actionRotateCounterclockwise);
        actionRotateCounterclockwise->setShortcuts(createAnyModifierShortcuts(Qt::Key_L));
        actionRotateCounterclockwise->setMenuRole(QAction::NoRole);
        menuEdit->addAction(actionRotateClockwise);
        actionRotateClockwise->setShortcuts(createAnyModifierShortcuts(Qt::Key_R));
        actionRotateClockwise->setMenuRole(QAction::NoRole);
        menuEdit->addAction(actionFlipHorizontal);
        actionFlipHorizontal->setShortcuts(createAnyModifierShortcuts(Qt::Key_H));
        actionFlipHorizontal->setMenuRole(QAction::NoRole);
        menuEdit->addAction(actionFlipVertical);
        actionFlipVertical->setShortcuts(createAnyModifierShortcuts(Qt::Key_V));
        actionFlipVertical->setMenuRole(QAction::NoRole);
        menuEdit->addSeparator();
        menuEdit->addAction(actionDeleteFile);
#if defined (Q_OS_MAC)
        actionDeleteFile->setShortcuts(QList<QKeySequence>() << Qt::Key_Backspace << Qt::Key_Delete);
#else
        actionDeleteFile->setShortcuts(QList<QKeySequence>() << Qt::Key_Delete/* << Qt::Key_Backspace*/);
#endif
        actionDeleteFile->setMenuRole(QAction::NoRole);

        menuView->addAction(actionZoomOut);
        actionZoomOut->setShortcuts(QList<QKeySequence>() << Qt::Key_Minus << Qt::Key_Underscore);
        actionZoomOut->setMenuRole(QAction::NoRole);
        menuView->addAction(actionZoomIn);
        actionZoomIn->setShortcuts(QList<QKeySequence>() << Qt::Key_Plus << Qt::Key_Equal);
        actionZoomIn->setMenuRole(QAction::NoRole);
        menuView->addAction(actionZoomFitToWindow);
        actionZoomFitToWindow->setShortcuts(createAnyModifierShortcuts(Qt::Key_F));
        actionZoomFitToWindow->setMenuRole(QAction::NoRole);
        actionZoomFitToWindow->setCheckable(true);
        menuView->addAction(actionZoomOriginalSize);
        actionZoomOriginalSize->setShortcuts(createAnyModifierShortcuts(Qt::Key_G));
        actionZoomOriginalSize->setMenuRole(QAction::NoRole);
        actionZoomOriginalSize->setCheckable(true);
        menuView->addSeparator();
        menuView->addAction(actionZoomFullScreen);
#if defined (Q_OS_MAC)
        actionZoomFullScreen->setShortcuts(QList<QKeySequence>() << Qt::CTRL + Qt::META + Qt::Key_F << Qt::Key_F11);
#else
        actionZoomFullScreen->setShortcuts(QList<QKeySequence>() << Qt::Key_F11 << Qt::CTRL + Qt::META + Qt::Key_F);
#endif
        actionZoomFullScreen->setMenuRole(QAction::NoRole);
        actionZoomFullScreen->setCheckable(true);
        menuView->addSeparator();
#if !defined (Q_OS_MAC)
        menuView->addAction(actionShowMenuBar);
        actionShowMenuBar->setShortcuts(createAnyModifierShortcuts(Qt::Key_M));
#endif
        actionShowMenuBar->setMenuRole(QAction::NoRole);
        actionShowMenuBar->setCheckable(true);
        menuView->addAction(actionShowToolBar);
        actionShowToolBar->setShortcuts(createAnyModifierShortcuts(Qt::Key_T));
        actionShowToolBar->setMenuRole(QAction::NoRole);
        actionShowToolBar->setCheckable(true);

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
        actionFlipHorizontal->setIcon           (ThemeUtils::GetIcon(ThemeUtils::ICON_FLIP_HORIZONTAL           , menuHasDarkTheme));
        actionFlipVertical->setIcon             (ThemeUtils::GetIcon(ThemeUtils::ICON_FLIP_VERTICAL             , menuHasDarkTheme));
        actionDeleteFile->setIcon               (ThemeUtils::GetIcon(ThemeUtils::ICON_DELETE                    , menuHasDarkTheme));
        actionZoomOut->setIcon                  (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_OUT                  , menuHasDarkTheme));
        actionZoomIn->setIcon                   (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_IN                   , menuHasDarkTheme));
        actionZoomFitToWindow->setIcon          (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_EMPTY                , menuHasDarkTheme));
        actionZoomOriginalSize->setIcon         (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_IDENTITY             , menuHasDarkTheme));
        actionZoomFullScreen->setIcon           (ThemeUtils::GetIcon(ThemeUtils::ICON_FULLSCREEN                , menuHasDarkTheme));
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

        mainWindow->ensurePolished();
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
        flipHorizontal->setToolTip(qApp->translate("MainWindow", "Flip Horizontal"));
        flipVertical->setToolTip(qApp->translate("MainWindow", "Flip Vertical"));
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
        actionFlipHorizontal->setText(qApp->translate("MainWindow", "Flip &Horizontal"));
        actionFlipVertical->setText(qApp->translate("MainWindow", "Flip &Vertical"));
        actionDeleteFile->setText(QApplication::translate("MainWindow", "&Delete File"));
        actionZoomOut->setText(QApplication::translate("MainWindow", "Zoom &Out"));
        actionZoomIn->setText(QApplication::translate("MainWindow", "Zoom &In"));
        actionZoomFitToWindow->setText(QApplication::translate("MainWindow", "Fit Image To &Window Size"));
        actionZoomOriginalSize->setText(QApplication::translate("MainWindow", "Original &Size"));
        actionZoomFullScreen->setText(QApplication::translate("MainWindow", "&Full Screen"));
        actionShowMenuBar->setText(QApplication::translate("MainWindow", "Show &Menu Bar"));
        actionShowToolBar->setText(QApplication::translate("MainWindow", "Show &Tool Bar"));
        actionEnglish->setText(QApplication::translate("MainWindow", "&English"));
        actionRussian->setText(QApplication::translate("MainWindow", "&Russian"));
        actionAbout->setText(QApplication::translate("MainWindow", "&About"));
        actionAboutQt->setText(QApplication::translate("MainWindow", "About &Qt"));

        setSlideShowMode(isSlideShowMode);
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
        flipHorizontal->setEnabled(isEnabled);
        actionFlipHorizontal->setEnabled(isEnabled);
        flipVertical->setEnabled(isEnabled);
        actionFlipVertical->setEnabled(isEnabled);
        rotateClockwise->setEnabled(isEnabled);
        actionRotateClockwise->setEnabled(isEnabled);
        saveFileAs->setEnabled(isEnabled);
        actionSaveAs->setEnabled(isEnabled);
    }

    void setSlideShowMode(bool isSlideShow)
    {
        isSlideShowMode = isSlideShow;
        if(!isSlideShowMode)
        {
            startSlideShow->setToolTip(qApp->translate("MainWindow", "Start Slideshow"));
            startSlideShow->setIcon(ThemeUtils::GetIcon(ThemeUtils::ICON_PLAY, ThemeUtils::WidgetHasDarkTheme(startSlideShow)));
            actionStartSlideShow->setText(qApp->translate("MainWindow", "Start S&lideshow"));
            actionStartSlideShow->setIcon(ThemeUtils::GetIcon(ThemeUtils::ICON_PLAY, ThemeUtils::WidgetHasDarkTheme(menuFile)));
        }
        else
        {
            startSlideShow->setToolTip(qApp->translate("MainWindow", "Stop Slideshow"));
            startSlideShow->setIcon(ThemeUtils::GetIcon(ThemeUtils::ICON_STOP, ThemeUtils::WidgetHasDarkTheme(startSlideShow)));
            actionStartSlideShow->setText(qApp->translate("MainWindow", "Stop S&lideshow"));
            actionStartSlideShow->setIcon(ThemeUtils::GetIcon(ThemeUtils::ICON_STOP, ThemeUtils::WidgetHasDarkTheme(menuFile)));
        }
    }

private:

    QWidget *createVerticalSeparator(QWidget *parent) const
    {
        CREATE_OBJECT(separator, QFrame, (parent));
        return separator;
    }

    QAction *createWidgetAction(QWidget *widget)
    {
        QAction *action = new QAction(widget);
        widget->addAction(action);
        return action;
    }

    QList<QKeySequence> createAnyModifierShortcuts(Qt::Key key, int defaultModifier = 0)
    {
        static const QList<int> modifiers = QList<int>()
                << 0
                << Qt::SHIFT
                << Qt::META
                << Qt::CTRL
                << Qt::ALT
                << Qt::SHIFT + Qt::META
                << Qt::SHIFT + Qt::CTRL
                << Qt::SHIFT + Qt::ALT
                << Qt::META + Qt::CTRL
                << Qt::META + Qt::ALT
                << Qt::SHIFT + Qt::META + Qt::CTRL
                << Qt::SHIFT + Qt::META + Qt::ALT
                << Qt::SHIFT + Qt::CTRL + Qt::ALT
                << Qt::SHIFT + Qt::META + Qt::CTRL + Qt::ALT;
        QList<QKeySequence> result;
        result.append(key + defaultModifier);
        for(QList<int>::ConstIterator it = modifiers.constBegin(); it != modifiers.constEnd(); ++it)
        {
            const int modifier = *it;
            if(modifier != defaultModifier)
                result.append(key + modifier);
        }
        return result;
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
