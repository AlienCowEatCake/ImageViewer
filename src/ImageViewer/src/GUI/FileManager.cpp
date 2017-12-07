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

#include "FileManager.h"

#include <cassert>
#include <algorithm>

#include <QString>
#include <QStringList>
#include <QVector>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>

#include "Utils/FileUtils.h"
#include "Utils/SignalBlocker.h"

// ====================================================================================================

namespace {

bool numericLessThan(const QString &s1, const QString &s2)
{
    const QString sl1 = s1.toLower(), sl2 = s2.toLower();
    QString::ConstIterator it1 = sl1.constBegin(), it2 = sl2.constBegin();
    for(; it1 != sl1.constEnd() && it2 != sl2.constEnd(); ++it1, ++it2)
    {
        QChar c1 = *it1, c2 = *it2;
        if(c1.isNumber() && c2.isNumber())
        {
            QString num1, num2;
            while(c1.isNumber())
            {
                num1.append(c1);
                if((++it1) == sl1.constEnd())
                    break;
                c1 = *it1;
            }
            while(c2.isNumber())
            {
                num2.append(c2);
                if((++it2) == sl2.constEnd())
                    break;
                c2 = *it2;
            }
            if(num1 != num2)
            {
                return num1.toLongLong() < num2.toLongLong();
            }
            else
            {
                if(it1 == sl1.constEnd() || it2 == sl2.constEnd())
                    break;
                --it1;
                --it2;
            }
        }
        else if(c1 != c2)
        {
            return c1 < c2;
        }
    }
    if(it1 == sl1.constEnd() && it2 != sl2.constEnd())
        return true;
    if(it1 != sl1.constEnd() && it2 == sl2.constEnd())
        return false;
    return s1 < s2;
}

const int INVALID_INDEX = -1;

QStringList supportedFilesInDirectory(const QStringList &supportedFormatsWithWildcards, const QDir &dir)
{
    QStringList list = dir.entryList(supportedFormatsWithWildcards, QDir::Files | QDir::Readable, QDir::NoSort);
    std::sort(list.begin(), list.end(), &numericLessThan);
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

    virtual bool selectByIndex(int index) = 0;
    virtual void update() = 0;

    virtual QFileSystemWatcher * watcher() = 0;
};


class DirContentModel : public IFilesModel
{
public:
    DirContentModel(const QStringList &supportedFormatsWithWildcards, const QString &filePath)
        : m_supportedFormats(supportedFormatsWithWildcards)
        , m_currentFilePath(filePath)
        , m_currentIndex(INVALID_INDEX)
        , m_directoryPath(QFileInfo(filePath).absolutePath())
        , m_canDeleteCurrentFile(false)
    {
        assert(!filePath.isEmpty());
        assert(QFileInfo(filePath).isFile());
        update();
        m_watcher.addPath(m_directoryPath);
    }

    QString currentFilePath() const     { return m_currentFilePath; }
    int currentFileIndex() const        { return m_currentIndex; }
    int filesCount() const              { return m_filesList.size(); }
    bool canDeleteCurrentFile() const   { return m_canDeleteCurrentFile; }

    bool selectByIndex(int index)
    {
        if(index < 0 || index >= m_filesList.size())
            return false;
        m_currentIndex = index;
        m_currentFilePath = QDir(m_directoryPath).absoluteFilePath(m_filesList[m_currentIndex]);
        m_canDeleteCurrentFile = canDeleteFile(m_currentFilePath);
        return true;
    }

    void update()
    {
        const QSignalBlocker watcherBlocker(m_watcher);
        const QFileInfo fileInfo = QFileInfo(m_currentFilePath);
        const QString fileName = fileInfo.fileName();
        m_filesList.clear();
        m_currentIndex = INVALID_INDEX;
        const QStringList list = supportedFilesInDirectory(m_supportedFormats, fileInfo.dir());
        for(QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it)
        {
            const QString &name = *it;
            m_filesList.append(name);
            if(m_currentIndex == INVALID_INDEX && name == fileName)
                m_currentIndex = m_filesList.size() - 1;
        }
        if(m_currentIndex < 0)
            m_filesList.clear();
        m_canDeleteCurrentFile = canDeleteFile(m_currentFilePath);
    }

