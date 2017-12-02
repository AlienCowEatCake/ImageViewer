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
#include <QStringList>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QGraphicsItem>
#include <QMessageBox>
#include <QFileInfo>
#include <QTimer>

#include "Utils/SettingsWrapper.h"
#include "Utils/Workarounds.h"
#include "Utils/ImageSaver.h"

#include "Decoders/DecodersManager.h"
#include "GUISettings.h"

struct MainWindow::Impl
{
    MainWindow * const mainWindow;
    GUISettings * const settings;
    UI ui;
    UIState uiState;
    ImageSaver imageSaver;
    QTimer slideShowTimer;
    bool isSlideShowMode;
    bool isFullScreenMode;

    Impl(MainWindow *mainWindow, GUISettings *settings)
        : mainWindow(mainWindow)
        , settings(settings)
        , ui(mainWindow)
        , imageSaver(mainWindow)
        , slideShowTimer(mainWindow)
        , isSlideShowMode(false)
        , isFullScreenMode(false)
    {}

    bool isFileOpened() const
    {
        return ui.imageViewerWidget->imageSize().isValid();
    }

    void syncFullScreen()
    {
        if((isFullScreenMode && mainWindow->isFullScreen()) || (!isFullScreenMode && !mainWindow->isFullScreen()))
            return;

        const bool toFullScreenMode = mainWindow->isFullScreen();
        isFullScreenMode = toFullScreenMode;
        if(toFullScreenMode)
        {
            ui.menubar->setVisible(false);
            ui.toolbar->setVisible(false);
        }
        else
        {
            ui.menubar->setVisible(settings->menuBarVisible());
            ui.toolbar->setVisible(settings->toolBarVisible());
        }
        ui.zoomFullScreen->setChecked(toFullScreenMode);
        ui.actionZoomFullScreen->setChecked(toFullScreenMode);
        mainWindow->updateBackgroundColor();
    }
};

