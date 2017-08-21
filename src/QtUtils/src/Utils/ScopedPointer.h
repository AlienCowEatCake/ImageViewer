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

#include "_backport/Qt4.6/qscopedpointer.h"

#endif

#endif // QTUTILS_SCOPEDPOINTER_H_INCLUDED

