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
#include "FileManager_p.h"

#include <algorithm>
#include <cassert>
#include <limits>

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QMutexLocker>
#include <QPointer>
#include <QVector>

#include "Utils/FileUtils.h"
#include "Utils/Logging.h"
#include "Utils/SignalBlocker.h"
#include "Utils/StringUtils.h"

// #define QT_NO_DEBUG_OUTPUT

// ====================================================================================================

namespace {

const int INVALID_INDEX = -1;

bool canDeleteFile(const QString &filePath)
{
    if(filePath.isEmpty())
        return false;
    const QFileInfo fileInfo(filePath);
    return fileInfo.exists() && fileInfo.isWritable();
}

} // namespace

// ====================================================================================================

FilesScanner::FilesScanner(QObject *parent)
    : QThread(parent)
    , m_watcher(new QFileSystemWatcher(Q_NULLPTR))
    , m_watcherConfigured(0)
    , m_stopPending(0)
    , m_updateTimer(new QTimer(this))
    , m_scannerIsDirty(0)
    , m_scannerHasResult(0)
{
    m_watcher->moveToThread(this);
    connect(m_watcher, SIGNAL(directoryChanged(QString)), this, SLOT(tryUpdate()));
    connect(m_watcher, SIGNAL(fileChanged(QString)), this, SLOT(tryUpdate()));

    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(ensureUpdated()));
    m_updateTimer->setInterval(1000);
    m_updateTimer->setSingleShot(false);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    m_updateTimer->setTimerType(Qt::VeryCoarseTimer);
#endif
}

FilesScanner::~FilesScanner()
{
    reset();
    m_watcher->disconnect();
    m_watcher->deleteLater();
}

bool FilesScanner::hasScanResult() const
{
    return QAtomicInt_loadAcquire(m_scannerHasResult) != 0;
}

QStringList FilesScanner::getScanResult()
{
    const QMutexLocker guard(&m_scanResultMutex);
    return m_scanResult;
}

bool FilesScanner::configureForDirContent(const QStringList &supportedFormats, const QString &directoryPath)
{
    reset();
    m_supportedFormats = supportedFormats;
    m_directoryPath = directoryPath;
    m_fixedPathsList.clear();
    if(supportedFormats.isEmpty() || m_directoryPath.isEmpty())
        return false;
    update();
    return true;
}

bool FilesScanner::configureForFilxedList(const QStringList &supportedFormats, const QStringList &fixedPathsList)
{
    reset();
    m_supportedFormats = supportedFormats;
    m_directoryPath.clear();
    m_fixedPathsList = fixedPathsList;
    if(supportedFormats.isEmpty() || m_fixedPathsList.isEmpty())
        return false;
    update();
    return true;
}

void FilesScanner::reset()
{
    QAtomicInt_storeRelease(m_stopPending, 1);

    m_watcherMutex.lock();
    if(isWatcherConfigured())
    {
        const QSignalBlocker watcherBlocker(m_watcher);
        const QStringList directories = m_watcher->directories();
        if(!directories.isEmpty())
            m_watcher->removePaths(directories);
        const QStringList files = m_watcher->files();
        if(!files.isEmpty())
            m_watcher->removePaths(files);
        QAtomicInt_storeRelease(m_watcherConfigured, 0);
    }
    m_watcherMutex.unlock();

    m_updateTimer->stop();
    wait();

    m_scanResultMutex.lock();
    m_scanResult.clear();
    m_scanResultMutex.unlock();

    m_supportedFormats.clear();
    m_directoryPath.clear();
    m_fixedPathsList.clear();

    QAtomicInt_storeRelease(m_scannerIsDirty, 0);
    QAtomicInt_storeRelease(m_scannerHasResult, 0);
    QAtomicInt_storeRelease(m_stopPending, 0);
}

