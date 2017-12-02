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

#include "MainController.h"

#include <QApplication>
#include <QProcess>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>

#include "Utils/ObjectsConnector.h"

#include "Decoders/DecodersManager.h"
#include "AboutDialog.h"
#include "GUISettings.h"
#include "MainWindow.h"
#include "ObjectsConnectorIDs.h"
#include "SettingsDialog.h"

struct MainController::Impl
{
    MainController * const mainController;
    const QStringList supportedFormats;
    FileManager fileManager;
    GUISettings settings;
    MainWindow mainWindow;

    bool lastHasCurrentFile;
    bool lastHasCurrentFileIndex;

    Impl(MainController *mainController)
        : mainController(mainController)
        , supportedFormats(DecodersManager::getInstance().supportedFormatsWithWildcards())
        , fileManager(supportedFormats)
        , mainWindow(&settings)
        , lastHasCurrentFile(false)
        , lastHasCurrentFileIndex(false)
    {}

    bool hasCurrentFile() const
    {
        return !fileManager.currentFilePath().isEmpty();
    }

    bool hasCurrentFileIndex() const
    {
        const int currentIndex = fileManager.currentFileIndex();
        const int count = fileManager.filesCount();
        return currentIndex >= 0 && count > 0 && currentIndex < count;
    }

    bool hasNextFile() const
    {
        return hasCurrentFileIndex() && fileManager.currentFileIndex() + 1 < fileManager.filesCount();
    }

    bool hasPreviousFile() const
    {
        return hasCurrentFileIndex() && fileManager.currentFileIndex() > 0;
    }
};

MainController::MainController(QObject *parent)
    : QObject(parent)
    , m_impl(new Impl(this))
{
    ObjectsConnector::RegisterEmitter(UI_STATE_CHANGED_ID, this, SIGNAL(uiStateChanged(const UIState&, const UIChangeFlags&)));
    connect(&m_impl->fileManager, SIGNAL(stateChanged(const FileManager::ChangeFlags&)), this, SLOT(onFileManagerStateChanged(const FileManager::ChangeFlags&)));

    MainWindow * const mainWindow = &m_impl->mainWindow;
    ObjectsConnector::RegisterReceiver(OPEN_NEW_WINDOW_ID       , this          , SLOT(openNewWindow())                 );
    ObjectsConnector::RegisterReceiver(SELECT_NEXT_ID           , this          , SLOT(selectNextFile())                );
    ObjectsConnector::RegisterReceiver(SELECT_PREVIOUS_ID       , this          , SLOT(selectPreviousFile())            );
    ObjectsConnector::RegisterReceiver(SELECT_FIRST_ID          , this          , SLOT(selectFirstFile())               );
    ObjectsConnector::RegisterReceiver(SELECT_LAST_ID           , this          , SLOT(selectLastFile())                );
    ObjectsConnector::RegisterReceiver(DELETE_FILE_ID           , this          , SLOT(deleteCurrentFile())             );
    ObjectsConnector::RegisterReceiver(OPEN_SINGLE_PATH_ID      , this          , SLOT(openPath(const QString&))        );
    ObjectsConnector::RegisterReceiver(OPEN_MULTIPLE_PATHS_ID   , this          , SLOT(openPaths(const QStringList&))   );
    ObjectsConnector::RegisterReceiver(OPEN_FILE_WITH_DIALOG_ID , this          , SLOT(openFileWithDialog())            );
    ObjectsConnector::RegisterReceiver(SHOW_ABOUT_ID            , this          , SLOT(showAbout())                     );
    ObjectsConnector::RegisterReceiver(SHOW_PREFERENCES_ID      , this          , SLOT(showPreferences())               );
    ObjectsConnector::RegisterReceiver(SHOW_ABOUT_QT_ID         , qApp          , SLOT(aboutQt())                       );
    ObjectsConnector::RegisterReceiver(QUIT_APPLICATION_ID      , qApp          , SLOT(quit())                          );
    ObjectsConnector::RegisterEmitter (QUIT_APPLICATION_ID      , mainWindow    , SIGNAL(closed())                      );
}

MainController::~MainController()
{}

bool MainController::openPath(const QString &path)
{
    return m_impl->fileManager.openPath(path);
}

bool MainController::openPaths(const QStringList &paths)
{
    return m_impl->fileManager.openPaths(paths);
}

bool MainController::openFileWithDialog()
{
    const QString formatString = QString::fromLatin1("%2 (%1);;%3 (*.*)")
            .arg(m_impl->supportedFormats.join(QString::fromLatin1(" ")))
            .arg(tr("All Supported Images")).arg(tr("All Files"));
    const QString filePath = QFileDialog::getOpenFileName(&m_impl->mainWindow, tr("Open File"), m_impl->settings.lastOpenedPath(), formatString);
    if(filePath.isEmpty())
        return false;
    return openPath(filePath);
}

