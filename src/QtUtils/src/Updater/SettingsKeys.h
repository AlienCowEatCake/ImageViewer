/*
   Copyright (C) 2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (UPDATER_SETTINGS_KEYS_H_INCLUDED)
#define UPDATER_SETTINGS_KEYS_H_INCLUDED

#include <QString>

static const QString GROUP_UPDATE_MANAGER       = QString::fromLatin1("UpdateManager");

static const QString KEY_SKIPPED_VERSION        = QString::fromLatin1("SkippedVersion");
static const QString KEY_AUTO_CHECK_FOR_UPDATES = QString::fromLatin1("AutoCheckForUpdates");

#endif // UPDATER_SETTINGS_KEYS_H_INCLUDED

