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

#include "MainController.h"

#include <algorithm>

#include <QApplication>
#include <QProcess>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>

#include "Utils/ScopedPointer.h"
#include "Utils/SignalBlocker.h"
#include "Widgets/StylesheetEditor.h"

#include "Decoders/DecodersManager.h"
#include "Dialogs/AboutDialog.h"
#include "Dialogs/InfoDialog.h"
#include "Dialogs/SettingsDialog.h"
#include "MainWindow/MainWindow.h"
#include "GUISettings.h"

struct MainController::Impl
{
    MainController * const mainController;
    FileManager fileManager;
    GUISettings settings;
    MainWindow mainWindow;
    QScopedPointer<StylesheetEditor> stylesheetEditor;

    QSharedPointer<IImageData> imageData;

    bool lastHasCurrentFile;
    bool lastHasCurrentFileIndex;

    Impl(MainController *mainController)
        : mainController(mainController)
        , fileManager()
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

    UIState createUIState() const
    {
        UIState uiState;
        uiState.hasCurrentFile          = hasCurrentFile();
        uiState.hasCurrentFileIndex     = hasCurrentFileIndex();
        uiState.currentFilePath         = fileManager.currentFilePath();
        uiState.currentFileIndex        = fileManager.currentFileIndex();
        uiState.filesCount              = fileManager.filesCount();
        uiState.canDeleteCurrentFile    = fileManager.canDeleteCurrentFile();
        uiState.imageData               = imageData;
        return uiState;
    }
};

MainController::MainController(QObject *parent)
    : QObject(parent)
    , m_impl(new Impl(this))
{
    MainWindow * const mainWindow = &m_impl->mainWindow;

    connect(&m_impl->fileManager, SIGNAL(stateChanged(const FileManager::ChangeFlags&)), this, SLOT(onFileManagerStateChanged(const FileManager::ChangeFlags&)));
    connect(this, SIGNAL(uiStateChanged(const UIState&, const UIChangeFlags&)), mainWindow, SLOT(updateUIState(const UIState&, const UIChangeFlags&)));

    connect(mainWindow, SIGNAL(selectFirstRequested())                  , this, SLOT(selectFirstFile())                     );
    connect(mainWindow, SIGNAL(selectLastRequested())                   , this, SLOT(selectLastFile())                      );
    connect(mainWindow, SIGNAL(selectPreviousRequested())               , this, SLOT(selectPreviousFile())                  );
    connect(mainWindow, SIGNAL(selectNextRequested())                   , this, SLOT(selectNextFile())                      );
    connect(mainWindow, SIGNAL(openPathRequested(const QString&))       , this, SLOT(openPath(const QString&))              );
    connect(mainWindow, SIGNAL(openPathsRequested(const QStringList&))  , this, SLOT(openPaths(const QStringList&))         );
    connect(mainWindow, SIGNAL(openFileWithDialogRequested())           , this, SLOT(openFileWithDialog())                  );
    connect(mainWindow, SIGNAL(openFolderWithDialogRequested())         , this, SLOT(openFolderWithDialog())                );
    connect(mainWindow, SIGNAL(deleteFileRequested())                   , this, SLOT(deleteCurrentFile())                   );
    connect(mainWindow, SIGNAL(reopenWithRequested(const QString&))     , this, SLOT(onReopenWithRequested(const QString&)) );
    connect(mainWindow, SIGNAL(newWindowRequested())                    , this, SLOT(openNewWindow())                       );
    connect(mainWindow, SIGNAL(imageInformationRequested())             , this, SLOT(showImageInformation())                );
    connect(mainWindow, SIGNAL(preferencesRequested())                  , this, SLOT(showPreferences())                     );
    connect(mainWindow, SIGNAL(aboutRequested())                        , this, SLOT(showAbout())                           );
    connect(mainWindow, SIGNAL(aboutQtRequested())                      , qApp, SLOT(aboutQt())                             );
    connect(mainWindow, SIGNAL(editStylesheetRequested())               , this, SLOT(showStylesheetEditor())                );
    connect(mainWindow, SIGNAL(closed())                                , qApp, SLOT(quit())                                );
}

MainController::~MainController()
{}

bool MainController::openPath(const QString &path)
{
    m_impl->fileManager.setSupportedFormatsWithWildcards(DecodersManager::getInstance().supportedFormatsWithWildcards());
    return m_impl->fileManager.openPath(path);
}

bool MainController::openPaths(const QStringList &paths)
{
    m_impl->fileManager.setSupportedFormatsWithWildcards(DecodersManager::getInstance().supportedFormatsWithWildcards());
    return m_impl->fileManager.openPaths(paths);
}

bool MainController::openFileWithDialog()
{
    const QString formatString = QString::fromLatin1("%2 (%1);;%3 (*.*)")
            .arg(DecodersManager::getInstance().supportedFormatsWithWildcards().join(QString::fromLatin1(" ")))
            .arg(tr("All Supported Images")).arg(tr("All Files"));
    const QStringList filePaths = QFileDialog::getOpenFileNames(&m_impl->mainWindow, tr("Open File"), m_impl->settings.lastOpenedPath(), formatString);
    if(filePaths.isEmpty())
        return false;
    const bool status = openPaths(filePaths);
    if(!status)
    {
        const QString errorMessage = (filePaths.size() == 1)
                ? (tr("Failed to open file \"%1\"").arg(filePaths.first()))
                : (tr("Failed to open files \"%1\"").arg(filePaths.join(QString::fromLatin1("\", \""))));
        QMessageBox::critical(&m_impl->mainWindow, tr("Error"), errorMessage);
    }
    return status;
}

