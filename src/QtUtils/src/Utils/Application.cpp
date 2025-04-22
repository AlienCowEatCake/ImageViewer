/*
   Copyright (C) 2011-2025 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "Application.h"
#include <QEvent>
#include <QFileOpenEvent>

/**
 * @brief Constructor
 * @param[inout] argc Arguments count
 * @param[inout] argv Array of C-strings - arguments values
 */
Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
{}

/**
 * @brief Get path of last file from QFileOpenEvent
 * @return Path of last file from QFileOpenEvent
 */
const QString &Application::getLastOpenFilePath() const
{
    return m_lastOpenFilePath;
}

/**
 * @brief Check existence of remembered path of last file from QFileOpenEvent
 * @return true - path was saved, false - path was not saved
 */
bool Application::hasLastOpenFilePath() const
{
    return !m_lastOpenFilePath.isEmpty();
}

/**
 * @brief Event handler
 * @param[in] event Event
 * @return true - event was handled, false - otherwise
 */
bool Application::event(QEvent *event)
{
    if(event->type() == QEvent::FileOpen)
    {
        QFileOpenEvent *fileOpenEvent = static_cast<QFileOpenEvent*>(event);
        if(fileOpenEvent)
        {
            m_lastOpenFilePath = fileOpenEvent->file();
            if(hasLastOpenFilePath())
            {
                Q_EMIT openFileEvent(m_lastOpenFilePath);
                return true;
            }
        }
    }
    return QApplication::event(event);
}
