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

#include "GUISettings.h"
#include "Utils/SettingsWrapper.h"

struct GUISettings::Impl
{
    Impl()
        : settings(QString::fromLatin1("GUISettings"))
    {}
    SettingsWrapper settings;
};

GUISettings::GUISettings(QObject *parent)
    : QObject(parent)
    , m_impl(new Impl)
{}

GUISettings::~GUISettings()
{}

bool GUISettings::askBeforeDelete() const
{
    const bool defaultValue = true;
    QVariant value = m_impl->settings.value(QString::fromLatin1("AskBeforeDelete"), defaultValue);
    return value.isValid() ? value.toBool() : defaultValue;
}

void GUISettings::setAskBeforeDelete(bool enabled)
{
    const bool oldValue = askBeforeDelete();
    if(enabled != oldValue)
        emit askBeforeDeleteChanged(enabled);
    m_impl->settings.setValue(QString::fromLatin1("AskBeforeDelete"), enabled);
}

bool GUISettings::moveToTrash() const
{
    const bool defaultValue = true;
    QVariant value = m_impl->settings.value(QString::fromLatin1("MoveToTrash"), defaultValue);
    return value.isValid() ? value.toBool() : defaultValue;
}

void GUISettings::setMoveToTrash(bool enabled)
{
    const bool oldValue = moveToTrash();
    if(enabled != oldValue)
        emit moveToTrashChanged(enabled);
    m_impl->settings.setValue(QString::fromLatin1("MoveToTrash"), enabled);
}
