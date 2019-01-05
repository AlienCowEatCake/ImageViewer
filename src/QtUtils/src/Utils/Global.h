/*
   Copyright (C) 2018-2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (QTUTILS_GLOBAL_H_INCLUDED)
#define QTUTILS_GLOBAL_H_INCLUDED

#include <QtGlobal>

#if (QT_VERSION < QT_VERSION_CHECK(4, 6, 0))

static inline bool qFuzzyIsNull(double d)
{
    return qAbs(d) <= 0.000000000001;
}

static inline bool qFuzzyIsNull(float f)
{
    return qAbs(f) <= 0.00001f;
}

#endif

#if !defined (Q_DECL_OVERRIDE)
#define Q_DECL_OVERRIDE
#endif

#endif // QTUTILS_GLOBAL_H_INCLUDED

