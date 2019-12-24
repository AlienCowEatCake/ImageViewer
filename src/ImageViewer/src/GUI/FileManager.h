/*
   Copyright (C) 2017-2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "Utils/Global.h"
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
        FlagCanDeleteCurrentFile    = 1 << 3,
        FlagChangeAll               = FlagCurrentFilePath
                                    | FlagCurrentFileIndex
                                    | FlagFilesCount
                                    | FlagCanDeleteCurrentFile
    };
    Q_DECLARE_FLAGS(ChangeFlags, ChangeFlag)

Q_SIGNALS:
    void stateChanged(const FileManager::ChangeFlags& changedFlags);

public:
    explicit FileManager(QObject *parent = Q_NULLPTR);
    ~FileManager();

    QStringList supportedFormatsWithWildcards() const;
    void setSupportedFormatsWithWildcards(const QStringList& supportedFormatsWithWildcards);

    QString currentFilePath() const;
    int currentFileIndex() const;
    int filesCount() const;
    bool canDeleteCurrentFile() const;

    QStringList currentOpenArguments() const;

public Q_SLOTS:
    void reset();
    void update();
    bool openPath(const QString &filePath);
    bool openPaths(const QStringList &filePaths);
    bool selectByPath(const QString &filePath);
    bool selectByIndex(int index);
    bool deleteCurrentFile();
    bool moveToTrashCurrentFile(QString *errorDescription = Q_NULLPTR);

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // FILEMANAGER_H_INCLUDED
