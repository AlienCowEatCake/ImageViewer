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

#include "UIState.h"
#include "Utils/ObjectsConnector.h"

namespace {

const char *UI_STATE_CHANGED_ID = "UI_STATE_CHANGED";

} // namespace


void RegisterUIStateChangedEmitter(QObject *object, const char *signal)
{
    ObjectsConnector::RegisterEmitter(UI_STATE_CHANGED_ID, object, signal);
}

void UnregisterUIStateChangedEmitter(QObject *object, const char *signal)
{
    ObjectsConnector::UnregisterEmitter(UI_STATE_CHANGED_ID, object, signal);
}

void RegisterUIStateChangedReceiver(QObject *object, const char *slot)
{
    ObjectsConnector::RegisterReceiver(UI_STATE_CHANGED_ID, object, slot);
}

void UnregisterUIStateChangedReceiver(QObject *object, const char *slot)
{
    ObjectsConnector::UnregisterReceiver(UI_STATE_CHANGED_ID, object, slot);
}
