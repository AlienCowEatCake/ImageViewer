# URL: https://libexif.github.io/ + https://github.com/libexif/libexif
# License: GNU LGPL v2.1 - https://github.com/libexif/libexif/blob/master/COPYING

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_libexif

QT -= core gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_LIBEXIF_PATH = $${PWD}/libexif-9266d14
THIRDPARTY_LIBEXIF_CONFIG_PATH = $${PWD}/config
THIRDPARTY_LIBEXIF_INCLUDE_PATH = $${PWD}/include

include(../CommonSettings.pri)

INCLUDEPATH = $${THIRDPARTY_LIBEXIF_CONFIG_PATH} $${THIRDPARTY_LIBEXIF_INCLUDE_PATH} $${THIRDPARTY_LIBEXIF_PATH} $${INCLUDEPATH}

DEFINES += GETTEXT_PACKAGE=\\\"libexif-12\\\"
*msvc*: DEFINES += ssize_t=__int64 inline=

SOURCES += \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-byte-order.c \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-content.c \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-data.c \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-entry.c \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-format.c \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-ifd.c \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-loader.c \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-log.c \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-mem.c \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-mnote-data.c \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-tag.c \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-utils.c \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/canon/exif-mnote-data-canon.c \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/canon/mnote-canon-entry.c \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/canon/mnote-canon-tag.c \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/fuji/exif-mnote-data-fuji.c \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/fuji/mnote-fuji-entry.c \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/fuji/mnote-fuji-tag.c \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/olympus/exif-mnote-data-olympus.c \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/olympus/mnote-olympus-entry.c \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/olympus/mnote-olympus-tag.c \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/pentax/exif-mnote-data-pentax.c \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/pentax/mnote-pentax-entry.c \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/pentax/mnote-pentax-tag.c

HEADERS += \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-byte-order.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-content.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-data.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-data-type.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-entry.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-format.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-ifd.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-loader.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-log.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-mem.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-mnote-data.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-mnote-data-priv.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-system.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-tag.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/exif-utils.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/i18n.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/canon/exif-mnote-data-canon.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/canon/mnote-canon-entry.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/canon/mnote-canon-tag.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/fuji/exif-mnote-data-fuji.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/fuji/mnote-fuji-entry.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/fuji/mnote-fuji-tag.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/olympus/exif-mnote-data-olympus.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/olympus/mnote-olympus-entry.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/olympus/mnote-olympus-tag.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/pentax/exif-mnote-data-pentax.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/pentax/mnote-pentax-entry.h \
    $${THIRDPARTY_LIBEXIF_PATH}/libexif/pentax/mnote-pentax-tag.h \
    $${THIRDPARTY_LIBEXIF_INCLUDE_PATH}/libexif/_stdint.h \
    $${THIRDPARTY_LIBEXIF_CONFIG_PATH}/config.h

TR_EXCLUDE += $${THIRDPARTY_LIBEXIF_PATH}/*

