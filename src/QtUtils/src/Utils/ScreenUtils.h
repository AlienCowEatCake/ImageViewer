/*
   Copyright (C) 2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (QTUTILS_SCREENUTILS_H_INCLUDED)
#define QTUTILS_SCREENUTILS_H_INCLUDED

#include <QList>

class QPoint;
class QRect;

class Screen
{
    friend class ScreenProvider;

private:
    Screen();
    explicit Screen(quintptr underlying);

public:
    Screen(const Screen &other);
    ~Screen();

    Screen &operator = (const Screen &other);

    QRect geometry() const;
    QRect availableGeometry() const;

private:
    quintptr m_underlying;
};

class ScreenProvider
{
public:
    static QList<Screen> screens();
    static Screen primaryScreen();
};

#endif // QTUTILS_SCREENUTILS_H_INCLUDED

