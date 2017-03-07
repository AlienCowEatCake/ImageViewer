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
#endif

#include <QApplication>
#include <QString>
#include <QFileInfo>
#include <QDebug>

namespace FileUtils {

/// @brief Удаление указанного файла или директории в корзину
/// @attention Используется удаление без запроса. В случае отсутствия на целевой системе
///         корзины, либо если корзина программно отключена поведение этой функции строго
///         не специфицируется. Предполагаемое поведение - удаление файла мимо корзины.
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

    /// @todo
    if(errorDescription)
        *errorDescription = qApp->translate("FileUtils", "Move to Trash is not implemented yet for your system");
    return false;

#endif

}

} // namespace FileUtils

