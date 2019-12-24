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

#if !defined (UPDATER_UPDATE_MANAGER_H_INCLUDED)
#define UPDATER_UPDATE_MANAGER_H_INCLUDED

#include <QObject>

#include "Utils/Global.h"
#include "Utils/ScopedPointer.h"

#include "RemoteType.h"

struct ReleaseInfo;

class UpdateManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(UpdateManager)

public:
    UpdateManager(RemoteType remoteType, const QString &owner, const QString &repo, const QString &currentVersion, bool autoCheck = true, QObject *parent = Q_NULLPTR);
    ~UpdateManager();

    void setParentForDialogs(QWidget *parentWidget);

public Q_SLOTS:
    void checkForUpdates(bool silent);
    void checkForUpdates();
    void silentCheckForUpdates();

private Q_SLOTS:
    void onUpdateNotFound(const ReleaseInfo &currentRelease);
    void onUpdateFound(const ReleaseInfo &currentRelease, const QList<ReleaseInfo> &newReleases);
    void onUpdateError(const QString &errorString);

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // UPDATER_UPDATE_MANAGER_H_INCLUDED

