# https://github.com/qt/qtimageformats

lessThan(QT_MAJOR_VERSION, 5): error(This project requires Qt 5 or later)

TEMPLATE = lib
CONFIG += staticlib
TARGET = QtImageFormats

THIRDPARTY_QTIMAGEFORMATS_PATH = $${PWD}/qtimageformats
THIRDPARTY_QTIMAGEFORMATS_LEGACY_PATH = $${PWD}/qtimageformats_legacy
THIRDPARTY_QTIMAGEFORMATS_QTBASE_PATH = $${PWD}/qtbase
THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH = $${PWD}/wrapper

QT += core gui widgets

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

greaterThan(QT_MAJOR_VERSION, 5) | greaterThan(QT_MINOR_VERSION_VERSION, 4) {

    SOURCES += \
        $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/tiff/qtiffhandler.cpp

    HEADERS += \
        $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/tiff/qtiffhandler_p.h

    DEFINES += WRAPPER_USE_TIFF_HANDLER

} else {

    SOURCES += \
        $${THIRDPARTY_QTIMAGEFORMATS_LEGACY_PATH}/src/plugins/imageformats/tiff/qtiffhandler.cpp

    HEADERS += \
        $${THIRDPARTY_QTIMAGEFORMATS_LEGACY_PATH}/src/plugins/imageformats/tiff/qtiffhandler_p.h

    DEFINES += WRAPPER_USE_LEGACY_TIFF_HANDLER

}

include($${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/3rdparty/libtiff.pri)

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

include($${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/3rdparty/libwebp.pri)

DEFINES += WRAPPER_USE_WEBP_HANDLER

# --------------------------------------------------------------------------------

include($${THIRDPARTY_QTIMAGEFORMATS_QTBASE_PATH}/src/3rdparty/zlib.pri)

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH}/QtImageFormatsImageReader.cpp \
    $${THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH}/QtImageFormatsMovie.cpp \
    $${THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH}/QtImageFormatsMovieLabel.cpp

HEADERS += \
    $${THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH}/QtImageFormatsImageReader.h \
    $${THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH}/QtImageFormatsMovie.h \
    $${THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH}/QtImageFormatsMovieLabel.h

# --------------------------------------------------------------------------------

DESTDIR = .
OBJECTS_DIR = build/objects
MOC_DIR = build/moc
RCC_DIR = build/rcc
UI_DIR = build/ui

