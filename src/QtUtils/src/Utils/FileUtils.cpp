/*
   Copyright (C) 2011-2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "FileUtils.h"

#include <cstring>
#include <string>
#include <algorithm>

#include <QtGlobal>
#if defined (Q_OS_MAC)
#include <CoreServices/CoreServices.h>
#elif defined (Q_OS_WIN)
#include <windows.h>
#include <shellapi.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#include <QApplication>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QProcessEnvironment>
#include <QUrl>
#include <QDebug>

#if !defined (Q_OS_MAC) && !defined (Q_OS_WIN)
// http://programtalk.com/vs2/?source=python/5435/send2trash/send2trash/plat_other.py
namespace MoveToTrashInternal {

//# Copyright 2013 Hardcoded Software (http://www.hardcoded.net)
//
//# This software is licensed under the "BSD" License as described in the "LICENSE" file,
//# which should be included with this package. The terms are also available at
//# http://www.hardcoded.net/licenses/bsd_license
//
//# This is a reimplementation of plat_other.py with reference to the
//# freedesktop.org trash specification:
//#   [1] http://www.freedesktop.org/wiki/Specifications/trash-spec
//#   [2] http://www.ramendik.ru/docs/trashspec.html
//# See also:
//#   [3] http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html
//#
//# For external volumes this implementation will raise an exception if it can't
//# find or create the user's trash directory.

//FILES_DIR = 'files'
static const QString &getFilesDir()
{
    static const QString FILES_DIR = QString::fromLatin1("files");
    return FILES_DIR;
}

//INFO_DIR = 'info'
static const QString &getInfoDir()
{
    static const QString INFO_DIR = QString::fromLatin1("info");
    return INFO_DIR;
}

//INFO_SUFFIX = '.trashinfo'
static const QString &getInfoSuffix()
{
    static const QString INFO_SUFFIX = QString::fromLatin1(".trashinfo");
    return INFO_SUFFIX;
}

//# Default of ~/.local/share [3]
//XDG_DATA_HOME = op.expanduser(os.environ.get('XDG_DATA_HOME', '~/.local/share'))
static const QString &getXdgDataHome()
{
    static const QString XDG_DATA_HOME = QFileInfo(QProcessEnvironment().value(QString::fromLatin1("XDG_DATA_HOME"), QString::fromLatin1("%1/.local/share").arg(QDir::homePath()))).absoluteFilePath();
    return XDG_DATA_HOME;
}

//HOMETRASH = op.join(XDG_DATA_HOME, 'Trash')
static const QString &getHomeTrash()
{
    static const QString HOMETRASH = QDir(getXdgDataHome()).absoluteFilePath(QString::fromLatin1("Trash"));
    return HOMETRASH;
}

//uid = os.getuid()
static const QString &getUid()
{
    static const QString uid = QString::number(getuid());
    return uid;
}

//TOPDIR_TRASH = '.Trash'
static const QString &getTopDirTrash()
{
    static const QString TOPDIR_TRASH = QString::fromLatin1(".Trash");
    return TOPDIR_TRASH;
}

//TOPDIR_FALLBACK = '.Trash-' + str(uid)
static const QString &getTopDirFallback()
{
    static const QString TOPDIR_FALLBACK = QString::fromLatin1(".Trash-") + getUid();
    return TOPDIR_FALLBACK;
}

//def is_parent(parent, path):
//    path = op.realpath(path) # In case it's a symlink
//    parent = op.realpath(parent)
//    return path.startswith(parent)
static bool isParent(const QString &parent, const QString &path)
{
    const QString canonicalPath = QFileInfo(path).canonicalFilePath();
    const QString canonicalParent = QFileInfo(parent).canonicalFilePath();
    return canonicalPath.startsWith(canonicalParent);
}

//def format_date(date):
//    return date.strftime("%Y-%m-%dT%H:%M:%S")
static QString formatDate(const QDateTime &date)
{
    return date.toString(QString::fromLatin1("yyyy-MM-ddTHH:mm:ss"));
}

//def info_for(src, topdir):
//    # ...it MUST not include a ".." directory, and for files not "under" that
//    # directory, absolute pathnames must be used. [2]
//    if topdir is None or not is_parent(topdir, src):
//        src = op.abspath(src)
//    else:
//        src = op.relpath(src, topdir)
//
//    info  = "[Trash Info]\n"
//    info += "Path=" + quote(src) + "\n"
//    info += "DeletionDate=" + format_date(datetime.now()) + "\n"
//    return info
static QString infoFor(const QString &src, const QString &topDir)
{
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

//def check_create(dir):
//    # use 0700 for paths [3]
//    if not op.exists(dir):
//        os.makedirs(dir, 0o700)
static bool checkCreate(const QString &dir)
{
    const QByteArray name = QFileInfo(dir).absoluteFilePath().toLocal8Bit();
    struct stat st;
    memset(&st, 0, sizeof(st));
    if(stat(name.data(), &st) != 0)
        return mkdir(name.data(), 0700) == 0;
    return true;
}

//os.path.splitext(path)
//   Split the pathname path into a pair (root, ext) such that root + ext == path, and ext is empty
//   or begins with a period and contains at most one period. Leading periods on the basename are
//   ignored; splitext('.cshrc') returns ('.cshrc', '').
static void pathSplit(const QString& path, QString &root, QString &ext)
{
    const QString fileName = QFileInfo(path).fileName();
    const int dotLastIndex = fileName.lastIndexOf(QChar::fromLatin1('.'));
    ext = fileName.right(fileName.length() - dotLastIndex);
    root = path;
    if(ext == fileName)
        ext.clear();
    else
        root.remove(path.length() - fileName.length() + dotLastIndex, fileName.length());
}

//def trash_move(src, dst, topdir=None):
//    filename = op.basename(src)
//    filespath = op.join(dst, FILES_DIR)
//    infopath = op.join(dst, INFO_DIR)
//    base_name, ext = op.splitext(filename)
//
//    counter = 0
//    destname = filename
//    while op.exists(op.join(filespath, destname)) or op.exists(op.join(infopath, destname + INFO_SUFFIX)):
//        counter += 1
//        destname = '%s %s%s' % (base_name, counter, ext)
//
//    check_create(filespath)
//    check_create(infopath)
//
//    os.rename(src, op.join(filespath, destname))
//    f = open(op.join(infopath, destname + INFO_SUFFIX), 'w')
//    f.write(info_for(src, topdir))
//    f.close()
static bool trashMove(const QString &src, const QString &dst, const QString &topDir = QString())
{
    const QString fileName = QFileInfo(src).fileName();
    const QString filesPath = QDir(dst).absoluteFilePath(getFilesDir());
    const QString infoPath = QDir(dst).absoluteFilePath(getInfoDir());
    QString baseName, ext;
    pathSplit(fileName, baseName, ext);

    int counter = 0;
    QString destName = fileName;
    while(QFileInfo(QDir(filesPath).absoluteFilePath(destName)).exists() || QFileInfo(QDir(filesPath).absoluteFilePath(destName + getInfoSuffix())).exists())
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

//def find_mount_point(path):
//    # Even if something's wrong, "/" is a mount point, so the loop will exit.
//    # Use realpath in case it's a symlink
//    path = op.realpath(path) # Required to avoid infinite loop
//    while not op.ismount(path):
//        path = op.split(path)[0]
//    return path
static QString findMountPoint(const QString &path)
{
    const QFileInfo info = path;
    QDir currentDir = info.isDir() ? info.canonicalFilePath() : info.canonicalPath();
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

//def find_ext_volume_global_trash(volume_root):
//    # from [2] Trash directories (1) check for a .Trash dir with the right
//    # permissions set.
//    trash_dir = op.join(volume_root, TOPDIR_TRASH)
//    if not op.exists(trash_dir):
//        return None
//
//    mode = os.lstat(trash_dir).st_mode
//    # vol/.Trash must be a directory, cannot be a symlink, and must have the
//    # sticky bit set.
//    if not op.isdir(trash_dir) or op.islink(trash_dir) or not (mode & stat.S_ISVTX):
//        return None
//
//    trash_dir = op.join(trash_dir, str(uid))
//    try:
//        check_create(trash_dir)
//    except OSError:
//        return None
//    return trash_dir
static QString findExtVolumeGlobalTrash(const QString &volumeRoot)
{
    QString trashDir = QDir(volumeRoot).absoluteFilePath(getTopDirTrash());
    const QFileInfo trashInfo(trashDir);
    if(!trashInfo.exists() || !trashInfo.isDir() || trashInfo.isSymLink())
        return QString();

    struct stat trashStat;
    memset(&trashStat, 0, sizeof(trashStat));
    if(lstat(trashDir.toLocal8Bit().data(), &trashStat) != 0 || !(trashStat.st_mode & S_ISVTX))
        return QString();

    trashDir = QDir(trashDir).absoluteFilePath(getUid());
    if(!checkCreate(trashDir))
        return QString();
    return trashDir;
}

//def find_ext_volume_fallback_trash(volume_root):
//    # from [2] Trash directories (1) create a .Trash-$uid dir.
//    trash_dir = op.join(volume_root, TOPDIR_FALLBACK)
//    # Try to make the directory, if we can't the OSError exception will escape
//    # be thrown out of send2trash.
//    check_create(trash_dir)
//    return trash_dir
static QString findExtVolumeFallbackTrash(const QString &volumeRoot)
{
    const QString trashDir = QDir(volumeRoot).absoluteFilePath(getTopDirFallback());
    checkCreate(trashDir);
    return trashDir;
}

//def find_ext_volume_trash(volume_root):
//    trash_dir = find_ext_volume_global_trash(volume_root)
//    if trash_dir is None:
//        trash_dir = find_ext_volume_fallback_trash(volume_root)
//    return trash_dir
static QString findExtVolumeTrash(const QString &volumeRoot)
{
    QString trashDir = findExtVolumeGlobalTrash(volumeRoot);
    if(trashDir.isEmpty())
        trashDir = findExtVolumeFallbackTrash(volumeRoot);
    return trashDir;
}

//# Pull this out so it's easy to stub (to avoid stubbing lstat itself)
//def get_dev(path):
//    return os.lstat(path).st_dev
static dev_t getDev(const QString &path)
{
    struct stat pathStat;
    memset(&pathStat, 0, sizeof(pathStat));
    if(lstat(path.toLocal8Bit().data(), &pathStat) != 0)
        return static_cast<dev_t>(0);
    return pathStat.st_dev;
}

//def send2trash(path):
//    if not isinstance(path, str):
//        path = str(path, sys.getfilesystemencoding())
//    if not op.exists(path):
//        raise OSError("File not found: %s" % path)
//    # ...should check whether the user has the necessary permissions to delete
//    # it, before starting the trashing operation itself. [2]
//    if not os.access(path, os.W_OK):
//        raise OSError("Permission denied: %s" % path)
//    # if the file to be trashed is on the same device as HOMETRASH we
//    # want to move it there.
//    path_dev = get_dev(path)
//
//    # If XDG_DATA_HOME or HOMETRASH do not yet exist we need to stat the
//    # home directory, and these paths will be created further on if needed.
//    trash_dev = get_dev(op.expanduser('~'))
//
//    if path_dev == trash_dev:
//        topdir = XDG_DATA_HOME
//        dest_trash = HOMETRASH
//    else:
//        topdir = find_mount_point(path)
//        trash_dev = get_dev(topdir)
//        if trash_dev != path_dev:
//            raise OSError("Couldn't find mount point for %s" % path)
//        dest_trash = find_ext_volume_trash(topdir)
//    trash_move(path, dest_trash, topdir)
static bool sendToTrash(const QString &path, QString *errorDescription)
{
    const QFileInfo pathInfo(path);
    if(!pathInfo.exists())
    {
        if(errorDescription)
            *errorDescription = qApp->translate("FileUtils", "File not found: %1").arg(path);
        return false;
    }

    struct stat pathStat;
    memset(&pathStat, 0, sizeof(pathStat));
    if(!(lstat(pathInfo.absoluteFilePath().toLocal8Bit().data(), &pathStat) == 0 &&
         pathInfo.isReadable() && QFileInfo(pathInfo.absolutePath()).isWritable() &&
         !((pathStat.st_mode & S_ISVTX) && (pathStat.st_uid != geteuid()))))
    {
        if(errorDescription)
            *errorDescription = qApp->translate("FileUtils", "Permission denied: %1").arg(path);
        return false;
    }

    dev_t pathDev = getDev(path);
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
                *errorDescription = qApp->translate("FileUtils", "Couldn't find mount point for %1").arg(path);
            return false;
        }
        destTrash = findExtVolumeTrash(topDir);
    }
    if(!trashMove(path, destTrash, topDir))
    {
        if(errorDescription)
            *errorDescription = qApp->translate("FileUtils", "Couldn't move to trash: %1").arg(path);
        return false;
    }
    return true;
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
    QFileInfo info(path);
    if(!info.exists())
    {
        if(errorDescription)
            *errorDescription = qApp->translate("FileUtils", "The specified path does not exist");
        return false;
    }
    const QString absolutePath = info.absoluteFilePath();

#if defined (Q_OS_MAC)

    // http://programtalk.com/vs2/?source=python/5435/send2trash/send2trash/plat_osx.py

    FSRef ref;
    memset(&ref, 0, sizeof(ref));
    const QByteArray utf8Path = absolutePath.toUtf8();
    const UInt8 *utf8PathData = reinterpret_cast<const UInt8*>(utf8Path.data());
    OSStatus status = FSPathMakeRefWithOptions(utf8PathData, kFSPathMakeRefDoNotFollowLeafSymlink, &ref, NULL);
    if(status)
    {
        const QString description = QString::fromUtf8(GetMacOSStatusCommentString(status));
        if(errorDescription)
            *errorDescription = description;
        qWarning() << "[FileUtils::MoveToTrash]: Unable to FSPathMakeRefWithOptions for file" << absolutePath;
        qWarning() << "[FileUtils::MoveToTrash]: Status Comment:" << description;
        return false;
    }
    status = FSMoveObjectToTrashSync(&ref, NULL, kFSFileOperationDefaultOptions);
    if(status)
    {
        const QString description = QString::fromUtf8(GetMacOSStatusCommentString(status));
        if(errorDescription)
            *errorDescription = description;
        qWarning() << "[FileUtils::MoveToTrash]: Unable to FSMoveObjectToTrashSync for file" << absolutePath;
        qWarning() << "[FileUtils::MoveToTrash]: Status Comment:" << description;
        return false;
    }
    return true;

#elif defined (Q_OS_WIN)

    // http://programtalk.com/vs2/?source=python/5435/send2trash/send2trash/plat_win.py

    HMODULE hShell32 = LoadLibraryA("shell32.dll");
    typedef int(*SHFileOperationW_t)(LPSHFILEOPSTRUCTW);
    SHFileOperationW_t SHFileOperationW_f = (SHFileOperationW_t)GetProcAddress(hShell32, "SHFileOperationW");

    HMODULE hKernel32 = LoadLibraryA("kernel32.dll");
    typedef DWORD(WINAPI *FormatMessageW_t)(DWORD, LPCVOID, DWORD, DWORD, LPWSTR, DWORD, va_list);
    FormatMessageW_t FormatMessageW_f = (FormatMessageW_t)GetProcAddress(hKernel32, "FormatMessageW");

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
        fileop.pTo = NULL;
        fileop.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
        fileop.fAnyOperationsAborted = 0;
        fileop.hNameMappings = 0;
        fileop.lpszProgressTitle = NULL;
        int status = SHFileOperationW_f(&fileop);
        if(status)
        {
            WCHAR * errorRawStr = NULL;
            FormatMessageW_f(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                           NULL, status, 0, (LPWSTR)(&errorRawStr), 0, NULL);
            const QString description = QString::fromStdWString(std::wstring(errorRawStr));
            LocalFree(errorRawStr);
            if(errorDescription)
                *errorDescription = description;
            qWarning() << "[FileUtils::MoveToTrash]: Unable to SHFileOperationW for file" << absolutePath;
            qWarning() << "[FileUtils::MoveToTrash]: Description:" << description;
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
        fileop.pTo = NULL;
        fileop.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
        fileop.fAnyOperationsAborted = 0;
        fileop.hNameMappings = 0;
        fileop.lpszProgressTitle = NULL;
        int status = SHFileOperationA(&fileop);
        if(status)
        {
            char * errorRawStr = NULL;
            FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                           NULL, status, 0, (LPSTR)(&errorRawStr), 0, NULL);
            const QString description = QString::fromLocal8Bit(errorRawStr);
            LocalFree(errorRawStr);
            if(errorDescription)
                *errorDescription = description;
            qWarning() << "[FileUtils::MoveToTrash]: Unable to SHFileOperationA for file" << absolutePath;
            qWarning() << "[FileUtils::MoveToTrash]: Description:" << description;
            return false;
        }
        return true;
    }

#else

    // http://programtalk.com/vs2/?source=python/5435/send2trash/send2trash/plat_other.py

    return MoveToTrashInternal::sendToTrash(absolutePath, errorDescription);

#endif

}

} // namespace FileUtils

