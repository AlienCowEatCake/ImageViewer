/*
   Copyright (C) 2019-2020 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "UpdateManager.h"

#include <QApplication>
#include <QCheckBox>
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QTextEdit>
#include <QTimer>

#include "Utils/SettingsWrapper.h"

#include "SettingsKeys.h"
#include "UpdateChecker.h"

static const qint64 DAYS_TO_NEXT_AUTO_CHECK = 1;

struct UpdateManager::Impl
{
    UpdateChecker *checker;
    SettingsWrapper updaterSettings;
    QPointer<QWidget> parentWidget;
    QTimer updateTimer;
    bool silent;
    bool inProgress;

    Impl(UpdateManager *manager, RemoteType remoteType, const QString &owner, const QString &repo, const QString &currentVersion, bool autoCheck)
        : checker(new UpdateChecker(remoteType, owner, repo, currentVersion, manager))
        , updaterSettings(GROUP_UPDATE_MANAGER)
        , silent(false)
        , inProgress(false)
    {
        updaterSettings.setValue(KEY_AUTO_CHECK_FOR_UPDATES, updaterSettings.value(KEY_AUTO_CHECK_FOR_UPDATES, autoCheck));
        updaterSettings.setValue(KEY_SKIPPED_VERSION, updaterSettings.value(KEY_SKIPPED_VERSION, QString()));
        updaterSettings.setValue(KEY_LAST_CHECK_TIMESTAMP, updaterSettings.value(KEY_LAST_CHECK_TIMESTAMP, QString::fromLatin1("1970-01-01T00:00:00Z")));
    }
};

UpdateManager::UpdateManager(RemoteType remoteType, const QString &owner, const QString &repo, const QString &currentVersion, bool autoCheck, QObject *parent)
    : QObject(parent)
    , m_impl(new Impl(this, remoteType, owner, repo, currentVersion, autoCheck))
{
    connect(m_impl->checker, SIGNAL(updateNotFound(const ReleaseInfo&)), this, SLOT(onUpdateNotFound(const ReleaseInfo&)));
    connect(m_impl->checker, SIGNAL(updateFound(const ReleaseInfo&, const QList<ReleaseInfo>&)), this, SLOT(onUpdateFound(const ReleaseInfo&, const QList<ReleaseInfo>&)));
    connect(m_impl->checker, SIGNAL(updateError(const QString&)), this, SLOT(onUpdateError(const QString&)));
    connect(&m_impl->updateTimer, SIGNAL(timeout()), this, SLOT(silentCheckForUpdates()));
    m_impl->updateTimer.setInterval(1000);
    m_impl->updateTimer.setSingleShot(true);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    m_impl->updateTimer.setTimerType(Qt::VeryCoarseTimer);
#endif
    if(autoCheck && m_impl->updaterSettings.value(KEY_AUTO_CHECK_FOR_UPDATES).toBool())
    {
        const QDateTime lastCheck = QDateTime::fromString(m_impl->updaterSettings.value(KEY_LAST_CHECK_TIMESTAMP).toString(), Qt::ISODate);
        if(!lastCheck.isValid() || QDateTime::currentDateTime() >= lastCheck.addDays(DAYS_TO_NEXT_AUTO_CHECK))
            m_impl->updateTimer.start();
    }
}

UpdateManager::~UpdateManager()
{}

void UpdateManager::setParentForDialogs(QWidget *parentWidget)
{
    m_impl->parentWidget = parentWidget;
}

void UpdateManager::checkForUpdates(bool silent)
{
    if(m_impl->inProgress)
    {
        if(silent)
            return;

        QMessageBox box(m_impl->parentWidget);
        box.setIcon(QMessageBox::Critical);
        box.setWindowTitle(qApp->translate("UpdateManager", "Error"));
        box.setText(qApp->translate("UpdateManager", "An update is already in progress. Check back later."));
        box.setStandardButtons(QMessageBox::Ok);
        box.setDefaultButton(QMessageBox::Ok);
        box.exec();
        return;
    }

    m_impl->inProgress = true;
    m_impl->silent = silent;
    m_impl->checker->checkForUpdates();
    m_impl->updaterSettings.setValue(KEY_LAST_CHECK_TIMESTAMP, QDateTime::currentDateTime().toString(Qt::ISODate));
}

void UpdateManager::checkForUpdates()
{
    checkForUpdates(false);
}

void UpdateManager::silentCheckForUpdates()
{
    checkForUpdates(true);
}

void UpdateManager::onUpdateNotFound(const ReleaseInfo &currentRelease)
{
    if(!m_impl->silent)
    {
        QMessageBox box(m_impl->parentWidget);
        box.setIcon(QMessageBox::Information);
        box.setWindowTitle(qApp->translate("UpdateManager", "Information"));
        box.setText(qApp->translate("UpdateManager", "You have the latest version."));
        box.setInformativeText(QString::fromLatin1("%1 <b>%2</b>")
                .arg(qApp->translate("UpdateManager", "Your version:"))
                .arg(currentRelease.version.string()));
        box.setStandardButtons(QMessageBox::Ok);
        box.setDefaultButton(QMessageBox::Ok);
        box.exec();
    }
    m_impl->inProgress = false;
}

void UpdateManager::onUpdateFound(const ReleaseInfo &currentRelease, const QList<ReleaseInfo> &newReleases)
{
    const Version skippedVersion = Version(m_impl->updaterSettings.value(KEY_SKIPPED_VERSION).toString());
    if(m_impl->silent && skippedVersion.isValid() && newReleases.first().version <= skippedVersion)
    {
        onUpdateNotFound(currentRelease);
        return;
    }

    QMessageBox box(m_impl->parentWidget);
    box.setIcon(QMessageBox::Information);
    box.setWindowTitle(qApp->translate("UpdateManager", "Information"));
    box.setText(qApp->translate("UpdateManager", "New updates are available."));
    const QString informativeText = QString::fromLatin1("%1 <b>%2</b><br>%3 <b>%4</b>")
            .arg(qApp->translate("UpdateManager", "Your version:"))
            .arg(currentRelease.version.string())
            .arg(qApp->translate("UpdateManager", "Latest version:"))
            .arg(newReleases.first().version.string());
    box.setInformativeText(informativeText);
    QPushButton *downloadButton = box.addButton(qApp->translate("UpdateManager", "&Download")           , QMessageBox::AcceptRole);
    QPushButton *cancelButton   = box.addButton(qApp->translate("UpdateManager", "&Cancel")             , QMessageBox::RejectRole);
    QPushButton *skipButton     = box.addButton(qApp->translate("UpdateManager", "&Skip This Version")  , QMessageBox::DestructiveRole);
    box.setDefaultButton(downloadButton);
    QString changelog;
    for(QList<ReleaseInfo>::ConstIterator it = newReleases.constBegin(), itEnd = newReleases.constEnd(); it != itEnd; ++it)
        changelog += (changelog.isEmpty() ? QString() : QString::fromLatin1("\r\n\r\n")) + QString::fromLatin1("# %1\r\n\r\n%2\r\n").arg(it->name).arg(it->body);
    box.setDetailedText(changelog);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    if(QTextEdit *textEdit = box.findChild<QTextEdit*>())
        textEdit->setMarkdown(changelog);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    QCheckBox *checkBox = new QCheckBox();
    checkBox->setChecked(m_impl->updaterSettings.value(KEY_AUTO_CHECK_FOR_UPDATES).toBool());
    checkBox->setText(qApp->translate("UpdateManager", "Automatically check for updates"));
    box.setCheckBox(checkBox);
#endif
    box.exec();
    if(box.clickedButton() == downloadButton)
        QDesktopServices::openUrl(newReleases.first().htmlUrl);
    else if(box.clickedButton() == skipButton)
        m_impl->updaterSettings.setValue(KEY_SKIPPED_VERSION, newReleases.first().version.string());
    Q_UNUSED(cancelButton);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    m_impl->updaterSettings.setValue(KEY_AUTO_CHECK_FOR_UPDATES, checkBox->isChecked());
#endif
    m_impl->inProgress = false;
}

void UpdateManager::onUpdateError(const QString &errorString)
{
    if(!m_impl->silent)
    {
        QMessageBox box(m_impl->parentWidget);
        box.setIcon(QMessageBox::Critical);
        box.setWindowTitle(qApp->translate("UpdateManager", "Error"));
        box.setText(qApp->translate("UpdateManager", "Can't check for updates. Try again later."));
        box.setInformativeText(errorString);
        box.setStandardButtons(QMessageBox::Ok);
        box.setDefaultButton(QMessageBox::Ok);
        box.exec();
    }
    m_impl->inProgress = false;
}

