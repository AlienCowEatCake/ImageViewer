/*
   Copyright (C) 2019-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "UpdateChecker.h"

#include <algorithm>
#include <cassert>

#include <QApplication>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "Utils/JsonArray.h"
#include "Utils/JsonDocument.h"
#include "Utils/JsonObject.h"

static const qint64 RELEASES_LIMIT = 9999;

static bool releaseInfoGreater(const ReleaseInfo &lhs, const ReleaseInfo &rhs)
{
    if(lhs.version > rhs.version)
        return true;
    if(lhs.version < rhs.version)
        return false;

    if(!lhs.prerelease && rhs.prerelease)
        return true;
    if(lhs.prerelease && !rhs.prerelease)
        return false;

    if(lhs.publishedAt > rhs.publishedAt)
        return true;
    if(lhs.publishedAt < rhs.publishedAt)
        return false;

    if(lhs.createdAt > rhs.createdAt)
        return true;
    if(lhs.createdAt < rhs.createdAt)
        return false;

    return false;
}

static bool isPrerelase(const ReleaseInfo &info)
{
    return info.prerelease;
}

struct UpdateChecker::Impl
{
    const QString owner;
    const QString repo;
    const ReleaseVersion currentVersion;
    QNetworkAccessManager *networkManager;
    QNetworkReply *activeReply;

    Impl(const QString &owner, const QString &repo, const QString &currentVersion)
        : owner(owner)
        , repo(repo)
        , currentVersion(currentVersion)
        , networkManager(Q_NULLPTR)
        , activeReply(Q_NULLPTR)
    {}
};

UpdateChecker::UpdateChecker(RemoteType remoteType, const QString &owner, const QString &repo, const QString &currentVersion, QObject *parent)
    : QObject(parent)
    , m_impl(new Impl(owner, repo, currentVersion))
{
    assert(remoteType == RemoteTypeGitHub);
    Q_UNUSED(remoteType);
}

UpdateChecker::~UpdateChecker()
{}

void UpdateChecker::checkForUpdates()
{
    if(!m_impl->networkManager)
    {
        m_impl->networkManager = new QNetworkAccessManager(this);
        connect(m_impl->networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    }
    QNetworkRequest request;
    request.setUrl(QUrl(QString::fromLatin1("https://api.github.com/repos/%1/%2/releases?per_page=%3").arg(m_impl->owner).arg(m_impl->repo).arg(RELEASES_LIMIT)));
    request.setRawHeader("User-Agent", QString::fromLatin1("%1/%2").arg(qApp->applicationName()).arg(qApp->applicationVersion()).toLatin1());
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#endif
    m_impl->activeReply = m_impl->networkManager->get(request);
}

void UpdateChecker::cancel()
{
    if(!m_impl->activeReply)
        return;
     m_impl->activeReply->abort();
     m_impl->activeReply = Q_NULLPTR;
}

void UpdateChecker::replyFinished(QNetworkReply *reply)
{
    m_impl->activeReply = Q_NULLPTR;

    if(reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "[UpdateChecker]" << reply->errorString();
        Q_EMIT updateError(reply->errorString());
        return;
    }

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &error);
    if(error.error != QJsonParseError::NoError)
    {
        qWarning() << "[UpdateChecker]" << error.errorString();
        Q_EMIT updateError(error.errorString());
        return;
    }

    ReleaseInfo currentRelease;
    QList<ReleaseInfo> newReleases;

    const QJsonArray array = document.array();
    for(QJsonArray::ConstIterator it = array.constBegin(), itEnd = array.constEnd(); it != itEnd; ++it)
    {
        const QJsonObject object = (*it).toObject();
        if(object.value(QString::fromLatin1("draft")).toBool())
            continue;

        ReleaseInfo info;
        info.htmlUrl = QUrl(object.value(QString::fromLatin1("html_url")).toString());
        info.tagName = object.value(QString::fromLatin1("tag_name")).toString();
        info.name = object.value(QString::fromLatin1("name")).toString();
        info.body = object.value(QString::fromLatin1("body")).toString();
        info.prerelease = object.value(QString::fromLatin1("prerelease")).toBool();
        info.createdAt = QDateTime::fromString(object.value(QString::fromLatin1("created_at")).toString(), Qt::ISODate);
        info.publishedAt = QDateTime::fromString(object.value(QString::fromLatin1("published_at")).toString(), Qt::ISODate);
        info.version = ReleaseVersion::fromTag(info.tagName);

        if(info.version == m_impl->currentVersion)
            currentRelease = info;
        else if(info.version > m_impl->currentVersion)
            newReleases.append(info);
    }

    if(currentRelease.version.isValid() && !currentRelease.prerelease)
        newReleases.erase(std::remove_if(newReleases.begin(), newReleases.end(), isPrerelase), newReleases.end());

    if(!currentRelease.version.isValid())
        currentRelease.version = m_impl->currentVersion;

    if(newReleases.empty())
    {
        Q_EMIT updateNotFound(currentRelease);
        return;
    }

    std::stable_sort(newReleases.begin(), newReleases.end(), releaseInfoGreater);
    Q_EMIT updateFound(currentRelease, newReleases);
}

