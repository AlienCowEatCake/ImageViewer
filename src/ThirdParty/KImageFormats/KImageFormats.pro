# URL: https://invent.kde.org/frameworks/kimageformats + https://github.com/KDE/kimageformats
# License: GNU LGPL v2.1 - https://invent.kde.org/frameworks/kimageformats/-/blob/master/README.md

lessThan(QT_MAJOR_VERSION, 5): error(This project requires Qt 5.15 or later)
equals(QT_MAJOR_VERSION, 5) : lessThan(QT_MINOR_VERSION, 15) : error(This project requires Qt 5.15 or later)

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_KImageFormats

QT += core gui widgets
#QT += printsupport

CONFIG -= warn_on
CONFIG += warn_off

THIRDPARTY_KIMAGEFORMATS_PATH = $${PWD}/kimageformats-master
THIRDPARTY_KIMAGEFORMATS_WRAPPER_PATH = $${PWD}/wrapper

include(../../Features.pri)
include(../CommonSettings.pri)
include(../libavif/libavif.pri)
include(../OpenEXR/OpenEXR.pri)
include(../libheif/libheif.pri)
include(../libjxl/libjxl.pri)
include(../LibRaw/LibRaw.pri)
include(../jxrlib/jxrlib.pri)
include(../OpenJPEG/OpenJPEG.pri)

INCLUDEPATH += $${THIRDPARTY_KIMAGEFORMATS_WRAPPER_PATH}

#INCLUDEPATH += /usr/include/KF5/KArchive

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/microexif.cpp \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/scanlineconverter.cpp \

HEADERS += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/fastmath_p.h \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/gimp_p.h \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/microexif_p.h \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/rle_p.h \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/scanlineconverter_p.h \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/util_p.h \

DEFINES += ScanLineConverter=tp_ScanLineConverter
DEFINES += MicroExif=tp_MicroExif

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/ani.cpp

HEADERS += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/ani_p.h

DEFINES += WRAPPER_USE_ANI_HANDLER

DEFINES += ANIHandler=tp_ANIHandler
DEFINES += ANIPlugin=tp_ANIPlugin

# --------------------------------------------------------------------------------

#!disable_libavif {
#    SOURCES += \
#        $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/avif.cpp
#
#    HEADERS += \
#        $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/avif_p.h
#
#    DEFINES += WRAPPER_USE_AVIF_HANDLER
#
#    DEFINES += QAVIFHandler=tp_QAVIFHandler
#    DEFINES += QAVIFPlugin=tp_QAVIFPlugin
#}

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/dds.cpp

HEADERS += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/dds_p.h

DEFINES += WRAPPER_USE_DDS_HANDLER

DEFINES += QDDSHandler=tp_QDDSHandler
DEFINES += QDDSPlugin=tp_QDDSPlugin
DEFINES += writeA16B16G16R16F=tp_writeA16B16G16R16F
DEFINES += writeA32B32G32R32F=tp_writeA32B32G32R32F
DEFINES += writeA8R8G8B8=tp_writeA8R8G8B8
DEFINES += writeL8=tp_writeL8
DEFINES += writeP8=tp_writeP8
DEFINES += writeR8G8B8=tp_writeR8G8B8
DEFINES += DDSPixelFormat=tp_DDSPixelFormat
DEFINES += DDSHeaderDX10=tp_DDSHeaderDX10
DEFINES += DDSHeader=tp_DDSHeader

# --------------------------------------------------------------------------------

#SOURCES += \
#    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/eps.cpp
#
#HEADERS += \
#    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/eps_p.h
#
#DEFINES += WRAPPER_USE_EPS_HANDLER
#
#DEFINES += EPSHandler=tp_EPSHandler
#DEFINES += EPSPlugin=tp_EPSPlugin
#DEFINES += EPSPLUGIN=tp_EPSPLUGIN

# --------------------------------------------------------------------------------

!disable_openexr {
    SOURCES += \
        $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/exr.cpp

    HEADERS += \
        $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/exr_p.h

    DEFINES += WRAPPER_USE_EXR_HANDLER

    DEFINES += EXRHandler=tp_EXRHandler
    DEFINES += EXRPlugin=tp_EXRPlugin
    DEFINES += K_IStream=tp_K_IStream
    DEFINES += K_OStream=tp_K_OStream
    DEFINES += printAttributes=tp_printAttributes
    DEFINES += RgbaToQrgba=tp_RgbaToQrgba
    DEFINES += makePreview=tp_makePreview
}

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/hdr.cpp

