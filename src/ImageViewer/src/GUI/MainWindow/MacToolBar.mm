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

#include "MacToolBar.h"

#import <AppKit/AppKit.h>

#include <QApplication>
#include <QMacToolBarItem>

#include "Utils/ObjectsUtils.h"
#include "Utils/ThemeUtils.h"

struct MacToolBar::Impl : public ControlsContainerEmitter
{
    bool isSlideShowMode;

    QMacToolBarItem * const navigatePrevious;
    QMacToolBarItem * const navigateNext;
    QMacToolBarItem * const startSlideShow;
    QMacToolBarItem * const zoomOut;
    QMacToolBarItem * const zoomIn;
    QMacToolBarItem * const zoomFitToWindow;
    QMacToolBarItem * const zoomOriginalSize;
    QMacToolBarItem * const zoomFullScreen;
    QMacToolBarItem * const rotateCounterclockwise;
    QMacToolBarItem * const rotateClockwise;
    QMacToolBarItem * const flipHorizontal;
    QMacToolBarItem * const flipVertical;
    QMacToolBarItem * const openFile;
    QMacToolBarItem * const saveFileAs;
    QMacToolBarItem * const deleteFile;
    QMacToolBarItem * const preferences;
    QMacToolBarItem * const exit;

    QMacToolBarItem * const space;
    QMacToolBarItem * const flexibleSpace;

    Impl(MacToolBar *macToolBar)
        : isSlideShowMode(false)
        , CONSTRUCT_OBJECT(navigatePrevious, QMacToolBarItem, (macToolBar))
        , CONSTRUCT_OBJECT(navigateNext, QMacToolBarItem, (macToolBar))
        , CONSTRUCT_OBJECT(startSlideShow, QMacToolBarItem, (macToolBar))
        , CONSTRUCT_OBJECT(zoomOut, QMacToolBarItem, (macToolBar))
        , CONSTRUCT_OBJECT(zoomIn, QMacToolBarItem, (macToolBar))
        , CONSTRUCT_OBJECT(zoomFitToWindow, QMacToolBarItem, (macToolBar))
        , CONSTRUCT_OBJECT(zoomOriginalSize, QMacToolBarItem, (macToolBar))
        , CONSTRUCT_OBJECT(zoomFullScreen, QMacToolBarItem, (macToolBar))
        , CONSTRUCT_OBJECT(rotateCounterclockwise, QMacToolBarItem, (macToolBar))
        , CONSTRUCT_OBJECT(rotateClockwise, QMacToolBarItem, (macToolBar))
        , CONSTRUCT_OBJECT(flipHorizontal, QMacToolBarItem, (macToolBar))
        , CONSTRUCT_OBJECT(flipVertical, QMacToolBarItem, (macToolBar))
        , CONSTRUCT_OBJECT(openFile, QMacToolBarItem, (macToolBar))
        , CONSTRUCT_OBJECT(saveFileAs, QMacToolBarItem, (macToolBar))
        , CONSTRUCT_OBJECT(deleteFile, QMacToolBarItem, (macToolBar))
        , CONSTRUCT_OBJECT(preferences, QMacToolBarItem, (macToolBar))
        , CONSTRUCT_OBJECT(exit, QMacToolBarItem, (macToolBar))
        , CONSTRUCT_OBJECT(space, QMacToolBarItem, (macToolBar))
        , CONSTRUCT_OBJECT(flexibleSpace, QMacToolBarItem, (macToolBar))
    {
        space->setStandardItem(QMacToolBarItem::Space);
        flexibleSpace->setStandardItem(QMacToolBarItem::FlexibleSpace);

        macToolBar->setAllowedItems(QList<QMacToolBarItem*>()
                << navigatePrevious
                << navigateNext
                << startSlideShow
                << zoomOut
                << zoomIn
                << zoomFitToWindow
                << zoomOriginalSize
                << zoomFullScreen
                << rotateCounterclockwise
                << rotateClockwise
                << flipHorizontal
                << flipVertical
                << openFile
                << saveFileAs
                << deleteFile
                << preferences
                << exit
                << space
                << flexibleSpace);

        macToolBar->setItems(QList<QMacToolBarItem*>()
                << openFile
                << saveFileAs
                << deleteFile
                << flexibleSpace
                << navigatePrevious
                << navigateNext
                << flexibleSpace
                << rotateCounterclockwise
                << rotateClockwise
                << flexibleSpace
                << zoomOut
                << zoomIn
                << zoomFitToWindow
                << zoomOriginalSize);

        NSToolbar *nativeToolbar = macToolBar->nativeToolbar();
//        [nativeToolbar setAllowsUserCustomization:NO];
        [nativeToolbar setDisplayMode:NSToolbarDisplayModeIconAndLabel];
        [nativeToolbar setSizeMode:NSToolbarSizeModeSmall];

        retranslate();
        updateIcons();
    }

