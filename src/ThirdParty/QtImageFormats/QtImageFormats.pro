# URL: https://github.com/qt/qtimageformats
# License: GNU GPL v2 or GNU LGPL v3 or Commercial - https://www.qt.io/licensing/

lessThan(QT_MAJOR_VERSION, 5): error(This project requires Qt 5 or later)

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_QtImageFormats

QT += core gui widgets

CONFIG -= warn_on
CONFIG += exceptions_off warn_off

THIRDPARTY_QTIMAGEFORMATS_PATH = $${PWD}/qtimageformats
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

INCLUDEPATH += $${THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH}

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/dds/ddsheader.cpp \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/dds/qddshandler.cpp

HEADERS += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/dds/ddsheader.h \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/dds/qddshandler.h

DEFINES += WRAPPER_USE_DDS_HANDLER

DEFINES += DDSPixelFormat=tp_DDSPixelFormat
DEFINES += DDSHeader=tp_DDSHeader
DEFINES += DDSHeaderDX10=tp_DDSHeaderDX10
DEFINES += QDDSHandler=tp_QDDSHandler

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/icns/qicnshandler.cpp

HEADERS += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/icns/qicnshandler_p.h

DEFINES += WRAPPER_USE_ICNS_HANDLER

DEFINES += ICNSBlockHeader=tp_ICNSBlockHeader
DEFINES += ICNSEntry=tp_ICNSEntry
DEFINES += QICNSHandler=tp_QICNSHandler

# --------------------------------------------------------------------------------

!disable_libjasper {

    SOURCES += \
        $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/jp2/qjp2handler.cpp

    HEADERS += \
        $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/jp2/qjp2handler_p.h

    DEFINES += WRAPPER_USE_JP2_HANDLER

    DEFINES += QJp2HandlerPrivate=tp_QJp2HandlerPrivate
    DEFINES += QJp2Handler=tp_QJp2Handler
    DEFINES += Jpeg2000JasperReader=tp_Jpeg2000JasperReader

}

# --------------------------------------------------------------------------------

!disable_libmng {

    SOURCES += \
        $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/mng/qmnghandler.cpp

    HEADERS += \
        $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/mng/qmnghandler_p.h

    DEFINES += WRAPPER_USE_MNG_HANDLER

    DEFINES += QMngHandlerPrivate=tp_QMngHandlerPrivate
    DEFINES += QMngHandler=tp_QMngHandler

}

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/tga/qtgafile.cpp \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/tga/qtgahandler.cpp

HEADERS += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/tga/qtgafile.h \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/tga/qtgahandler.h

DEFINES += WRAPPER_USE_TGA_HANDLER

DEFINES += QTgaFile=tp_QTgaFile
DEFINES += QTgaHandler=tp_QTgaHandler
DEFINES += TgaReader=tp_TgaReader
DEFINES += Tga16Reader=tp_Tga16Reader
DEFINES += Tga24Reader=tp_Tga24Reader
DEFINES += Tga32Reader=tp_Tga32Reader

# --------------------------------------------------------------------------------

!disable_libtiff {

    greaterThan(QT_MAJOR_VERSION, 5) | greaterThan(QT_MINOR_VERSION, 4) {

        SOURCES += \
            $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/tiff/qtiffhandler.cpp

        HEADERS += \
            $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/tiff/qtiffhandler_p.h

        DEFINES += WRAPPER_USE_TIFF_HANDLER

        DEFINES += QTiffHandlerPrivate=tp_QTiffHandlerPrivate
        DEFINES += QTiffHandler=tp_QTiffHandler
        DEFINES += qtiffReadProc=tp_qtiffReadProc
        DEFINES += qtiffWriteProc=tp_qtiffWriteProc
        DEFINES += qtiffSeekProc=tp_qtiffSeekProc
        DEFINES += qtiffCloseProc=tp_qtiffCloseProc
        DEFINES += qtiffSizeProc=tp_qtiffSizeProc
        DEFINES += qtiffMapProc=tp_qtiffMapProc
        DEFINES += qtiffUnmapProc=tp_qtiffUnmapProc

    }

}

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/wbmp/qwbmphandler.cpp

HEADERS += \
    $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/wbmp/qwbmphandler_p.h

DEFINES += WRAPPER_USE_WBMP_HANDLER

DEFINES += WBMPReader=tp_WBMPReader
DEFINES += QWbmpHandler=tp_QWbmpHandler

# --------------------------------------------------------------------------------

!disable_libwebp {

    SOURCES += \
        $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/webp/qwebphandler.cpp

    HEADERS += \
        $${THIRDPARTY_QTIMAGEFORMATS_PATH}/src/plugins/imageformats/webp/qwebphandler_p.h

    DEFINES += WRAPPER_USE_WEBP_HANDLER

    DEFINES += QWebpHandler=tp_QWebpHandler

}

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH}/QtImageFormatsImageReader.cpp \
    $${THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH}/QtImageFormatsMovie.cpp

HEADERS += \
    $${THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH}/QtImageFormatsImageReader.h \
    $${THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH}/QtImageFormatsMovie.h

# --------------------------------------------------------------------------------

