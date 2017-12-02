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

#if !defined(UI_STATE_H_INCLUDED)
#define UI_STATE_H_INCLUDED

#include <QtGlobal>
#include <QString>

class QObject;

enum UIChangeFlag
{
    UICF_Nothing                = 0,
    UICF_HasCurrentFile         = 1 << 0,
    UICF_HasCurrentFileIndex    = 1 << 1,
    UICF_CurrentFilePath        = 1 << 2,
    UICF_CurrentFileIndex       = 1 << 3,
    UICF_FilesCount             = 1 << 4,
    UICF_CanDeleteCurrentFile   = 1 << 5,
    UICF_All                    = UICF_HasCurrentFile
                                | UICF_HasCurrentFileIndex
                                | UICF_CurrentFilePath
                                | UICF_CurrentFileIndex
                                | UICF_FilesCount
                                | UICF_CanDeleteCurrentFile
};
Q_DECLARE_FLAGS(UIChangeFlags, UIChangeFlag)

struct UIState
{
    UIState()
        : hasCurrentFile(false)
        , hasCurrentFileIndex(false)
        , currentFilePath(QString())
        , currentFileIndex(-1)
        , filesCount(0)
        , canDeleteCurrentFile(false)
    {}

    bool hasCurrentFile;
    bool hasCurrentFileIndex;
    QString currentFilePath;
    int currentFileIndex;
    int  filesCount;
    bool canDeleteCurrentFile;
};

// $NAME(const UIState &state, const UIChangeFlags &changeFlags)
void RegisterUIStateChangedEmitter(QObject *object, const char *signal);
void UnregisterUIStateChangedEmitter(QObject *object, const char *signal);
void RegisterUIStateChangedReceiver(QObject *object, const char *slot);
void UnregisterUIStateChangedReceiver(QObject *object, const char *slot);

#endif // UI_STATE_H_INCLUDED
