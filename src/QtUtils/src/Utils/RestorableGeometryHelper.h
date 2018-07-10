/*
   Copyright (C) 2017-2018 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (QTUTILS_RESTORABLE_GEOMETRY_HELPER_H_INCLUDED)
#define QTUTILS_RESTORABLE_GEOMETRY_HELPER_H_INCLUDED

#include "ScopedPointer.h"

class QWidget;
class QByteArray;

class RestorableGeometryHelper
{
    Q_DISABLE_COPY(RestorableGeometryHelper)

public:
    RestorableGeometryHelper(QWidget *window);
    ~RestorableGeometryHelper();

    QByteArray serialize() const;
    void deserialize(const QByteArray &data);

    void saveGeometry();
    void restoreGeometry();

    void block();
    void unblock();

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // QTUTILS_RESTORABLE_GEOMETRY_HELPER_H_INCLUDED

