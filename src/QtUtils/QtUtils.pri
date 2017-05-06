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
    LIBS += -framework AppKit
}

win32 {
    *g++*|*clang* {
        LIBS += -lshell32 -luser32
    } else {
        LIBS += shell32.lib user32.lib
    }
}

OUT_LIB_TARGET = QtUtils
OUT_LIB_DIR = $${OUT_PWD}/../QtUtils
OUT_LIB_NAME =
OUT_LIB_LINK =
win32 {
    CONFIG(release, debug|release) {
        OUT_LIB_DIR = $${OUT_LIB_DIR}/release
    } else:CONFIG(debug, debug|release) {
        OUT_LIB_DIR = $${OUT_LIB_DIR}/debug
    }
    *g++*|*clang* {
        OUT_LIB_NAME = lib$${OUT_LIB_TARGET}.a
        OUT_LIB_LINK = -l$${OUT_LIB_TARGET}
    } else {
        OUT_LIB_NAME = $${OUT_LIB_TARGET}.lib
        OUT_LIB_LINK = $${OUT_LIB_NAME}
    }
} else {
    OUT_LIB_DIR = $${OUT_LIB_DIR}
    OUT_LIB_NAME = lib$${OUT_LIB_TARGET}.a
    OUT_LIB_LINK = -l$${OUT_LIB_TARGET}
}
LIBS += -L$${OUT_LIB_DIR} $${OUT_LIB_LINK}
PRE_TARGETDEPS += $${OUT_LIB_DIR}/$${OUT_LIB_NAME}