void FilesScanner::run()
{
#if !defined (QT_NO_DEBUG_OUTPUT)
#define CHECK_INTERRUPTION do { if(Q_UNLIKELY(isStopPending())) { LOG_DEBUG() << LOGGING_CTX << "Interrupted"; return; } } while(false)
#else
#define CHECK_INTERRUPTION if(Q_UNLIKELY(isStopPending())) return
#endif
    CHECK_INTERRUPTION;
    if(!m_directoryPath.isEmpty())
    {
        QMutexLocker watcherMutexGuard(&m_watcherMutex);
        CHECK_INTERRUPTION;
        if(Q_UNLIKELY(!isWatcherConfigured()))
        {
#if !defined (QT_NO_DEBUG_OUTPUT)
            QElapsedTimer timer;
            timer.start();
#endif
            m_watcher->addPath(m_directoryPath);
            QAtomicInt_storeRelease(m_watcherConfigured, 1);
#if !defined (QT_NO_DEBUG_OUTPUT)
            LOG_DEBUG() << LOGGING_CTX << "Watcher configured, elapsed time =" << static_cast<qint64>(timer.elapsed()) << "ms";
#endif
        }
        watcherMutexGuard.unlock();
        CHECK_INTERRUPTION;

#if !defined (QT_NO_DEBUG_OUTPUT)
        QElapsedTimer timer;
        timer.start();
#endif
        QStringList list = collectDirContent(m_directoryPath);
#if !defined (QT_NO_DEBUG_OUTPUT)
        LOG_DEBUG() << LOGGING_CTX << "DirContent scanned, elapsed time =" << static_cast<qint64>(timer.elapsed()) << "ms, size =" << list.size();
#endif

        CHECK_INTERRUPTION;
        m_scanResultMutex.lock();
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
        m_scanResult.swap(list);
#else
        m_scanResult = list;
#endif
        m_scanResultMutex.unlock();

        QAtomicInt_storeRelease(m_scannerHasResult, 1);
        Q_EMIT updated();
    }
    else if(!m_fixedPathsList.isEmpty())
    {
        QMutexLocker watcherMutexGuard(&m_watcherMutex);
        if(Q_UNLIKELY(!isWatcherConfigured()))
        {
#if !defined (QT_NO_DEBUG_OUTPUT)
            QElapsedTimer timer;
            timer.start();
#endif
            for(QStringList::ConstIterator it = m_fixedPathsList.begin(), itEnd = m_fixedPathsList.end(); it != itEnd; ++it)
            {
                CHECK_INTERRUPTION;
                m_watcher->addPath(*it);
            }
            QAtomicInt_storeRelease(m_watcherConfigured, 1);
#if !defined (QT_NO_DEBUG_OUTPUT)
            LOG_DEBUG() << LOGGING_CTX << "Watcher configured, elapsed time =" << static_cast<qint64>(timer.elapsed()) << "ms";
#endif
        }
        watcherMutexGuard.unlock();
        CHECK_INTERRUPTION;

#if !defined (QT_NO_DEBUG_OUTPUT)
        QElapsedTimer timer;
        timer.start();
#endif
        QStringList list;
        for(QStringList::ConstIterator it = m_fixedPathsList.begin(), itEnd = m_fixedPathsList.end(); it != itEnd; ++it)
        {
            CHECK_INTERRUPTION;
            const QFileInfo fileInfo(*it);
            if(!fileInfo.exists())
                continue;

            if(fileInfo.isDir())
            {
                const QString directoryPath = fileInfo.absoluteFilePath();
                const QStringList directoryContent = collectDirContent(directoryPath);
                QDir directory(directoryPath);
                for(QStringList::ConstIterator jt = directoryContent.begin(), jtEnd = directoryContent.end(); jt != jtEnd; ++jt)
                {
                    CHECK_INTERRUPTION;
                    list.append(directory.absoluteFilePath(*jt));
                }
            }
            else
            {
                list.append(fileInfo.absoluteFilePath());
            }
        }
#if !defined (QT_NO_DEBUG_OUTPUT)
        LOG_DEBUG() << LOGGING_CTX << "FilxedList scanned, elapsed time =" << static_cast<qint64>(timer.elapsed()) << "ms, size =" << list.size();
#endif

        CHECK_INTERRUPTION;
        m_scanResultMutex.lock();
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
        m_scanResult.swap(list);
#else
        m_scanResult = list;
#endif
        m_scanResultMutex.unlock();

        QAtomicInt_storeRelease(m_scannerHasResult, 1);
        Q_EMIT updated();
    }
#undef CHECK_INTERRUPTION
}

