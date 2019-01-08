/*
   Copyright (C) 2018-2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined(EFFECTS_STORAGE_H_INCLUDED)
#define EFFECTS_STORAGE_H_INCLUDED

#include <QObject>
#include <QHash>

#include "Utils/Global.h"

#include "../UIState.h"

class ImageViewerWidget;

class EffectsStorage : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(EffectsStorage)

public:
    EffectsStorage(QObject *parent = Q_NULLPTR);
    ~EffectsStorage();

public slots:
    void updateUIState(const UIState &state, const UIChangeFlags &changeFlags);
    void applySavedEffects(ImageViewerWidget *imageViewerWidget);

    void rotateClockwise();
    void rotateCounterclockwise();
    void flipHorizontal();
    void flipVertical();

private:
    void rotate(int angle);

private:
    struct EffectsData;
    QHash<QString, EffectsData> m_storage;
    QString m_currentFilePath;
};

#endif // EFFECTS_STORAGE_H_INCLUDED
