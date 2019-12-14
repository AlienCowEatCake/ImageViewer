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

#if !defined (UPDATER_UPDATE_CHECKER_H_INCLUDED)
#define UPDATER_UPDATE_CHECKER_H_INCLUDED

#include <QObject>

#include "Utils/Global.h"
#include "Utils/ScopedPointer.h"

#include "RemoteType.h"

class QNetworkReply;
struct ReleaseInfo;

class UpdateChecker : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(UpdateChecker)

signals:
    void updateNotFound(const ReleaseInfo &currentRelease);
    void updateFound(const ReleaseInfo &currentRelease, const QList<ReleaseInfo> &newReleases);
    void updateError(const QString &errorString);

public:
    UpdateChecker(RemoteType remoteType, const QString &owner, const QString &repo, const QString &currentVersion, QObject *parent = Q_NULLPTR);
    ~UpdateChecker();

public slots:
    void checkForUpdates();
    void cancel();

private slots:
    void replyFinished(QNetworkReply *reply);

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // UPDATER_UPDATE_CHECKER_H_INCLUDED

