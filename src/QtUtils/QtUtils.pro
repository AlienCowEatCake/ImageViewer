#
#  Copyright (C) 2011-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>
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

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += warn_on

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_KEYWORDS

*g++*|*clang*|*llvm*|*xcode* {
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

macx : *clang* {
    QMAKE_CXXFLAGS += -Wno-invalid-constexpr
}

HEADERS += \
    $$files(src/Utils/*.h) \
    $$files(src/Widgets/*.h) \
    $$files(src/Workarounds/*.h)

SOURCES += \
    $$files(src/Utils/*.cpp) \
    $$files(src/Widgets/*.cpp)

TRANSLATIONS += \
    resources/translations/qtutils_en.ts \
    resources/translations/qtutils_ru.ts \
    resources/translations/qtutils_zh_CN.ts \
    resources/translations/qtutils_zh_TW.ts

macx {
    OBJECTIVE_SOURCES += \
        $$files(src/Utils/*.mm) \
        $$files(src/Workarounds/*.mm)
}

!greaterThan(QT_MAJOR_VERSION, 4) {
    BACKPORT50_JSON_PATH = $${PWD}/src/Utils/_backport/Qt5.0/json
    INCLUDEPATH += $${BACKPORT50_JSON_PATH}
    HEADERS += $$files($${BACKPORT50_JSON_PATH}/*.h)
    SOURCES += $$files($${BACKPORT50_JSON_PATH}/*.cpp)
}

enable_update_checking {
    QT += network

    HEADERS += \
        $$files(src/Updater/*.h)

    SOURCES += \
        $$files(src/Updater/*.cpp)
}

lupdate_only {
    SOURCES += $${OBJECTIVE_SOURCES}
    OBJECTIVE_SOURCES =
}

RESOURCES += \
    resources/icons/qtutils_icons.qrc \
    resources/style/qtutils_style.qrc \
    resources/translations/qtutils_translations.qrc

QMAKE_RESOURCE_FLAGS += -threshold 0 -compress 9
!lessThan(QT_MAJOR_VERSION, 5) {
    greaterThan(QT_MAJOR_VERSION, 5) | greaterThan(QT_MINOR_VERSION, 12) {
        QMAKE_RESOURCE_FLAGS += -compress-algo zlib
    }
}

