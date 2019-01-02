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

#include "ToolBar.h"

#include <QApplication>
#include <QString>
#include <QStyle>
#include <QStyleFactory>
#include <QToolButton>
#include <QHBoxLayout>
#include <QEvent>

#include "Utils/ObjectsUtils.h"
#include "Utils/ThemeUtils.h"

struct ToolBar::Impl : public ControlsContainerEmitter
{
    bool isSlideShowMode;
    bool toolBarButtonsHasDarkTheme;

    QToolButton * const navigatePrevious;
    QToolButton * const navigateNext;
    QToolButton * const startSlideShow;
    QToolButton * const zoomOut;
    QToolButton * const zoomIn;
    QToolButton * const zoomFitToWindow;
    QToolButton * const zoomOriginalSize;
    QToolButton * const zoomFullScreen;
    QToolButton * const rotateCounterclockwise;
    QToolButton * const rotateClockwise;
    QToolButton * const flipHorizontal;
    QToolButton * const flipVertical;
    QToolButton * const openFile;
    QToolButton * const saveFileAs;
    QToolButton * const deleteFile;
    QToolButton * const preferences;
    QToolButton * const exit;

    Impl(ToolBar *toolbar)
        : isSlideShowMode(false)
        , toolBarButtonsHasDarkTheme(false)
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
    {
        zoomFitToWindow->setCheckable(true);
        zoomOriginalSize->setCheckable(true);
        zoomFullScreen->setCheckable(true);

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

        retranslate();
        updateIcons();
    }

    QWidget *createVerticalSeparator(QWidget *parent) const
    {
        CREATE_OBJECT(separator, QFrame, (parent));
        return separator;
    }

    void retranslate()
    {
        navigatePrevious->setToolTip(qApp->translate("ToolBar", "Previous"));
        navigateNext->setToolTip(qApp->translate("ToolBar", "Next"));
        zoomOut->setToolTip(qApp->translate("ToolBar", "Zoom Out"));
        zoomIn->setToolTip(qApp->translate("ToolBar", "Zoom In"));
        zoomFitToWindow->setToolTip(qApp->translate("ToolBar", "Fit Image To Window Size"));
        zoomOriginalSize->setToolTip(qApp->translate("ToolBar", "Original Size"));
        zoomFullScreen->setToolTip(qApp->translate("ToolBar", "Full Screen"));
        rotateCounterclockwise->setToolTip(qApp->translate("ToolBar", "Rotate Counterclockwise"));
        rotateClockwise->setToolTip(qApp->translate("ToolBar", "Rotate Clockwise"));
        flipHorizontal->setToolTip(qApp->translate("ToolBar", "Flip Horizontal"));
        flipVertical->setToolTip(qApp->translate("ToolBar", "Flip Vertical"));
        openFile->setToolTip(qApp->translate("ToolBar", "Open File"));
        saveFileAs->setToolTip(qApp->translate("ToolBar", "Save File As"));
        deleteFile->setToolTip(qApp->translate("ToolBar", "Delete File"));
        preferences->setToolTip(qApp->translate("ToolBar", "Preferences"));
        exit->setToolTip(qApp->translate("ToolBar", "Exit"));
        setSlideShowMode(isSlideShowMode);
    }

