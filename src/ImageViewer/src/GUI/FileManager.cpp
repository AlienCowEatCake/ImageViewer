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

#include "FileManager.h"

#include <cassert>
#include <algorithm>

#include <QString>
#include <QVector>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QTimer>
#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
#include <QElapsedTimer>
#else
#include <QTime>
typedef QTime QElapsedTimer;
#endif

#include "Utils/FileUtils.h"
#include "Utils/Global.h"
#include "Utils/SignalBlocker.h"
#include "Utils/StringUtils.h"

// ====================================================================================================

namespace {

const int INVALID_INDEX = -1;

QStringList supportedFilesInDirectory(const QStringList &supportedFormatsWithWildcards, const QDir &dir)
{
    if(supportedFormatsWithWildcards.empty())
        return QStringList();
    QStringList list = dir.entryList(supportedFormatsWithWildcards, QDir::Files | QDir::Readable, QDir::NoSort);
    std::sort(list.begin(), list.end(), &StringUtils::PlatformNumericLessThan);
    return list;
}

QStringList supportedPathsInDirectory(const QStringList &supportedFormatsWithWildcards, const QDir &dir)
{
    QStringList list = supportedFilesInDirectory(supportedFormatsWithWildcards, dir);
    for(QStringList::Iterator it = list.begin(), itEnd = list.end(); it != itEnd; ++it)
        *it = dir.absoluteFilePath(*it);
    return list;
}

bool canDeleteFile(const QString &filePath)
{
    if(filePath.isEmpty())
        return false;
    const QFileInfo fileInfo(filePath);
    return fileInfo.exists() && fileInfo.isWritable();
}

} // namespace

// ====================================================================================================

namespace {

class IFilesModel
{
public:
    virtual ~IFilesModel() {}

    virtual QString currentFilePath() const = 0;
    virtual int currentFileIndex() const = 0;
    virtual int filesCount() const = 0;
    virtual bool canDeleteCurrentFile() const = 0;

    virtual bool selectByPath(const QString &path) = 0;
    virtual bool selectByIndex(int index) = 0;
    virtual void update() = 0;

    virtual QFileSystemWatcher * watcher() = 0;
};


class DirContentModel : public IFilesModel
{
public:
    DirContentModel(const QStringList &supportedFormatsWithWildcards, const QString &filePath)
        : m_supportedFormats(supportedFormatsWithWildcards)
        , m_currentIndex(INVALID_INDEX)
        , m_canDeleteCurrentFile(false)
    {
        assert(!filePath.isEmpty());
        const QFileInfo fileInfo = QFileInfo(filePath);
        if(fileInfo.isFile())
        {
            m_currentFilePath = filePath;
            m_directoryPath = fileInfo.absolutePath();
        }
        else
        {
            m_directoryPath = fileInfo.absoluteFilePath();
        }
        m_watcher.addPath(m_directoryPath);
        DirContentModel::update();
    }

    QString currentFilePath() const Q_DECL_OVERRIDE     { return m_currentFilePath; }
    int currentFileIndex() const Q_DECL_OVERRIDE        { return m_currentIndex; }
    int filesCount() const Q_DECL_OVERRIDE              { return m_filesList.size(); }
    bool canDeleteCurrentFile() const Q_DECL_OVERRIDE   { return m_canDeleteCurrentFile; }

    bool selectByPath(const QString &path) Q_DECL_OVERRIDE
    {
        const QString absolutePath = QFileInfo(path).absoluteFilePath();
        for(int i = 0; i < m_filesList.size(); ++i)
            if(QDir(m_directoryPath).absoluteFilePath(m_filesList[i]) == absolutePath)
                return selectByIndex(i);
        const QString absolutePathC = absolutePath.normalized(QString::NormalizationForm_C);
        for(int i = 0; i < m_filesList.size(); ++i)
            if(QDir(m_directoryPath).absoluteFilePath(m_filesList[i]).normalized(QString::NormalizationForm_C) == absolutePathC)
                return selectByIndex(i);
        return false;
    }

    bool selectByIndex(int index) Q_DECL_OVERRIDE
    {
        if(index < 0 || index >= m_filesList.size())
            return false;
        m_currentIndex = index;
        m_currentFilePath = QDir(m_directoryPath).absoluteFilePath(m_filesList[m_currentIndex]);
        m_canDeleteCurrentFile = canDeleteFile(m_currentFilePath);
        return true;
    }