HEADERS += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/hdr_p.h

DEFINES += WRAPPER_USE_HDR_HANDLER

DEFINES += HDRHandler=tp_HDRHandler
DEFINES += HDRPlugin=tp_HDRPlugin
DEFINES += HDRPLUGIN=tp_HDRPLUGIN
DEFINES += imageFormat=tp_imageFormat

# --------------------------------------------------------------------------------

!disable_libheif {
    SOURCES += \
        $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/heif.cpp

    HEADERS += \
        $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/heif_p.h

    DEFINES += WRAPPER_USE_HEIF_HANDLER

    DEFINES += HEIFHandler=tp_HEIFHandler
    DEFINES += HEIFPlugin=tp_HEIFPlugin
}

# --------------------------------------------------------------------------------

!disable_openjpeg {
    SOURCES += \
        $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/jp2.cpp

    HEADERS += \
        $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/jp2_p.h

    DEFINES += WRAPPER_USE_JP2_HANDLER

    DEFINES += JP2Handler=tp_JP2Handler
    DEFINES += JP2Plugin=tp_JP2Plugin
    DEFINES += JP2HandlerPrivate=tp_JP2HandlerPrivate
}

# --------------------------------------------------------------------------------

!disable_libjxl {
    SOURCES += \
        $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/jxl.cpp

    HEADERS += \
        $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/jxl_p.h

    DEFINES += WRAPPER_USE_JXL_HANDLER

    DEFINES += QJpegXLHandler=tp_QJpegXLHandler
    DEFINES += QJpegXLPlugin=tp_QJpegXLPlugin
    DEFINES += DISABLE_JXL_ENCODER DISABLE_JXL_THREADS
}

# --------------------------------------------------------------------------------

!disable_jxrlib {
    SOURCES += \
        $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/jxr.cpp

    HEADERS += \
        $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/jxr_p.h

    DEFINES += WRAPPER_USE_JXR_HANDLER

    DEFINES += JXRHandler=tp_JXRHandler
    DEFINES += JXRPlugin=tp_JXRPlugin
    DEFINES += JXRHandlerPrivate=tp_JXRHandlerPrivate
    DEFINES += LOG_JXRPLUGIN=tp_LOG_JXRPLUGIN
}

# --------------------------------------------------------------------------------

#SOURCES += \
#    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/kra.cpp
#
#HEADERS += \
#    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/kra.h
#
#DEFINES += WRAPPER_USE_KRA_HANDLER
#
#DEFINES += KraHandler=tp_KraHandler
#DEFINES += KraPlugin=tp_KraPlugin

# --------------------------------------------------------------------------------

#SOURCES += \
#    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/ora.cpp
#
#HEADERS += \
#    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/ora.h
#
#DEFINES += WRAPPER_USE_ORA_HANDLER
#
#DEFINES += OraHandler=tp_OraHandler
#DEFINES += OraPlugin=tp_OraPlugin

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/pcx.cpp

HEADERS += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/pcx_p.h

DEFINES += WRAPPER_USE_PCX_HANDLER

DEFINES += PCXHandler=tp_PCXHandler
DEFINES += PCXPlugin=tp_PCXPlugin
DEFINES += RGB=tp_RGB
DEFINES += Palette=tp_Palette
DEFINES += PCXHEADER=tp_PCXHEADER
DEFINES += peekHeader=tp_peekHeader

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/pfm.cpp

HEADERS += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/pfm_p.h

DEFINES += WRAPPER_USE_PFM_HANDLER

DEFINES += PFMHandler=tp_PFMHandler
DEFINES += PFMPlugin=tp_PFMPlugin
DEFINES += PFMHandlerPrivate=tp_PFMHandlerPrivate
DEFINES += LOG_PFMPLUGIN=tp_LOG_PFMPLUGIN
DEFINES += PFMHeader=tp_PFMHeader

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/pic.cpp

HEADERS += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/pic_p.h

DEFINES += WRAPPER_USE_PIC_HANDLER

DEFINES += SoftimagePICHandler=tp_SoftimagePICHandler
DEFINES += SoftimagePICPlugin=tp_SoftimagePICPlugin
DEFINES += PicHeader=tp_PicHeader
DEFINES += PicChannel=tp_PicChannel

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/psd.cpp

