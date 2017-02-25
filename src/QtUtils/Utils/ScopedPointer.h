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

#if !defined (QTUTILS_SCOPEDPOINTER_H_INCLUDED)
#define QTUTILS_SCOPEDPOINTER_H_INCLUDED

#include <QtGlobal>

#if (QT_VERSION >= QT_VERSION_CHECK(4, 6, 0))

#include <QScopedPointer>

#else

/// @brief The QScopedPointer class stores a pointer to a dynamically allocated object, and deletes it upon destruction.
/// @attention Implementation is incomplete!
template<typename T>
class QScopedPointer
{
public:

    /// @brief Constructs this QScopedPointer instance and sets its pointer to p.
    QScopedPointer(T *p = NULL)
        : m_data(p)
    {}

    /// @brief Destroys this QScopedPointer object. Delete the object its pointer points to.
    ~QScopedPointer()
    {
        delete m_data;
    }

    /// @brief Returns the value of the pointer referenced by this object. QScopedPointer still owns the object pointed to.
    T *data() const
    {
        return m_data;
    }

    /// @brief Returns true if this object is holding a pointer that is null.
    bool isNull() const
    {
        return m_data == NULL;
    }

    /// @brief Deletes the existing object it is pointing to if any, and sets its pointer to other.
    /// @note QScopedPointer now owns other and will delete it in its destructor.
    void reset(T *other = NULL)
    {
        delete m_data;
        m_data = other;
    }

    /// @brief Swap this pointer with other.
    void swap(QScopedPointer<T> & other)
    {
        T *tmp = m_data;
        m_data = other.data;
        other.data = tmp;
    }

    /// @brief Returns the value of the pointer referenced by this object. The pointer of this QScopedPointer object will be reset to null.
    /// @note Callers of this function take ownership of the pointer.
    T *take()
    {
        T *data = m_data;
        m_data = NULL;
        return data;
    }

    /// @brief Returns true if this object is not null. This function is suitable for use in if-constructs.
    operator bool() const
    {
        return !isNull();
    }

    /// @brief Returns true if the pointer referenced by this object is null, otherwise returns false.
    bool operator!() const
    {
        return isNull();
    }

    /// @brief Provides access to the scoped pointer's object.
    /// @note If the contained pointer is null, behavior is undefined.
    T &operator*() const
    {
        return *m_data;
    }

    /// @brief Provides access to the scoped pointer's object.
    /// @note If the contained pointer is null, behavior is undefined.
    T *operator->() const
    {
        return m_data;
    }

private:

    T * m_data;
};

#endif

#endif // QTUTILS_SCOPEDPOINTER_H_INCLUDED

