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

#if !defined(QTUTILS_APPLICATION_H_INCLUDED)
#define QTUTILS_APPLICATION_H_INCLUDED

#include <QApplication>
#include <QString>

/**
 * @brief Класс-обертка над QApplication, выполняющий обработку событий типа QFileOpenEvent
 * @note https://stackoverflow.com/questions/26849866/unable-to-open-file-with-qt-app-on-mac
 */
class Application : public QApplication
{
    Q_OBJECT
public:
    /**
     * @brief Конструктор
     * @param[inout] argc Количество аргументов
     * @param[inout] argv Массив C-строк - значений аргументов
     */
    Application(int &argc, char **argv);

    /**
     * @brief Получить имя последнего файла, который пришел в QFileOpenEvent
     * @return Имя последнего файла, который пришел в QFileOpenEvent
     */
    const QString &getLastOpenFilename() const;

    /**
     * @brief Узнать, сохранено ли имя последнего файла, который пришел в QFileOpenEvent
     * @return true - имя сохранено, false - имя не сохранено
     */
    bool hasLastOpenFilename() const;

protected:
    /**
     * @brief Обработчик событий
     * @param[in] event Событие
     * @return true - событие распознано и обработано, false - иначе
     */
    bool event(QEvent *event);

signals:
    /**
     * @brief Сигнал о том, что пришло событие QFileOpenEvent
     * @param filename Имя файла
     */
    void openFileEvent(const QString &filename);

private:
    /**
     * @brief Имя последнего файла, который пришел в QFileOpenEvent
     */
    QString m_lastOpenFilename;
};

#endif // QTUTILS_APPLICATION_H_INCLUDED

