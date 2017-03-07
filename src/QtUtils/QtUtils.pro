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

TEMPLATE = lib
TARGET = QtUtils
CONFIG += staticlib

INCLUDEPATH += src

QT += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += object_with_source object_parallel_to_source no_batch warn_on

DEFINES += QT_NO_CAST_FROM_ASCII

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
    $$files(src/Utils/*.h)

SOURCES += \
    $$files(src/Utils/*.cpp)

TRANSLATIONS += \
    resources/translations/qtutils_en.ts \
    resources/translations/qtutils_ru.ts

macx {
    OBJECTIVE_SOURCES += \
        $$files(src/Utils/*.mm)
}

RESOURCES += \
    resources/icons/qtutils_icons_png.qrc \
    resources/icons/qtutils_icons_svg.qrc \
    resources/translations/qtutils_translations.qrc

QMAKE_RESOURCE_FLAGS += -threshold 0 -compress 9

DESTDIR = .
OBJECTS_DIR = build/objects
MOC_DIR = build/moc
RCC_DIR = build/rcc
UI_DIR = build/ui

