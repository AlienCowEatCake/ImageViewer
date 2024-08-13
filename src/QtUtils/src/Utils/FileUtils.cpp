/*
   Copyright (C) 2017-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

   This file is part of the `QtUtils' library.

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

/// @note qDebug macro conflicts
#if defined (__APPLE__)
#include <CoreServices/CoreServices.h>
#endif

#include "FileUtils.h"

#include <cstring>
#include <string>
#include <algorithm>
#include <cstdlib>

#include <QApplication>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QUrl>
#include <QProcess>
#include <QStringList>

#include "Global.h"
#include "Logging.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))

namespace MoveToTrashInternal {

static bool MoveToTrashImpl(const QString &absolutePath, QString * /*errorDescription*/)
{
    return QFile::moveToTrash(absolutePath);
}

} // namespace MoveToTrashInternal

#elif defined (Q_OS_MAC)

#include <CoreServices/CoreServices.h>

namespace MoveToTrashInternal {

static bool MoveToTrashViaFinderScript(const QString &absolutePath, QString *errorDescription)
{
    int status = QProcess::execute(QString::fromLatin1("osascript"), QStringList()
            << QString::fromLatin1("-e")
            << QString::fromLatin1("tell application \"Finder\"\n"
                                   "move POSIX file \"%1\" to trash\n"
                                   "end tell").arg(absolutePath));
    switch(status)
    {
    case -2:
        LOG_WARNING() << LOGGING_CTX << "osascript process cannot be started";
        break;
    case -1:
        LOG_WARNING() << LOGGING_CTX << "osascript process crashed";
        break;
    case 0:
        return true;
    default:
        LOG_WARNING() << LOGGING_CTX << "The specified path could not be moved to Trash" << absolutePath << QString::fromLatin1(" (%1)").arg(status);
        break;
    }

    if(errorDescription)
        *errorDescription = qApp->translate("FileUtils", "The specified path could not be moved to Trash").append(QString::fromLatin1(" (%1)").arg(status));

    return false;
}

static bool MoveToTrashViaFSMoveObject(const QString &absolutePath, QString *errorDescription)
{
    // http://programtalk.com/vs2/?source=python/5435/send2trash/send2trash/plat_osx.py

    typedef OSStatus (*FSPathMakeRefWithOptions_t)(const UInt8 *path, OptionBits options, FSRef *ref, Boolean *isDirectory);
    typedef OSStatus (*FSMoveObjectToTrashSync_t)(const FSRef *source, FSRef *target, OptionBits options);
    typedef const char *(*GetMacOSStatusCommentString_t)(OSStatus err);
    CFBundleRef coreServicesBundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.CoreServices"));
    if(!coreServicesBundle)
    {
        if(errorDescription)
            *errorDescription = qApp->translate("FileUtils", "The bundle with identifier `com.apple.CoreServices` was not found.");
        LOG_WARNING() << LOGGING_CTX << "The bundle with identifier `com.apple.CoreServices` was not found.";
        return false;
    }

    const FSPathMakeRefWithOptions_t FSPathMakeRefWithOptions_f = reinterpret_cast<FSPathMakeRefWithOptions_t>(CFBundleGetFunctionPointerForName(coreServicesBundle, CFSTR("FSPathMakeRefWithOptions")));
    const FSMoveObjectToTrashSync_t FSMoveObjectToTrashSync_f = reinterpret_cast<FSMoveObjectToTrashSync_t>(CFBundleGetFunctionPointerForName(coreServicesBundle, CFSTR("FSMoveObjectToTrashSync")));
    const GetMacOSStatusCommentString_t GetMacOSStatusCommentString_f = reinterpret_cast<GetMacOSStatusCommentString_t>(CFBundleGetFunctionPointerForName(coreServicesBundle, CFSTR("GetMacOSStatusCommentString")));
    if(!FSPathMakeRefWithOptions_f)
    {
        if(errorDescription)
            *errorDescription = qApp->translate("FileUtils", "The function with name `FSPathMakeRefWithOptions` was not found.");
        LOG_WARNING() << LOGGING_CTX << "The function with name `FSPathMakeRefWithOptions` was not found.";
        return false;
    }
    if(!FSMoveObjectToTrashSync_f)
    {
        if(errorDescription)
            *errorDescription = qApp->translate("FileUtils", "The function with name `FSMoveObjectToTrashSync` was not found.");
        LOG_WARNING() << LOGGING_CTX << "The function with name `FSMoveObjectToTrashSync` was not found.";
        return false;
    }
    if(!GetMacOSStatusCommentString_f)
    {
        if(errorDescription)
            *errorDescription = qApp->translate("FileUtils", "The function with name `GetMacOSStatusCommentString` was not found.");
        LOG_WARNING() << LOGGING_CTX << "The function with name `GetMacOSStatusCommentString` was not found.";
        return false;
    }

    FSRef ref;
    memset(&ref, 0, sizeof(ref));
    const QByteArray utf8Path = absolutePath.toUtf8();
    const UInt8 *utf8PathData = reinterpret_cast<const UInt8*>(utf8Path.data());
    OSStatus status = FSPathMakeRefWithOptions_f(utf8PathData, kFSPathMakeRefDoNotFollowLeafSymlink, &ref, Q_NULLPTR);
    if(status)
    {
        const QString description = QString::fromUtf8(GetMacOSStatusCommentString_f(status));
        if(errorDescription)
            *errorDescription = description;
        LOG_WARNING() << LOGGING_CTX << "Unable to FSPathMakeRefWithOptions for file" << absolutePath;
        LOG_WARNING() << LOGGING_CTX << "Status Comment:" << description;
        return false;
    }
    status = FSMoveObjectToTrashSync_f(&ref, Q_NULLPTR, kFSFileOperationDefaultOptions);
    if(status)
    {
        const QString description = QString::fromUtf8(GetMacOSStatusCommentString_f(status));
        if(errorDescription)
            *errorDescription = description;
        LOG_WARNING() << LOGGING_CTX << "Unable to FSMoveObjectToTrashSync for file" << absolutePath;
        LOG_WARNING() << LOGGING_CTX << "Status Comment:" << description;
        return false;
    }
    return true;
}

static bool MoveToTrashImpl(const QString &absolutePath, QString *errorDescription)
{
    if(MoveToTrashViaFinderScript(absolutePath, errorDescription))
        return true;
    if(MoveToTrashViaFSMoveObject(absolutePath, errorDescription))
        return true;
    return false;
}

} // namespace MoveToTrashInternal

