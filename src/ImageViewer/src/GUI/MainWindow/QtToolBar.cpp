/*
   Copyright (C) 2024-2026 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "QtToolBar.h"

#include <QAction>
#include <QApplication>
#include <QEvent>
#include <QString>
#include <QStyle>
#include <QStyleFactory>

#include "Utils/IconThemeManager.h"
#include "Utils/ThemeUtils.h"

struct QtToolBar::Impl : public ControlsContainerEmitter
{
    bool isSlideShowMode;
    bool toolBarButtonsHasDarkTheme;
    bool toolBarButtonsFallbackIconRequired;

    QtToolBar * const toolbar;
    QAction *openFile;
    QAction *saveFileAs;
    QAction *deleteFile;
    QAction *navigatePrevious;
    QAction *navigateNext;
    QAction *startSlideShow;
    QAction *zoomOut;
    QAction *zoomIn;
    QAction *zoomFitToWindow;
    QAction *zoomOriginalSize;
    QAction *zoomFullScreen;
    QAction *rotateCounterclockwise;
    QAction *rotateClockwise;
    QAction *flipHorizontal;
    QAction *flipVertical;
    QAction *preferences;
    QAction *exit;

    explicit Impl(QtToolBar *toolbar)
        : isSlideShowMode(false)
        , toolBarButtonsHasDarkTheme(false)
        , toolBarButtonsFallbackIconRequired(true)
        , toolbar(toolbar)
    {
        openFile = toolbar->addAction(QString());
        saveFileAs = toolbar->addAction(QString());
        deleteFile = toolbar->addAction(QString());
        toolbar->addSeparator();
        navigatePrevious = toolbar->addAction(QString());
        navigateNext = toolbar->addAction(QString());
        startSlideShow = toolbar->addAction(QString());
        toolbar->addSeparator();
        zoomOut = toolbar->addAction(QString());
        zoomIn = toolbar->addAction(QString());
        zoomFitToWindow = toolbar->addAction(QString());
        zoomOriginalSize = toolbar->addAction(QString());
        zoomFullScreen = toolbar->addAction(QString());
        toolbar->addSeparator();
        rotateCounterclockwise = toolbar->addAction(QString());
        rotateClockwise = toolbar->addAction(QString());
        flipHorizontal = toolbar->addAction(QString());
        flipVertical = toolbar->addAction(QString());
        toolbar->addSeparator();
        preferences = toolbar->addAction(QString());
        exit = toolbar->addAction(QString());

        startSlideShow->setCheckable(true);
        zoomFitToWindow->setCheckable(true);
        zoomOriginalSize->setCheckable(true);
        zoomFullScreen->setCheckable(true);

        retranslate();
        updateIcons();
    }

    void retranslate()
    {
        navigatePrevious->setToolTip(QCoreApplication::translate("QtToolBar", "Previous"));
        navigatePrevious->setText(QCoreApplication::translate("QtToolBar", "Previous", "Short form of 'Previous'"));
        navigateNext->setToolTip(QCoreApplication::translate("QtToolBar", "Next"));
        navigateNext->setText(QCoreApplication::translate("QtToolBar", "Next", "Short form of 'Next'"));
        startSlideShow->setText(QCoreApplication::translate("QtToolBar", "Slideshow", "Short form of 'Slideshow'"));
        zoomOut->setToolTip(QCoreApplication::translate("QtToolBar", "Zoom Out"));
        zoomOut->setText(QCoreApplication::translate("QtToolBar", "Zoom Out", "Short form of 'Zoom Out'"));
        zoomIn->setToolTip(QCoreApplication::translate("QtToolBar", "Zoom In"));
        zoomIn->setText(QCoreApplication::translate("QtToolBar", "Zoom In", "Short form of 'Zoom In'"));
        zoomFitToWindow->setToolTip(QCoreApplication::translate("QtToolBar", "Fit Image To Window Size"));
        zoomFitToWindow->setText(QCoreApplication::translate("QtToolBar", "Fit", "Short form of 'Fit Image To Window Size'"));
        zoomOriginalSize->setToolTip(QCoreApplication::translate("QtToolBar", "Original Size"));
        zoomOriginalSize->setText(QCoreApplication::translate("QtToolBar", "1:1", "Short form of 'Original Size'"));
        zoomFullScreen->setToolTip(QCoreApplication::translate("QtToolBar", "Full Screen"));
        zoomFullScreen->setText(QCoreApplication::translate("QtToolBar", "Full Screen", "Short form of 'Full Screen'"));
        rotateCounterclockwise->setToolTip(QCoreApplication::translate("QtToolBar", "Rotate Counterclockwise"));
        rotateCounterclockwise->setText(QCoreApplication::translate("QtToolBar", "Rotate Counterclockwise", "Short form of 'Rotate Counterclockwise'"));
        rotateClockwise->setToolTip(QCoreApplication::translate("QtToolBar", "Rotate Clockwise"));
        rotateClockwise->setText(QCoreApplication::translate("QtToolBar", "Rotate Clockwise", "Short form of 'Rotate Clockwise'"));
        flipHorizontal->setToolTip(QCoreApplication::translate("QtToolBar", "Flip Horizontal"));
        flipHorizontal->setText(QCoreApplication::translate("QtToolBar", "Flip Horizontal", "Short form of 'Flip Horizontal'"));
        flipVertical->setToolTip(QCoreApplication::translate("QtToolBar", "Flip Vertical"));
        flipVertical->setText(QCoreApplication::translate("QtToolBar", "Flip Vertical", "Short form of 'Flip Vertical'"));
        openFile->setToolTip(QCoreApplication::translate("QtToolBar", "Open File"));
        openFile->setText(QCoreApplication::translate("QtToolBar", "Open", "Short form of 'Open File'"));
        saveFileAs->setToolTip(QCoreApplication::translate("QtToolBar", "Save File As"));
        saveFileAs->setText(QCoreApplication::translate("QtToolBar", "Save", "Short form of 'Save File As'"));
        deleteFile->setToolTip(QCoreApplication::translate("QtToolBar", "Delete File"));
        deleteFile->setText(QCoreApplication::translate("QtToolBar", "Delete", "Short form of 'Delete File'"));
        preferences->setToolTip(QCoreApplication::translate("QtToolBar", "Preferences"));
        preferences->setText(QCoreApplication::translate("QtToolBar", "Preferences", "Short form of 'Preferences'"));
        exit->setToolTip(QCoreApplication::translate("QtToolBar", "Quit"));
        exit->setText(QCoreApplication::translate("QtToolBar", "Quit", "Short form of 'Quit'"));
        setSlideShowMode(isSlideShowMode);
    }

    void updateIcons()
    {
        IconThemeManager *iconThemeManager = IconThemeManager::instance();
        toolBarButtonsHasDarkTheme = ThemeUtils::WidgetHasDarkTheme(toolbar);
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
            startSlideShow->setToolTip(QCoreApplication::translate("QtToolBar", "Start Slideshow"));
            startSlideShow->setIcon(IconThemeManager::instance()->GetIcon(ThemeUtils::ICON_MEDIA_PLAYBACK_START, toolBarButtonsFallbackIconRequired, toolBarButtonsHasDarkTheme));
        }
        else
        {
            startSlideShow->setChecked(true);
            startSlideShow->setToolTip(QCoreApplication::translate("QtToolBar", "Stop Slideshow"));
            startSlideShow->setIcon(IconThemeManager::instance()->GetIcon(ThemeUtils::ICON_MEDIA_PLAYBACK_STOP, toolBarButtonsFallbackIconRequired, toolBarButtonsHasDarkTheme));
        }
    }
};


QtToolBar::QtToolBar(QWidget *parent)
    : QToolBar(parent)
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

    connect(m_impl->navigatePrevious        , SIGNAL(triggered()), emitter(), SIGNAL(navigatePreviousRequested())       );
    connect(m_impl->navigateNext            , SIGNAL(triggered()), emitter(), SIGNAL(navigateNextRequested())           );
    connect(m_impl->startSlideShow          , SIGNAL(triggered()), emitter(), SIGNAL(startSlideShowRequested())         );
    connect(m_impl->zoomOut                 , SIGNAL(triggered()), emitter(), SIGNAL(zoomOutRequested())                );
    connect(m_impl->zoomIn                  , SIGNAL(triggered()), emitter(), SIGNAL(zoomInRequested())                 );
    connect(m_impl->zoomFitToWindow         , SIGNAL(triggered()), emitter(), SIGNAL(zoomFitToWindowRequested())        );
    connect(m_impl->zoomOriginalSize        , SIGNAL(triggered()), emitter(), SIGNAL(zoomOriginalSizeRequested())       );
    connect(m_impl->zoomFullScreen          , SIGNAL(triggered()), emitter(), SIGNAL(zoomFullScreenRequested())         );
    connect(m_impl->rotateCounterclockwise  , SIGNAL(triggered()), emitter(), SIGNAL(rotateCounterclockwiseRequested()) );
    connect(m_impl->rotateClockwise         , SIGNAL(triggered()), emitter(), SIGNAL(rotateClockwiseRequested())        );
    connect(m_impl->flipHorizontal          , SIGNAL(triggered()), emitter(), SIGNAL(flipHorizontalRequested())         );
    connect(m_impl->flipVertical            , SIGNAL(triggered()), emitter(), SIGNAL(flipVerticalRequested())           );
    connect(m_impl->openFile                , SIGNAL(triggered()), emitter(), SIGNAL(openFileRequested())               );
    connect(m_impl->saveFileAs              , SIGNAL(triggered()), emitter(), SIGNAL(saveAsRequested())                 );
    connect(m_impl->deleteFile              , SIGNAL(triggered()), emitter(), SIGNAL(deleteFileRequested())             );
    connect(m_impl->preferences             , SIGNAL(triggered()), emitter(), SIGNAL(preferencesRequested())            );
    connect(m_impl->exit                    , SIGNAL(triggered()), emitter(), SIGNAL(exitRequested())                   );

    connect(this, SIGNAL(polished()), this, SLOT(onPolished()), Qt::QueuedConnection);
    connect(IconThemeManager::instance(), SIGNAL(themeChanged(QString)), this, SLOT(onIconThemeChanged()));
}

QtToolBar::~QtToolBar()
{}

ControlsContainerEmitter *QtToolBar::emitter()
{
    return m_impl.data();
}

void QtToolBar::onPolished()
{
    m_impl->updateIcons();
}

void QtToolBar::onIconThemeChanged()
{
    m_impl->updateIcons();
}

bool QtToolBar::event(QEvent *event)
{
    bool result = QToolBar::event(event);
    if(event->type() == QEvent::Polish)
        Q_EMIT polished();
    return result;
}

void QtToolBar::changeEvent(QEvent *event)
{
    QToolBar::changeEvent(event);
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

CONTROLS_CONTAINER_SET_ENABLED_IMPL(QtToolBar, setOpenFileEnabled, m_impl->openFile)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(QtToolBar, setOpenFolderEnabled)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(QtToolBar, setSaveAsEnabled, m_impl->saveFileAs)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(QtToolBar, setNewWindowEnabled)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(QtToolBar, setNavigatePreviousEnabled, m_impl->navigatePrevious)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(QtToolBar, setNavigateNextEnabled, m_impl->navigateNext)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(QtToolBar, setStartSlideShowEnabled, m_impl->startSlideShow)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(QtToolBar, setImageInformationEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(QtToolBar, setPrintEnabled)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(QtToolBar, setPreferencesEnabled, m_impl->preferences)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(QtToolBar, setExitEnabled, m_impl->exit)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(QtToolBar, setRotateCounterclockwiseEnabled, m_impl->rotateCounterclockwise)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(QtToolBar, setRotateClockwiseEnabled, m_impl->rotateClockwise)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(QtToolBar, setFlipHorizontalEnabled, m_impl->flipHorizontal)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(QtToolBar, setFlipVerticalEnabled, m_impl->flipVertical)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(QtToolBar, setDeleteFileEnabled, m_impl->deleteFile)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(QtToolBar, setZoomOutEnabled, m_impl->zoomOut)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(QtToolBar, setZoomInEnabled, m_impl->zoomIn)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(QtToolBar, setZoomResetEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(QtToolBar, setZoomCustomEnabled)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(QtToolBar, setZoomFitToWindowEnabled, m_impl->zoomFitToWindow)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(QtToolBar, setZoomOriginalSizeEnabled, m_impl->zoomOriginalSize)
CONTROLS_CONTAINER_SET_ENABLED_IMPL(QtToolBar, setZoomFullScreenEnabled, m_impl->zoomFullScreen)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(QtToolBar, setShowMenuBarEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(QtToolBar, setShowToolBarEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(QtToolBar, setAboutEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(QtToolBar, setAboutQtEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(QtToolBar, setCheckForUpdatesEnabled)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(QtToolBar, setEditStylesheetEnabled)

CONTROLS_CONTAINER_SET_CHECKED_IMPL(QtToolBar, setZoomFitToWindowChecked, m_impl->zoomFitToWindow)
CONTROLS_CONTAINER_SET_CHECKED_IMPL(QtToolBar, setZoomOriginalSizeChecked, m_impl->zoomOriginalSize)
CONTROLS_CONTAINER_SET_CHECKED_IMPL(QtToolBar, setZoomFullScreenChecked, m_impl->zoomFullScreen)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(QtToolBar, setShowMenuBarChecked)
CONTROLS_CONTAINER_EMPTY_BOOL_ARG_FUNCTION_IMPL(QtToolBar, setShowToolBarChecked)

CONTROLS_CONTAINER_BOOL_ARG_FUNCTION_IMPL(QtToolBar, setSlideShowMode, m_impl, setSlideShowMode)