bool MainController::openFolderWithDialog()
{
    const QString lastDirPath = QFileInfo(m_impl->settings.lastOpenedPath()).absolutePath();
    const QString dirPath = QFileDialog::getExistingDirectory(&m_impl->mainWindow, tr("Open Directory"), lastDirPath);
    if(dirPath.isEmpty())
        return false;
    return openPath(dirPath);
}

bool MainController::deleteCurrentFile()
{
    if(!m_impl->hasCurrentFile())
        return false;

    if(m_impl->settings.askBeforeDelete())
    {
        if(QMessageBox::warning(&m_impl->mainWindow, tr("Delete File"), tr("Are you sure you want to delete current file?"),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
            return false;
    }

    if(m_impl->settings.moveToTrash())
    {
        QString errorDescription;
        if(!m_impl->fileManager.moveToTrashCurrentFile(&errorDescription))
        {
            QMessageBox::critical(&m_impl->mainWindow, tr("Error"), errorDescription);
            return false;
        }
    }
    else
    {
        if(!m_impl->fileManager.deleteCurrentFile())
        {
            QMessageBox::critical(&m_impl->mainWindow, tr("Error"), tr("Failed to delete file \"%1\"").arg(m_impl->fileManager.currentFilePath()));
            return false;
        }
    }

    return true;
}

bool MainController::selectNextFile()
{
    if(m_impl->hasNextFile())
        return m_impl->fileManager.selectByIndex(m_impl->fileManager.currentFileIndex() + 1);
    return selectFirstFile();
}

bool MainController::selectPreviousFile()
{
    if(m_impl->hasPreviousFile())
        return m_impl->fileManager.selectByIndex(m_impl->fileManager.currentFileIndex() - 1);
    return selectLastFile();
}

bool MainController::selectFirstFile()
{
    if(m_impl->fileManager.filesCount() <= 0)
        return false;
    return m_impl->fileManager.selectByIndex(0);
}

bool MainController::selectLastFile()
{
    if(m_impl->fileManager.filesCount() <= 0)
        return false;
    return m_impl->fileManager.selectByIndex(m_impl->fileManager.filesCount() - 1);
}

void MainController::showMainWindow()
{
    m_impl->mainWindow.showNormal();
}

void MainController::showAbout()
{
    AboutDialog dialog(&m_impl->mainWindow);
    dialog.exec();
}

void MainController::showPreferences()
{
    SettingsDialog dialog(&m_impl->settings, &m_impl->mainWindow);
    dialog.exec();
}

void MainController::openNewWindow()
{
    const QStringList args = m_impl->hasCurrentFile() ? QStringList(m_impl->fileManager.currentFilePath()) : QStringList();
    QProcess::startDetached(QApplication::applicationFilePath(), args, QDir::currentPath());
}

void MainController::onFileManagerStateChanged(const FileManager::ChangeFlags &changeFlags)
{
    m_impl->settings.setLastOpenedPath(m_impl->fileManager.currentFilePath());

    UIState uiState;
    uiState.hasCurrentFile          = m_impl->hasCurrentFile();
    uiState.hasCurrentFileIndex     = m_impl->hasCurrentFileIndex();
    uiState.currentFilePath         = m_impl->fileManager.currentFilePath();
    uiState.currentFileIndex        = m_impl->fileManager.currentFileIndex();
    uiState.filesCount              = m_impl->fileManager.filesCount();
    uiState.canDeleteCurrentFile    = m_impl->fileManager.canDeleteCurrentFile();

    UIChangeFlags uiChangeFlags;
    uiChangeFlags.setFlag(UICF_HasCurrentFile       , uiState.hasCurrentFile != m_impl->lastHasCurrentFile);
    uiChangeFlags.setFlag(UICF_HasCurrentFileIndex  , uiState.hasCurrentFileIndex != m_impl->lastHasCurrentFileIndex);
    uiChangeFlags.setFlag(UICF_CurrentFilePath      , changeFlags.testFlag(FileManager::FlagCurrentFilePath));
    uiChangeFlags.setFlag(UICF_CurrentFileIndex     , changeFlags.testFlag(FileManager::FlagCurrentFileIndex));
    uiChangeFlags.setFlag(UICF_FilesCount           , changeFlags.testFlag(FileManager::FlagFilesCount));
    uiChangeFlags.setFlag(UICF_CanDeleteCurrentFile , changeFlags.testFlag(FileManager::FlagCanDeleteCurrentFile));

    m_impl->lastHasCurrentFile = uiState.hasCurrentFile;
    m_impl->lastHasCurrentFileIndex = uiState.hasCurrentFileIndex;

    emit uiStateChanged(uiState, uiChangeFlags);
}
