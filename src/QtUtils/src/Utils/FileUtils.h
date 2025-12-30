/*
   Copyright (C) 2017-2025 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (QTUTILS_FILEUTILS_H_INCLUDED)
#define QTUTILS_FILEUTILS_H_INCLUDED

#include "Global.h"

class QString;

namespace FileUtils {

/// @brief Delete specified file or directory (recursively)
/// @param[in] path - path to file or directory
/// @return - true - file or directory was deleted, false - otherwise
bool DeleteRecursively(const QString &path);

/// @brief Check current operating system supports moving files to trash
/// @return - true - file or directory can be moved to trash, false - otherwise
bool SupportsMoveToTrash();

/// @brief Move specified file or directory to trash
/// @attention Operation is performed without confirmation request. Behavior of this
///            function is not specified if system has no trash or trash support is
///            disabled. This can be platform specific, e.g. return with false or delete
///            file permanently. Error description may not be set in this case
/// @param[in] path - path to file or directory
/// @param[out] errorDescription - information that describes error if that has occurred
/// @return - true - file or directory was moved to trash or deleted, false - otherwise
bool MoveToTrash(const QString &path, QString* errorDescription = Q_NULLPTR);

/// @brief Move specified file or directory to trash or delete
/// @attention Operation is performed without confirmation request. File or directory
///            will be deleted permanently if system has no trash or trash support is
///            disabled
/// @param[in] path - path to file or directory
/// @param[out] errorDescription - information that describes error if that has occurred
/// @return - true - file or directory was moved to trash or deleted, false - otherwise
bool MoveToTrashOrDelete(const QString &path, QString* errorDescription = Q_NULLPTR);

} // namespace FileUtils

#endif // QTUTILS_FILEUTILS_H_INCLUDED

