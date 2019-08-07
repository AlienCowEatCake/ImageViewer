# URL: http://www.simplesystems.org/libtiff/
# License: LibTiff License - https://gitlab.com/libtiff/libtiff/blob/master/COPYRIGHT

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_libtiff

#QT -= core gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_LIBTIFF_PATH = $${PWD}/tiff-4.0.10
THIRDPARTY_LIBTIFF_CONFIG_PATH = $${PWD}/config

include(../CommonSettings.pri)
include(../zlib/zlib.pri)
include(../Zstandard/Zstandard.pri)
include(../JBIGKit/JBIGKit.pri)
include(../libjpeg/libjpeg.pri)
include(../XZUtils/XZUtils.pri)
include(../LibWebP/LibWebP.pri)

INCLUDEPATH = $${THIRDPARTY_LIBTIFF_CONFIG_PATH} $${THIRDPARTY_LIBTIFF_PATH}/libtiff $${INCLUDEPATH}

DEFINES += TIFF_PREFIX DISABLE_CHECK_TIFFSWABMACROS

*msvc*: DEFINES += inline=__inline
win32:  DEFINES += TIF_PLATFORM_CONSOLE

SOURCES += \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_aux.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_close.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_codec.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_color.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_compress.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_dir.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_dirinfo.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_dirread.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_dirwrite.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_dumpmode.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_error.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_extension.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_fax3.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_fax3sm.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_flush.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_getimage.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_jbig.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_jpeg.c \
\#    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_jpeg_12.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_luv.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_lzma.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_lzw.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_next.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_ojpeg.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_open.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_packbits.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_pixarlog.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_predict.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_print.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_read.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_strip.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_swab.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_thunder.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_tile.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_version.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_warning.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_webp.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_write.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_zip.c \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_zstd.c

win32 {
    SOURCES += \
        $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_win32.c
} else {
    SOURCES += \
        $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_unix.c
}

*msvc* {
    SOURCES += \
        $${THIRDPARTY_LIBTIFF_PATH}/port/snprintf.c
}

HEADERS += \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/t4.h \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_dir.h \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_fax3.h \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tiff.h \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tiffio.h \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tiffiop.h \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tiffvers.h \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/tif_predict.h \
    $${THIRDPARTY_LIBTIFF_PATH}/libtiff/uvcode.h

HEADERS += \
    $${THIRDPARTY_LIBTIFF_CONFIG_PATH}/tif_config.h \
    $${THIRDPARTY_LIBTIFF_CONFIG_PATH}/tiffconf.h

HEADERS += \
    $${THIRDPARTY_LIBTIFF_CONFIG_PATH}/tiffprefix.h

TR_EXCLUDE += $${THIRDPARTY_LIBPNG_PATH}/*

