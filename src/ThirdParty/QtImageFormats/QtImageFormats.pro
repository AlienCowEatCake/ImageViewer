# https://github.com/qt/qtimageformats

TEMPLATE = lib
CONFIG += staticlib
TARGET = QtImageFormats

THIRDPARTY_QTIMAGEFORMATS_PATH = $${PWD}/qtimageformats
THIRDPARTY_QTIMAGEFORMATS_QTBASE_PATH = $${PWD}/qtbase
THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH = $${PWD}/wrapper

INCLUDEPATH += $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins

QT += core gui

CONFIG -= warn_on

*g++*|*clang* {
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE *= -O3
    QMAKE_CXXFLAGS_RELEASE *= -DNDEBUG
    QMAKE_CXXFLAGS_RELEASE *= -DQT_NO_DEBUG_OUTPUT
    QMAKE_CFLAGS += -w
}

*msvc* {
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE *= -Ox
    QMAKE_CXXFLAGS_RELEASE -= -GS
    QMAKE_CXXFLAGS_RELEASE *= -GS-
    QMAKE_CXXFLAGS_RELEASE *= -DQT_NO_DEBUG_OUTPUT
}

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/dds/ddsheader.cpp \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/dds/qddshandler.cpp

HEADERS += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/dds/ddsheader.h \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/dds/qddshandler.h

DEFINES += WRAPPER_USE_DDS_HANDLER

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/icns/qicnshandler.cpp

HEADERS += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/icns/qicnshandler_p.h

DEFINES += WRAPPER_USE_ICNS_HANDLER

# --------------------------------------------------------------------------------

include($${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/jp2/qjp2handler.pri)

DEFINES += WRAPPER_USE_JP2_HANDLER

# --------------------------------------------------------------------------------

include($${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/mng/qmnghandler.pri)

DEFINES += WRAPPER_USE_MNG_HANDLER

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/tga/qtgafile.cpp \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/tga/qtgahandler.cpp

HEADERS += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/tga/qtgafile.h \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/tga/qtgahandler.h

DEFINES += WRAPPER_USE_TGA_HANDLER

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/tiff/qtiffhandler.cpp

HEADERS += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/tiff/qtiffhandler_p.h

config_libtiff {
    unix|mingw: LIBS += -ltiff
    else:win32: LIBS += libtiff.lib
} else {
    include($${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/3rdparty/libtiff.pri)
}

DEFINES += WRAPPER_USE_TIFF_HANDLER

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/wbmp/qwbmphandler.cpp

HEADERS += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/wbmp/qwbmphandler_p.h

DEFINES += WRAPPER_USE_WBMP_HANDLER

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/webp/qwebphandler.cpp

HEADERS += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/webp/qwebphandler_p.h

config_libwebp {
    unix|win32-g++*: LIBS += -lwebp -lwebpdemux
    else:win32: LIBS += libwebp.lib libwebpdemux.lib
} else {
    include($${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/3rdparty/libwebp.pri)
}

DEFINES += WRAPPER_USE_WEBP_HANDLER

# --------------------------------------------------------------------------------

include($${THIRDPARTY_QTIMAGEFORMATS_QTBASE_PATH}/src/3rdparty/zlib.pri)

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH}/QtImageFormatsReaderWrapper.cpp

HEADERS += \
    $${THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH}/QtImageFormatsReaderWrapper.h

# --------------------------------------------------------------------------------

DESTDIR = .
OBJECTS_DIR = build/objects
MOC_DIR = build/moc
RCC_DIR = build/rcc
UI_DIR = build/ui

