#
#  Copyright (C) 2017-2021 Peter S. Zhigalov <peter.zhigalov@gmail.com>
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

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_KEYWORDS

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

enable_update_checking {
    DEFINES += ENABLE_UPDATE_CHECKING
}

SOURCES += \
    src/GUI/FileManager.cpp \
    src/GUI/GUISettings.cpp \
    src/GUI/MainController.cpp \
    src/GUI/Dialogs/AboutDialog.cpp \
    src/GUI/Dialogs/InfoDialog.cpp \
    src/GUI/Dialogs/SettingsDialog.cpp \
    src/GUI/MainWindow/EffectsStorage.cpp \
    src/GUI/MainWindow/ImageViewerWidget.cpp \
    src/GUI/MainWindow/MainWindow.cpp \
    src/GUI/MainWindow/MenuBar.cpp \
    src/Decoders/DecodersManager.cpp \
    src/Decoders/Impl/DecoderQImage.cpp \
    src/Decoders/Impl/DecoderQMovie.cpp \
    src/Decoders/Impl/Internal/Animation/AnimationObject.cpp \
    src/Decoders/Impl/Internal/Animation/AnimationWidget.cpp \
    src/Decoders/Impl/Internal/Animation/DelayCalculator.cpp \
    src/Decoders/Impl/Internal/Animation/FramesCompositor.cpp \
    src/Decoders/Impl/Internal/Animation/AbstractAnimationProvider.cpp \
    src/Decoders/Impl/Internal/GraphicsItems/GraphicsItemUtils.cpp \
    src/Decoders/Impl/Internal/GraphicsItems/AbstractSVGWebBrowser.cpp \
    src/Decoders/Impl/Internal/GraphicsItems/AbstractSVGWebBrowserNoJS.cpp \
    src/Decoders/Impl/Internal/GraphicsItems/ProgressiveResampledImageGraphicsItem.cpp \
    src/Decoders/Impl/Internal/GraphicsItems/RasterizedImageGraphicsItem.cpp \
    src/Decoders/Impl/Internal/GraphicsItems/ResampledImageGraphicsItem.cpp \
    src/Decoders/Impl/Internal/Scaling/AbstractScalingManager.cpp \
    src/Decoders/Impl/Internal/Scaling/AbstractScalingWorker.cpp \
    src/Decoders/Impl/Internal/Scaling/AbstractScalingWorkerHandler.cpp \
    src/Decoders/Impl/Internal/Scaling/AutoUpdatedScalingWorkerHandler.cpp \
    src/Decoders/Impl/Internal/Utils/CmsUtils.cpp \
    src/Decoders/Impl/Internal/Utils/LibraryUtils.cpp \
    src/Decoders/Impl/Internal/Utils/MappedBuffer.cpp \
    src/Decoders/Impl/Internal/GraphicsItemsFactory.cpp \
    src/Decoders/Impl/Internal/ImageData.cpp \
    src/Decoders/Impl/Internal/ImageMetaData.cpp \
    src/main.cpp