QStringList FilesScanner::collectDirContent(const QString &directoryPath) const
{
#if !defined (QT_NO_DEBUG_OUTPUT)
#define CHECK_INTERRUPTION do { if(Q_UNLIKELY(isStopPending())) { LOG_DEBUG() << LOGGING_CTX << "Interrupted"; return QStringList(); } } while(false)
#else
#define CHECK_INTERRUPTION if(Q_UNLIKELY(isStopPending())) return QStringList()
#endif
#if !defined (QT_NO_DEBUG_OUTPUT)
    QElapsedTimer timer;
    timer.start();
#endif
    QStringList list;
    QDirIterator it(directoryPath, m_supportedFormats, QDir::Files | QDir::NoDotAndDotDot | QDir::Readable);
    while(it.hasNext())
    {
        CHECK_INTERRUPTION;
        it.next();
        list.append(it.fileName());
    }
    CHECK_INTERRUPTION;
#if !defined (QT_NO_DEBUG_OUTPUT)
    LOG_DEBUG() << LOGGING_CTX << "Directory content collected, elapsed time =" << static_cast<qint64>(timer.elapsed()) << "ms, size =" << list.size();
    timer.restart();
#endif
    std::sort(list.begin(), list.end(), &StringUtils::PlatformNumericLessThan);
#if !defined (QT_NO_DEBUG_OUTPUT)
    LOG_DEBUG() << LOGGING_CTX << "Directory content sorted, elapsed time =" << static_cast<qint64>(timer.elapsed()) << "ms, size =" << list.size();
#endif
    return list;
#undef CHECK_INTERRUPTION
}

bool FilesScanner::isStopPending() const
{
    return QAtomicInt_loadAcquire(m_stopPending) != 0;
}

bool FilesScanner::isWatcherConfigured() const
{
    return QAtomicInt_loadAcquire(m_watcherConfigured) != 0;
}

void FilesScanner::update()
{
    if(isRunning())
        return;
    start();
    QAtomicInt_storeRelease(m_scannerIsDirty, 0);
    m_timeFromLastUpdate.restart();
}

void FilesScanner::tryUpdate()
{
    if(static_cast<qint64>(m_timeFromLastUpdate.elapsed()) >= static_cast<qint64>(m_updateTimer->interval()))
        update();
    else
        QAtomicInt_storeRelease(m_scannerIsDirty, 1);
}

void FilesScanner::ensureUpdated()
{
    if(QAtomicInt_loadAcquire(m_scannerIsDirty) != 0)
        update();
}

// ====================================================================================================

namespace {

class IFilesModel
{
public:
    virtual ~IFilesModel() {}

    virtual bool isFullyInitialized() const = 0;

    virtual QString currentFilePath() const = 0;
    virtual int currentFileIndex() const = 0;
    virtual int filesCount() const = 0;
    virtual bool canDeleteCurrentFile() const = 0;

    virtual bool selectByPath(const QString &path) = 0;
    virtual bool selectByIndex(int index) = 0;
    virtual void update() = 0;
};


class DirContentModel : public IFilesModel
{
public:
    DirContentModel(FilesScanner *filesScanner, const QStringList &supportedFormatsWithWildcards, const QString &filePath)
        : m_filesScanner(filesScanner)
        , m_currentIndex(INVALID_INDEX)
        , m_canDeleteCurrentFile(false)
        , m_isFullyInitialized(false)
    {
        assert(!filePath.isEmpty());
        const QFileInfo fileInfo = QFileInfo(filePath);
        if(fileInfo.isFile())
        {
            m_currentFilePath = filePath;
            m_directoryPath = fileInfo.absolutePath();
            filesScanner->configureForDirContent(supportedFormatsWithWildcards, m_directoryPath);
        }
        else
        {
            m_directoryPath = fileInfo.absoluteFilePath();
            if(filesScanner->configureForDirContent(supportedFormatsWithWildcards, m_directoryPath))
            {
                filesScanner->wait();
                DirContentModel::update();
            }
        }
    }