    void retranslate()
    {
        navigatePrevious->setText(qApp->translate("MacToolBar", "Previous"));
        navigateNext->setText(qApp->translate("MacToolBar", "Next"));
        zoomOut->setText(qApp->translate("MacToolBar", "Zoom Out"));
        zoomIn->setText(qApp->translate("MacToolBar", "Zoom In"));
        zoomFitToWindow->setText(qApp->translate("MacToolBar", "Fit To Window"));
        zoomOriginalSize->setText(qApp->translate("MacToolBar", "Original"));
        zoomFullScreen->setText(qApp->translate("MacToolBar", "Full Screen"));
        rotateCounterclockwise->setText(qApp->translate("MacToolBar", "Rotate CCW"));
        rotateClockwise->setText(qApp->translate("MacToolBar", "Rotate CW"));
        flipHorizontal->setText(qApp->translate("MacToolBar", "Flip Horizontal"));
        flipVertical->setText(qApp->translate("MacToolBar", "Flip Vertical"));
        openFile->setText(qApp->translate("MacToolBar", "Open"));
        saveFileAs->setText(qApp->translate("MacToolBar", "Save As"));
        deleteFile->setText(qApp->translate("MacToolBar", "Delete"));
        preferences->setText(qApp->translate("MacToolBar", "Preferences"));
        exit->setText(qApp->translate("MacToolBar", "Exit"));
        setSlideShowMode(isSlideShowMode);
    }

    void updateIcons()
    {
        navigatePrevious->setIcon       (ThemeUtils::GetIcon(ThemeUtils::ICON_LEFT                      , true));
        navigateNext->setIcon           (ThemeUtils::GetIcon(ThemeUtils::ICON_RIGHT                     , true));
        zoomOut->setIcon                (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_OUT                  , false));
        zoomIn->setIcon                 (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_IN                   , false));
        zoomFitToWindow->setIcon        (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_EMPTY                , false));
        zoomOriginalSize->setIcon       (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_IDENTITY             , false));
        zoomFullScreen->setIcon         (ThemeUtils::GetIcon(ThemeUtils::ICON_FULLSCREEN                , false));
        rotateCounterclockwise->setIcon (ThemeUtils::GetIcon(ThemeUtils::ICON_ROTATE_COUNTERCLOCKWISE   , false));
        rotateClockwise->setIcon        (ThemeUtils::GetIcon(ThemeUtils::ICON_ROTATE_CLOCKWISE          , false));
        flipHorizontal->setIcon         (ThemeUtils::GetIcon(ThemeUtils::ICON_FLIP_HORIZONTAL           , false));
        flipVertical->setIcon           (ThemeUtils::GetIcon(ThemeUtils::ICON_FLIP_VERTICAL             , false));
        openFile->setIcon               (ThemeUtils::GetIcon(ThemeUtils::ICON_OPEN                      , false));
        saveFileAs->setIcon             (ThemeUtils::GetIcon(ThemeUtils::ICON_SAVE_AS                   , false));
        deleteFile->setIcon             (ThemeUtils::GetIcon(ThemeUtils::ICON_DELETE                    , false));
        preferences->setIcon            (ThemeUtils::GetIcon(ThemeUtils::ICON_SETTINGS                  , false));
        exit->setIcon                   (ThemeUtils::GetIcon(ThemeUtils::ICON_EXIT                      , false));
        startSlideShow->setIcon         (ThemeUtils::GetIcon(isSlideShowMode ? ThemeUtils::ICON_STOP : ThemeUtils::ICON_PLAY, false));
    }

    void setSlideShowMode(bool isSlideShow)
    {
        isSlideShowMode = isSlideShow;
        if(!isSlideShowMode)
        {
            startSlideShow->setText(qApp->translate("MacToolBar", "Start Slideshow"));
            startSlideShow->setIcon(ThemeUtils::GetIcon(ThemeUtils::ICON_PLAY, false));
        }
        else
        {
            startSlideShow->setText(qApp->translate("MacToolBar", "Stop Slideshow"));
            startSlideShow->setIcon(ThemeUtils::GetIcon(ThemeUtils::ICON_STOP, false));
        }
    }

};

