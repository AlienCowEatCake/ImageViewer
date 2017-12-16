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

#if !defined (QTUTILS_CF_TYPE_PTR_H_INCLUDED)
#define QTUTILS_CF_TYPE_PTR_H_INCLUDED

#include <CoreFoundation/CoreFoundation.h>

/// @brief Умный указатель для типов Core Foundation и им подобных
template <typename T>
class CFTypePtr
{
public:
    CFTypePtr()
        : m_dataRef(static_cast<T>(0))
    {}

    CFTypePtr(T dataRef, bool retain = true)
        : m_dataRef(dataRef)
    {
        if(m_dataRef && retain)
            CFRetain(m_dataRef);
    }

    CFTypePtr(const CFTypePtr &other)
        : m_dataRef(other.m_dataRef)
    {
        if(m_dataRef)
            CFRetain(m_dataRef);
    }

    ~CFTypePtr()
    {
        if(m_dataRef)
            CFRelease(m_dataRef);
    }

    CFTypePtr & operator = (const CFTypePtr &other)
    {
        CFTypePtr(other).swap(*this);
        return *this;
    }

    CFTypePtr & operator = (T other)
    {
        CFTypePtr(other).swap(*this);
        return *this;
    }

    void swap(const CFTypePtr &other)
    {
        T tmp = m_dataRef;
        m_dataRef = other.m_dataRef;
        other.m_dataRef = tmp;
    }

    void reset()
    {
        CFTypePtr().swap(*this);
    }

    void reset(T other)
    {
        CFTypePtr(other).swap(*this);
    }

    void reset(T other, bool retain)
    {
        CFTypePtr(other, retain).swap(*this);
    }

    T get() const
    {
        return m_dataRef;
    }

    T detach()
    {
        T dataRef = m_dataRef;
        m_dataRef = static_cast<T>(0);
        return dataRef;
    }

    operator bool () const
    {
        return m_dataRef;
    }

    operator T () const
    {
        return m_dataRef;
    }

    T * operator & ()
    {
        return &m_dataRef;
    }

private:
    T m_dataRef;
};

/// @brief Создает указатель для объекта, полученного через функции вида "Get"
/// @note При создании указателя счетчик ссылок увеличивается
template<typename T>
CFTypePtr<T> CFTypePtrFromGet(T p)
{
    return CFTypePtr<T>(p, true);
}

/// @brief Создает указатель для объекта, полученного через функции вида "Create" или "Copy"
/// @note При создании указателя счетчик ссылок НЕ увеличивается
template<typename T>
CFTypePtr<T> CFTypePtrFromCreate(T p)
{
    return CFTypePtr<T>(p, false);
}

#endif // QTUTILS_CF_TYPE_PTR_H_INCLUDED