    bool isFullyInitialized() const Q_DECL_OVERRIDE     { return m_isFullyInitialized; }
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
        m_filesList.clear();
        m_currentIndex = INVALID_INDEX;
        m_canDeleteCurrentFile = false;
        if(m_directoryPath.isEmpty())
            return;
        if(!m_filesScanner || !m_filesScanner->hasScanResult())
            return;

        m_filesList = m_filesScanner->getScanResult();
        m_isFullyInitialized = true;
        if(m_filesList.isEmpty())
            return;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        if(m_filesList.size() >= std::numeric_limits<int>::max())
        {
            LOG_WARNING() << LOGGING_CTX << QString::fromLatin1("Files list overflow: current=%1, max=%2")
                    .arg(m_filesList.size())
                    .arg(std::numeric_limits<int>::max())
                    .toLocal8Bit().data();
            m_filesList.clear();
            return;
        }
#endif

        const QDir dir = QDir(m_directoryPath);
        if(m_currentFilePath.isEmpty())
        {
            m_currentIndex = 0;
            m_currentFilePath = dir.absoluteFilePath(m_filesList.first());
        }

        if(m_currentIndex == INVALID_INDEX)
        {
            const QFileInfo fileInfo = QFileInfo(m_currentFilePath);
            const QString fileName = fileInfo.fileName();
            for(int i = 0; i < m_filesList.size(); ++i)
            {
                if(m_filesList[i] != fileName)
                    continue;
                m_currentIndex = i;
                break;
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
        }
        m_canDeleteCurrentFile = canDeleteFile(m_currentFilePath);
    }

private:
    QPointer<FilesScanner> m_filesScanner;
    QStringList m_filesList;
    QString m_currentFilePath;
    int m_currentIndex;
    QString m_directoryPath;
    bool m_canDeleteCurrentFile;
    bool m_isFullyInitialized;
};


class FilxedListModel : public IFilesModel
{
public:
    explicit FilxedListModel(FilesScanner *filesScanner, const QStringList &supportedFormatsWithWildcards, const QStringList &filePaths)
        : m_filesScanner(filesScanner)
        , m_currentIndex(INVALID_INDEX)
        , m_canDeleteCurrentFile(false)
        , m_isFullyInitialized(false)
    {
        assert(!filePaths.isEmpty());
        if(QFileInfo(filePaths.first()).isFile())
        {
            m_currentFilePath = filePaths.first();
            filesScanner->configureForFilxedList(supportedFormatsWithWildcards, filePaths);
        }
        else
        {
            if(filesScanner->configureForFilxedList(supportedFormatsWithWildcards, filePaths))
            {
                filesScanner->wait();
                FilxedListModel::update();
            }
        }
    }

    bool isFullyInitialized() const Q_DECL_OVERRIDE     { return m_isFullyInitialized; }
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
        m_pathsList.clear();
        m_currentIndex = INVALID_INDEX;
        m_canDeleteCurrentFile = false;
        if(!m_filesScanner || !m_filesScanner->hasScanResult())
            return;

        m_pathsList = m_filesScanner->getScanResult();
        m_isFullyInitialized = true;
        if(m_pathsList.isEmpty())
            return;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        if(m_pathsList.size() >= std::numeric_limits<int>::max())
        {
            LOG_WARNING() << LOGGING_CTX << QString::fromLatin1("Files list overflow: current=%1, max=%2")
                    .arg(m_pathsList.size())
                    .arg(std::numeric_limits<int>::max())
                    .toLocal8Bit().data();
            m_pathsList.clear();
            return;
        }
#endif

