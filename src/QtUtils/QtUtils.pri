#
#  Copyright (C) 2011-2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>
#
#  This file is part of the `QtUtils' library.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

INCLUDEPATH += $${PWD}/src
DEPENDPATH += $${PWD}/src

QT += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

macx {
    LIBS += -framework Foundation
    LIBS += -framework CoreServices
}

win32 {
    *g++*|*clang* {
        LIBS += -lshell32
    } else {
        LIBS += shell32.lib
    }
}

LIBS += -L$${OUT_PWD}/../QtUtils
*g++*|*clang* {
    LIBS += -lQtUtils
} else {
    LIBS += QtUtils.lib
}