    QFileSystemWatcher * watcher() { return &m_watcher; }

private:
    const QStringList m_supportedFormats;
    QFileSystemWatcher m_watcher;
    QVector<QString> m_filesList;
    QString m_currentFilePath;
    int m_currentIndex;
    const QString m_directoryPath;
    bool m_canDeleteCurrentFile;
};


class FilxedListModel : public IFilesModel
{
public:
    FilxedListModel(const QStringList &filePaths)
        : m_pathsList(filePaths.toVector())
        , m_currentIndex(0)
        , m_canDeleteCurrentFile(false)
    {
        assert(!filePaths.isEmpty());
        m_watcher.addPaths(filePaths);
        m_currentFilePath = filePaths.first();
        m_canDeleteCurrentFile = canDeleteFile(m_currentFilePath);
    }

    QString currentFilePath() const     { return m_currentFilePath; }
    int currentFileIndex() const        { return m_currentIndex; }
    int filesCount() const              { return m_pathsList.size(); }
    bool canDeleteCurrentFile() const   { return m_canDeleteCurrentFile; }

    bool selectByIndex(int index)
    {
        if(index < 0 || index >= m_pathsList.size())
            return false;
        m_currentIndex = index;
        m_currentFilePath = m_pathsList[m_currentIndex];
        m_canDeleteCurrentFile = canDeleteFile(m_currentFilePath);
        return true;
    }

    void update()
    {
        const QSignalBlocker watcherBlocker(m_watcher);
        bool wasChanged = false;
        for(QVector<QString>::Iterator it = m_pathsList.begin(); it != m_pathsList.end();)
        {
            if(!QFileInfo(*it).exists())
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

    QFileSystemWatcher * watcher() { return &m_watcher; }

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
    const QStringList supportedFormats;
    QScopedPointer<IFilesModel> filesModel;
    QStringList currentOpenArguments;

    Impl(FileManager *fileManager, const QStringList &supportedFormatsWithWildcards)
        : fileManager(fileManager)
        , supportedFormats(supportedFormatsWithWildcards)
    {}

    void resetFilesModel(IFilesModel *newFilesModel = NULL)
    {
        currentOpenArguments.clear();
        filesModel.reset(newFilesModel);
        if(!newFilesModel)
            return;
        QObject::connect(filesModel->watcher(), SIGNAL(directoryChanged(const QString&)), fileManager, SLOT(update()));
        QObject::connect(filesModel->watcher(), SIGNAL(fileChanged(const QString&)), fileManager, SLOT(update()));
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
    ChangedGuard(FileManager *manager)
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
            emit m_manager->stateChanged(flags);
    }

private:
    FileManager * const m_manager;
    QString m_currentFilePath;
    int m_currentFileIndex;
    int m_filesCount;
    bool m_canDeleteCurrentFile;
};

// ====================================================================================================

FileManager::FileManager(const QStringList &supportedFormatsWithWildcards, QObject *parent)
    : QObject(parent)
    , m_impl(new Impl(this, supportedFormatsWithWildcards))
{}

FileManager::~FileManager()
{}

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
}

bool FileManager::openPath(const QString &filePath)
{
    const QFileInfo fileInfo(filePath);
    if(!fileInfo.exists())
        return false;

    const bool pathIsDir = fileInfo.isDir();
    const Impl::ChangedGuard changedGuard(this);
    const QStringList files = supportedPathsInDirectory(m_impl->supportedFormats, pathIsDir ? QDir(fileInfo.absoluteFilePath()) : fileInfo.absoluteDir());
    if(!files.isEmpty())
        m_impl->resetFilesModel(new DirContentModel(m_impl->supportedFormats, pathIsDir ? files.first() : fileInfo.absoluteFilePath()));
    else if(!pathIsDir)
        m_impl->resetFilesModel(new FilxedListModel(QStringList() << fileInfo.absoluteFilePath()));
    else
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
    if(!FileUtils::MoveToTrash(currentFilePath(), errorDescription))
        return false;
    return m_impl->selectAnotherFileAfterDeletion();
}

// ====================================================================================================
