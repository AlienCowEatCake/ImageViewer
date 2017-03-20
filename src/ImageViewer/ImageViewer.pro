#
#  Copyright (C) 2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>
#
#  This file is part of the `ImageViewer' program.
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

TEMPLATE = app
TARGET = ImageViewer

INCLUDEPATH += src

QT += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += warn_on

DEFINES += QT_NO_CAST_FROM_ASCII

include(../QtUtils/QtUtils.pri)
include(../ThirdParty/QtExtended/QtExtended.pri)
include(../ThirdParty/STB/STB.pri)
greaterThan(QT_MAJOR_VERSION, 4) {
    include(../ThirdParty/QtImageFormats/QtImageFormats.pri)
}

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

SOURCES += \
    src/GUI/GUISettings.cpp \
    src/GUI/MainWindow.cpp \
    src/GUI/ImageViewerWidget.cpp \
    src/GUI/SettingsDialog.cpp \
    src/Decoders/DecodersManager.cpp \
    src/Decoders/DecoderQImage.cpp \
    src/Decoders/DecoderQMovie.cpp \
    src/Decoders/DecoderQtSVG.cpp \
    src/Decoders/Internal/ExifUtils.cpp \
    src/main.cpp

HEADERS += \
    src/GUI/GUISettings.h \
    src/GUI/MainWindow.h \
    src/GUI/MainWindow_p.h \
    src/GUI/ImageViewerWidget.h \
    src/GUI/SettingsDialog.h \
    src/GUI/SettingsDialog_p.h \
    src/Decoders/DecodersManager.h \
    src/Decoders/IDecoder.h \
    src/Decoders/DecoderQImage.h \
    src/Decoders/DecoderQMovie.h \
    src/Decoders/DecoderQtSVG.h \
    src/Decoders/Internal/DecoderAutoRegistrator.h \
    src/Decoders/Internal/ExifUtils.h

has_thirdparty_stb {
    SOURCES += \
        src/Decoders/DecoderSTB.cpp
    HEADERS += \
        src/Decoders/DecoderSTB.h
}

has_thirdparty_qtimageformats {
    SOURCES += \
        src/Decoders/DecoderQtImageFormatsImage.cpp \
        src/Decoders/DecoderQtImageFormatsMovie.cpp
    HEADERS += \
        src/Decoders/DecoderQtImageFormatsImage.h \
        src/Decoders/DecoderQtImageFormatsMovie.h
}

TRANSLATIONS += \
    resources/translations/imageviewer_en.ts \
    resources/translations/imageviewer_ru.ts

win32 {
    RC_FILE += resources/platform/windows/Resources.rc
    DEFINES += NOMINMAX
}

macx {
    greaterThan(QT_MAJOR_VERSION, 4): QT += macextras
    OBJECTIVE_SOURCES += \
        src/Decoders/Internal/MacImageUtils.mm \
        src/Decoders/DecoderNSImage.mm \
        src/Decoders/DecoderMacWebKit.mm
    HEADERS += \
        src/Decoders/Internal/MacImageUtils.h \
        src/Decoders/DecoderNSImage.h \
        src/Decoders/DecoderMacWebKit.h
    LIBS += -framework AppKit
    LIBS += -framework Foundation
    LIBS += -framework WebKit

    QMAKE_INFO_PLIST = resources/platform/macosx/Info.plist
    ICON = resources/icon/icon.icns
    TARGET = "Image Viewer"
    QMAKE_CXXFLAGS += -Wno-invalid-constexpr
}

RESOURCES += \
    resources/translations/imageviewer_translations.qrc \
    resources/icon/icon.qrc \
    resources/style/style.qrc

QMAKE_RESOURCE_FLAGS += -threshold 0 -compress 9

# qmake CONFIG+=use_static_qjpeg
use_static_qjpeg {
    QTPLUGIN += qjpeg
    DEFINES += USE_STATIC_QJPEG
}

# qmake CONFIG+=use_static_qtiff
use_static_qtiff {
    QTPLUGIN += qtiff
    DEFINES += USE_STATIC_QTIFF
}

# qmake CONFIG+=use_static_qico
use_static_qico {
    QTPLUGIN += qico
    DEFINES += USE_STATIC_QICO
}