bool MainController::openFolderWithDialog()
{
    const QString lastDirPath = QFileInfo(m_impl->settings.lastOpenedPath()).absolutePath();
    const QString dirPath = QFileDialog::getExistingDirectory(&m_impl->mainWindow, tr("Open Directory"), lastDirPath);
    if(dirPath.isEmpty())
        return false;
    const bool status = openPath(dirPath);
    if(!status)
        QMessageBox::critical(&m_impl->mainWindow, tr("Error"), tr("Failed to open folder \"%1\"").arg(dirPath));
    return status;
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
    const QStringList oldBlackList = DecodersManager::getInstance().blackListedDecoders();
    SettingsDialog dialog(&m_impl->settings, &m_impl->mainWindow);
    if(dialog.exec() != QDialog::Accepted)
        return;

    QStringList newBlackList = DecodersManager::getInstance().blackListedDecoders();
    if(oldBlackList.size() == newBlackList.size())
    {
        for(QStringList::ConstIterator it = oldBlackList.constBegin(), itEnd = oldBlackList.constEnd(); it != itEnd; ++it)
        {
            QStringList::iterator found = std::find(newBlackList.begin(), newBlackList.end(), *it);
            if(found == newBlackList.end())
                break;
            newBlackList.erase(found);
        }
        if(newBlackList.empty())
            return;
    }

    /// @note Перезагружаем все, чтобы применились настройки декодеров
    {
        QSignalBlocker blocker(m_impl->fileManager);
        openPaths(m_impl->fileManager.currentOpenArguments());
    }
    onFileManagerStateChanged(FileManager::FlagChangeAll);
}

void MainController::showImageInformation()
{
    InfoDialog dialog(m_impl->imageData, &m_impl->mainWindow);
    dialog.exec();
}

void MainController::showStylesheetEditor()
{
    if(!m_impl->stylesheetEditor)
    {
        m_impl->stylesheetEditor.reset(new StylesheetEditor());
        m_impl->stylesheetEditor->setProtected(false);
    }
    m_impl->stylesheetEditor->show();
}

void MainController::openNewWindow()
{
    m_impl->mainWindow.saveGeometrySettings();
    m_impl->settings.flush();
    QProcess::startDetached(QApplication::applicationFilePath(), m_impl->fileManager.currentOpenArguments(), QDir::currentPath());
}

void MainController::onReopenWithRequested(const QString &decoderName)
{
    const QString currentFilePath = m_impl->fileManager.currentFilePath();
    if(currentFilePath.isEmpty())
    {
        m_impl->imageData = QSharedPointer<IImageData>();
    }
    else
    {
        /// @note Не все декодеры поддерживают одновременное открытие нескольких файлов
        if(m_impl->imageData)
        {
            m_impl->imageData = QSharedPointer<IImageData>();
            emit uiStateChanged(m_impl->createUIState(), UICF_ImageData);
        }
        m_impl->imageData = DecodersManager::getInstance().loadImage(currentFilePath, decoderName);
    }
    UIState uiState = m_impl->createUIState();
    UIChangeFlags uiChangeFlags = UICF_ImageData;
    emit uiStateChanged(uiState, uiChangeFlags);
    if(!currentFilePath.isEmpty() && !m_impl->imageData)
        QMessageBox::critical(&m_impl->mainWindow, tr("Error"), tr("Failed to open file \"%1\"").arg(uiState.currentFilePath));
}

void MainController::onFileManagerStateChanged(const FileManager::ChangeFlags &changeFlags)
{
    const QString currentFilePath = m_impl->fileManager.currentFilePath();
    if(!currentFilePath.isEmpty())
        m_impl->settings.setLastOpenedPath(currentFilePath);
    if(changeFlags.testFlag(FileManager::FlagCurrentFilePath))
    {
        if(currentFilePath.isEmpty())
        {
            m_impl->imageData = QSharedPointer<IImageData>();
        }
        else
        {
            /// @note Не все декодеры поддерживают одновременное открытие нескольких файлов
            if(m_impl->imageData)
            {
                m_impl->imageData = QSharedPointer<IImageData>();
                emit uiStateChanged(m_impl->createUIState(), UICF_ImageData);
            }
            m_impl->imageData = DecodersManager::getInstance().loadImage(currentFilePath);
        }
    }

    UIState uiState = m_impl->createUIState();
    UIChangeFlags uiChangeFlags;
    if(uiState.hasCurrentFile != m_impl->lastHasCurrentFile)
        uiChangeFlags |= UICF_HasCurrentFile;
    if(uiState.hasCurrentFileIndex != m_impl->lastHasCurrentFileIndex)
        uiChangeFlags |= UICF_HasCurrentFileIndex;
    if(changeFlags.testFlag(FileManager::FlagCurrentFilePath))
        uiChangeFlags |= UICF_CurrentFilePath | UICF_ImageData;
    if(changeFlags.testFlag(FileManager::FlagCurrentFileIndex))
        uiChangeFlags |= UICF_CurrentFileIndex;
    if(changeFlags.testFlag(FileManager::FlagFilesCount))
        uiChangeFlags |= UICF_FilesCount;
    if(changeFlags.testFlag(FileManager::FlagCanDeleteCurrentFile))
        uiChangeFlags |= UICF_CanDeleteCurrentFile;

    m_impl->lastHasCurrentFile = uiState.hasCurrentFile;
    m_impl->lastHasCurrentFileIndex = uiState.hasCurrentFileIndex;

    emit uiStateChanged(uiState, uiChangeFlags);

    if(!currentFilePath.isEmpty() && !m_impl->imageData)
        QMessageBox::critical(&m_impl->mainWindow, tr("Error"), tr("Failed to open file \"%1\"").arg(uiState.currentFilePath));
}
