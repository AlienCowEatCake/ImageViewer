/*
   Copyright (C) 2017-2026 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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
#include <QStyle>

#include "Utils/MenuUtils.h"
#include "Utils/IconThemeManager.h"
#include "Utils/ObjectsUtils.h"
#include "Utils/ThemeUtils.h"

struct MenuBar::Impl : public ControlsContainerEmitter
{
    bool isSlideShowMode;
    bool menuActionsHasDarkTheme;
    bool menuActionsFallbackIconRequired;

    class ActionsEventFilter : public QObject
    {
    public:
        explicit ActionsEventFilter(QObject *parent = Q_NULLPTR)
            : QObject(parent)
        {}

    protected:
        bool eventFilter(QObject *o, QEvent *e) Q_DECL_OVERRIDE
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
    QAction * const actionPrint;
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
    QAction * const actionCheckForUpdates;
    QAction * const actionEditStylesheet;

    Impl(QWidget *parent, MenuBar *menubar)
        : isSlideShowMode(false)
        , menuActionsHasDarkTheme(false)
        , menuActionsFallbackIconRequired(false)
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
        , CONSTRUCT_OBJECT_FROM_POINTER(actionPrint                 , createWidgetAction(parent))
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
        , CONSTRUCT_OBJECT_FROM_POINTER(actionCheckForUpdates       , createWidgetAction(parent))
        , CONSTRUCT_OBJECT_FROM_POINTER(actionEditStylesheet        , createWidgetAction(parent))
    {
        menuFile->addAction(actionOpenFile);
        actionOpenFile->setShortcut(QKeySequence::Open);
        actionOpenFile->setMenuRole(QAction::NoRole);
        menuFile->addAction(actionOpenFolder);
        actionOpenFolder->setShortcut(Qt::CTRL | Qt::ALT | Qt::Key_O);
        actionOpenFolder->setMenuRole(QAction::NoRole);
        menuFile->addAction(actionSaveAs);
        actionSaveAs->setShortcut(QKeySequence::Save);
        actionSaveAs->setMenuRole(QAction::NoRole);
        menuFile->addAction(actionNewWindow);
        actionNewWindow->setShortcut(Qt::CTRL | Qt::Key_N);
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
        actionStartSlideShow->setShortcuts(createAnyModifierShortcuts(Qt::Key_W, 0, (QList<QKeySequence>() << (Qt::CTRL | Qt::Key_W))));
        actionStartSlideShow->setMenuRole(QAction::NoRole);
        actionStartSlideShow->setCheckable(true);
        menuFile->addSeparator();
        menuFile->addAction(actionImageInformation);
        actionImageInformation->setShortcuts(createAnyModifierShortcuts(Qt::Key_I));
        actionImageInformation->setMenuRole(QAction::NoRole);
#if defined (ENABLE_PRINT_SUPPORT)
        menuFile->addSeparator();
        menuFile->addAction(actionPrint);
        actionPrint->setMenuRole(QAction::NoRole);
        actionPrint->setShortcut(QKeySequence::Print);
#endif
        menuFile->addSeparator();
        menuFile->addAction(actionPreferences);
#if defined (Q_OS_MAC)
        actionPreferences->setShortcuts(QList<QKeySequence>() << (Qt::CTRL | Qt::Key_Comma) << createAnyModifierShortcuts(Qt::Key_P, 0, (QList<QKeySequence>() << QKeySequence::Print)));
#else
        actionPreferences->setShortcuts(QList<QKeySequence>() << createAnyModifierShortcuts(Qt::Key_P, 0, (QList<QKeySequence>() << QKeySequence::Print)) << (Qt::CTRL | Qt::Key_Comma));
#endif
        actionPreferences->setMenuRole(QAction::PreferencesRole);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);
#if defined (Q_OS_WIN)
        actionExit->setShortcuts(QList<QKeySequence>() << (Qt::ALT | Qt::Key_F4) << (Qt::CTRL | Qt::Key_Q) << (Qt::CTRL | Qt::Key_W));
#else
        actionExit->setShortcuts(QList<QKeySequence>() << (Qt::CTRL | Qt::Key_Q) << (Qt::ALT | Qt::Key_F4) << (Qt::CTRL | Qt::Key_W));
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
        actionZoomFitToWindow->setShortcuts(createAnyModifierShortcuts(Qt::Key_F, 0, (QList<QKeySequence>() << (Qt::CTRL | Qt::META | Qt::Key_F))));
        actionZoomFitToWindow->setMenuRole(QAction::NoRole);
        actionZoomFitToWindow->setCheckable(true);
        menuView->addAction(actionZoomOriginalSize);
        actionZoomOriginalSize->setShortcuts(createAnyModifierShortcuts(Qt::Key_G));
        actionZoomOriginalSize->setMenuRole(QAction::NoRole);
        actionZoomOriginalSize->setCheckable(true);
        menuView->addSeparator();
        menuView->addAction(actionZoomFullScreen);
#if defined (Q_OS_MAC)
        actionZoomFullScreen->setShortcuts(QList<QKeySequence>() << (Qt::CTRL | Qt::META | Qt::Key_F) << Qt::Key_F11);
#else
        actionZoomFullScreen->setShortcuts(QList<QKeySequence>() << Qt::Key_F11 << (Qt::CTRL | Qt::META | Qt::Key_F));
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
#if defined(ENABLE_UPDATE_CHECKING)
        menuHelp->addSeparator();
        menuHelp->addAction(actionCheckForUpdates);
        actionCheckForUpdates->setMenuRole(QAction::ApplicationSpecificRole);
#endif
#if !defined (NDEBUG)
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
        menuFile->setTitle(QCoreApplication::translate("MenuBar", "&File"));
        menuEdit->setTitle(QCoreApplication::translate("MenuBar", "&Edit"));
        menuView->setTitle(QCoreApplication::translate("MenuBar", "&View"));
        menuHelp->setTitle(QCoreApplication::translate("MenuBar", "&Help"));
        menuReopenWith->setTitle(QCoreApplication::translate("MenuBar", "&Reopen With"));

        actionOpenFile->setText(QCoreApplication::translate("MenuBar", "&Open File"));
        actionOpenFolder->setText(QCoreApplication::translate("MenuBar", "Open &Folder"));
        actionSaveAs->setText(QCoreApplication::translate("MenuBar", "&Save As"));
        actionNewWindow->setText(QCoreApplication::translate("MenuBar", "New &Window"));
        actionNavigatePrevious->setText(QCoreApplication::translate("MenuBar", "Pre&vious"));
        actionNavigateNext->setText(QCoreApplication::translate("MenuBar", "&Next"));
        actionImageInformation->setText(QCoreApplication::translate("MenuBar", "Image &Information"));
        actionPrint->setText(QCoreApplication::translate("MenuBar", "&Print"));
        actionPreferences->setText(QCoreApplication::translate("MenuBar", "Pr&eferences"));
        actionExit->setText(QCoreApplication::translate("MenuBar", "&Quit"));
        actionRotateCounterclockwise->setText(QCoreApplication::translate("MenuBar", "Rotate &Counterclockwise"));
        actionRotateClockwise->setText(QCoreApplication::translate("MenuBar", "&Rotate Clockwise"));
        actionFlipHorizontal->setText(QCoreApplication::translate("MenuBar", "Flip &Horizontal"));
        actionFlipVertical->setText(QCoreApplication::translate("MenuBar", "Flip &Vertical"));
        actionDeleteFile->setText(QCoreApplication::translate("MenuBar", "&Delete File"));
        actionZoomOut->setText(QCoreApplication::translate("MenuBar", "Zoom &Out"));
        actionZoomIn->setText(QCoreApplication::translate("MenuBar", "Zoom &In"));
        actionZoomReset->setText(QCoreApplication::translate("MenuBar", "&Reset Zoom"));
        actionZoomCustom->setText(QCoreApplication::translate("MenuBar", "&Zoom\xe2\x80\xa6"));
        actionZoomFitToWindow->setText(QCoreApplication::translate("MenuBar", "Fit Image To &Window Size"));
        actionZoomOriginalSize->setText(QCoreApplication::translate("MenuBar", "Original &Size"));
        actionZoomFullScreen->setText(QCoreApplication::translate("MenuBar", "&Full Screen"));
        actionShowMenuBar->setText(QCoreApplication::translate("MenuBar", "Show &Menu Bar"));
        actionShowToolBar->setText(QCoreApplication::translate("MenuBar", "Show &Toolbar"));
        actionAbout->setText(QCoreApplication::translate("MenuBar", "&About"));
        actionAboutQt->setText(QCoreApplication::translate("MenuBar", "About &Qt"));
        actionCheckForUpdates->setText(QCoreApplication::translate("MenuBar", "Check for &Updates"));

        setSlideShowMode(isSlideShowMode);
    }

    void updateIcons()
    {
        menuActionsHasDarkTheme = ThemeUtils::WidgetHasDarkTheme(menuFile);
        const bool menuBarIsRtl = menuFile->layoutDirection() == Qt::RightToLeft;
        menuReopenWith->setIcon                 (getMenuIcon(ThemeUtils::ICON_DOCUMENT_OPEN_WITH    ));
        actionOpenFile->setIcon                 (getMenuIcon(ThemeUtils::ICON_DOCUMENT_OPEN         ));
        actionOpenFolder->setIcon               (getMenuIcon(ThemeUtils::ICON_FOLDER_OPEN           ));
        actionSaveAs->setIcon                   (getMenuIcon(ThemeUtils::ICON_DOCUMENT_SAVE_AS      ));
        actionNewWindow->setIcon                (getMenuIcon(ThemeUtils::ICON_WINDOW_NEW            ));
        actionNavigatePrevious->setIcon         (getMenuIcon(menuBarIsRtl ? ThemeUtils::ICON_GO_NEXT     : ThemeUtils::ICON_GO_PREVIOUS));
        actionNavigateNext->setIcon             (getMenuIcon(menuBarIsRtl ? ThemeUtils::ICON_GO_PREVIOUS : ThemeUtils::ICON_GO_NEXT    ));
        actionImageInformation->setIcon         (getMenuIcon(ThemeUtils::ICON_DOCUMENT_PROPERTIES   ));
        actionPrint->setIcon                    (getMenuIcon(ThemeUtils::ICON_DOCUMENT_PRINT        ));
        actionPreferences->setIcon              (getMenuIcon(ThemeUtils::ICON_EDIT_PREFERENCES      ));
        actionExit->setIcon                     (getMenuIcon(ThemeUtils::ICON_APPLICATION_EXIT      ));
        actionRotateCounterclockwise->setIcon   (getMenuIcon(ThemeUtils::ICON_OBJECT_ROTATE_LEFT    ));
        actionRotateClockwise->setIcon          (getMenuIcon(ThemeUtils::ICON_OBJECT_ROTATE_RIGHT   ));
        actionFlipHorizontal->setIcon           (getMenuIcon(ThemeUtils::ICON_OBJECT_FLIP_HORIZONTAL));
        actionFlipVertical->setIcon             (getMenuIcon(ThemeUtils::ICON_OBJECT_FLIP_VERTICAL  ));
        actionDeleteFile->setIcon               (getMenuIcon(ThemeUtils::ICON_EDIT_DELETE           ));
        actionZoomOut->setIcon                  (getMenuIcon(ThemeUtils::ICON_ZOOM_OUT              ));
        actionZoomIn->setIcon                   (getMenuIcon(ThemeUtils::ICON_ZOOM_IN               ));
        actionZoomReset->setIcon                (getMenuIcon(ThemeUtils::ICON_VIEW_REFRESH          ));
        actionZoomCustom->setIcon               (getMenuIcon(ThemeUtils::ICON_ZOOM_CUSTOM           ));
        actionZoomFitToWindow->setIcon          (getMenuIcon(ThemeUtils::ICON_ZOOM_FIT_BEST         ));
        actionZoomOriginalSize->setIcon         (getMenuIcon(ThemeUtils::ICON_ZOOM_ORIGINAL         ));
        actionZoomFullScreen->setIcon           (getMenuIcon(ThemeUtils::ICON_VIEW_FULLSCREEN       ));
        actionAbout->setIcon                    (getMenuIcon(ThemeUtils::ICON_HELP_ABOUT            ));
        actionAboutQt->setIcon                  (getMenuIcon(ThemeUtils::ICON_HELP_ABOUT_QT         ));
        actionCheckForUpdates->setIcon          (getMenuIcon(ThemeUtils::ICON_SYNC_SYNCHRONIZING    ));
        actionEditStylesheet->setIcon           (getMenuIcon(ThemeUtils::ICON_EDIT_PREFERENCES      ));
        actionStartSlideShow->setIcon(getMenuIcon(isSlideShowMode ? ThemeUtils::ICON_MEDIA_PLAYBACK_STOP : ThemeUtils::ICON_MEDIA_PLAYBACK_START));
    }

    void setSlideShowMode(bool isSlideShow)
    {
        isSlideShowMode = isSlideShow;
        if(!isSlideShowMode)
        {
            actionStartSlideShow->setChecked(false);
            actionStartSlideShow->setText(QCoreApplication::translate("MenuBar", "Start S&lideshow"));
            actionStartSlideShow->setIcon(getMenuIcon(ThemeUtils::ICON_MEDIA_PLAYBACK_START));
        }
        else
        {
            actionStartSlideShow->setChecked(true);
            actionStartSlideShow->setText(QCoreApplication::translate("MenuBar", "Stop S&lideshow"));
            actionStartSlideShow->setIcon(getMenuIcon(ThemeUtils::ICON_MEDIA_PLAYBACK_STOP));
        }
    }

    QIcon getMenuIcon(ThemeUtils::IconTypes type) const
    {
        QIcon icon = IconThemeManager::instance()->GetIcon(type, menuActionsFallbackIconRequired, menuActionsHasDarkTheme);

        // https://bugreports.qt.io/browse/QTBUG-140898
#if defined (Q_OS_WIN) && (QT_VERSION >= QT_VERSION_CHECK(6, 9, 2))
        static const QList<QIcon::Mode> modes = QList<QIcon::Mode>() << QIcon::Normal << QIcon::Disabled << QIcon::Active << QIcon::Selected;
        static const QList<QIcon::State> states = QList<QIcon::State>() << QIcon::On << QIcon::Off;
        const qreal menuIconSize = qApp->style()->pixelMetric(QStyle::PM_SmallIconSize);

        QIcon fixedIcon;
        for(QList<QIcon::Mode>::ConstIterator modeIt = modes.constBegin(), modeItEnd = modes.constEnd(); modeIt != modeItEnd; ++modeIt)
        {
            const QIcon::Mode mode = *modeIt;
            for(QList<QIcon::State>::ConstIterator stateIt = states.constBegin(), stateItEnd = states.constEnd(); stateIt != stateItEnd; ++stateIt)
            {
                const QIcon::State state = *stateIt;
                const QList<QSize> sizes = icon.availableSizes(mode, state);
                for(QList<QSize>::ConstIterator sizeIt = sizes.constBegin(), sizeItEnd = sizes.constEnd(); sizeIt != sizeItEnd; ++sizeIt)
                {
                    const QSize &size = *sizeIt;
                    QPixmap pixmap = icon.pixmap(size, mode, state);
                    pixmap.setDevicePixelRatio(qMax(size.width(), size.height()) / menuIconSize);
                    fixedIcon.addPixmap(pixmap, mode, state);
                }
            }
        }
        if(!fixedIcon.isNull())
            icon = fixedIcon;
#endif

        return icon;
    }

    QAction *createWidgetAction(QWidget *widget)
    {
        QAction *action = new QAction(widget);
        widget->addAction(action);
        action->installEventFilter(actionsEventFilter);
        return action;
    }

    QList<QKeySequence> createAnyModifierShortcuts(Qt::Key key, int defaultModifier = 0, const QList<QKeySequence> &blacklist = QList<QKeySequence>()) const
    {
        // Qt::ALT modifier is reserved for accelerators
        static const QList<int> modifiers = QList<int>()
                << 0
                << Qt::SHIFT
                << Qt::META
                << Qt::CTRL
                << (Qt::SHIFT | Qt::META)
                << (Qt::SHIFT | Qt::CTRL)
                << (Qt::META | Qt::CTRL)
                << (Qt::SHIFT | Qt::META | Qt::CTRL);
        QList<QKeySequence> result;
        result.append(key + defaultModifier);
        for(QList<int>::ConstIterator it = modifiers.constBegin(); it != modifiers.constEnd(); ++it)
        {
            const int modifier = *it;
            if(modifier != defaultModifier && !match(key + modifier, blacklist))
                result.append(key + modifier);
        }
        return result;
    }

    QList<QKeySequence> createAnyModifierConjugatedShortcuts(Qt::Key master, Qt::Key slave, int defaultModifier = 0, const QList<QKeySequence> &blacklist = QList<QKeySequence>()) const
    {
        // Qt::ALT modifier is reserved for accelerators
        static const QList<int> modifiers = QList<int>()
                << 0
                << Qt::META
                << Qt::CTRL
                << (Qt::META | Qt::CTRL);
        QList<QKeySequence> result;
        result.append(master + defaultModifier);
        for(QList<int>::ConstIterator it = modifiers.constBegin(); it != modifiers.constEnd(); ++it)
        {
            const int modifier = *it;
            if(modifier != defaultModifier && !match(master + modifier, blacklist))
                result.append(master + modifier);
            if(!match(slave + modifier, blacklist))
                result.append(slave + modifier);
        }
        return result;
    }

    bool match(const QKeySequence &sequence, const QList<QKeySequence> &list) const
    {
        for(QList<QKeySequence>::ConstIterator it = list.begin(); it != list.end(); ++it)
            if(it->matches(sequence) == QKeySequence::ExactMatch)
                return true;
        return false;
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
    connect(m_impl->actionPrint                 , SIGNAL(triggered()), emitter, SIGNAL(printRequested())                    );
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
    connect(m_impl->actionCheckForUpdates       , SIGNAL(triggered()), emitter, SIGNAL(checkForUpdatesRequested())          );
    connect(m_impl->actionEditStylesheet        , SIGNAL(triggered()), emitter, SIGNAL(editStylesheetRequested())           );

    connect(this, SIGNAL(polished()), this, SLOT(onPolished()), Qt::QueuedConnection);
    connect(IconThemeManager::instance(), SIGNAL(themeChanged(QString)), this, SLOT(onIconThemeChanged()));
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

void MenuBar::onPolished()
{
    m_impl->updateIcons();
}

void MenuBar::onIconThemeChanged()
{
    m_impl->updateIcons();
}

bool MenuBar::event(QEvent *event)
{
    const bool result = QMenuBar::event(event);
    if(event->type() == QEvent::Polish)
        Q_EMIT polished();
    return result;
}

void MenuBar::changeEvent(QEvent *event)
{
    QMenuBar::changeEvent(event);
    switch(event->type())
    {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    case QEvent::ThemeChange:
#endif
    case QEvent::StyleChange:
    case QEvent::PaletteChange:
    case QEvent::LayoutDirectionChange:
        m_impl->updateIcons();
        break;
    case QEvent::LanguageChange:
        m_impl->retranslate();
        break;
    default:
        break;
    }
}

CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setOpenFileEnabled, m_impl->actionOpenFile)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setOpenFolderEnabled, m_impl->actionOpenFolder)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setSaveAsEnabled, m_impl->actionSaveAs)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setNewWindowEnabled, m_impl->actionNewWindow)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setNavigatePreviousEnabled, m_impl->actionNavigatePrevious)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setNavigateNextEnabled, m_impl->actionNavigateNext)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setStartSlideShowEnabled, m_impl->actionStartSlideShow)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setImageInformationEnabled, m_impl->actionImageInformation)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setPrintEnabled, m_impl->actionPrint)
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
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setCheckForUpdatesEnabled, m_impl->actionCheckForUpdates)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(MenuBar, setEditStylesheetEnabled, m_impl->actionEditStylesheet)

CONTROLS_CONTAINER_SET_CHECKED_IMPL(MenuBar, setZoomFitToWindowChecked, m_impl->actionZoomFitToWindow)
CONTROLS_CONTAINER_SET_CHECKED_IMPL(MenuBar, setZoomOriginalSizeChecked, m_impl->actionZoomOriginalSize)
CONTROLS_CONTAINER_SET_CHECKED_IMPL(MenuBar, setZoomFullScreenChecked, m_impl->actionZoomFullScreen)
CONTROLS_CONTAINER_SET_CHECKED_IMPL(MenuBar, setShowMenuBarChecked, m_impl->actionShowMenuBar)
CONTROLS_CONTAINER_SET_CHECKED_IMPL(MenuBar, setShowToolBarChecked, m_impl->actionShowToolBar)

CONTROLS_CONTAINER_BOOL_ARG_FUNCTION_IMPL(MenuBar, setSlideShowMode, m_impl, setSlideShowMode)
