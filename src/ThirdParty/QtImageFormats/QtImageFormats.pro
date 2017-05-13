# URL: https://github.com/qt/qtimageformats

lessThan(QT_MAJOR_VERSION, 5): error(This project requires Qt 5 or later)

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_QtImageFormats

QT += core gui widgets

CONFIG -= warn_on
CONFIG += exceptions_off warn_off

THIRDPARTY_QTIMAGEFORMATS_PATH = $${PWD}/qtimageformats
THIRDPARTY_QTIMAGEFORMATS_LEGACY_PATH = $${PWD}/qtimageformats_legacy
THIRDPARTY_QTIMAGEFORMATS_QTBASE_PATH = $${PWD}/qtbase
THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH = $${PWD}/wrapper

include(../../Features.pri)
include(../CommonSettings.pri)
include(../libmng/libmng.pri)
include(../JasPer/JasPer.pri)
include(../libtiff/libtiff.pri)
include(../libjpeg/libjpeg.pri)
include(../LibWebP/LibWebP.pri)
include(../LittleCMS2/LittleCMS2.pri)
include(../libexif/libexif.pri)
include(../zlib/zlib.pri)

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

!disable_libjasper {

    include($${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/jp2/qjp2handler.pri)

    DEFINES += WRAPPER_USE_JP2_HANDLER

}

# --------------------------------------------------------------------------------

!disable_zlib {

    include($${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/mng/qmnghandler.pri)

    DEFINES += WRAPPER_USE_MNG_HANDLER

}

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/tga/qtgafile.cpp \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/tga/qtgahandler.cpp

HEADERS += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/tga/qtgafile.h \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/tga/qtgahandler.h

DEFINES += WRAPPER_USE_TGA_HANDLER

# --------------------------------------------------------------------------------

!disable_libtiff {

    greaterThan(QT_MAJOR_VERSION, 5) | greaterThan(QT_MINOR_VERSION, 4) {

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

}

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/wbmp/qwbmphandler.cpp

HEADERS += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/wbmp/qwbmphandler_p.h

DEFINES += WRAPPER_USE_WBMP_HANDLER

# --------------------------------------------------------------------------------

!disable_libwebp {

    SOURCES += \
        $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/webp/qwebphandler.cpp

    HEADERS += \
        $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/webp/qwebphandler_p.h

    DEFINES += WRAPPER_USE_WEBP_HANDLER

}

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