HEADERS += \
    src/GUI/FileManager.h \
    src/GUI/GUISettings.h \
    src/GUI/MainController.h \
    src/GUI/UIState.h \
    src/GUI/Dialogs/AboutDialog.h \
    src/GUI/Dialogs/AboutDialog_p.h \
    src/GUI/Dialogs/InfoDialog.h \
    src/GUI/Dialogs/InfoDialog_p.h \
    src/GUI/Dialogs/SettingsDialog.h \
    src/GUI/Dialogs/SettingsDialog_p.h \
    src/GUI/MainWindow/EffectsStorage.h \
    src/GUI/MainWindow/IControlsContainer.h \
    src/GUI/MainWindow/ImageViewerWidget.h \
    src/GUI/MainWindow/MainWindow.h \
    src/GUI/MainWindow/MainWindow_p.h \
    src/GUI/MainWindow/MenuBar.h \
    src/Decoders/DecodersManager.h \
    src/Decoders/IDecoder.h \
    src/Decoders/IImageData.h \
    src/Decoders/IImageMetaData.h \
    src/Decoders/GraphicsItemFeatures/IGrabImage.h \
    src/Decoders/GraphicsItemFeatures/ITransformationMode.h \
    src/Decoders/Impl/Internal/Animation/AnimationObject.h \
    src/Decoders/Impl/Internal/Animation/AnimationWidget.h \
    src/Decoders/Impl/Internal/Animation/DelayCalculator.h \
    src/Decoders/Impl/Internal/Animation/FramesCompositor.h \
    src/Decoders/Impl/Internal/Animation/IAnimationProvider.h \
    src/Decoders/Impl/Internal/Animation/AbstractAnimationProvider.h \
    src/Decoders/Impl/Internal/Animation/MovieAnimationProvider.h \
    src/Decoders/Impl/Internal/GraphicsItems/GraphicsItemUtils.h \
    src/Decoders/Impl/Internal/GraphicsItems/AbstractSVGWebBrowser.h \
    src/Decoders/Impl/Internal/GraphicsItems/AbstractSVGWebBrowserNoJS.h \
    src/Decoders/Impl/Internal/GraphicsItems/ProgressiveResampledImageGraphicsItem.h \
    src/Decoders/Impl/Internal/GraphicsItems/RasterizedImageGraphicsItem.h \
    src/Decoders/Impl/Internal/GraphicsItems/ResampledImageGraphicsItem.h \
    src/Decoders/Impl/Internal/Scaling/AbstractProgressiveImageProvider.h \
    src/Decoders/Impl/Internal/Scaling/AbstractScalingManager.h \
    src/Decoders/Impl/Internal/Scaling/AbstractScalingWorker.h \
    src/Decoders/Impl/Internal/Scaling/AbstractScalingWorkerHandler.h \
    src/Decoders/Impl/Internal/Scaling/AutoUpdatedScalingWorkerHandler.h \
    src/Decoders/Impl/Internal/Scaling/IScaledImageProvider.h \
    src/Decoders/Impl/Internal/Utils/CmsUtils.h \
    src/Decoders/Impl/Internal/Utils/LibraryUtils.h \
    src/Decoders/Impl/Internal/Utils/MappedBuffer.h \
    src/Decoders/Impl/Internal/DecoderAutoRegistrator.h \
    src/Decoders/Impl/Internal/GraphicsItemsFactory.h \
    src/Decoders/Impl/Internal/ImageData.h \
    src/Decoders/Impl/Internal/ImageMetaData.h \
    src/Decoders/Impl/Internal/PayloadWithMetaData.h \
    src/Decoders/Impl/Internal/SVGWebBrowserDecoderTemplate.h

!disable_zlib {
    SOURCES += \
        src/Decoders/Impl/Internal/Utils/ZLibUtils.cpp
    HEADERS += \
        src/Decoders/Impl/Internal/Utils/ZLibUtils.h
}

!disable_stb {
    SOURCES += \
        src/Decoders/Impl/DecoderSTB.cpp
}