    void update() Q_DECL_OVERRIDE
    {
        const QSignalBlocker watcherBlocker(m_watcher);
        m_filesList.clear();
        m_currentIndex = INVALID_INDEX;
        m_canDeleteCurrentFile = false;
        if(m_directoryPath.isEmpty())
            return;

        const QDir dir = QDir(m_directoryPath);
        const QStringList list = supportedFilesInDirectory(m_supportedFormats, dir);
        if(list.isEmpty())
            return;

        if(m_currentFilePath.isEmpty())
        {
            m_currentIndex = 0;
            m_currentFilePath = dir.absoluteFilePath(list.first());
        }

        const QFileInfo fileInfo = QFileInfo(m_currentFilePath);
        const QString fileName = fileInfo.fileName();
        for(QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it)
        {
            const QString &name = *it;
            m_filesList.append(name);
            if(m_currentIndex == INVALID_INDEX && name == fileName)
                m_currentIndex = m_filesList.size() - 1;
        }
        if(m_currentIndex == INVALID_INDEX)
        {
            const QString fileNameC = fileName.normalized(QString::NormalizationForm_C);
            for(int i = 0; i < m_filesList.size(); ++i)
            {
                if(m_filesList[i].normalized(QString::NormalizationForm_C) != fileNameC)
                    continue;
                m_currentIndex = i;
                m_currentFilePath = dir.absoluteFilePath(m_filesList[i]);
                break;
            }
        }
        if(m_currentIndex < 0)
            m_filesList.clear();
        m_canDeleteCurrentFile = canDeleteFile(m_currentFilePath);
    }

    QFileSystemWatcher * watcher() Q_DECL_OVERRIDE { return &m_watcher; }

private:
    const QStringList m_supportedFormats;
    QFileSystemWatcher m_watcher;
    QVector<QString> m_filesList;
    QString m_currentFilePath;
    int m_currentIndex;
    QString m_directoryPath;
    bool m_canDeleteCurrentFile;
};


class FilxedListModel : public IFilesModel
{
public:
    explicit FilxedListModel(const QStringList &filePaths)
        : m_pathsList(filePaths.toVector())
        , m_currentIndex(0)
        , m_canDeleteCurrentFile(false)
    {
        assert(!filePaths.isEmpty());
        m_watcher.addPaths(filePaths);
        m_currentFilePath = filePaths.first();
        m_canDeleteCurrentFile = canDeleteFile(m_currentFilePath);
    }

    QString currentFilePath() const Q_DECL_OVERRIDE     { return m_currentFilePath; }
    int currentFileIndex() const Q_DECL_OVERRIDE        { return m_currentIndex; }
    int filesCount() const Q_DECL_OVERRIDE              { return m_pathsList.size(); }
    bool canDeleteCurrentFile() const Q_DECL_OVERRIDE   { return m_canDeleteCurrentFile; }

    bool selectByPath(const QString &path) Q_DECL_OVERRIDE
    {
        const QString absolutePath = QFileInfo(path).absoluteFilePath();
        for(int i = 0; i < m_pathsList.size(); ++i)
            if(QFileInfo(m_pathsList[i]).absoluteFilePath() == absolutePath)
                return selectByIndex(i);
        const QString absolutePathC = absolutePath.normalized(QString::NormalizationForm_C);
        for(int i = 0; i < m_pathsList.size(); ++i)
            if(QFileInfo(m_pathsList[i]).absoluteFilePath().normalized(QString::NormalizationForm_C) == absolutePathC)
                return selectByIndex(i);
        return false;
    }

    bool selectByIndex(int index) Q_DECL_OVERRIDE
    {
        if(index < 0 || index >= m_pathsList.size())
            return false;
        m_currentIndex = index;
        m_currentFilePath = m_pathsList[m_currentIndex];
        m_canDeleteCurrentFile = canDeleteFile(m_currentFilePath);
        return true;
    }

    void update() Q_DECL_OVERRIDE
    {
        const QSignalBlocker watcherBlocker(m_watcher);
        bool wasChanged = false;
        for(QVector<QString>::Iterator it = m_pathsList.begin(); it != m_pathsList.end();)
        {
            if(!QFileInfo_exists(*it))
            {
                it = m_pathsList.erase(it);
                wasChanged = true;
            }
            else
            {
                ++it;
            }
        }
        if(wasChanged)
            m_currentIndex = m_pathsList.indexOf(m_currentFilePath);
        m_canDeleteCurrentFile = canDeleteFile(m_currentFilePath);
    }

