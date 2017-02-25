/*
   Copyright (C) 2011-2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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
 * @brief Конструктор
 * @param[inout] argc Количество аргументов
 * @param[inout] argv Массив C-строк - значений аргументов
 */
Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
{}

/**
 * @brief Получить имя последнего файла, который пришел в QFileOpenEvent
 * @return Имя последнего файла, который пришел в QFileOpenEvent
 */
const QString &Application::getLastOpenFilename() const
{
    return m_lastOpenFilename;
}

/**
 * @brief Узнать, сохранено ли имя последнего файла, который пришел в QFileOpenEvent
 * @return true - имя сохранено, false - имя не сохранено
 */
bool Application::hasLastOpenFilename() const
{
    return !m_lastOpenFilename.isEmpty();
}

/**
 * @brief Обработчик событий
 * @param[in] event Событие
 * @return true - событие распознано и обработано, false - иначе
 */
bool Application::event(QEvent *event)
{
    if(event->type() == QEvent::FileOpen)
    {
        QFileOpenEvent *fileOpenEvent = static_cast<QFileOpenEvent*>(event);
        if(fileOpenEvent)
        {
            m_lastOpenFilename = fileOpenEvent->file();
            if(hasLastOpenFilename())
            {
                emit openFileEvent(m_lastOpenFilename);
                return true;
            }
        }
    }
    return QApplication::event(event);
}
