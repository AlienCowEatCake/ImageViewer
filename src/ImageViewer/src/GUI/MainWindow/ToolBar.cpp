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

#include "ToolBar.h"

#include <QApplication>
#include <QString>
#include <QStyle>
#include <QStyleFactory>
#include <QToolButton>
#include <QHBoxLayout>
#include <QEvent>

#include "Utils/IconThemeManager.h"
#include "Utils/ObjectsUtils.h"
#include "Utils/ThemeUtils.h"

struct ToolBar::Impl : public ControlsContainerEmitter
{
    bool isSlideShowMode;
    bool toolBarButtonsHasDarkTheme;
    bool toolBarButtonsFallbackIconRequired;

    ToolBar * const toolbar;
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

    explicit Impl(ToolBar *toolbar)
        : isSlideShowMode(false)
        , toolBarButtonsHasDarkTheme(false)
        , toolBarButtonsFallbackIconRequired(true)
        , toolbar(toolbar)
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
        startSlideShow->setCheckable(true);
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
        navigatePrevious->setToolTip(QCoreApplication::translate("ToolBar", "Previous"));
        navigateNext->setToolTip(QCoreApplication::translate("ToolBar", "Next"));
        zoomOut->setToolTip(QCoreApplication::translate("ToolBar", "Zoom Out"));
        zoomIn->setToolTip(QCoreApplication::translate("ToolBar", "Zoom In"));
        zoomFitToWindow->setToolTip(QCoreApplication::translate("ToolBar", "Fit Image To Window Size"));
        zoomOriginalSize->setToolTip(QCoreApplication::translate("ToolBar", "Original Size"));
        zoomFullScreen->setToolTip(QCoreApplication::translate("ToolBar", "Full Screen"));
        rotateCounterclockwise->setToolTip(QCoreApplication::translate("ToolBar", "Rotate Counterclockwise"));
        rotateClockwise->setToolTip(QCoreApplication::translate("ToolBar", "Rotate Clockwise"));
        flipHorizontal->setToolTip(QCoreApplication::translate("ToolBar", "Flip Horizontal"));
        flipVertical->setToolTip(QCoreApplication::translate("ToolBar", "Flip Vertical"));
        openFile->setToolTip(QCoreApplication::translate("ToolBar", "Open File"));
        saveFileAs->setToolTip(QCoreApplication::translate("ToolBar", "Save File As"));
        deleteFile->setToolTip(QCoreApplication::translate("ToolBar", "Delete File"));
        preferences->setToolTip(QCoreApplication::translate("ToolBar", "Preferences"));
        exit->setToolTip(QCoreApplication::translate("ToolBar", "Quit"));
        setSlideShowMode(isSlideShowMode);
    }

    void updateIcons()
    {
        IconThemeManager *iconThemeManager = IconThemeManager::instance();
        toolBarButtonsHasDarkTheme = ThemeUtils::WidgetHasDarkTheme(openFile);
        const bool toolBarIsRtl = toolbar->layoutDirection() == Qt::RightToLeft;
        navigatePrevious->setIcon       (iconThemeManager->GetIcon(toolBarIsRtl ? ThemeUtils::ICON_GO_NEXT      : ThemeUtils::ICON_GO_PREVIOUS  , toolBarButtonsFallbackIconRequired, toolBarButtonsHasDarkTheme));
        navigateNext->setIcon           (iconThemeManager->GetIcon(toolBarIsRtl ? ThemeUtils::ICON_GO_PREVIOUS  : ThemeUtils::ICON_GO_NEXT      , toolBarButtonsFallbackIconRequired, toolBarButtonsHasDarkTheme));
        zoomOut->setIcon                (iconThemeManager->GetIcon(ThemeUtils::ICON_ZOOM_OUT                , toolBarButtonsFallbackIconRequired, toolBarButtonsHasDarkTheme));
        zoomIn->setIcon                 (iconThemeManager->GetIcon(ThemeUtils::ICON_ZOOM_IN                 , toolBarButtonsFallbackIconRequired, toolBarButtonsHasDarkTheme));
        zoomFitToWindow->setIcon        (iconThemeManager->GetIcon(ThemeUtils::ICON_ZOOM_FIT_BEST           , toolBarButtonsFallbackIconRequired, toolBarButtonsHasDarkTheme));
        zoomOriginalSize->setIcon       (iconThemeManager->GetIcon(ThemeUtils::ICON_ZOOM_ORIGINAL           , toolBarButtonsFallbackIconRequired, toolBarButtonsHasDarkTheme));
        zoomFullScreen->setIcon         (iconThemeManager->GetIcon(ThemeUtils::ICON_VIEW_FULLSCREEN         , toolBarButtonsFallbackIconRequired, toolBarButtonsHasDarkTheme));
        rotateCounterclockwise->setIcon (iconThemeManager->GetIcon(ThemeUtils::ICON_OBJECT_ROTATE_LEFT      , toolBarButtonsFallbackIconRequired, toolBarButtonsHasDarkTheme));
        rotateClockwise->setIcon        (iconThemeManager->GetIcon(ThemeUtils::ICON_OBJECT_ROTATE_RIGHT     , toolBarButtonsFallbackIconRequired, toolBarButtonsHasDarkTheme));
        flipHorizontal->setIcon         (iconThemeManager->GetIcon(ThemeUtils::ICON_OBJECT_FLIP_HORIZONTAL  , toolBarButtonsFallbackIconRequired, toolBarButtonsHasDarkTheme));
        flipVertical->setIcon           (iconThemeManager->GetIcon(ThemeUtils::ICON_OBJECT_FLIP_VERTICAL    , toolBarButtonsFallbackIconRequired, toolBarButtonsHasDarkTheme));
        openFile->setIcon               (iconThemeManager->GetIcon(ThemeUtils::ICON_DOCUMENT_OPEN           , toolBarButtonsFallbackIconRequired, toolBarButtonsHasDarkTheme));
        saveFileAs->setIcon             (iconThemeManager->GetIcon(ThemeUtils::ICON_DOCUMENT_SAVE_AS        , toolBarButtonsFallbackIconRequired, toolBarButtonsHasDarkTheme));
        deleteFile->setIcon             (iconThemeManager->GetIcon(ThemeUtils::ICON_EDIT_DELETE             , toolBarButtonsFallbackIconRequired, toolBarButtonsHasDarkTheme));
        preferences->setIcon            (iconThemeManager->GetIcon(ThemeUtils::ICON_EDIT_PREFERENCES        , toolBarButtonsFallbackIconRequired, toolBarButtonsHasDarkTheme));
        exit->setIcon                   (iconThemeManager->GetIcon(ThemeUtils::ICON_APPLICATION_EXIT        , toolBarButtonsFallbackIconRequired, toolBarButtonsHasDarkTheme));
        startSlideShow->setIcon         (iconThemeManager->GetIcon(isSlideShowMode ? ThemeUtils::ICON_MEDIA_PLAYBACK_STOP : ThemeUtils::ICON_MEDIA_PLAYBACK_START, toolBarButtonsFallbackIconRequired, toolBarButtonsHasDarkTheme));
    }

    void setSlideShowMode(bool isSlideShow)
    {
        isSlideShowMode = isSlideShow;
        if(!isSlideShowMode)
        {
            startSlideShow->setChecked(false);
            startSlideShow->setToolTip(QCoreApplication::translate("ToolBar", "Start Slideshow"));
            startSlideShow->setIcon(IconThemeManager::instance()->GetIcon(ThemeUtils::ICON_MEDIA_PLAYBACK_START, toolBarButtonsFallbackIconRequired, toolBarButtonsHasDarkTheme));
        }
        else
        {
            startSlideShow->setChecked(true);
            startSlideShow->setToolTip(QCoreApplication::translate("ToolBar", "Stop Slideshow"));
            startSlideShow->setIcon(IconThemeManager::instance()->GetIcon(ThemeUtils::ICON_MEDIA_PLAYBACK_STOP, toolBarButtonsFallbackIconRequired, toolBarButtonsHasDarkTheme));
        }
    }
};