    QFileSystemWatcher * watcher() Q_DECL_OVERRIDE { return &m_watcher; }

private:
    QFileSystemWatcher m_watcher;
    QVector<QString> m_pathsList;
    QString m_currentFilePath;
    int m_currentIndex;
    bool m_canDeleteCurrentFile;
};

} // namespace

// ====================================================================================================

struct FileManager::Impl
{
    class ChangedGuard;

    FileManager *fileManager;
    QStringList supportedFormats;
    QScopedPointer<IFilesModel> filesModel;
    QStringList currentOpenArguments;
    QTimer *updateTimer;
    bool filesModelIsDirty;
    QElapsedTimer timeFromLastUpdate;

    explicit Impl(FileManager *fileManager)
        : fileManager(fileManager)
        , supportedFormats(QString::fromLatin1("*.*"))
        , updateTimer(new QTimer(fileManager))
        , filesModelIsDirty(false)
    {
        timeFromLastUpdate.start();
        QObject::connect(updateTimer, SIGNAL(timeout()), fileManager, SLOT(ensureUpdated()));
        updateTimer->setInterval(1000);
        updateTimer->setSingleShot(false);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        updateTimer->setTimerType(Qt::VeryCoarseTimer);
#endif
    }

    void resetFilesModel(IFilesModel *newFilesModel = Q_NULLPTR)
    {
        updateTimer->stop();
        currentOpenArguments.clear();
        filesModel.reset(newFilesModel);
        filesModelIsDirty = false;
        timeFromLastUpdate.restart();
        if(!newFilesModel)
            return;
        QObject::connect(filesModel->watcher(), SIGNAL(directoryChanged(QString)), fileManager, SLOT(tryUpdate()));
        QObject::connect(filesModel->watcher(), SIGNAL(fileChanged(QString)), fileManager, SLOT(tryUpdate()));
        updateTimer->start();
    }

    bool selectAnotherFileAfterDeletion()
    {
        if(!filesModel)
            return false;
        if(filesModel->currentFileIndex() + 1 < filesModel->filesCount())
            return filesModel->selectByIndex(filesModel->currentFileIndex() + 1);
        if(filesModel->currentFileIndex() - 1 >= 0)
            return filesModel->selectByIndex(filesModel->currentFileIndex() - 1);
        resetFilesModel();
        return true;
    }
};

// ====================================================================================================

class FileManager::Impl::ChangedGuard
{
public:
    explicit ChangedGuard(FileManager *manager)
        : m_manager(manager)
        , m_currentFilePath(manager->currentFilePath())
        , m_currentFileIndex(manager->currentFileIndex())
        , m_filesCount(manager->filesCount())
        , m_canDeleteCurrentFile(manager->canDeleteCurrentFile())
    {}

    ~ChangedGuard()
    {
        FileManager::ChangeFlags flags;
        if(m_currentFilePath != m_manager->currentFilePath())
            flags |= FileManager::FlagCurrentFilePath;
        if(m_currentFileIndex != m_manager->currentFileIndex())
            flags |= FileManager::FlagCurrentFileIndex;
        if(m_filesCount != m_manager->filesCount())
            flags |= FileManager::FlagFilesCount;
        if(m_canDeleteCurrentFile != m_manager->canDeleteCurrentFile())
            flags |= FileManager::FlagCanDeleteCurrentFile;
        if(flags != FileManager::ChangeFlags())
            Q_EMIT m_manager->stateChanged(flags);
    }

private:
    FileManager * const m_manager;
    QString m_currentFilePath;
    int m_currentFileIndex;
    int m_filesCount;
    bool m_canDeleteCurrentFile;
};

// ====================================================================================================

FileManager::FileManager(QObject *parent)
    : QObject(parent)
    , m_impl(new Impl(this))
{}

FileManager::~FileManager()
{}

QStringList FileManager::supportedFormatsWithWildcards() const
{
    return m_impl->supportedFormats;
}

void FileManager::setSupportedFormatsWithWildcards(const QStringList &supportedFormatsWithWildcards)
{
    m_impl->supportedFormats = supportedFormatsWithWildcards;
}

QString FileManager::currentFilePath() const
{
    return m_impl->filesModel ? m_impl->filesModel->currentFilePath() : QString();
}