    void updateIcons()
    {
        toolBarButtonsHasDarkTheme = ThemeUtils::WidgetHasDarkTheme(openFile);
        navigatePrevious->setIcon       (ThemeUtils::GetIcon(ThemeUtils::ICON_LEFT                      , toolBarButtonsHasDarkTheme));
        navigateNext->setIcon           (ThemeUtils::GetIcon(ThemeUtils::ICON_RIGHT                     , toolBarButtonsHasDarkTheme));
        zoomOut->setIcon                (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_OUT                  , toolBarButtonsHasDarkTheme));
        zoomIn->setIcon                 (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_IN                   , toolBarButtonsHasDarkTheme));
        zoomFitToWindow->setIcon        (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_EMPTY                , toolBarButtonsHasDarkTheme));
        zoomOriginalSize->setIcon       (ThemeUtils::GetIcon(ThemeUtils::ICON_ZOOM_IDENTITY             , toolBarButtonsHasDarkTheme));
        zoomFullScreen->setIcon         (ThemeUtils::GetIcon(ThemeUtils::ICON_FULLSCREEN                , toolBarButtonsHasDarkTheme));
        rotateCounterclockwise->setIcon (ThemeUtils::GetIcon(ThemeUtils::ICON_ROTATE_COUNTERCLOCKWISE   , toolBarButtonsHasDarkTheme));
        rotateClockwise->setIcon        (ThemeUtils::GetIcon(ThemeUtils::ICON_ROTATE_CLOCKWISE          , toolBarButtonsHasDarkTheme));
        flipHorizontal->setIcon         (ThemeUtils::GetIcon(ThemeUtils::ICON_FLIP_HORIZONTAL           , toolBarButtonsHasDarkTheme));
        flipVertical->setIcon           (ThemeUtils::GetIcon(ThemeUtils::ICON_FLIP_VERTICAL             , toolBarButtonsHasDarkTheme));
        openFile->setIcon               (ThemeUtils::GetIcon(ThemeUtils::ICON_OPEN                      , toolBarButtonsHasDarkTheme));
        saveFileAs->setIcon             (ThemeUtils::GetIcon(ThemeUtils::ICON_SAVE_AS                   , toolBarButtonsHasDarkTheme));
        deleteFile->setIcon             (ThemeUtils::GetIcon(ThemeUtils::ICON_DELETE                    , toolBarButtonsHasDarkTheme));
        preferences->setIcon            (ThemeUtils::GetIcon(ThemeUtils::ICON_SETTINGS                  , toolBarButtonsHasDarkTheme));
        exit->setIcon                   (ThemeUtils::GetIcon(ThemeUtils::ICON_EXIT                      , toolBarButtonsHasDarkTheme));
        startSlideShow->setIcon         (ThemeUtils::GetIcon(isSlideShowMode ? ThemeUtils::ICON_STOP : ThemeUtils::ICON_PLAY, toolBarButtonsHasDarkTheme));
    }

    void setSlideShowMode(bool isSlideShow)
    {
        isSlideShowMode = isSlideShow;
        if(!isSlideShowMode)
        {
            startSlideShow->setToolTip(qApp->translate("ToolBar", "Start Slideshow"));
            startSlideShow->setIcon(ThemeUtils::GetIcon(ThemeUtils::ICON_PLAY, toolBarButtonsHasDarkTheme));
        }
        else
        {
            startSlideShow->setToolTip(qApp->translate("ToolBar", "Stop Slideshow"));
            startSlideShow->setIcon(ThemeUtils::GetIcon(ThemeUtils::ICON_STOP, toolBarButtonsHasDarkTheme));
        }
    }
};


