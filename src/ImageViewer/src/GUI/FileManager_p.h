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

#if !defined (FILEMANAGER_P_H_INCLUDED)
#define FILEMANAGER_P_H_INCLUDED

#include <QAtomicInt>
#include <QFileSystemWatcher>
#include <QMutex>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QTimer>

#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
#include <QElapsedTimer>
#else
#include <QTime>
typedef QTime QElapsedTimer;
#endif

#include "Utils/Global.h"

class FilesScanner : public QThread
{
    Q_OBJECT
    Q_DISABLE_COPY(FilesScanner)

Q_SIGNALS:
    void updated();

public:
    FilesScanner(QObject *parent = Q_NULLPTR);
    ~FilesScanner();

    QStringList getScanResult();

    bool configureForDirContent(const QStringList &supportedFormats, const QString &directoryPath);
    bool configureForFilxedList(const QStringList &supportedFormats, const QStringList &fixedPathsList);
    void reset();

protected:
    void run() Q_DECL_OVERRIDE;
    QStringList collectDirContent(const QString &directoryPath) const;

private Q_SLOTS:
    void update();
    void tryUpdate();
    void ensureUpdated();

private:
    QFileSystemWatcher *m_watcher;
    QMutex m_watcherMutex;
    QAtomicInt m_watcherConfigured;
    QAtomicInt m_stopPending;
    QTimer *m_updateTimer;
    QAtomicInt m_scannerIsDirty;
    QElapsedTimer m_timeFromLastUpdate;

    QStringList m_supportedFormats;
    QString m_directoryPath;
    QStringList m_fixedPathsList;

    QStringList m_scanResult;
    QMutex m_scanResultMutex;
};

#endif // FILEMANAGER_P_H_INCLUDED