#elif defined (Q_OS_WIN)

#include <windows.h>
#include <shellapi.h>

namespace MoveToTrashInternal {

static bool MoveToTrashImpl(const QString &absolutePath, QString *errorDescription)
{
    // http://programtalk.com/vs2/?source=python/5435/send2trash/send2trash/plat_win.py

    HMODULE hShell32 = LoadLibraryA("shell32.dll");
    typedef int(WINAPI *SHFileOperationW_t)(LPSHFILEOPSTRUCTW);
    SHFileOperationW_t SHFileOperationW_f = reinterpret_cast<SHFileOperationW_t>(GetProcAddress(hShell32, "SHFileOperationW"));

    HMODULE hKernel32 = LoadLibraryA("kernel32.dll");
    typedef DWORD(WINAPI *FormatMessageW_t)(DWORD, LPCVOID, DWORD, DWORD, LPWSTR, DWORD, va_list);
    FormatMessageW_t FormatMessageW_f = reinterpret_cast<FormatMessageW_t>(GetProcAddress(hKernel32, "FormatMessageW"));

    if(SHFileOperationW_f && FormatMessageW_f)
    {
        std::wstring wstringPath = absolutePath.toStdWString();
        std::replace(wstringPath.begin(), wstringPath.end(), L'/', L'\\');
        wstringPath.push_back(L'\0');
        SHFILEOPSTRUCTW fileop;
        memset(&fileop, 0, sizeof(fileop));
        fileop.hwnd = 0;
        fileop.wFunc = FO_DELETE;
        fileop.pFrom = wstringPath.c_str();
        fileop.pTo = Q_NULLPTR;
        fileop.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
        fileop.fAnyOperationsAborted = 0;
        fileop.hNameMappings = 0;
        fileop.lpszProgressTitle = Q_NULLPTR;
        DWORD status = static_cast<DWORD>(SHFileOperationW_f(&fileop));
        if(status)
        {
            WCHAR * errorRawStr = Q_NULLPTR;
            FormatMessageW_f(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                           Q_NULLPTR, status, 0, reinterpret_cast<LPWSTR>(&errorRawStr), 0, Q_NULLPTR);
            const QString description = QString::fromStdWString(std::wstring(errorRawStr));
            LocalFree(errorRawStr);
            if(errorDescription)
                *errorDescription = description;
            LOG_WARNING() << LOGGING_CTX << "Unable to SHFileOperationW for file" << absolutePath;
            LOG_WARNING() << LOGGING_CTX << "Description:" << description;
            FreeLibrary(hShell32);
            FreeLibrary(hKernel32);
            return false;
        }
        FreeLibrary(hShell32);
        FreeLibrary(hKernel32);
        return true;
    }
    else
    {
        FreeLibrary(hShell32);
        FreeLibrary(hKernel32);
        std::string stringPath = absolutePath.toStdString();
        std::replace(stringPath.begin(), stringPath.end(), '/', '\\');
        stringPath.push_back('\0');
        SHFILEOPSTRUCTA fileop;
        memset(&fileop, 0, sizeof(fileop));
        fileop.hwnd = 0;
        fileop.wFunc = FO_DELETE;
        fileop.pFrom = stringPath.c_str();
        fileop.pTo = Q_NULLPTR;
        fileop.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
        fileop.fAnyOperationsAborted = 0;
        fileop.hNameMappings = 0;
        fileop.lpszProgressTitle = Q_NULLPTR;
        DWORD status = static_cast<DWORD>(SHFileOperationA(&fileop));
        if(status)
        {
            char * errorRawStr = Q_NULLPTR;
            FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                           Q_NULLPTR, status, 0, reinterpret_cast<LPSTR>(&errorRawStr), 0, Q_NULLPTR);
            const QString description = QString::fromLocal8Bit(errorRawStr);
            LocalFree(errorRawStr);
            if(errorDescription)
                *errorDescription = description;
            LOG_WARNING() << LOGGING_CTX << "Unable to SHFileOperationA for file" << absolutePath;
            LOG_WARNING() << LOGGING_CTX << "Description:" << description;
            return false;
        }
        return true;
    }
}

} // namespace MoveToTrashInternal