HEADERS += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/psd_p.h

DEFINES += WRAPPER_USE_PSD_HANDLER

DEFINES += PSDHandler=tp_PSDHandler
DEFINES += PSDPlugin=tp_PSDPlugin
DEFINES += PSDHandlerPrivate=tp_PSDHandlerPrivate

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/pxr.cpp

HEADERS += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/pxr_p.h

DEFINES += WRAPPER_USE_PXR_HANDLER

DEFINES += PXRHandler=tp_PXRHandler
DEFINES += PXRPlugin=tp_PXRPlugin
DEFINES += PXRHandlerPrivate=tp_PXRHandlerPrivate
DEFINES += LOG_PXRPLUGIN=tp_LOG_PXRPLUGIN
DEFINES += PXRHeader=tp_PXRHeader

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/qoi.cpp

HEADERS += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/qoi_p.h

DEFINES += WRAPPER_USE_QOI_HANDLER

DEFINES += QOIHandler=tp_QOIHandler
DEFINES += QOIPlugin=tp_QOIPlugin
DEFINES += QOIHandlerPrivate=tp_QOIHandlerPrivate

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/ras.cpp

HEADERS += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/ras_p.h

DEFINES += WRAPPER_USE_RAS_HANDLER

DEFINES += RASHandler=tp_RASHandler
DEFINES += RASPlugin=tp_RASPlugin
DEFINES += RASHandlerPrivate=tp_RASHandlerPrivate

# --------------------------------------------------------------------------------

!disable_libraw {
    SOURCES += \
        $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/raw.cpp

    HEADERS += \
        $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/raw_p.h

    DEFINES += WRAPPER_USE_RAW_HANDLER

    DEFINES += RAWHandler=tp_RAWHandler
    DEFINES += RAWPlugin=tp_RAWPlugin
}

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/rgb.cpp

HEADERS += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/rgb_p.h

DEFINES += WRAPPER_USE_RGB_HANDLER

DEFINES += RGBHandler=tp_RGBHandler
DEFINES += RGBPlugin=tp_RGBPlugin
DEFINES += RLEData=tp_RLEData
DEFINES += RLEMap=tp_RLEMap
DEFINES += SGIImagePrivate=tp_SGIImagePrivate

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/sct.cpp

HEADERS += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/sct_p.h

DEFINES += WRAPPER_USE_SCT_HANDLER

DEFINES += ScitexHandler=tp_ScitexHandler
DEFINES += ScitexPlugin=tp_ScitexPlugin
DEFINES += ScitexCtrlBlock=tp_ScitexCtrlBlock
DEFINES += ScitexParamsBlock=tp_ScitexParamsBlock
DEFINES += ScitexHandlerPrivate=tp_ScitexHandlerPrivate
DEFINES += LOG_IFFPLUGIN=tp_LOG_IFFPLUGIN

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/tga.cpp

HEADERS += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/tga_p.h

DEFINES += WRAPPER_USE_TGA_HANDLER

DEFINES += TGAHandler=tp_TGAHandler
DEFINES += TGAPlugin=tp_TGAPlugin
DEFINES += TGAHandlerPrivate=tp_TGAHandlerPrivate

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/xcf.cpp

HEADERS += \
    $${THIRDPARTY_KIMAGEFORMATS_PATH}/src/imageformats/xcf_p.h

DEFINES += WRAPPER_USE_XCF_HANDLER

DEFINES += XCFHandler=tp_XCFHandler
DEFINES += XCFPlugin=tp_XCFPlugin
DEFINES += XCFPLUGIN=tp_XCFPLUGIN
DEFINES += XCFImageFormat=tp_XCFImageFormat

# --------------------------------------------------------------------------------

SOURCES += \
    $${THIRDPARTY_KIMAGEFORMATS_WRAPPER_PATH}/KImageFormatsImageReader.cpp \
    $${THIRDPARTY_KIMAGEFORMATS_WRAPPER_PATH}/KImageFormatsMovie.cpp

HEADERS += \
    $${THIRDPARTY_KIMAGEFORMATS_WRAPPER_PATH}/KImageFormatsImageReader.h \
    $${THIRDPARTY_KIMAGEFORMATS_WRAPPER_PATH}/KImageFormatsMovie.h

# --------------------------------------------------------------------------------

