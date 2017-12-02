/*
   Copyright (C) 2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (FILEMANAGER_H_INCLUDED)
#define FILEMANAGER_H_INCLUDED

#include <QObject>
#include "Utils/ScopedPointer.h"

class QString;
class QStringList;

class FileManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FileManager)

public:
    enum ChangeFlag
    {
        FlagCurrentFilePath         = 1 << 0,
        FlagCurrentFileIndex        = 1 << 1,
        FlagFilesCount              = 1 << 2,
        FlagCanDeleteCurrentFile    = 1 << 3
    };
    Q_DECLARE_FLAGS(ChangeFlags, ChangeFlag)

signals:
    void stateChanged(const FileManager::ChangeFlags& changedFlags);

public:
    FileManager(const QStringList &supportedFormatsWithWildcards, QObject *parent = NULL);
    ~FileManager();

    QString currentFilePath() const;
    int currentFileIndex() const;
    int filesCount() const;
    bool canDeleteCurrentFile() const;

public slots:
    void reset();
    void update();
    bool openPath(const QString &filePath);
    bool openPaths(const QStringList &filePaths);
    bool selectByIndex(int index);
    bool deleteCurrentFile();
    bool moveToTrashCurrentFile(QString *errorDescription = NULL);

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // FILEMANAGER_H_INCLUDED