MainWindow::MainWindow(GUISettings *settings, QWidget *parent)
    : QMainWindow(parent)
    , m_impl(new Impl(this, settings))
{
    setAcceptDrops(true);

    UI &ui = m_impl->ui;
    ImageViewerWidget *imageViewerWidget = ui.imageViewerWidget;

    connect(imageViewerWidget, SIGNAL(zoomLevelChanged(qreal)), this, SLOT(updateWindowTitle()));

    connect(ui.navigatePrevious             , SIGNAL(clicked())     , this              , SIGNAL(selectPreviousRequested())     );
    connect(ui.navigateNext                 , SIGNAL(clicked())     , this              , SIGNAL(selectNextRequested())         );
    connect(ui.startSlideShow               , SIGNAL(clicked())     , this              , SLOT(switchSlideShowMode())           );
    connect(ui.zoomIn                       , SIGNAL(clicked())     , imageViewerWidget , SLOT(zoomIn())                        );
    connect(ui.zoomOut                      , SIGNAL(clicked())     , imageViewerWidget , SLOT(zoomOut())                       );
    connect(ui.zoomFitToWindow              , SIGNAL(clicked())     , this              , SLOT(onZoomFitToWindowRequested())    );
    connect(ui.zoomOriginalSize             , SIGNAL(clicked())     , this              , SLOT(onZoomOriginalSizeRequested())   );
    connect(ui.zoomFullScreen               , SIGNAL(clicked())     , this              , SLOT(switchFullScreenMode())          );
    connect(ui.rotateCounterclockwise       , SIGNAL(clicked())     , imageViewerWidget , SLOT(rotateCounterclockwise())        );
    connect(ui.rotateClockwise              , SIGNAL(clicked())     , imageViewerWidget , SLOT(rotateClockwise())               );
    connect(ui.flipHorizontal               , SIGNAL(clicked())     , imageViewerWidget , SLOT(flipHorizontal())                );
    connect(ui.flipVertical                 , SIGNAL(clicked())     , imageViewerWidget , SLOT(flipVertical())                  );
    connect(ui.openFile                     , SIGNAL(clicked())     , this              , SIGNAL(openFileWithDialogRequested()) );
    connect(ui.saveFileAs                   , SIGNAL(clicked())     , this              , SLOT(onSaveAsRequested())             );
    connect(ui.deleteFile                   , SIGNAL(clicked())     , this              , SIGNAL(deleteFileRequested())         );
    connect(ui.preferences                  , SIGNAL(clicked())     , this              , SIGNAL(showPreferencesRequested())    );
    connect(ui.exit                         , SIGNAL(clicked())     , this              , SLOT(close())                         );
    connect(ui.actionOpen                   , SIGNAL(triggered())   , this              , SIGNAL(openFileWithDialogRequested()) );
    connect(ui.actionSaveAs                 , SIGNAL(triggered())   , this              , SLOT(onSaveAsRequested())             );
    connect(ui.actionNavigatePrevious       , SIGNAL(triggered())   , this              , SIGNAL(selectPreviousRequested())     );
    connect(ui.actionNavigateNext           , SIGNAL(triggered())   , this              , SIGNAL(selectNextRequested())         );
    connect(ui.actionStartSlideShow         , SIGNAL(triggered())   , this              , SLOT(switchSlideShowMode())           );
    connect(ui.actionPreferences            , SIGNAL(triggered())   , this              , SIGNAL(showPreferencesRequested())    );
    connect(ui.actionExit                   , SIGNAL(triggered())   , this              , SLOT(close())                         );
    connect(ui.actionRotateCounterclockwise , SIGNAL(triggered())   , imageViewerWidget , SLOT(rotateCounterclockwise())        );
    connect(ui.actionRotateClockwise        , SIGNAL(triggered())   , imageViewerWidget , SLOT(rotateClockwise())               );
    connect(ui.actionFlipHorizontal         , SIGNAL(triggered())   , imageViewerWidget , SLOT(flipHorizontal())                );
    connect(ui.actionFlipVertical           , SIGNAL(triggered())   , imageViewerWidget , SLOT(flipVertical())                  );
    connect(ui.actionDeleteFile             , SIGNAL(triggered())   , this              , SIGNAL(deleteFileRequested())         );
    connect(ui.actionZoomIn                 , SIGNAL(triggered())   , imageViewerWidget , SLOT(zoomIn())                        );
    connect(ui.actionZoomOut                , SIGNAL(triggered())   , imageViewerWidget , SLOT(zoomOut())                       );
    connect(ui.actionZoomReset              , SIGNAL(triggered())   , imageViewerWidget , SLOT(resetZoom())                     );
    connect(ui.actionZoomFitToWindow        , SIGNAL(triggered())   , this              , SLOT(onZoomFitToWindowRequested())    );
    connect(ui.actionZoomOriginalSize       , SIGNAL(triggered())   , this              , SLOT(onZoomOriginalSizeRequested())   );
    connect(ui.actionZoomFullScreen         , SIGNAL(triggered())   , this              , SLOT(switchFullScreenMode())          );
    connect(ui.actionShowMenuBar            , SIGNAL(triggered())   , this              , SLOT(switchShowMenuBar())             );
    connect(ui.actionShowToolBar            , SIGNAL(triggered())   , this              , SLOT(switchShowToolBar())             );
    connect(ui.actionEnglish                , SIGNAL(triggered())   , this              , SLOT(onActionEnglishTriggered())      );
    connect(ui.actionRussian                , SIGNAL(triggered())   , this              , SLOT(onActionRussianTriggered())      );
    connect(ui.actionAbout                  , SIGNAL(triggered())   , this              , SIGNAL(showAboutRequested())          );
    connect(ui.actionAboutQt                , SIGNAL(triggered())   , this              , SIGNAL(showAboutQtRequested())        );

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
    setLanguage();

    ui.menubar->setVisible(settings->menuBarVisible());
    ui.actionShowMenuBar->setChecked(settings->menuBarVisible());
    ui.toolbar->setVisible(settings->toolBarVisible());
    ui.actionShowToolBar->setChecked(settings->toolBarVisible());

    restoreState(settings->mainWindowState());
    restoreGeometry(settings->mainWindowGeometry());

    onUIStateChanged(m_impl->uiState, UICF_All);
    RegisterUIStateChangedReceiver(this, SLOT(onUIStateChanged(const UIState&, const UIChangeFlags&)));
}

MainWindow::~MainWindow()
{
    m_impl->settings->setZoomLevel(m_impl->ui.imageViewerWidget->zoomLevel());
}

