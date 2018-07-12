/*
   Copyright (C) 2017-2018 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "MainWindow.h"
#include "MainWindow_p.h"

#include <cassert>

#include <QApplication>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QGraphicsItem>
#include <QMessageBox>
#include <QFileInfo>
#include <QTimer>

#include "Utils/SettingsWrapper.h"
#include "Utils/SignalBlocker.h"
#include "Utils/ImageSaver.h"
#include "Utils/RestorableGeometryHelper.h"
#include "Utils/WindowUtils.h"

#include "Decoders/DecodersManager.h"
#include "Decoders/IImageData.h"
#include "../GUISettings.h"

struct MainWindow::Impl
{
    MainWindow * const mainWindow;
    GUISettings * const settings;
    UI ui;
    UIState uiState;
    QSharedPointer<IImageData> imageData;
    ImageSaver imageSaver;
    RestorableGeometryHelper geometryHelper;
    QTimer slideShowTimer;
    bool isSlideShowMode;
    bool isFullScreenMode;

    Impl(MainWindow *mainWindow, GUISettings *settings)
        : mainWindow(mainWindow)
        , settings(settings)
        , ui(mainWindow)
        , imageSaver(mainWindow)
        , geometryHelper(mainWindow)
        , slideShowTimer(mainWindow)
        , isSlideShowMode(false)
        , isFullScreenMode(false)
    {}

    void setImageControlsEnabled(bool isEnabled)
    {
        for(QList<IControlsContainer*>::Iterator it = ui.controlsContainers.begin(), itEnd = ui.controlsContainers.end(); it != itEnd; ++it)
        {
            IControlsContainer* container = *it;
            container->setZoomOutEnabled(isEnabled);
            container->setZoomInEnabled(isEnabled);
            container->setZoomResetEnabled(isEnabled);
            container->setZoomFitToWindowEnabled(isEnabled);
            container->setZoomOriginalSizeEnabled(isEnabled);
            container->setRotateCounterclockwiseEnabled(isEnabled);
            container->setFlipHorizontalEnabled(isEnabled);
            container->setFlipVerticalEnabled(isEnabled);
            container->setRotateClockwiseEnabled(isEnabled);
            container->setSaveAsEnabled(isEnabled);
        }
        updateMenuReopenWith();
    }

    bool isFileOpened() const
    {
        return imageData && !imageData->isEmpty() && imageData->size().isValid();
    }

    void syncFullScreen()
    {
        if((isFullScreenMode && mainWindow->isFullScreen()) || (!isFullScreenMode && !mainWindow->isFullScreen()))
            return;

        const bool toFullScreenMode = mainWindow->isFullScreen();
        isFullScreenMode = toFullScreenMode;
        if(toFullScreenMode)
        {
            geometryHelper.block();
            ui.menubar->setVisible(false);
            ui.toolbar->setVisible(false);
            geometryHelper.unblock();
        }
        else
        {
            geometryHelper.block();
            ui.menubar->setVisible(settings->menuBarVisible());
            ui.toolbar->setVisible(settings->toolBarVisible());
            geometryHelper.unblock();
            geometryHelper.restoreGeometry();
        }

        for(QList<IControlsContainer*>::Iterator it = ui.controlsContainers.begin(), itEnd = ui.controlsContainers.end(); it != itEnd; ++it)
            (*it)->setZoomFullScreenChecked(toFullScreenMode);
        mainWindow->updateBackgroundColor();
    }

    void updateMenuReopenWith()
    {
        QMenu *menuReopenWith = ui.menubar->menuReopenWith();
        const bool enabled = uiState.hasCurrentFile;
        menuReopenWith->setEnabled(enabled);
        QActionGroup *actionGroup = menuReopenWith->findChild<QActionGroup*>();
        if(!actionGroup)
            return;
        QSignalBlocker blocker(actionGroup);
        actionGroup->setEnabled(enabled);
        if(imageData || !enabled)
            if(QAction *checkedAction = actionGroup->checkedAction())
                checkedAction->setChecked(false);
        if(!imageData)
            return;
        QList<QAction*> childActions = actionGroup->actions();
        for(QList<QAction*>::Iterator it = childActions.begin(), itEnd = childActions.end(); it != itEnd; ++it)
            if((*it)->data().toString() == imageData->decoderName())
                (*it)->setChecked(true);
    }

    void fillMenuReopenWith()
    {
        QMenu *menuReopenWith = ui.menubar->menuReopenWith();
        QActionGroup *actionGroup = menuReopenWith->findChild<QActionGroup*>();
        if(!actionGroup)
        {
            actionGroup = new QActionGroup(menuReopenWith);
            actionGroup->setExclusive(true);
            connect(actionGroup, SIGNAL(triggered(QAction*)), mainWindow, SLOT(onActionReopenWithTriggered(QAction*)));
        }
        QList<QAction*> childActions = actionGroup->actions();
        for(QList<QAction*>::Iterator it = childActions.begin(), itEnd = childActions.end(); it != itEnd; ++it)
        {
            menuReopenWith->removeAction(*it);
            (*it)->deleteLater();
        }
        DecodersManager &decodersManager = DecodersManager::getInstance();
        QStringList registeredDecoders = decodersManager.registeredDecoders();
        registeredDecoders.sort();
        for(QStringList::ConstIterator it = registeredDecoders.constBegin(), itEnd = registeredDecoders.constEnd(); it != itEnd; ++it)
        {
            QString humanReadableName = *it;
            const QString decoderPrefix = QString::fromLatin1("Decoder");
            if(humanReadableName.startsWith(decoderPrefix))
                humanReadableName = humanReadableName.mid(decoderPrefix.length());
            QAction *action = new QAction(humanReadableName, actionGroup);
            menuReopenWith->addAction(action);
            actionGroup->addAction(action);
            action->setCheckable(true);
            action->setData(*it);
        }
        updateMenuReopenWith();
    }

    void loadImageData(const QSharedPointer<IImageData> &data)
    {
        imageData = data;
        if(imageData && !imageData->isEmpty())
        {
            ui.imageViewerWidget->setZoomMode(settings->zoomMode());
            ui.imageViewerWidget->setGraphicsItem(imageData->graphicsItem());
            setImageControlsEnabled(true);
        }
        else
        {
            imageData = QSharedPointer<IImageData>();
            ui.imageViewerWidget->clear();
            setImageControlsEnabled(false);
            QMessageBox::critical(mainWindow, qApp->translate("MainWindow", "Error"), qApp->translate("MainWindow", "Failed to open file \"%1\"").arg(uiState.currentFilePath));
        }
    }
};

MainWindow::MainWindow(GUISettings *settings, QWidget *parent)
    : QMainWindow(parent)
    , m_impl(new Impl(this, settings))
{
    setAcceptDrops(true);

    UI &ui = m_impl->ui;
    ImageViewerWidget *imageViewerWidget = ui.imageViewerWidget;

    m_impl->fillMenuReopenWith();

    connect(imageViewerWidget, SIGNAL(zoomLevelChanged(qreal)), this, SLOT(updateWindowTitle()));

    for(QList<IControlsContainer*>::Iterator it = ui.controlsContainers.begin(), itEnd = ui.controlsContainers.end(); it != itEnd; ++it)
    {
        QObject *object = (*it)->emitter();
        connect(object, SIGNAL(openFileRequested())                 , this              , SIGNAL(openFileWithDialogRequested())     );
        connect(object, SIGNAL(openFolderRequested())               , this              , SIGNAL(openFolderWithDialogRequested())   );
        connect(object, SIGNAL(saveAsRequested())                   , this              , SLOT(onSaveAsRequested())                 );
        connect(object, SIGNAL(newWindowRequested())                , this              , SIGNAL(newWindowRequested())              );
        connect(object, SIGNAL(navigatePreviousRequested())         , this              , SIGNAL(selectPreviousRequested())         );
        connect(object, SIGNAL(navigateNextRequested())             , this              , SIGNAL(selectNextRequested())             );
        connect(object, SIGNAL(startSlideShowRequested())           , this              , SLOT(switchSlideShowMode())               );
        connect(object, SIGNAL(preferencesRequested())              , this              , SIGNAL(preferencesRequested())            );
        connect(object, SIGNAL(exitRequested())                     , this              , SLOT(close())                             );
        connect(object, SIGNAL(rotateCounterclockwiseRequested())   , imageViewerWidget , SLOT(rotateCounterclockwise())            );
        connect(object, SIGNAL(rotateClockwiseRequested())          , imageViewerWidget , SLOT(rotateClockwise())                   );
        connect(object, SIGNAL(flipHorizontalRequested())           , imageViewerWidget , SLOT(flipHorizontal())                    );
        connect(object, SIGNAL(flipVerticalRequested())             , imageViewerWidget , SLOT(flipVertical())                      );
        connect(object, SIGNAL(deleteFileRequested())               , this              , SIGNAL(deleteFileRequested())             );
        connect(object, SIGNAL(zoomOutRequested())                  , imageViewerWidget , SLOT(zoomOut())                           );
        connect(object, SIGNAL(zoomInRequested())                   , imageViewerWidget , SLOT(zoomIn())                            );
        connect(object, SIGNAL(zoomResetRequested())                , imageViewerWidget , SLOT(resetZoom())                         );
        connect(object, SIGNAL(zoomFitToWindowRequested())          , this              , SLOT(onZoomFitToWindowRequested())        );
        connect(object, SIGNAL(zoomOriginalSizeRequested())         , this              , SLOT(onZoomOriginalSizeRequested())       );
        connect(object, SIGNAL(zoomFullScreenRequested())           , this              , SLOT(switchFullScreenMode())              );
        connect(object, SIGNAL(showMenuBarRequested())              , this              , SLOT(switchShowMenuBar())                 );
        connect(object, SIGNAL(showToolBarRequested())              , this              , SLOT(switchShowToolBar())                 );
        connect(object, SIGNAL(aboutRequested())                    , this              , SIGNAL(aboutRequested())                  );
        connect(object, SIGNAL(aboutQtRequested())                  , this              , SIGNAL(aboutQtRequested())                );
    }

    connect(settings, SIGNAL(normalBackgroundColorChanged(const QColor&))       , this              , SLOT(updateBackgroundColor())                     );
    connect(settings, SIGNAL(fullScreenBackgroundColorChanged(const QColor&))   , this              , SLOT(updateBackgroundColor())                     );
    connect(settings, SIGNAL(smoothTransformationChanged(bool))                 , imageViewerWidget , SLOT(setSmoothTransformation(bool))               );
    connect(settings, SIGNAL(slideShowIntervalChanged(int))                     , this              , SLOT(updateSlideShowInterval())                   );
    connect(settings, SIGNAL(wheelModeChanged(ImageViewerWidget::WheelMode))    , imageViewerWidget , SLOT(setWheelMode(ImageViewerWidget::WheelMode))  );

    connect(&m_impl->slideShowTimer, SIGNAL(timeout()), this, SIGNAL(selectNextRequested()));

    imageViewerWidget->setZoomLevel(settings->zoomLevel());
    imageViewerWidget->setBackgroundColor(settings->normalBackgroundColor());
    imageViewerWidget->setSmoothTransformation(settings->smoothTransformation());
    imageViewerWidget->setWheelMode(settings->wheelMode());
    onZoomModeChanged(settings->zoomMode());
    updateSlideShowInterval();

    for(QList<IControlsContainer*>::Iterator it = ui.controlsContainers.begin(), itEnd = ui.controlsContainers.end(); it != itEnd; ++it)
    {
        IControlsContainer* container = *it;
        container->setShowMenuBarChecked(settings->menuBarVisible());
        container->setShowToolBarChecked(settings->toolBarVisible());
    }

    ui.menubar->setVisible(false);
    ui.toolbar->setVisible(false);
    RestorableGeometryHelper &geometryHelper = m_impl->geometryHelper;
    geometryHelper.saveGeometry();
    geometryHelper.deserialize(settings->mainWindowGeometry());
    geometryHelper.block();
    ui.menubar->setVisible(settings->menuBarVisible());
    ui.toolbar->setVisible(settings->toolBarVisible());
    geometryHelper.unblock();
    geometryHelper.restoreGeometry();
    updateUIState(m_impl->uiState, UICF_All);
}

MainWindow::~MainWindow()
{
    m_impl->settings->setZoomLevel(m_impl->ui.imageViewerWidget->zoomLevel());
}

void MainWindow::updateWindowTitle()
{
    QString label;
    const int currentFileIndex = m_impl->uiState.currentFileIndex;
    if(m_impl->isFileOpened())
    {
        const QSize imageSize = m_impl->imageData->size();
        label = QFileInfo(m_impl->uiState.currentFilePath).fileName();
        label.append(QString::fromLatin1(" (%1x%2) %3% - %4")
                     .arg(imageSize.width())
                     .arg(imageSize.height())
                     .arg(static_cast<quint64>(m_impl->ui.imageViewerWidget->zoomLevel() * 100))
                     .arg(qApp->applicationName()));
    }
    else if(m_impl->uiState.hasCurrentFile)
    {
        label = QFileInfo(m_impl->uiState.currentFilePath).fileName();
        label.append(QString::fromLatin1(" (%1) - %2")
                     .arg(tr("Error"))
                     .arg(qApp->applicationName()));
    }
    else
    {
        label = qApp->applicationName();
    }
    if(m_impl->uiState.hasCurrentFileIndex)
    {
        label.prepend(QString::fromLatin1("[%1/%2] ")
                      .arg(currentFileIndex + 1)
                      .arg(m_impl->uiState.filesCount));
    }
    setWindowTitle(label);
}

void MainWindow::switchFullScreenMode()
{
    const bool toFullScreenMode = !m_impl->isFullScreenMode;
    if(toFullScreenMode)
        m_impl->geometryHelper.saveGeometry();

#if defined (Q_OS_MAC)
    WindowUtils::ToggleFullScreenMode(this);
#else
    if(toFullScreenMode)
        setWindowState(windowState() | Qt::WindowFullScreen);
    else
        setWindowState(windowState() & ~Qt::WindowFullScreen);
#endif
}

void MainWindow::switchSlideShowMode()
{
    m_impl->isSlideShowMode = !m_impl->isSlideShowMode;
    for(QList<IControlsContainer*>::Iterator it = m_impl->ui.controlsContainers.begin(), itEnd = m_impl->ui.controlsContainers.end(); it != itEnd; ++it)
        (*it)->setSlideShowMode(m_impl->isSlideShowMode);
    if(m_impl->isSlideShowMode)
        m_impl->slideShowTimer.start();
    else
        m_impl->slideShowTimer.stop();
}

void MainWindow::switchShowMenuBar()
{
    const bool newValue = !m_impl->settings->menuBarVisible();
    m_impl->settings->setMenuBarVisible(newValue);
    for(QList<IControlsContainer*>::Iterator it = m_impl->ui.controlsContainers.begin(), itEnd = m_impl->ui.controlsContainers.end(); it != itEnd; ++it)
        (*it)->setShowMenuBarChecked(newValue);
    if(m_impl->isFullScreenMode)
        return;
    RestorableGeometryHelper &geometryHelper = m_impl->geometryHelper;
    geometryHelper.saveGeometry();
    geometryHelper.block();
    m_impl->ui.menubar->setVisible(newValue);
    geometryHelper.unblock();
    geometryHelper.restoreGeometry();
}

void MainWindow::switchShowToolBar()
{
    const bool newValue = !m_impl->settings->toolBarVisible();
    m_impl->settings->setToolBarVisible(newValue);
    for(QList<IControlsContainer*>::Iterator it = m_impl->ui.controlsContainers.begin(), itEnd = m_impl->ui.controlsContainers.end(); it != itEnd; ++it)
        (*it)->setShowToolBarChecked(newValue);
    if(m_impl->isFullScreenMode)
        return;
    RestorableGeometryHelper &geometryHelper = m_impl->geometryHelper;
    geometryHelper.saveGeometry();
    geometryHelper.block();
    m_impl->ui.toolbar->setVisible(newValue);
    geometryHelper.unblock();
    geometryHelper.restoreGeometry();
}

void MainWindow::onZoomModeChanged(ImageViewerWidget::ZoomMode mode)
{
    const bool modeIsFitToWindow = mode == ImageViewerWidget::ZOOM_FIT_TO_WINDOW;
    const bool modeIsOriginalSize = mode == ImageViewerWidget::ZOOM_IDENTITY;
    m_impl->ui.imageViewerWidget->setZoomMode(mode);
    for(QList<IControlsContainer*>::Iterator it = m_impl->ui.controlsContainers.begin(), itEnd = m_impl->ui.controlsContainers.end(); it != itEnd; ++it)
    {
        IControlsContainer* container = *it;
        container->setZoomFitToWindowChecked(modeIsFitToWindow);
        container->setZoomOriginalSizeChecked(modeIsOriginalSize);
    }
    m_impl->settings->setZoomMode(mode);
    updateWindowTitle();
}

void MainWindow::onSaveAsRequested()
{
    if(!m_impl->isFileOpened())
        return;
    const QString openedPath = m_impl->uiState.currentFilePath;
    m_impl->imageSaver.setDefaultFilePath(openedPath);
    m_impl->imageSaver.save(m_impl->ui.imageViewerWidget->grabImage(), openedPath);
}

void MainWindow::onZoomFitToWindowRequested()
{
    ImageViewerWidget::ZoomMode mode;
    if(m_impl->settings->zoomMode() != ImageViewerWidget::ZOOM_FIT_TO_WINDOW)
        mode = ImageViewerWidget::ZOOM_FIT_TO_WINDOW;
    else
        mode = ImageViewerWidget::ZOOM_CUSTOM;
    onZoomModeChanged(mode);
}

void MainWindow::onZoomOriginalSizeRequested()
{
    ImageViewerWidget::ZoomMode mode;
    if(m_impl->settings->zoomMode() != ImageViewerWidget::ZOOM_IDENTITY)
        mode = ImageViewerWidget::ZOOM_IDENTITY;
    else
        mode = ImageViewerWidget::ZOOM_CUSTOM;
    onZoomModeChanged(mode);
}

void MainWindow::updateUIState(const UIState &state, const UIChangeFlags &changeFlags)
{
    m_impl->uiState = state;

    if(changeFlags.testFlag(UICF_CurrentFilePath))
    {
        if(state.currentFilePath.isEmpty())
        {
            m_impl->imageData = QSharedPointer<IImageData>();
            m_impl->ui.imageViewerWidget->clear();
            m_impl->setImageControlsEnabled(false);
        }
        else
        {
            m_impl->loadImageData(DecodersManager::getInstance().loadImage(state.currentFilePath));
        }
    }

    if(changeFlags.testFlag(UICF_CanDeleteCurrentFile))
    {
        for(QList<IControlsContainer*>::Iterator it = m_impl->ui.controlsContainers.begin(), itEnd = m_impl->ui.controlsContainers.end(); it != itEnd; ++it)
            (*it)->setDeleteFileEnabled(state.canDeleteCurrentFile);
    }

    if(changeFlags.testFlag(UICF_HasCurrentFileIndex))
    {
        for(QList<IControlsContainer*>::Iterator it = m_impl->ui.controlsContainers.begin(), itEnd = m_impl->ui.controlsContainers.end(); it != itEnd; ++it)
        {
            IControlsContainer* container = *it;
            container->setNavigatePreviousEnabled(state.hasCurrentFileIndex);
            container->setNavigateNextEnabled(state.hasCurrentFileIndex);
            container->setStartSlideShowEnabled(state.hasCurrentFileIndex);
        }
    }

    if(changeFlags.testFlag(UICF_HasCurrentFile) ||
       changeFlags.testFlag(UICF_HasCurrentFileIndex) ||
       changeFlags.testFlag(UICF_CurrentFilePath) ||
       changeFlags.testFlag(UICF_CurrentFileIndex) ||
       changeFlags.testFlag(UICF_FilesCount))
        updateWindowTitle();
}

void MainWindow::saveGeometrySettings()
{
    if(!m_impl->isFullScreenMode)
        m_impl->geometryHelper.saveGeometry();
    m_impl->settings->setMainWindowGeometry(m_impl->geometryHelper.serialize());
}

void MainWindow::updateSlideShowInterval()
{
    m_impl->slideShowTimer.setInterval(m_impl->settings->slideShowInterval() * 1000);
}

void MainWindow::updateBackgroundColor()
{
    const QColor color = m_impl->isFullScreenMode
            ? m_impl->settings->fullScreenBackgroundColor()
            : m_impl->settings->normalBackgroundColor();
    m_impl->ui.imageViewerWidget->setBackgroundColor(color);
}

void MainWindow::onActionReopenWithTriggered(QAction *action)
{
    assert(m_impl->uiState.hasCurrentFile);
    if(!m_impl->uiState.hasCurrentFile)
        return;
    m_impl->loadImageData(DecodersManager::getInstance().loadImage(m_impl->uiState.currentFilePath, action->data().toString()));
    updateWindowTitle();
}

void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    switch(event->type())
    {
    case QEvent::LanguageChange:
        updateWindowTitle();
        break;
    case QEvent::WindowStateChange:
        if(isVisible())
            m_impl->syncFullScreen();
        break;
    default:
        break;
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveGeometrySettings();
    QMainWindow::closeEvent(event);
    emit closed();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData = event->mimeData();
    if(mimeData->hasUrls())
    {
        const QList<QUrl> urlList = mimeData->urls();
        QStringList pathsList;
        for(QList<QUrl>::ConstIterator it = urlList.constBegin(), itEnd = urlList.constEnd(); it != itEnd; ++it)
            pathsList.append(it->toLocalFile());
        if(pathsList.size() == 1)
            emit openPathRequested(pathsList.first());
        else
            emit openPathsRequested(pathsList);
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Home:
        emit selectFirstRequested();
        break;
    case Qt::Key_End:
        emit selectLastRequested();
        break;
    case Qt::Key_Escape:
        if(m_impl->isFullScreenMode)
            switchFullScreenMode();
        break;
    case Qt::Key_Up:
        m_impl->ui.imageViewerWidget->scrollUp();
        break;
    case Qt::Key_Down:
        m_impl->ui.imageViewerWidget->scrollDown();
        break;
    case Qt::Key_Left:
        m_impl->ui.imageViewerWidget->scrollLeft();
        break;
    case Qt::Key_Right:
        m_impl->ui.imageViewerWidget->scrollRight();
        break;
    default:
        break;
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    m_impl->ui.menubar->contextMenu()->exec(event->globalPos());
    QMainWindow::contextMenuEvent(event);
}