#elif defined (Q_OS_HAIKU)

#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <FindDirectory.h>
#include <Path.h>
#include <String.h>

namespace MoveToTrashInternal {

static bool MoveToTrashImpl(const QString &absolutePath, QString *errorDescription)
{
    // https://github.com/haiku/haiku/blob/r1beta1/src/apps/mediaplayer/playlist/FilePlaylistItem.cpp#L485

    const QByteArray absolutePathData = absolutePath.toLocal8Bit();

    BEntry entry(absolutePathData.data(), false);
    status_t err = entry.InitCheck();
    if(err != B_OK)
    {
        LOG_WARNING() << LOGGING_CTX << "Failed to init BEntry for specified path:" << strerror(err);
        if(errorDescription)
            *errorDescription = qApp->translate("FileUtils", "Failed to init BEntry for specified path: %1").arg(QString::fromLocal8Bit(strerror(err)));
        return false;
    }

    entry_ref entryRef;
    err = entry.GetRef(&entryRef);
    if(err != B_OK)
    {
        LOG_WARNING() << LOGGING_CTX << "Failed to get entry_ref for specified path:" << strerror(err);
        if(errorDescription)
            *errorDescription = qApp->translate("FileUtils", "Failed to get entry_ref for specified path: %1").arg(QString::fromLocal8Bit(strerror(err)));
        return false;
    }

    char trashPath[B_PATH_NAME_LENGTH];
    err = find_directory(B_TRASH_DIRECTORY, entryRef.device, true, trashPath, B_PATH_NAME_LENGTH);
    if(err != B_OK)
    {
        LOG_WARNING() << LOGGING_CTX << "Failed to find Trash" << strerror(err);
        if(errorDescription)
            *errorDescription = qApp->translate("FileUtils", "Failed to find Trash: %1").arg(QString::fromLocal8Bit(strerror(err)));
        return false;
    }

    BDirectory trashDir(trashPath);
    err = trashDir.InitCheck();
    if(err != B_OK)
    {
        LOG_WARNING() << LOGGING_CTX << "Failed to init BDirectory for" << trashPath << ":" << strerror(err);
        if(errorDescription)
            *errorDescription = qApp->translate("FileUtils", "Failed to init BDirectory for %1: %2").arg(QString::fromLocal8Bit(trashPath), QString::fromLocal8Bit(strerror(err)));
        return false;
    }

    // Find a unique name for the entry in the trash
    BString nameInTrash = entryRef.name;
    for(int32 uniqueNameIndex = 1; true; uniqueNameIndex++)
    {
        BEntry test(&trashDir, nameInTrash.String());
        if(!test.Exists())
            break;
        nameInTrash = entryRef.name;
        nameInTrash << ' ' << uniqueNameIndex;
    }

    // Remember the original path
    BPath originalPath;
    entry.GetPath(&originalPath);

    // Finally, move the entry into the trash
    err = entry.MoveTo(&trashDir, nameInTrash.String());
    if(err != B_OK)
    {
        LOG_WARNING() << LOGGING_CTX << "Failed to move entry into trash" << trashPath << ":" << strerror(err);
        if(errorDescription)
            *errorDescription = qApp->translate("FileUtils", "Failed to move entry into trash %1: %2").arg(QString::fromLocal8Bit(trashPath), QString::fromLocal8Bit(strerror(err)));
        return false;
    }

    // Allow Tracker to restore this entry
    BNode node(&entry);
    BString originalPathString(originalPath.Path());
    node.WriteAttrString("_trk/original_path", &originalPathString);

    return true;
}

} // namespace MoveToTrashInternal

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