        if(m_currentFilePath.isEmpty())
        {
            m_currentIndex = 0;
            m_currentFilePath = m_pathsList.first();
        }

        if(m_currentIndex == INVALID_INDEX)
        {
            for(int i = 0; i < m_pathsList.size(); ++i)
            {
                if(m_pathsList[i] != m_currentFilePath)
                    continue;
                m_currentIndex = i;
                break;
            }
            if(m_currentIndex == INVALID_INDEX)
            {
                const QString filePathC = m_currentFilePath.normalized(QString::NormalizationForm_C);
                for(int i = 0; i < m_pathsList.size(); ++i)
                {
                    if(m_pathsList[i].normalized(QString::NormalizationForm_C) != filePathC)
                        continue;
                    m_currentIndex = i;
                    m_currentFilePath = m_pathsList[i];
                    break;
                }
            }
        }
        m_canDeleteCurrentFile = canDeleteFile(m_currentFilePath);
    }

private:
    QPointer<FilesScanner> m_filesScanner;
    QStringList m_pathsList;
    QString m_currentFilePath;
    int m_currentIndex;
    bool m_canDeleteCurrentFile;
    bool m_isFullyInitialized;
};

} // namespace

// ====================================================================================================

struct FileManager::Impl
{
    class ChangedGuard;

    FileManager *fileManager;
    QStringList supportedFormats;
    FilesScanner filesScanner;
    QScopedPointer<IFilesModel> filesModel;
    QStringList currentOpenArguments;

    explicit Impl(FileManager *fileManager)
        : fileManager(fileManager)
        , supportedFormats(QString::fromLatin1("*.*"))
    {
        QObject::connect(&filesScanner, SIGNAL(updated()), fileManager, SLOT(update()), Qt::QueuedConnection);
    }

    void resetFilesModel(IFilesModel *newFilesModel = Q_NULLPTR)
    {
        if(!newFilesModel)
            filesScanner.reset();
        currentOpenArguments.clear();
        filesModel.reset(newFilesModel);
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

bool FileManager::isReady() const
{
    if(m_impl->filesModel)
        return m_impl->filesModel->isFullyInitialized();
    return true;
}

void FileManager::waitForReady()
{
    if(!isReady())
    {
        m_impl->filesScanner.wait();
        update();
    }
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

void FileManager::ensureUpdated()
{
    /// @todo What should we do here?
}

bool FileManager::openPath(const QString &filePath)
{
    const QFileInfo fileInfo(filePath);
    if(!fileInfo.exists())
        return false;

    const Impl::ChangedGuard changedGuard(this);
    const QSignalBlocker recursiveChangedBlocker(this);
    m_impl->resetFilesModel(new DirContentModel(&m_impl->filesScanner, m_impl->supportedFormats, fileInfo.absoluteFilePath()));
    if(m_impl->filesModel->currentFilePath().isEmpty())
    {
        m_impl->resetFilesModel();
        return false;
    }

    m_impl->currentOpenArguments = QStringList(filePath);
    return true;
}

bool FileManager::openPaths(const QStringList &filePaths)
{
    QStringList pathsList;
    for(QStringList::ConstIterator it = filePaths.constBegin(), itEnd = filePaths.constEnd(); it != itEnd; ++it)
    {
        const QFileInfo fileInfo(*it);
        if(fileInfo.exists())
            pathsList.append(fileInfo.absoluteFilePath());
    }

    if(pathsList.isEmpty())
        return false;

    if(filePaths.size() == 1)
        return openPath(filePaths.first());

    const Impl::ChangedGuard changedGuard(this);
    const QSignalBlocker recursiveChangedBlocker(this);
    m_impl->resetFilesModel(new FilxedListModel(&m_impl->filesScanner, m_impl->supportedFormats, pathsList));
    if(m_impl->filesModel->currentFilePath().isEmpty())
    {
        m_impl->resetFilesModel();
        return false;
    }

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
