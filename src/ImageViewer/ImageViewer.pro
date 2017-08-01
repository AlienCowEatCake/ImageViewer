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

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += warn_on

DEFINES += QT_NO_CAST_FROM_ASCII

include(../Features.pri)
include(../QtUtils/QtUtils.pri)
include(../ThirdParty/ThirdParty.pri)

*g++*|*clang* {
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE *= -O3
    QMAKE_CXXFLAGS_RELEASE *= -DNDEBUG
    QMAKE_CXXFLAGS_RELEASE *= -DQT_NO_DEBUG_OUTPUT
#    QMAKE_LFLAGS += -Wl,--whole-archive
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
    src/GUI/AboutDialog.cpp \
    src/Decoders/DecodersManager.cpp \
    src/Decoders/Impl/DecoderQImage.cpp \
    src/Decoders/Impl/DecoderQMovie.cpp \
    src/Decoders/Impl/Internal/Animation/AnimationObject.cpp \
    src/Decoders/Impl/Internal/Animation/AnimationUtils.cpp \
    src/Decoders/Impl/Internal/Animation/AnimationWidget.cpp \
    src/Decoders/Impl/Internal/Animation/FramesCompositor.cpp \
    src/Decoders/Impl/Internal/Animation/AbstractAnimationProvider.cpp \
    src/Decoders/Impl/Internal/Utils/ExifUtils.cpp \
    src/Decoders/Impl/Internal/Utils/CmsUtils.cpp \
    src/Decoders/Impl/Internal/Utils/ZLibUtils.cpp \
    src/main.cpp

HEADERS += \
    src/GUI/GUISettings.h \
    src/GUI/MainWindow.h \
    src/GUI/MainWindow_p.h \
    src/GUI/ImageViewerWidget.h \
    src/GUI/SettingsDialog.h \
    src/GUI/SettingsDialog_p.h \
    src/GUI/AboutDialog.h \
    src/GUI/AboutDialog_p.h \
    src/Decoders/DecodersManager.h \
    src/Decoders/IDecoder.h \
    src/Decoders/Impl/Internal/Animation/AnimationObject.h \
    src/Decoders/Impl/Internal/Animation/AnimationUtils.h \
    src/Decoders/Impl/Internal/Animation/AnimationWidget.h \
    src/Decoders/Impl/Internal/Animation/FramesCompositor.h \
    src/Decoders/Impl/Internal/Animation/IAnimationProvider.h \
    src/Decoders/Impl/Internal/Animation/AbstractAnimationProvider.h \
    src/Decoders/Impl/Internal/DecoderAutoRegistrator.h \
    src/Decoders/Impl/Internal/Utils/ExifUtils.h \
    src/Decoders/Impl/Internal/Utils/CmsUtils.h \
    src/Decoders/Impl/Internal/Utils/ZLibUtils.h

!disable_stb {
    SOURCES += \
        src/Decoders/Impl/DecoderSTB.cpp
}

!disable_qtimageformats {
    SOURCES += \
        src/Decoders/Impl/DecoderQtImageFormatsImage.cpp \
        src/Decoders/Impl/DecoderQtImageFormatsMovie.cpp
}

!disable_libjpeg {
    SOURCES += \
        src/Decoders/Impl/DecoderLibJpeg.cpp
}

!disable_libmng {
    SOURCES += \
        src/Decoders/Impl/DecoderLibMng.cpp
}

!disable_libpng {
    SOURCES += \
        src/Decoders/Impl/DecoderLibPng.cpp
}

!disable_libjasper {
    SOURCES += \
        src/Decoders/Impl/DecoderLibJasPer.cpp
}

!disable_jbigkit {
    SOURCES += \
        src/Decoders/Impl/DecoderJbigKit.cpp
}

!disable_libtiff {
    SOURCES += \
        src/Decoders/Impl/DecoderLibTiff.cpp
}

!disable_libwebp {
    SOURCES += \
        src/Decoders/Impl/DecoderLibWebP.cpp
}

!disable_libbpg {
    SOURCES += \
        src/Decoders/Impl/DecoderLibBpg.cpp
}

!disable_libwmf {
    SOURCES += \
        src/Decoders/Impl/DecoderLibWmf.cpp
}

!disable_qtsvg {
    QT += svg
    SOURCES += \
        src/Decoders/Impl/DecoderQtSVG.cpp
}

!disable_nsimage {
    OBJECTIVE_SOURCES += \
        src/Decoders/Impl/DecoderNSImage.mm
}

!disable_macwebkit {
    OBJECTIVE_SOURCES += \
        src/Decoders/Impl/Internal/MacWebKitRasterizerGraphicsItem.mm \
        src/Decoders/Impl/DecoderMacWebKit.mm
    HEADERS += \
        src/Decoders/Impl/Internal/MacWebKitRasterizerGraphicsItem.h
    LIBS += -framework WebKit
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
        src/Decoders/Impl/Internal/Utils/MacImageUtils.mm
    HEADERS += \
        src/Decoders/Impl/Internal/Utils/MacImageUtils.h
    LIBS += -framework AppKit
    LIBS += -framework Foundation

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

# qmake CONFIG+=use_static_qgif
use_static_qgif {
    QTPLUGIN += qgif
    DEFINES += USE_STATIC_QGIF
}

# qmake CONFIG+=use_static_qmng
use_static_qmng {
    QTPLUGIN += qmng
    DEFINES += USE_STATIC_QMNG
}

# qmake CONFIG+=use_static_qsvg
use_static_qsvg {
    QTPLUGIN += qsvg
    DEFINES += USE_STATIC_QSVG
}
