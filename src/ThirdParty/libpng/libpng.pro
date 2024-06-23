# URL: http://www.libpng.org/pub/png/libpng.html + https://sourceforge.net/projects/libpng-apng/
# License: PNG Reference Library License v2 - http://www.libpng.org/pub/png/src/libpng-LICENSE.txt

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_libpng

QT -= core gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_LIBPNG_PATH = $${PWD}/libpng-1.6.43
THIRDPARTY_LIBPNG_CONFIG_PATH = $${PWD}/config

include(../../Features.pri)
include(../CommonSettings.pri)
include(../zlib/zlib.pri)

INCLUDEPATH = $${THIRDPARTY_LIBPNG_CONFIG_PATH} $${THIRDPARTY_LIBPNG_PATH} $${INCLUDEPATH}

DEFINES += PNG_ARM_NEON_OPT=0 PNG_POWERPC_VSX_OPT=0 PNG_INTEL_SSE_OPT=0 PNG_MIPS_MSA_OPT=0 PNG_MIPS_MSA_OPT=0 PNG_LOONGARCH_LSX_OPT=0

# cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DPNG_PREFIX="tp_" -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_FRAMEWORK=OFF -DPNG_TESTS=OFF -DPNG_TOOLS=OFF -DPNG_EXECUTABLES=OFF -DPNG_DEBUG=OFF -DPNG_HARDWARE_OPTIMIZATIONS=OFF ..

# find . -maxdepth 1 -name '*.c' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_LIBPNG_PATH}| ; s|$| \\|'
SOURCES += \
\#    $${THIRDPARTY_LIBPNG_PATH}/example.c \
    $${THIRDPARTY_LIBPNG_PATH}/png.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngerror.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngget.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngmem.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngpread.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngread.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngrio.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngrtran.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngrutil.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngset.c \
\#    $${THIRDPARTY_LIBPNG_PATH}/pngtest.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngtrans.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngwio.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngwrite.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngwtran.c \
    $${THIRDPARTY_LIBPNG_PATH}/pngwutil.c \

# find . -maxdepth 1 -name '*.h' | LANG=C sort | sed 's|^\.|    $${THIRDPARTY_LIBPNG_PATH}| ; s|$| \\|'
HEADERS += \
    $${THIRDPARTY_LIBPNG_PATH}/png.h \
    $${THIRDPARTY_LIBPNG_PATH}/pngconf.h \
    $${THIRDPARTY_LIBPNG_PATH}/pngdebug.h \
    $${THIRDPARTY_LIBPNG_PATH}/pnginfo.h \
    $${THIRDPARTY_LIBPNG_PATH}/pngpriv.h \
    $${THIRDPARTY_LIBPNG_PATH}/pngstruct.h \
    $${THIRDPARTY_LIBPNG_CONFIG_PATH}/pnglibconf.h \
    $${THIRDPARTY_LIBPNG_CONFIG_PATH}/pngprefix.h \

TR_EXCLUDE += $${THIRDPARTY_LIBPNG_PATH}/* $${THIRDPARTY_LIBPNG_CONFIG_PATH}/*

