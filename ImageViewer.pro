#-------------------------------------------------
#
# Project created by QtCreator 2017-02-25T14:20:14
#
#-------------------------------------------------

QT += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageViewer
TEMPLATE = app

INCLUDEPATH += src/ImageViewer

DEFINES += QT_NO_CAST_FROM_ASCII

include(src/QtUtils/QtUtils.pri)

SOURCES += \
    src/ImageViewer/MainWindow.cpp \
    src/ImageViewer/main.cpp \
    src/ImageViewer/ImageViewerWidget.cpp

HEADERS += \
    src/ImageViewer/MainWindow.h \
    src/ImageViewer/MainWindow_p.h \
    src/ImageViewer/ImageViewerWidget.h

TRANSLATIONS += \
    src/ImageViewer/resources/translations/en.ts \
    src/ImageViewer/resources/translations/ru.ts

win32 {
##    RC_FILE += src/ImageViewer/resources/platform/windows/resources.rc
    DEFINES += NOMINMAX
}

macx {
##    QMAKE_INFO_PLIST = src/ImageViewer/resources/platform/macosx/Info.plist
##    ICON = src/ImageViewer/resources/icon/ball.icns
    TARGET = "ImageViewer"
    QMAKE_CXXFLAGS += -Wno-invalid-constexpr
}

RESOURCES += \
    src/ImageViewer/resources/translations/translations.qrc
##    src/ImageViewer/resources/icon/icon.qrc

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