// http://programtalk.com/vs2/?source=python/5435/send2trash/send2trash/plat_other.py
namespace MoveToTrashInternal {

static QString getEnvironment(const QString &name, const QString &defaultValue = QString())
{
    const char *value = getenv(name.toLocal8Bit().data());
    if(value)
        return QString::fromLocal8Bit(value);
    return defaultValue;
}

// freedesktop.org trash specification:
//   [1] http://www.freedesktop.org/wiki/Specifications/trash-spec
//   [2] http://www.ramendik.ru/docs/trashspec.html
// See also:
//   [3] http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html

static const QString &getFilesDir()
{
    static const QString FILES_DIR = QString::fromLatin1("files");
    return FILES_DIR;
}

static const QString &getInfoDir()
{
    static const QString INFO_DIR = QString::fromLatin1("info");
    return INFO_DIR;
}

static const QString &getInfoSuffix()
{
    static const QString INFO_SUFFIX = QString::fromLatin1(".trashinfo");
    return INFO_SUFFIX;
}

// Default of ~/.local/share [3]
static const QString &getXdgDataHome()
{
    static const QString XDG_DATA_HOME = QFileInfo(getEnvironment(QString::fromLatin1("XDG_DATA_HOME"), QString::fromLatin1("%1/.local/share").arg(QDir::homePath()))).absoluteFilePath();
    return XDG_DATA_HOME;
}

static const QString &getHomeTrash()
{
    static const QString HOMETRASH = QDir(getXdgDataHome()).absoluteFilePath(QString::fromLatin1("Trash"));
    return HOMETRASH;
}

static const QString &getUid()
{
    static const QString uid = QString::number(getuid());
    return uid;
}

static const QString &getTopDirTrash()
{
    static const QString TOPDIR_TRASH = QString::fromLatin1(".Trash");
    return TOPDIR_TRASH;
}

static const QString &getTopDirFallback()
{
    static const QString TOPDIR_FALLBACK = QString::fromLatin1(".Trash-") + getUid();
    return TOPDIR_FALLBACK;
}

static bool isParent(const QString &parent, const QString &path)
{
    const QString canonicalPath = QFileInfo(path).canonicalFilePath(); // In case it's a symlink
    const QString canonicalParent = QFileInfo(parent).canonicalFilePath();
    return canonicalPath.startsWith(canonicalParent);
}

static QString formatDate(const QDateTime &date)
{
    return date.toString(QString::fromLatin1("yyyy-MM-ddTHH:mm:ss"));
}

static QString infoFor(const QString &src, const QString &topDir)
{
    // ...it MUST not include a ".." directory, and for files not "under" that
    // directory, absolute pathnames must be used. [2]
    QString path;
    if(topDir.isEmpty() || !isParent(topDir, src))
        path = QFileInfo(src).absoluteFilePath();
    else
        path = QDir(topDir).relativeFilePath(src);
    QString info;
    info.append(QString::fromLatin1("[Trash Info]\n"));
    info.append(QString::fromLatin1("Path=%1\n").arg(QString::fromLatin1(QUrl::toPercentEncoding(path))));
    info.append(QString::fromLatin1("DeletionDate=%1\n").arg(formatDate(QDateTime::currentDateTime())));
    return info;
}

static bool checkCreate(const QString &dir)
{
    const QByteArray name = QFileInfo(dir).absoluteFilePath().toLocal8Bit();
    struct stat st;
    memset(&st, 0, sizeof(st));
    // use 0700 for paths [3]
    if(stat(name.data(), &st) != 0)
        return mkdir(name.data(), 0700) == 0;
    return true;
}

static void pathSplit(const QString& path, QString &root, QString &ext)
{
    // Split the pathname path into a pair (root, ext) such that root + ext == path, and ext is empty
    // or begins with a period and contains at most one period. Leading periods on the basename are
    // ignored; splitext('.cshrc') returns ('.cshrc', '').
    const QString fileName = QFileInfo(path).fileName();
    const int dotLastIndex = fileName.lastIndexOf(QChar::fromLatin1('.'));
    ext = fileName.right(fileName.length() - dotLastIndex);
    root = path;
    if(ext == fileName)
        ext.clear();
    else
        root.remove(path.length() - fileName.length() + dotLastIndex, fileName.length());
}

static bool trashMove(const QString &src, const QString &dst, const QString &topDir = QString())
{
    const QString fileName = QFileInfo(src).fileName();
    const QString filesPath = QDir(dst).absoluteFilePath(getFilesDir());
    const QString infoPath = QDir(dst).absoluteFilePath(getInfoDir());
    QString baseName, ext;
    pathSplit(fileName, baseName, ext);

    int counter = 0;
    QString destName = fileName;
    while(QFileInfo_exists(QDir(filesPath).absoluteFilePath(destName)) || QFileInfo_exists(QDir(filesPath).absoluteFilePath(destName + getInfoSuffix())))
    {
        counter++;
        destName = QString::fromLatin1("%1 %2%3").arg(baseName).arg(counter).arg(ext);
    }

    if(!checkCreate(filesPath))
        return false;
    if(!checkCreate(infoPath))
        return false;

    if(!QDir().rename(QFileInfo(src).absoluteFilePath(), QDir(filesPath).absoluteFilePath(destName)))
        return false;
    QFile file(QDir(infoPath).absoluteFilePath(destName + getInfoSuffix()));
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QDir().rename(QDir(filesPath).absoluteFilePath(destName), QFileInfo(src).absoluteFilePath());
        return false;
    }
    QTextStream out(&file);
    out << infoFor(src, topDir);
    file.close();
    return true;
}