int FileManager::currentFileIndex() const
{
    return m_impl->filesModel ? m_impl->filesModel->currentFileIndex() : INVALID_INDEX;
}

int FileManager::filesCount() const
{
    return m_impl->filesModel ? m_impl->filesModel->filesCount() : 0;
}

bool FileManager::canDeleteCurrentFile() const
{
    return m_impl->filesModel ? m_impl->filesModel->canDeleteCurrentFile() : false;
}

QStringList FileManager::currentOpenArguments() const
{
    return m_impl->currentOpenArguments;
}

void FileManager::reset()
{
    const Impl::ChangedGuard changedGuard(this);
    m_impl->resetFilesModel();
}

void FileManager::update()
{
    const Impl::ChangedGuard changedGuard(this);
    if(m_impl->filesModel)
        m_impl->filesModel->update();
    m_impl->filesModelIsDirty = false;
    m_impl->timeFromLastUpdate.restart();
}

void FileManager::tryUpdate()
{
    if(!m_impl->filesModel)
        return;

    if(static_cast<qint64>(m_impl->timeFromLastUpdate.elapsed()) >= static_cast<qint64>(m_impl->updateTimer->interval()))
        update();
    else
        m_impl->filesModelIsDirty = true;
}

void FileManager::ensureUpdated()
{
    if(m_impl->filesModelIsDirty)
        update();
}

bool FileManager::openPath(const QString &filePath)
{
    const QFileInfo fileInfo(filePath);
    if(!fileInfo.exists())
        return false;

    const Impl::ChangedGuard changedGuard(this);
    m_impl->resetFilesModel(new DirContentModel(m_impl->supportedFormats, fileInfo.absoluteFilePath()));
    if(m_impl->filesModel->filesCount() <= 0 && !fileInfo.isDir())
        m_impl->resetFilesModel(new FilxedListModel(QStringList() << fileInfo.absoluteFilePath()));
    if(m_impl->filesModel->filesCount() <= 0)
        return false;
    m_impl->currentOpenArguments = QStringList(filePath);
    return true;
}

bool FileManager::openPaths(const QStringList &filePaths)
{
    if(filePaths.size() == 0)
        return false;

    if(filePaths.size() == 1)
        return openPath(filePaths.first());

    QStringList pathsList;
    for(QStringList::ConstIterator it = filePaths.constBegin(), itEnd = filePaths.constEnd(); it != itEnd; ++it)
    {
        const QFileInfo fileInfo(*it);
        if(!fileInfo.exists())
            continue;

        if(fileInfo.isDir())
        {
            const QStringList pathsInDir = supportedPathsInDirectory(m_impl->supportedFormats, QDir(fileInfo.absoluteFilePath()));
            for(QStringList::ConstIterator jt = pathsInDir.constBegin(), jtEnd = pathsInDir.constEnd(); jt != jtEnd; ++jt)
                pathsList.append(*jt);
        }
        else
        {
            pathsList.append(*it);
        }
    }

    if(pathsList.isEmpty())
        return false;

    const Impl::ChangedGuard changedGuard(this);
    m_impl->resetFilesModel(new FilxedListModel(pathsList));
    m_impl->currentOpenArguments = filePaths;
    return true;
}

bool FileManager::selectByPath(const QString &filePath)
{
    const Impl::ChangedGuard changedGuard(this);
    if(m_impl->filesModel)
        return m_impl->filesModel->selectByPath(filePath);
    return false;
}

bool FileManager::selectByIndex(int index)
{
    const Impl::ChangedGuard changedGuard(this);
    if(m_impl->filesModel)
        return m_impl->filesModel->selectByIndex(index);
    return false;
}

bool FileManager::deleteCurrentFile()
{
    const Impl::ChangedGuard changedGuard(this);
    if(!m_impl->filesModel || !m_impl->filesModel->canDeleteCurrentFile())
        return false;
    if(!QFile(currentFilePath()).remove())
        return false;
    return m_impl->selectAnotherFileAfterDeletion();
}

bool FileManager::moveToTrashCurrentFile(QString *errorDescription)
{
    const Impl::ChangedGuard changedGuard(this);
    if(!m_impl->filesModel || !m_impl->filesModel->canDeleteCurrentFile())
        return false;
    if(!FileUtils::MoveToTrashOrDelete(currentFilePath(), errorDescription))
        return false;
    return m_impl->selectAnotherFileAfterDeletion();
}

// ====================================================================================================