ToolBar::ToolBar(QWidget *parent)
    : AdjustableFrame(parent)
    , m_impl(new Impl(this))
{
#if defined (Q_OS_MAC)
        QStyle *style = NULL;
        if(QStyleFactory::keys().contains(QString::fromLatin1("Fusion"), Qt::CaseInsensitive))
             style = QStyleFactory::create(QString::fromLatin1("Fusion"));
        else if(QStyleFactory::keys().contains(QString::fromLatin1("Windows"), Qt::CaseInsensitive))
            style = QStyleFactory::create(QString::fromLatin1("Windows"));
        if(style)
        {
            setStyle(style);
            const QList<QWidget*> toolbarChildren = findChildren<QWidget*>();
            for(QList<QWidget*>::ConstIterator it = toolbarChildren.constBegin(); it != toolbarChildren.constEnd(); ++it)
                (*it)->setStyle(style);
        }
#endif

    connect(m_impl->navigatePrevious        , SIGNAL(clicked()), emitter(), SIGNAL(navigatePreviousRequested())         );
    connect(m_impl->navigateNext            , SIGNAL(clicked()), emitter(), SIGNAL(navigateNextRequested())             );
    connect(m_impl->startSlideShow          , SIGNAL(clicked()), emitter(), SIGNAL(startSlideShowRequested())           );
    connect(m_impl->zoomOut                 , SIGNAL(clicked()), emitter(), SIGNAL(zoomOutRequested())                  );
    connect(m_impl->zoomIn                  , SIGNAL(clicked()), emitter(), SIGNAL(zoomInRequested())                   );
    connect(m_impl->zoomFitToWindow         , SIGNAL(clicked()), emitter(), SIGNAL(zoomFitToWindowRequested())          );
    connect(m_impl->zoomOriginalSize        , SIGNAL(clicked()), emitter(), SIGNAL(zoomOriginalSizeRequested())         );
    connect(m_impl->zoomFullScreen          , SIGNAL(clicked()), emitter(), SIGNAL(zoomFullScreenRequested())           );
    connect(m_impl->rotateCounterclockwise  , SIGNAL(clicked()), emitter(), SIGNAL(rotateCounterclockwiseRequested())   );
    connect(m_impl->rotateClockwise         , SIGNAL(clicked()), emitter(), SIGNAL(rotateClockwiseRequested())          );
    connect(m_impl->flipHorizontal          , SIGNAL(clicked()), emitter(), SIGNAL(flipHorizontalRequested())           );
    connect(m_impl->flipVertical            , SIGNAL(clicked()), emitter(), SIGNAL(flipVerticalRequested())             );
    connect(m_impl->openFile                , SIGNAL(clicked()), emitter(), SIGNAL(openFileRequested())                 );
    connect(m_impl->saveFileAs              , SIGNAL(clicked()), emitter(), SIGNAL(saveAsRequested())                   );
    connect(m_impl->deleteFile              , SIGNAL(clicked()), emitter(), SIGNAL(deleteFileRequested())               );
    connect(m_impl->preferences             , SIGNAL(clicked()), emitter(), SIGNAL(preferencesRequested())              );
    connect(m_impl->exit                    , SIGNAL(clicked()), emitter(), SIGNAL(exitRequested())                     );
}

ToolBar::~ToolBar()
{}

ControlsContainerEmitter *ToolBar::emitter()
{
    return m_impl.data();
}

void ToolBar::changeEvent(QEvent *event)
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
    AdjustableFrame::changeEvent(event);
}

CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setOpenFileEnabled, m_impl->openFile)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(ToolBar, setOpenFolderEnabled)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setSaveAsEnabled, m_impl->saveFileAs)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(ToolBar, setNewWindowEnabled)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setNavigatePreviousEnabled, m_impl->navigatePrevious)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setNavigateNextEnabled, m_impl->navigateNext)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setStartSlideShowEnabled, m_impl->startSlideShow)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(ToolBar, setImageInformationEnabled)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setPreferencesEnabled, m_impl->preferences)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setExitEnabled, m_impl->exit)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setRotateCounterclockwiseEnabled, m_impl->rotateCounterclockwise)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setRotateClockwiseEnabled, m_impl->rotateClockwise)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setFlipHorizontalEnabled, m_impl->flipHorizontal)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setFlipVerticalEnabled, m_impl->flipVertical)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setDeleteFileEnabled, m_impl->deleteFile)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setZoomOutEnabled, m_impl->zoomOut)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setZoomInEnabled, m_impl->zoomIn)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(ToolBar, setZoomResetEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(ToolBar, setZoomCustomEnabled)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setZoomFitToWindowEnabled, m_impl->zoomFitToWindow)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setZoomOriginalSizeEnabled, m_impl->zoomOriginalSize)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setZoomFullScreenEnabled, m_impl->zoomFullScreen)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(ToolBar, setShowMenuBarEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(ToolBar, setShowToolBarEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(ToolBar, setAboutEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(ToolBar, setAboutQtEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(ToolBar, setEditStylesheetEnabled)

CONTROLS_CONTAINER_SET_CHECKED_IMPL(ToolBar, setZoomFitToWindowChecked, m_impl->zoomFitToWindow)
CONTROLS_CONTAINER_SET_CHECKED_IMPL(ToolBar, setZoomOriginalSizeChecked, m_impl->zoomOriginalSize)
CONTROLS_CONTAINER_SET_CHECKED_IMPL(ToolBar, setZoomFullScreenChecked, m_impl->zoomFullScreen)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(ToolBar, setShowMenuBarChecked)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(ToolBar, setShowToolBarChecked)

CONTROLS_CONTAINER_BOOL_ARG_FUNCTION_IMPL(ToolBar, setSlideShowMode, m_impl, setSlideShowMode)