static QString findMountPoint(const QString &path)
{
    // Even if something's wrong, "/" is a mount point, so the loop will exit.
    // Use realpath in case it's a symlink
    const QFileInfo info = path;
    QDir currentDir = info.isDir() ? info.canonicalFilePath() : info.canonicalPath(); // Required to avoid infinite loop
    QString currentPath = currentDir.canonicalPath();
    while(!currentDir.isRoot())
    {
        struct stat dirStat;
        memset(&dirStat, 0, sizeof(dirStat));
        if(stat(currentPath.toLocal8Bit().data(), &dirStat) != 0)
            return currentPath;

        if(!currentDir.cdUp())
            return currentPath;

        struct stat parentStat;
        memset(&parentStat, 0, sizeof(parentStat));
        if(stat(currentDir.canonicalPath().toLocal8Bit().data(), &parentStat) != 0)
            return currentPath;

        if(dirStat.st_dev != parentStat.st_dev || dirStat.st_ino == parentStat.st_ino)
            return currentPath;

        currentPath = currentDir.canonicalPath();
    }
    return currentPath;
}

static QString findExtVolumeGlobalTrash(const QString &volumeRoot)
{
    // from [2] Trash directories (1) check for a .Trash dir with the right
    // permissions set.
    QString trashDir = QDir(volumeRoot).absoluteFilePath(getTopDirTrash());
    const QFileInfo trashInfo(trashDir);
    if(!trashInfo.exists() || !trashInfo.isDir() || trashInfo.isSymLink())
        return QString();

    // vol/.Trash must be a directory, cannot be a symlink, and must have the
    // sticky bit set.
    struct stat trashStat;
    memset(&trashStat, 0, sizeof(trashStat));
    if(lstat(trashDir.toLocal8Bit().data(), &trashStat) != 0 || !(trashStat.st_mode & S_ISVTX))
        return QString();

    trashDir = QDir(trashDir).absoluteFilePath(getUid());
    if(!checkCreate(trashDir))
        return QString();
    return trashDir;
}