MacToolBar::MacToolBar(QObject *parent)
    : QMacToolBar(parent)
    , m_impl(new Impl(this))
{
    connect(m_impl->navigatePrevious        , SIGNAL(activated()), emitter(), SIGNAL(navigatePreviousRequested())         );
    connect(m_impl->navigateNext            , SIGNAL(activated()), emitter(), SIGNAL(navigateNextRequested())             );
    connect(m_impl->startSlideShow          , SIGNAL(activated()), emitter(), SIGNAL(startSlideShowRequested())           );
    connect(m_impl->zoomOut                 , SIGNAL(activated()), emitter(), SIGNAL(zoomOutRequested())                  );
    connect(m_impl->zoomIn                  , SIGNAL(activated()), emitter(), SIGNAL(zoomInRequested())                   );
    connect(m_impl->zoomFitToWindow         , SIGNAL(activated()), emitter(), SIGNAL(zoomFitToWindowRequested())          );
    connect(m_impl->zoomOriginalSize        , SIGNAL(activated()), emitter(), SIGNAL(zoomOriginalSizeRequested())         );
    connect(m_impl->zoomFullScreen          , SIGNAL(activated()), emitter(), SIGNAL(zoomFullScreenRequested())           );
    connect(m_impl->rotateCounterclockwise  , SIGNAL(activated()), emitter(), SIGNAL(rotateCounterclockwiseRequested())   );
    connect(m_impl->rotateClockwise         , SIGNAL(activated()), emitter(), SIGNAL(rotateClockwiseRequested())          );
    connect(m_impl->flipHorizontal          , SIGNAL(activated()), emitter(), SIGNAL(flipHorizontalRequested())           );
    connect(m_impl->flipVertical            , SIGNAL(activated()), emitter(), SIGNAL(flipVerticalRequested())             );
    connect(m_impl->openFile                , SIGNAL(activated()), emitter(), SIGNAL(openFileRequested())                 );
    connect(m_impl->saveFileAs              , SIGNAL(activated()), emitter(), SIGNAL(saveAsRequested())                   );
    connect(m_impl->deleteFile              , SIGNAL(activated()), emitter(), SIGNAL(deleteFileRequested())               );
    connect(m_impl->preferences             , SIGNAL(activated()), emitter(), SIGNAL(preferencesRequested())              );
    connect(m_impl->exit                    , SIGNAL(activated()), emitter(), SIGNAL(exitRequested())                     );
}

MacToolBar::~MacToolBar()
{

}

ControlsContainerEmitter *MacToolBar::emitter()
{
    return m_impl.data();
}

CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setOpenFileEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setOpenFolderEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setSaveAsEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setNewWindowEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setNavigatePreviousEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setNavigateNextEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setStartSlideShowEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setPreferencesEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setExitEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setRotateCounterclockwiseEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setRotateClockwiseEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setFlipHorizontalEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setFlipVerticalEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setDeleteFileEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setZoomOutEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setZoomInEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setZoomResetEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setZoomFitToWindowEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setZoomOriginalSizeEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setZoomFullScreenEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setShowMenuBarEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setShowToolBarEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setAboutEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setAboutQtEnabled)

CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setZoomFitToWindowChecked)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setZoomOriginalSizeChecked)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setZoomFullScreenChecked)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setShowMenuBarChecked)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setShowToolBarChecked)

CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(MacToolBar, setSlideShowMode)