!disable_nanosvg {
    SOURCES += \
        src/Decoders/Impl/DecoderNanoSVG.cpp
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

!disable_lerc {
    SOURCES += \
        src/Decoders/Impl/DecoderLERC.cpp
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

!disable_openjpeg {
    SOURCES += \
        src/Decoders/Impl/DecoderOpenJPEG.cpp
}

!disable_giflib {
    SOURCES += \
        src/Decoders/Impl/DecoderGifLib.cpp
}

!disable_libraw {
    SOURCES += \
        src/Decoders/Impl/DecoderLibRaw.cpp
}

!disable_librsvg {
    SOURCES += \
        src/Decoders/Impl/DecoderLibRSVG.cpp
}

!disable_resvg {
    SOURCES += \
        src/Decoders/Impl/DecoderReSVGLt001100.cpp \
        src/Decoders/Impl/DecoderReSVGLt001300.cpp \
        src/Decoders/Impl/DecoderReSVG.cpp
}

!disable_libheif {
    SOURCES += \
        src/Decoders/Impl/DecoderLibHEIF.cpp
}

!disable_openexr {
    SOURCES += \
        src/Decoders/Impl/DecoderOpenEXR.cpp
}

!disable_libavif {
    SOURCES += \
        src/Decoders/Impl/DecoderLibAvif.cpp
}

!disable_flif {
    SOURCES += \
        src/Decoders/Impl/DecoderFLIF.cpp
}

!disable_jxrlib {
    SOURCES += \
        src/Decoders/Impl/DecoderJxrLib.cpp
}

!disable_magickcore {
    SOURCES += \
        src/Decoders/Impl/DecoderMagickCore.cpp
}

!disable_magickwand {
    SOURCES += \
        src/Decoders/Impl/DecoderMagickWand.cpp
}

!disable_graphicsmagick {
    SOURCES += \
        src/Decoders/Impl/DecoderGraphicsMagick.cpp
}

!disable_graphicsmagickwand {
    SOURCES += \
        src/Decoders/Impl/DecoderGraphicsMagickWand.cpp
}

!disable_qtsvg {
    QT += svg
    greaterThan(QT_MAJOR_VERSION, 5): QT += svgwidgets
    SOURCES += \
        src/Decoders/Impl/DecoderQtSVG.cpp
}

!disable_qtwebkit {
    QT +=  webkit network
    greaterThan(QT_MAJOR_VERSION, 4): QT += webkitwidgets
    SOURCES += \
        src/Decoders/Impl/Internal/GraphicsItems/QtWebKitSVGGraphicsItem.cpp \
        src/Decoders/Impl/DecoderQtWebKit.cpp
    HEADERS += \
        src/Decoders/Impl/Internal/GraphicsItems/QtWebKitSVGGraphicsItem.h
}

!disable_qtwebengine {
    QT +=  webenginewidgets
    SOURCES += \
        src/Decoders/Impl/Internal/GraphicsItems/QtWebEngineSVGGraphicsItem.cpp \
        src/Decoders/Impl/DecoderQtWebEngine.cpp
    HEADERS += \
        src/Decoders/Impl/Internal/GraphicsItems/QtWebEngineSVGGraphicsItem.h
}

!disable_qmlwebengine {
    QT += webengine quick
    SOURCES += \
        src/Decoders/Impl/Internal/GraphicsItems/QMLWebEngineSVGGraphicsItem.cpp \
        src/Decoders/Impl/DecoderQMLWebEngine.cpp
    HEADERS += \
        src/Decoders/Impl/Internal/GraphicsItems/QMLWebEngineSVGGraphicsItem.h
}

!disable_mshtml {
    SOURCES += \
        src/Decoders/Impl/DecoderMSHTML.cpp
    *g++*|*clang* {
        LIBS += -lgdi32
    } else {
        LIBS += gdi32.lib
    }
    *msvc* {
        QMAKE_CXXFLAGS_RELEASE -= -Zc:strictStrings
        QMAKE_CFLAGS_RELEASE -= -Zc:strictStrings
        QMAKE_CFLAGS -= -Zc:strictStrings
        QMAKE_CXXFLAGS -= -Zc:strictStrings
    }
}

!disable_wic {
    SOURCES += \
        src/Decoders/Impl/DecoderWIC.cpp
    *msvc* {
        QMAKE_CXXFLAGS_RELEASE -= -Zc:strictStrings
        QMAKE_CFLAGS_RELEASE -= -Zc:strictStrings
        QMAKE_CFLAGS -= -Zc:strictStrings
        QMAKE_CXXFLAGS -= -Zc:strictStrings
    }
}

!disable_nsimage {
    OBJECTIVE_SOURCES += \
        src/Decoders/Impl/DecoderNSImage.mm
}

!disable_macwebkit {
    OBJECTIVE_SOURCES += \
        src/Decoders/Impl/Internal/GraphicsItems/MacWebKitRasterizerGraphicsItem.mm \
        src/Decoders/Impl/DecoderMacWebKit.mm
    HEADERS += \
        src/Decoders/Impl/Internal/GraphicsItems/MacWebKitRasterizerGraphicsItem.h
    LIBS += -framework WebKit
}

!disable_mactoolbar {
    DEFINES += HAS_MAC_TOOLBAR
    OBJECTIVE_SOURCES += \
        src/GUI/MainWindow/MacToolBar.mm
    HEADERS += \
        src/GUI/MainWindow/MacToolBar.h
} else {
    SOURCES += \
        src/GUI/MainWindow/ToolBar.cpp
    HEADERS += \
        src/GUI/MainWindow/ToolBar.h
}

!disable_mactouchbar {
    DEFINES += HAS_MAC_TOUCHBAR
    OBJECTIVE_SOURCES += \
        src/GUI/MainWindow/MacTouchBar.mm
    HEADERS += \
        src/GUI/MainWindow/MacTouchBar.h
}

TRANSLATIONS += \
    resources/translations/imageviewer_en.ts \
    resources/translations/imageviewer_ru.ts

win32 {
    RC_FILE += resources/platform/windows/Resources.rc
    DEFINES += NOMINMAX
}

macx {
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
