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

#if !defined (MAIN_CONTROLLER_H_INCLUDED)
#define MAIN_CONTROLLER_H_INCLUDED

#include <QObject>

#include "Utils/Global.h"
#include "Utils/ScopedPointer.h"

#include "FileManager.h"
#include "UIState.h"

class QString;
class QStringList;

class MainController : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(MainController)

signals:
    void uiStateChanged(const UIState &state, const UIChangeFlags &changeFlags);

public:
    explicit MainController(QObject *parent = Q_NULLPTR);
    ~MainController();

public slots:
    bool openPath(const QString &path);
    bool openPaths(const QStringList &paths);
    bool openFileWithDialog();
    bool openFolderWithDialog();
    bool deleteCurrentFile();
    bool selectNextFile();
    bool selectPreviousFile();
    bool selectFirstFile();
    bool selectLastFile();

    void showMainWindow();
    void showAbout();
    void showPreferences();
    void showImageInformation();
    void showStylesheetEditor();
    void openNewWindow();

private slots:
    void onReopenWithRequested(const QString &decoderName);
    void onFileManagerStateChanged(const FileManager::ChangeFlags &changedFlags);

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // MAIN_CONTROLLER_H_INCLUDED
