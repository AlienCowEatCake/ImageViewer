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

#if !defined(QTUTILS_APPLICATION_H_INCLUDED)
#define QTUTILS_APPLICATION_H_INCLUDED

#include <QApplication>
#include <QString>

#include "Global.h"

/**
 * @brief QApplication wrapper for handling QFileOpenEvent
 * @note https://stackoverflow.com/questions/26849866/unable-to-open-file-with-qt-app-on-mac
 */
class Application : public QApplication
{
    Q_OBJECT
    Q_DISABLE_COPY(Application)

public:
    /**
     * @brief Constructor
     * @param[inout] argc Arguments count
     * @param[inout] argv Array of C-strings - arguments values
     */
    Application(int &argc, char **argv);

    /**
     * @brief Get path of last file from QFileOpenEvent
     * @return Path of last file from QFileOpenEvent
     */
    const QString &getLastOpenFilePath() const;

    /**
     * @brief Check existence of remembered path of last file from QFileOpenEvent
     * @return true - path was saved, false - path was not saved
     */
    bool hasLastOpenFilePath() const;

protected:
    /**
     * @brief Event handler
     * @param[in] event Event
     * @return true - event was handled, false - otherwise
     */
    bool event(QEvent *event) Q_DECL_OVERRIDE;

Q_SIGNALS:
    /**
     * @brief Signal which emitted when QFileOpenEvent was handled
     * @param filePath File path
     */
    void openFileEvent(const QString &filePath);

private:
    /**
     * @brief Path of last file from QFileOpenEvent
     */
    QString m_lastOpenFilePath;
};

#endif // QTUTILS_APPLICATION_H_INCLUDED

