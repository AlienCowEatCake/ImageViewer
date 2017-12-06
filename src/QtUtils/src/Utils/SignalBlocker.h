/*
   Copyright (C) 2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (QTUTILS_SIGNALBLOCKER_H_INCLUDED)
#define QTUTILS_SIGNALBLOCKER_H_INCLUDED

#include <QtGlobal>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))

#include <QSignalBlocker>

#else

#include <QObject>

namespace SignalBlockerImpl {

class QSignalBlocker
{
    Q_DISABLE_COPY(QSignalBlocker)

public:
    explicit QSignalBlocker(QObject *object)
        : m_object(object)
        , m_wasBlocked(object && object->blockSignals(true))
        , m_isRestored(false)
    {}

    explicit QSignalBlocker(QObject &object)
        : m_object(&object)
        , m_wasBlocked(object.blockSignals(true))
        , m_isRestored(false)
    {}

    ~QSignalBlocker()
    {
        if(!m_object || m_isRestored)
            return;
        m_object->blockSignals(m_wasBlocked);
    }

    void reblock()
    {
        if(!m_object)
            return;
        m_object->blockSignals(true);
        m_isRestored = false;
    }

    void unblock()
    {
        if(!m_object)
            return;
        m_object->blockSignals(m_wasBlocked);
        m_isRestored = true;
    }

private:
    QObject * const m_object;
    bool m_wasBlocked;
    bool m_isRestored;
};

} // namespace SignalBlockerImpl

#define QSignalBlocker ::SignalBlockerImpl::QSignalBlocker

#endif

#endif // QTUTILS_SIGNALBLOCKER_H_INCLUDED