void MainWindow::setLanguage(const QString &newLanguage)
{
    // Отображение название языка -> соответствующая ему менюшка
    static QMap<QString, QAction*> languagesMap;
    if(languagesMap.isEmpty())
    {
        languagesMap[QString::fromLatin1("en")] = m_impl->ui.actionEnglish;
        languagesMap[QString::fromLatin1("ru")] = m_impl->ui.actionRussian;
    }

    // Определим системную локаль
    static QString systemLang;
    if(systemLang.isEmpty())
    {
        const QString systemLocale = QLocale::system().name().toLower();
        for(QMap<QString, QAction*>::Iterator it = languagesMap.begin(); it != languagesMap.end(); ++it)
        {
            if(systemLocale.startsWith(it.key()))
            {
                systemLang = it.key();
                break;
            }
        }
        if(systemLang.isEmpty())
            systemLang = QString::fromLatin1("en");
    }

    // Посмотрим в настройки, не сохранен ли случайно в них язык
    SettingsWrapper settings;
    QString language = newLanguage;
    if(language.isEmpty())
    {
        const QVariant rawValue = settings.value(QString::fromLatin1("Language"), systemLang);
        const QString value = rawValue.isValid() ? rawValue.toString() : systemLang;
        language = languagesMap.find(value) != languagesMap.end() ? value : systemLang;
    }
    else
    {
        settings.setValue(QString::fromLatin1("Language"), language);
    }

    // Удалим старый перевод, установим новый
    static QTranslator qtTranslator;
    static QTranslator appTranslator;
    static QTranslator utilsTranslator;
    if(!qtTranslator.isEmpty())
        qApp->removeTranslator(&qtTranslator);
    if(!appTranslator.isEmpty())
        qApp->removeTranslator(&appTranslator);
    if(!utilsTranslator.isEmpty())
        qApp->removeTranslator(&utilsTranslator);
    qtTranslator.load(QString::fromLatin1("qt_%1").arg(language), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    appTranslator.load(QString::fromLatin1(":/translations/imageviewer_%1").arg(language));
    utilsTranslator.load(QString::fromLatin1(":/translations/qtutils_%1").arg(language));
    qApp->installTranslator(&qtTranslator);
    qApp->installTranslator(&appTranslator);
    qApp->installTranslator(&utilsTranslator);

    // Пофиксим шрифты
    Workarounds::FontsFix(language);

    // Проставим галочку на нужном нам языке и снимем с остальных
    languagesMap[language]->setChecked(true);

    // Перегрузим ресурсы в окнах
    m_impl->ui.retranslate();
    updateWindowTitle();
}

void MainWindow::updateWindowTitle()
{
    QString label;
    const int currentFileIndex = m_impl->uiState.currentFileIndex;
    if(m_impl->isFileOpened())
    {
        const QSize imageSize = m_impl->ui.imageViewerWidget->imageSize();
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
    GUISettings * const settings = m_impl->settings;
    if(toFullScreenMode)
    {
        settings->setMainWindowGeometry(saveGeometry());
        settings->setMainWindowState(saveState());
        showFullScreen();
    }
    else
    {
        showNormal();
        restoreState(settings->mainWindowState());
        restoreGeometry(settings->mainWindowGeometry());
    }
    m_impl->syncFullScreen();
}

void MainWindow::switchSlideShowMode()
{
    m_impl->isSlideShowMode = !m_impl->isSlideShowMode;
    m_impl->ui.setSlideShowMode(m_impl->isSlideShowMode);
    if(m_impl->isSlideShowMode)
        m_impl->slideShowTimer.start();
    else
        m_impl->slideShowTimer.stop();
}

void MainWindow::switchShowMenuBar()
{
    const bool newValue = !m_impl->settings->menuBarVisible();
    m_impl->settings->setMenuBarVisible(newValue);
    m_impl->ui.actionShowMenuBar->setChecked(newValue);
    if(!m_impl->isFullScreenMode)
        m_impl->ui.menubar->setVisible(newValue);
}

void MainWindow::switchShowToolBar()
{
    const bool newValue = !m_impl->settings->toolBarVisible();
    m_impl->settings->setToolBarVisible(newValue);
    m_impl->ui.actionShowToolBar->setChecked(newValue);
    if(!m_impl->isFullScreenMode)
        m_impl->ui.toolbar->setVisible(newValue);
}

void MainWindow::onZoomModeChanged(ImageViewerWidget::ZoomMode mode)
{
    const bool modeIsFitToWindow = mode == ImageViewerWidget::ZOOM_FIT_TO_WINDOW;
    const bool modeIsOriginalSize = mode == ImageViewerWidget::ZOOM_IDENTITY;
    m_impl->ui.imageViewerWidget->setZoomMode(mode);
    m_impl->ui.zoomFitToWindow->setChecked(modeIsFitToWindow);
    m_impl->ui.zoomOriginalSize->setChecked(modeIsOriginalSize);
    m_impl->ui.actionZoomFitToWindow->setChecked(modeIsFitToWindow);
    m_impl->ui.actionZoomOriginalSize->setChecked(modeIsOriginalSize);
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

void MainWindow::onActionEnglishTriggered()
{
    setLanguage(QString::fromLatin1("en"));
}

void MainWindow::onActionRussianTriggered()
{
    setLanguage(QString::fromLatin1("ru"));
}

void MainWindow::onUIStateChanged(const UIState &state, const UIChangeFlags &changeFlags)
{
    m_impl->uiState = state;

    if(changeFlags.testFlag(UICF_CurrentFilePath))
    {
        if(state.currentFilePath.isEmpty())
        {
            m_impl->ui.imageViewerWidget->clear();
            m_impl->ui.setImageControlsEnabled(false);
        }
        else
        {
            QGraphicsItem *item = DecodersManager::getInstance().loadImage(state.currentFilePath);
            if(item)
            {
                m_impl->ui.imageViewerWidget->setZoomMode(m_impl->settings->zoomMode());
                m_impl->ui.imageViewerWidget->setGraphicsItem(item);
                m_impl->ui.setImageControlsEnabled(true);
            }
            else
            {
                m_impl->ui.imageViewerWidget->clear();
                m_impl->ui.setImageControlsEnabled(false);
                QMessageBox::critical(this, tr("Error"), tr("Failed to open file \"%1\"").arg(state.currentFilePath));
            }
        }
    }

    if(changeFlags.testFlag(UICF_CanDeleteCurrentFile))
    {
        m_impl->ui.deleteFile->setEnabled(state.canDeleteCurrentFile);
        m_impl->ui.actionDeleteFile->setEnabled(state.canDeleteCurrentFile);
    }

    if(changeFlags.testFlag(UICF_HasCurrentFileIndex))
    {
        m_impl->ui.navigatePrevious->setEnabled(state.hasCurrentFileIndex);
        m_impl->ui.actionNavigatePrevious->setEnabled(state.hasCurrentFileIndex);
        m_impl->ui.navigateNext->setEnabled(state.hasCurrentFileIndex);
        m_impl->ui.actionNavigateNext->setEnabled(state.hasCurrentFileIndex);
    }

    if(changeFlags.testFlag(UICF_HasCurrentFile))
    {
        m_impl->ui.startSlideShow->setEnabled(state.hasCurrentFile);
        m_impl->ui.actionStartSlideShow->setEnabled(state.hasCurrentFile);
    }

    if(changeFlags.testFlag(UICF_HasCurrentFile) ||
       changeFlags.testFlag(UICF_HasCurrentFileIndex) ||
       changeFlags.testFlag(UICF_CurrentFilePath) ||
       changeFlags.testFlag(UICF_CurrentFileIndex) ||
       changeFlags.testFlag(UICF_FilesCount))
        updateWindowTitle();
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

void MainWindow::changeEvent(QEvent *event)
{
    switch(event->type())
    {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    case QEvent::ThemeChange:
#endif
    case QEvent::StyleChange:
    case QEvent::PaletteChange:
        m_impl->ui.updateIcons();
        break;
    case QEvent::WindowStateChange:
        m_impl->syncFullScreen();
        break;
    default:
        break;
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(!m_impl->isFullScreenMode)
    {
        m_impl->settings->setMainWindowGeometry(saveGeometry());
        m_impl->settings->setMainWindowState(saveState());
    }
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
    m_impl->ui.contextMenu->exec(event->globalPos());
    QMainWindow::contextMenuEvent(event);
}