ToolBar::ToolBar(QWidget *parent)
    : AdjustableFrame(parent)
    , m_impl(new Impl(this))
{
#if defined (Q_OS_MAC)
        QStyle *style = Q_NULLPTR;
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

    connect(this, SIGNAL(polished()), this, SLOT(onPolished()), Qt::QueuedConnection);
    connect(IconThemeManager::instance(), SIGNAL(themeChanged(QString)), this, SLOT(onIconThemeChanged()));
}

ToolBar::~ToolBar()
{}

ControlsContainerEmitter *ToolBar::emitter()
{
    return m_impl.data();
}

void ToolBar::onPolished()
{
    m_impl->updateIcons();
}

void ToolBar::onIconThemeChanged()
{
    m_impl->updateIcons();
}

bool ToolBar::event(QEvent *event)
{
    const bool result = AdjustableFrame::event(event);
    if(event->type() == QEvent::Polish)
        Q_EMIT polished();
    return result;
}

void ToolBar::changeEvent(QEvent *event)
{
    AdjustableFrame::changeEvent(event);
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

CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setOpenFileEnabled, m_impl->openFile)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(ToolBar, setOpenFolderEnabled)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setSaveAsEnabled, m_impl->saveFileAs)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(ToolBar, setNewWindowEnabled)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setNavigatePreviousEnabled, m_impl->navigatePrevious)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setNavigateNextEnabled, m_impl->navigateNext)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setStartSlideShowEnabled, m_impl->startSlideShow)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(ToolBar, setImageInformationEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(ToolBar, setPrintEnabled)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setPreferencesEnabled, m_impl->preferences)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(ToolBar, setExitEnabled, m_impl->exit)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(ToolBar, setCopyEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(ToolBar, setCopyPathEnabled)
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
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(ToolBar, setCheckForUpdatesEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(ToolBar, setEditStylesheetEnabled)

CONTROLS_CONTAINER_SET_CHECKED_IMPL(ToolBar, setZoomFitToWindowChecked, m_impl->zoomFitToWindow)
CONTROLS_CONTAINER_SET_CHECKED_IMPL(ToolBar, setZoomOriginalSizeChecked, m_impl->zoomOriginalSize)
CONTROLS_CONTAINER_SET_CHECKED_IMPL(ToolBar, setZoomFullScreenChecked, m_impl->zoomFullScreen)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(ToolBar, setShowMenuBarChecked)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(ToolBar, setShowToolBarChecked)

CONTROLS_CONTAINER_BOOL_ARG_FUNCTION_IMPL(ToolBar, setSlideShowMode, m_impl, setSlideShowMode)
