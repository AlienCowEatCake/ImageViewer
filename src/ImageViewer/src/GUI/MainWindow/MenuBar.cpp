/*
   Copyright (C) 2017-2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "MenuBar.h"

#include <QApplication>
#include <QAction>
#include <QMenu>
//#include <QActionGroup>
#include <QEvent>

#include "Utils/MenuUtils.h"
#include "Utils/ObjectsUtils.h"
#include "Utils/ThemeUtils.h"

struct MenuBar::Impl : public ControlsContainerEmitter
{
    bool isSlideShowMode;
    bool menuActionsHasDarkTheme;

    class ActionsEventFilter : public QObject
    {
    public:
        explicit ActionsEventFilter(QObject *parent = Q_NULLPTR)
            : QObject(parent)
        {}

    protected:
        bool eventFilter(QObject *o, QEvent *e)
        {
            if(e->type() != QEvent::Shortcut)
                return QObject::eventFilter(o, e);
            qobject_cast<QAction*>(o)->activate(QAction::Trigger);
            return true;
        }
    };
    ActionsEventFilter *actionsEventFilter;

    QMenu * const contextMenu;
#if defined (Q_OS_MAC)
    QMenu * const dockMenu;
#endif
    QMenu * const menuFile;
    QMenu * const menuEdit;
    QMenu * const menuView;
    QMenu * const menuHelp;
    QMenu * const menuReopenWith;

    QAction * const actionOpenFile;
    QAction * const actionOpenFolder;
    QAction * const actionSaveAs;
    QAction * const actionNewWindow;
    QAction * const actionNavigatePrevious;
    QAction * const actionNavigateNext;
    QAction * const actionStartSlideShow;
    QAction * const actionImageInformation;
    QAction * const actionPreferences;
    QAction * const actionExit;
    QAction * const actionRotateCounterclockwise;
    QAction * const actionRotateClockwise;
    QAction * const actionFlipHorizontal;
    QAction * const actionFlipVertical;
    QAction * const actionDeleteFile;
    QAction * const actionZoomOut;
    QAction * const actionZoomIn;
    QAction * const actionZoomReset;
    QAction * const actionZoomCustom;
    QAction * const actionZoomFitToWindow;
    QAction * const actionZoomOriginalSize;
    QAction * const actionZoomFullScreen;
    QAction * const actionShowMenuBar;
    QAction * const actionShowToolBar;
    QAction * const actionAbout;
    QAction * const actionAboutQt;
    QAction * const actionEditStylesheet;

    Impl(QWidget *parent, MenuBar *menubar)
        : isSlideShowMode(false)
        , menuActionsHasDarkTheme(false)
        , actionsEventFilter(new ActionsEventFilter(parent))
        , CONSTRUCT_OBJECT(contextMenu, QMenu, (parent))
#if defined (Q_OS_MAC)
        , CONSTRUCT_OBJECT(dockMenu, QMenu, (parent))
#endif
        , CONSTRUCT_OBJECT(menuFile, QMenu, (menubar))
        , CONSTRUCT_OBJECT(menuEdit, QMenu, (menubar))
        , CONSTRUCT_OBJECT(menuView, QMenu, (menubar))
        , CONSTRUCT_OBJECT(menuHelp, QMenu, (menubar))
        , CONSTRUCT_OBJECT(menuReopenWith, QMenu, (menuFile))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionOpenFile              , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionOpenFolder            , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionSaveAs                , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionNewWindow             , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionNavigatePrevious      , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionNavigateNext          , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionStartSlideShow        , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionImageInformation      , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionPreferences           , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionExit                  , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionRotateCounterclockwise, createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionRotateClockwise       , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionFlipHorizontal        , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionFlipVertical          , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionDeleteFile            , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionZoomOut               , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionZoomIn                , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionZoomReset             , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionZoomCustom            , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionZoomFitToWindow       , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionZoomOriginalSize      , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionZoomFullScreen        , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionShowMenuBar           , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionShowToolBar           , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionAbout                 , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionAboutQt               , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionEditStylesheet        , createWidgetAction(parent))
    {
        menuFile->addAction(actionOpenFile);
        actionOpenFile->setShortcut(QKeySequence::Open);
        actionOpenFile->setMenuRole(QAction::NoRole);
        menuFile->addAction(actionOpenFolder);
        actionOpenFolder->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_O);
        actionOpenFolder->setMenuRole(QAction::NoRole);
        menuFile->addAction(actionSaveAs);
        actionSaveAs->setShortcut(QKeySequence::Save);
        actionSaveAs->setMenuRole(QAction::NoRole);
        menuFile->addAction(actionNewWindow);
        actionNewWindow->setShortcut(Qt::CTRL + Qt::Key_N);
        actionNewWindow->setMenuRole(QAction::NoRole);
        menuFile->addSeparator();
        menuFile->addMenu(menuReopenWith);
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
        menuFile->addAction(actionImageInformation);
        actionImageInformation->setShortcuts(createAnyModifierShortcuts(Qt::Key_I));
        actionImageInformation->setMenuRole(QAction::NoRole);
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
        actionZoomOut->setShortcuts(createAnyModifierConjugatedShortcuts(Qt::Key_Minus, Qt::Key_Underscore));
        actionZoomOut->setMenuRole(QAction::NoRole);
        menuView->addAction(actionZoomIn);
        actionZoomIn->setShortcuts(createAnyModifierConjugatedShortcuts(Qt::Key_Plus, Qt::Key_Equal));
        actionZoomIn->setMenuRole(QAction::NoRole);
        menuView->addAction(actionZoomReset);
        actionZoomReset->setShortcuts(createAnyModifierConjugatedShortcuts(Qt::Key_0, Qt::Key_BracketLeft));
        actionZoomReset->setMenuRole(QAction::NoRole);
        menuView->addAction(actionZoomCustom);
        actionZoomCustom->setShortcuts(createAnyModifierShortcuts(Qt::Key_Z));
        actionZoomCustom->setMenuRole(QAction::NoRole);
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

        menuHelp->addAction(actionAbout);
        actionAbout->setMenuRole(QAction::AboutRole);
        menuHelp->addAction(actionAboutQt);
        actionAboutQt->setMenuRole(QAction::AboutQtRole);
#if !defined NDEBUG
        menuHelp->addSeparator();
        menuHelp->addAction(actionEditStylesheet);
        actionEditStylesheet->setMenuRole(QAction::NoRole);
        actionEditStylesheet->setText(QString::fromLatin1("Edit Stylesheet"));
#endif

        menubar->addMenu(menuFile);
        menubar->addMenu(menuEdit);
        menubar->addMenu(menuView);
        menubar->addMenu(menuHelp);

        contextMenu->addMenu(menuFile);
        contextMenu->addMenu(menuEdit);
        contextMenu->addMenu(menuView);
        contextMenu->addMenu(menuHelp);

#if defined (Q_OS_MAC)
        MenuUtils::DisableDictationMenuItem();
        MenuUtils::DisableCharacterPaletteMenuItem();
        MenuUtils::DisableShowTabBarMenuItem();
        MenuUtils::DisableEnterFullScreenMenuItem();

        dockMenu->addAction(actionNewWindow);
        MenuUtils::SetDockMenu(dockMenu);
#endif

        retranslate();
        updateIcons();
    }

    void retranslate()
    {
        menuFile->setTitle(qApp->translate("MenuBar", "&File"));
        menuEdit->setTitle(qApp->translate("MenuBar", "&Edit"));
        menuView->setTitle(qApp->translate("MenuBar", "&View"));
        menuHelp->setTitle(qApp->translate("MenuBar", "&Help"));
        menuReopenWith->setTitle(qApp->translate("MenuBar", "&Reopen With"));

        actionOpenFile->setText(qApp->translate("MenuBar", "&Open File"));
        actionOpenFolder->setText(qApp->translate("MenuBar", "Open &Folder"));
        actionSaveAs->setText(qApp->translate("MenuBar", "&Save As"));
        actionNewWindow->setText(qApp->translate("MenuBar", "New &Window"));
        actionNavigatePrevious->setText(qApp->translate("MenuBar", "Pre&vious"));
        actionNavigateNext->setText(qApp->translate("MenuBar", "&Next"));
        actionImageInformation->setText(qApp->translate("MenuBar", "Image &Information"));
        actionPreferences->setText(qApp->translate("MenuBar", "&Preferences"));
        actionExit->setText(qApp->translate("MenuBar", "&Exit"));
        actionRotateCounterclockwise->setText(qApp->translate("MenuBar", "Rotate &Counterclockwise"));
        actionRotateClockwise->setText(qApp->translate("MenuBar", "&Rotate Clockwise"));
        actionFlipHorizontal->setText(qApp->translate("MenuBar", "Flip &Horizontal"));
        actionFlipVertical->setText(qApp->translate("MenuBar", "Flip &Vertical"));
        actionDeleteFile->setText(qApp->translate("MenuBar", "&Delete File"));
        actionZoomOut->setText(qApp->translate("MenuBar", "Zoom &Out"));
        actionZoomIn->setText(qApp->translate("MenuBar", "Zoom &In"));
        actionZoomReset->setText(qApp->translate("MenuBar", "&Reset Zoom"));
        actionZoomCustom->setText(qApp->translate("MenuBar", "&Zoom..."));
        actionZoomFitToWindow->setText(qApp->translate("MenuBar", "Fit Image To &Window Size"));
        actionZoomOriginalSize->setText(qApp->translate("MenuBar", "Original &Size"));
        actionZoomFullScreen->setText(qApp->translate("MenuBar", "&Full Screen"));
        actionShowMenuBar->setText(qApp->translate("MenuBar", "Show &Menu Bar"));
        actionShowToolBar->setText(qApp->translate("MenuBar", "Show &Tool Bar"));
        actionAbout->setText(qApp->translate("MenuBar", "&About"));
        actionAboutQt->setText(qApp->translate("MenuBar", "About &Qt"));

        setSlideShowMode(isSlideShowMode);
    }

    void updateIcons()
    {
        menuActionsHasDarkTheme = ThemeUtils::WidgetHasDarkTheme(menuFile);
        menuReopenWith->setIcon                 (ThemeUtils::GetIcon(ThemeUtils::ICON_RESET                     , menuActionsHasDarkTheme));
        actionOpenFile->setIcon                 (ThemeUtils::GetIcon(ThemeUtils::ICON_OPEN                      , menuActionsHasDarkTheme));
        actionOpenFolder->setIcon               (ThemeUtils::GetIcon(ThemeUtils::ICON_OPEN                      , menuActionsHasDarkTheme));
        actionSaveAs->setIcon                   (ThemeUtils::GetIcon(ThemeUtils::ICON_SAVE_AS                   , menuActionsHasDarkTheme));
        actionNewWindow->setIcon                (ThemeUtils::GetIcon(ThemeUtils::ICON_NEW_WINDOW                , menuActionsHasDarkTheme));
        actionNavigatePrevious->setIcon         (ThemeUtils::GetIcon(ThemeUtils::ICON_LEFT                      , menuActionsHasDarkTheme));
        actionNavigateNext->setIcon             (ThemeUtils::GetIcon(ThemeUtils::ICON_RIGHT                     , menuActionsHasDarkTheme));
        actionImageInformation->setIcon         (ThemeUtils::GetIcon(ThemeUtils::ICON_ABOUT                     , menuActionsHasDarkTheme));
        actionPreferences->setIcon              (ThemeUtils::GetIcon(ThemeUtils::ICON_SETTINGS                  , menuActionsHasDarkTheme));
        actionExit->setIcon                     (ThemeUtils::GetIcon(ThemeUtils::ICON_EXIT                      , menuActionsHasDarkTheme));
        actionRotateCounterclockwise->setIcon   (ThemeUtils::GetIcon(ThemeUtils::ICON_ROTATE_COUNTERCLOCKWISE   , menuActionsHasDarkTheme));
        actionRotateClockwise->setIcon          (ThemeUtils::GetIcon(ThemeUtils::ICON_ROTATE_CLOCKWISE          , menuActionsHasDarkTheme));
        actionFlipHorizontal->setIcon           (ThemeUtils::GetIcon(ThemeUtils::ICON_FLIP_HORIZONTAL           , menuActionsHasDarkTheme));
        actionFlipVertical->setIcon             (ThemeUtils::GetIcon(ThemeUtils::ICON_FLIP_VERTICAL             , menuActionsHasDarkTheme));
        actionDeleteFile->setIcon               (ThemeUtils::GetIcon(ThemeUtils::ICON_DELETE                    , menuActionsHasDarkTheme));
        actionZoomOut->setIcon                  (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_OUT                  , menuActionsHasDarkTheme));
        actionZoomIn->setIcon                   (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_IN                   , menuActionsHasDarkTheme));
        actionZoomReset->setIcon                (ThemeUtils::GetIcon(ThemeUtils::ICON_RESET                     , menuActionsHasDarkTheme));
        actionZoomCustom->setIcon               (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_CUSTOM               , menuActionsHasDarkTheme));
        actionZoomFitToWindow->setIcon          (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_EMPTY                , menuActionsHasDarkTheme));
        actionZoomOriginalSize->setIcon         (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_IDENTITY             , menuActionsHasDarkTheme));
        actionZoomFullScreen->setIcon           (ThemeUtils::GetIcon(ThemeUtils::ICON_FULLSCREEN                , menuActionsHasDarkTheme));
        actionAbout->setIcon                    (ThemeUtils::GetIcon(ThemeUtils::ICON_ABOUT                     , menuActionsHasDarkTheme));
        actionAboutQt->setIcon                  (ThemeUtils::GetIcon(ThemeUtils::ICON_QT                        , menuActionsHasDarkTheme));
        actionEditStylesheet->setIcon           (ThemeUtils::GetIcon(ThemeUtils::ICON_SETTINGS                  , menuActionsHasDarkTheme));
        actionStartSlideShow->setIcon(ThemeUtils::GetIcon(isSlideShowMode ? ThemeUtils::ICON_STOP : ThemeUtils::ICON_PLAY, menuActionsHasDarkTheme));
    }

    void setSlideShowMode(bool isSlideShow)
    {
        isSlideShowMode = isSlideShow;
        if(!isSlideShowMode)
        {
            actionStartSlideShow->setText(qApp->translate("MenuBar", "Start S&lideshow"));
            actionStartSlideShow->setIcon(ThemeUtils::GetIcon(ThemeUtils::ICON_PLAY, menuActionsHasDarkTheme));
        }
        else
        {
            actionStartSlideShow->setText(qApp->translate("MenuBar", "Stop S&lideshow"));
            actionStartSlideShow->setIcon(ThemeUtils::GetIcon(ThemeUtils::ICON_STOP, menuActionsHasDarkTheme));
        }
    }

    QAction *createWidgetAction(QWidget *widget)
    {
        QAction *action = new QAction(widget);
        widget->addAction(action);
        action->installEventFilter(actionsEventFilter);
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
                << Qt::CTRL + Qt::ALT
                << Qt::SHIFT + Qt::META + Qt::CTRL
                << Qt::SHIFT + Qt::META + Qt::ALT
                << Qt::SHIFT + Qt::CTRL + Qt::ALT
                << Qt::META + Qt::CTRL + Qt::ALT
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

    QList<QKeySequence> createAnyModifierConjugatedShortcuts(Qt::Key master, Qt::Key slave, int defaultModifier = 0)
    {
        static const QList<int> modifiers = QList<int>()
                << 0
                << Qt::META
                << Qt::CTRL
                << Qt::ALT
                << Qt::META + Qt::CTRL
                << Qt::META + Qt::ALT
                << Qt::CTRL + Qt::ALT
                << Qt::META + Qt::CTRL + Qt::ALT;
        QList<QKeySequence> result;
        result.append(master + defaultModifier);
        for(QList<int>::ConstIterator it = modifiers.constBegin(); it != modifiers.constEnd(); ++it)
        {
            const int modifier = *it;
            if(modifier != defaultModifier)
                result.append(master + modifier);
            result.append(slave + modifier);
        }
        return result;
    }
};

MenuBar::MenuBar(QWidget *parent)
    : QMenuBar(parent)
    , m_impl(new Impl(parent, this))
{
    const ControlsContainerEmitter *emitter = MenuBar::emitter();
    connect(m_impl->actionOpenFile              , SIGNAL(triggered()), emitter, SIGNAL(openFileRequested())                 );
    connect(m_impl->actionOpenFolder            , SIGNAL(triggered()), emitter, SIGNAL(openFolderRequested())               );
    connect(m_impl->actionSaveAs                , SIGNAL(triggered()), emitter, SIGNAL(saveAsRequested())                   );
    connect(m_impl->actionNewWindow             , SIGNAL(triggered()), emitter, SIGNAL(newWindowRequested())                );
    connect(m_impl->actionNavigatePrevious      , SIGNAL(triggered()), emitter, SIGNAL(navigatePreviousRequested())         );
    connect(m_impl->actionNavigateNext          , SIGNAL(triggered()), emitter, SIGNAL(navigateNextRequested())             );
    connect(m_impl->actionStartSlideShow        , SIGNAL(triggered()), emitter, SIGNAL(startSlideShowRequested())           );
    connect(m_impl->actionImageInformation      , SIGNAL(triggered()), emitter, SIGNAL(imageInformationRequested())         );
    connect(m_impl->actionPreferences           , SIGNAL(triggered()), emitter, SIGNAL(preferencesRequested())              );
    connect(m_impl->actionExit                  , SIGNAL(triggered()), emitter, SIGNAL(exitRequested())                     );
    connect(m_impl->actionRotateCounterclockwise, SIGNAL(triggered()), emitter, SIGNAL(rotateCounterclockwiseRequested())   );
    connect(m_impl->actionRotateClockwise       , SIGNAL(triggered()), emitter, SIGNAL(rotateClockwiseRequested())          );
    connect(m_impl->actionFlipHorizontal        , SIGNAL(triggered()), emitter, SIGNAL(flipHorizontalRequested())           );
    connect(m_impl->actionFlipVertical          , SIGNAL(triggered()), emitter, SIGNAL(flipVerticalRequested())             );
    connect(m_impl->actionDeleteFile            , SIGNAL(triggered()), emitter, SIGNAL(deleteFileRequested())               );
    connect(m_impl->actionZoomOut               , SIGNAL(triggered()), emitter, SIGNAL(zoomOutRequested())                  );
    connect(m_impl->actionZoomIn                , SIGNAL(triggered()), emitter, SIGNAL(zoomInRequested())                   );
    connect(m_impl->actionZoomReset             , SIGNAL(triggered()), emitter, SIGNAL(zoomResetRequested())                );
    connect(m_impl->actionZoomCustom            , SIGNAL(triggered()), emitter, SIGNAL(zoomCustomRequested())               );
    connect(m_impl->actionZoomFitToWindow       , SIGNAL(triggered()), emitter, SIGNAL(zoomFitToWindowRequested())          );
    connect(m_impl->actionZoomOriginalSize      , SIGNAL(triggered()), emitter, SIGNAL(zoomOriginalSizeRequested())         );
    connect(m_impl->actionZoomFullScreen        , SIGNAL(triggered()), emitter, SIGNAL(zoomFullScreenRequested())           );
    connect(m_impl->actionShowMenuBar           , SIGNAL(triggered()), emitter, SIGNAL(showMenuBarRequested())              );
    connect(m_impl->actionShowToolBar           , SIGNAL(triggered()), emitter, SIGNAL(showToolBarRequested())              );
    connect(m_impl->actionAbout                 , SIGNAL(triggered()), emitter, SIGNAL(aboutRequested())                    );
    connect(m_impl->actionAboutQt               , SIGNAL(triggered()), emitter, SIGNAL(aboutQtRequested())                  );
    connect(m_impl->actionEditStylesheet        , SIGNAL(triggered()), emitter, SIGNAL(editStylesheetRequested())           );
}

MenuBar::~MenuBar()
{}

ControlsContainerEmitter *MenuBar::emitter()
{
    return m_impl.data();
}

QMenu *MenuBar::contextMenu()
{
    return m_impl->contextMenu;
}

QMenu *MenuBar::menuReopenWith()
{
    return m_impl->menuReopenWith;
}

void MenuBar::changeEvent(QEvent *event)
{
    switch(event->type())
    {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    case QEvent::ThemeChange:
#endif
    case QEvent::StyleChange:
    case QEvent::PaletteChange:
        m_impl->updateIcons();
        break;
    case QEvent::LanguageChange:
        m_impl->retranslate();
        break;
    default:
        break;
    }
    QMenuBar::changeEvent(event);
}

CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setOpenFileEnabled, m_impl->actionOpenFile)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setOpenFolderEnabled, m_impl->actionOpenFolder)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setSaveAsEnabled, m_impl->actionSaveAs)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setNewWindowEnabled, m_impl->actionNewWindow)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setNavigatePreviousEnabled, m_impl->actionNavigatePrevious)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setNavigateNextEnabled, m_impl->actionNavigateNext)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setStartSlideShowEnabled, m_impl->actionStartSlideShow)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setImageInformationEnabled, m_impl->actionImageInformation)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setPreferencesEnabled, m_impl->actionPreferences)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setExitEnabled, m_impl->actionExit)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setRotateCounterclockwiseEnabled, m_impl->actionRotateCounterclockwise)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setRotateClockwiseEnabled, m_impl->actionRotateClockwise)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setFlipHorizontalEnabled, m_impl->actionFlipHorizontal)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setFlipVerticalEnabled, m_impl->actionFlipVertical)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setDeleteFileEnabled, m_impl->actionDeleteFile)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setZoomOutEnabled, m_impl->actionZoomOut)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setZoomInEnabled, m_impl->actionZoomIn)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setZoomResetEnabled, m_impl->actionZoomReset)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setZoomCustomEnabled, m_impl->actionZoomCustom)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setZoomFitToWindowEnabled, m_impl->actionZoomFitToWindow)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setZoomOriginalSizeEnabled, m_impl->actionZoomOriginalSize)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setZoomFullScreenEnabled, m_impl->actionZoomFullScreen)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setShowMenuBarEnabled, m_impl->actionShowMenuBar)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setShowToolBarEnabled, m_impl->actionShowToolBar)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setAboutEnabled, m_impl->actionAbout)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setAboutQtEnabled, m_impl->actionAboutQt)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setEditStylesheetEnabled, m_impl->actionEditStylesheet)

CONTROLS_CONTAINER_SET_CHECKED_IMPL(MenuBar, setZoomFitToWindowChecked, m_impl->actionZoomFitToWindow)
CONTROLS_CONTAINER_SET_CHECKED_IMPL(MenuBar, setZoomOriginalSizeChecked, m_impl->actionZoomOriginalSize)
CONTROLS_CONTAINER_SET_CHECKED_IMPL(MenuBar, setZoomFullScreenChecked, m_impl->actionZoomFullScreen)
CONTROLS_CONTAINER_SET_CHECKED_IMPL(MenuBar, setShowMenuBarChecked, m_impl->actionShowMenuBar)
CONTROLS_CONTAINER_SET_CHECKED_IMPL(MenuBar, setShowToolBarChecked, m_impl->actionShowToolBar)

CONTROLS_CONTAINER_BOOL_ARG_FUNCTION_IMPL(MenuBar, setSlideShowMode, m_impl, setSlideShowMode)
