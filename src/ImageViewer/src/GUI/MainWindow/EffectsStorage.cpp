/*
   Copyright (C) 2018-2021 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "EffectsStorage.h"
#include "ImageViewerWidget.h"

EffectsStorage::EffectsStorage(QObject *parent)
    : QObject(parent)
{}

EffectsStorage::~EffectsStorage()
{}

EffectsStorage::EffectsData EffectsStorage::effectsData() const
{
    QHash<QString, EffectsData>::ConstIterator effectsData = m_storage.find(m_currentFilePath);
    if(effectsData == m_storage.constEnd())
        return EffectsData();
    return effectsData.value();
}

void EffectsStorage::updateUIState(const UIState &state, const UIChangeFlags &changeFlags)
{
    if(changeFlags.testFlag(UICF_CurrentFilePath))
        m_currentFilePath = state.currentFilePath;
}

void EffectsStorage::applySavedEffects(ImageViewerWidget *imageViewerWidget)
{
    QHash<QString, EffectsData>::ConstIterator effectsData = m_storage.find(m_currentFilePath);
    if(effectsData == m_storage.constEnd())
        return;

    int angle = effectsData.value().rotationAngle;
    while(angle > 0)
    {
        imageViewerWidget->rotateClockwise();
        angle -= 90;
    }
    while(angle < 0)
    {
        imageViewerWidget->rotateCounterclockwise();
        angle += 90;
    }

    if(effectsData.value().flipHorizontal)
        imageViewerWidget->flipHorizontal();
    if(effectsData.value().flipVertical)
        imageViewerWidget->flipVertical();
}

void EffectsStorage::rotateClockwise()
{
    rotate(+90);
}

void EffectsStorage::rotateCounterclockwise()
{
    rotate(-90);
}

void EffectsStorage::flipHorizontal()
{
    if(m_currentFilePath.isEmpty())
        return;
    QHash<QString, EffectsData>::Iterator effectsData = m_storage.find(m_currentFilePath);
    if(effectsData != m_storage.end())
    {
        effectsData.value().flipHorizontal = !effectsData.value().flipHorizontal;
        if(effectsData.value().isDefault())
            m_storage.remove(m_currentFilePath);
        return;
    }
    EffectsData newData;
    newData.flipHorizontal = true;
    m_storage.insert(m_currentFilePath, newData);
}

void EffectsStorage::flipVertical()
{
    if(m_currentFilePath.isEmpty())
        return;
    QHash<QString, EffectsData>::Iterator effectsData = m_storage.find(m_currentFilePath);
    if(effectsData != m_storage.end())
    {
        effectsData.value().flipVertical = !effectsData.value().flipVertical;
        if(effectsData.value().isDefault())
            m_storage.remove(m_currentFilePath);
        return;
    }
    EffectsData newData;
    newData.flipVertical = true;
    m_storage.insert(m_currentFilePath, newData);
}

void EffectsStorage::rotate(int angle)
{
    if(m_currentFilePath.isEmpty())
        return;
    QHash<QString, EffectsData>::Iterator effectsData = m_storage.find(m_currentFilePath);
    if(effectsData != m_storage.end())
    {
        int actualAngle = effectsData.value().rotationAngle + angle;
        if(actualAngle >= 360)
            actualAngle -= 360;
        if(actualAngle <= -360)
            actualAngle += 360;
        effectsData.value().rotationAngle = actualAngle;
        if(effectsData.value().isDefault())
            m_storage.remove(m_currentFilePath);
        return;
    }
    EffectsData newData;
    newData.rotationAngle = angle;
    m_storage.insert(m_currentFilePath, newData);
}