static QString findExtVolumeFallbackTrash(const QString &volumeRoot)
{
    // from [2] Trash directories (1) create a .Trash-$uid dir.
    const QString trashDir = QDir(volumeRoot).absoluteFilePath(getTopDirFallback());
    // Try to make the directory, if we can't the OSError exception will escape
    // be thrown out of send2trash.
    checkCreate(trashDir);
    return trashDir;
}

static QString findExtVolumeTrash(const QString &volumeRoot)
{
    QString trashDir = findExtVolumeGlobalTrash(volumeRoot);
    if(trashDir.isEmpty())
        trashDir = findExtVolumeFallbackTrash(volumeRoot);
    return trashDir;
}

// Pull this out so it's easy to stub (to avoid stubbing lstat itself)
static dev_t getDev(const QString &path)
{
    struct stat pathStat;
    memset(&pathStat, 0, sizeof(pathStat));
    if(lstat(path.toLocal8Bit().data(), &pathStat) != 0)
        return static_cast<dev_t>(0);
    return pathStat.st_dev;
}

static bool sendToTrash(const QString &path, QString *errorDescription)
{
    const QFileInfo pathInfo(path);
    if(!pathInfo.exists())
    {
        if(errorDescription)
            *errorDescription = qApp->translate("FileUtils", "The specified path was not found");
        LOG_WARNING() << LOGGING_CTX << "The specified path was not found:" << pathInfo.absoluteFilePath();
        return false;
    }

    // ...should check whether the user has the necessary permissions to delete
    // it, before starting the trashing operation itself. [2]
    struct stat pathStat;
    memset(&pathStat, 0, sizeof(pathStat));
    if(!(lstat(pathInfo.absoluteFilePath().toLocal8Bit().data(), &pathStat) == 0 &&
         pathInfo.isReadable() && QFileInfo(pathInfo.absolutePath()).isWritable() &&
         !((pathStat.st_mode & S_ISVTX) && (pathStat.st_uid != geteuid()))))
    {
        if(errorDescription)
            *errorDescription = qApp->translate("FileUtils", "Permission denied");
        LOG_WARNING() << LOGGING_CTX << "Permission denied:" << pathInfo.absoluteFilePath();
        return false;
    }

    // if the file to be trashed is on the same device as HOMETRASH we
    // want to move it there.
    dev_t pathDev = getDev(path);

    // If XDG_DATA_HOME or HOMETRASH do not yet exist we need to stat the
    // home directory, and these paths will be created further on if needed.
    dev_t trashDev = getDev(QDir::homePath());

    QString topDir, destTrash;
    if(pathDev == trashDev)
    {
        topDir = getXdgDataHome();
        destTrash = getHomeTrash();
    }
    else
    {
        topDir = findMountPoint(path);
        trashDev = getDev(topDir);
        if(trashDev != pathDev)
        {
            if(errorDescription)
                *errorDescription = qApp->translate("FileUtils", "Could not find mount point for specified path");
            LOG_WARNING() << LOGGING_CTX << "Could not find mount point for specified path:" << pathInfo.absoluteFilePath();
            return false;
        }
        destTrash = findExtVolumeTrash(topDir);
    }
    if(!trashMove(path, destTrash, topDir))
    {
        if(errorDescription)
            *errorDescription = qApp->translate("FileUtils", "The specified path could not be moved to Trash");
        LOG_WARNING() << LOGGING_CTX << "The specified path could not be moved to Trash:" << pathInfo.absoluteFilePath();
        return false;
    }
    return true;
}

