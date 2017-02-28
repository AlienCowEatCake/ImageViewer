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

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
CONFIG += object_with_source object_parallel_to_source no_batch warn_on

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

*g++*|*clang* {
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE *= -O3
    QMAKE_CXXFLAGS_RELEASE *= -DNDEBUG
    QMAKE_CXXFLAGS_RELEASE *= -DQT_NO_DEBUG_OUTPUT
}

*msvc* {
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE *= -Ox
    QMAKE_CXXFLAGS_RELEASE -= -GS
    QMAKE_CXXFLAGS_RELEASE *= -GS-
    QMAKE_CXXFLAGS_RELEASE *= -DQT_NO_DEBUG_OUTPUT
    DEFINES += _CRT_SECURE_NO_WARNINGS
    DEFINES += _CRT_SECURE_NO_DEPRECATE
    DEFINES += _USE_MATH_DEFINES
}

win32 {
    DEFINES += NOMINMAX
}

HEADERS += \
    $$files($$PWD/Themes/*.h) \
    $$files($$PWD/Utils/*.h)

SOURCES += \
    $$files($$PWD/Themes/*.cpp) \
    $$files($$PWD/Utils/*.cpp)

RESOURCES += \
    $$files($$PWD/Themes/icons/*.qrc)

macx {
    OBJECTIVE_SOURCES += \
        $$files($$PWD/Utils/*.mm)
    LIBS += -framework Foundation
    LIBS += -framework CoreServices
}

win32 {
    *g++*|*clang* {
        QMAKE_LIBS += -lshell32
    } else {
        QMAKE_LIBS += shell32.lib
    }
}