static bool MoveToTrashImpl(const QString &absolutePath, QString *errorDescription)
{
    int status = QProcess::execute(QString::fromLatin1("gvfs-trash"), QStringList() << absolutePath);
    switch(status)
    {
    case -2:
        LOG_INFO() << LOGGING_CTX << "gvfs-trash process cannot be started";
        break;
    case -1:
        LOG_INFO() << LOGGING_CTX << "gvfs-trash process crashed";
        break;
    case 0:
        return true;
    default:
        if(errorDescription)
            *errorDescription = qApp->translate("FileUtils", "The specified path could not be moved to Trash");
        LOG_WARNING() << LOGGING_CTX << "The specified path could not be moved to Trash" << absolutePath;
        return false;
    }

    // http://programtalk.com/vs2/?source=python/5435/send2trash/send2trash/plat_other.py

    return sendToTrash(absolutePath, errorDescription);
}

} // namespace MoveToTrashInternal

#endif

namespace FileUtils {

/// @brief Удаление указанного файла или директории в корзину
/// @attention Используется удаление без запроса. В случае отсутствия на целевой системе
///         корзины, либо если корзина программно отключена поведение этой функции строго
///         не специфицируется. Предполагаемое поведение - либо будет произведен выход с
///         false, либо произойдет удаление файла мимо корзины (может зависеть от текущей
///         платформы). Текстовое описание ошибки при этом устанавливаться не обязано.
/// @param[in] path - путь к файлу или директории
/// @param[out] errorDescription - текстовое описание ошибки в случае ее возникновения
/// @return - true в случае успешного удаления, false в случае ошибки
bool MoveToTrash(const QString &path, QString *errorDescription)
{
    const QFileInfo info(path);
    if(!info.exists())
    {
        if(errorDescription)
            *errorDescription = qApp->translate("FileUtils", "The specified path does not exist");
        return false;
    }
    const QString absolutePath = info.absoluteFilePath();
    return MoveToTrashInternal::MoveToTrashImpl(absolutePath, errorDescription);
}

} // namespace FileUtils

