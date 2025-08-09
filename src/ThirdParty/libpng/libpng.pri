# URL: http://www.libpng.org/pub/png/libpng.html + https://sourceforge.net/projects/libpng-apng/
# License: PNG Reference Library License v2 - http://www.libpng.org/pub/png/src/libpng-LICENSE.txt

include($${PWD}/../../Features.pri)

!disable_libpng {

    DEFINES += HAS_LIBPNG

    !system_libpng {

        THIRDPARTY_LIBPNG_PATH = $${PWD}/libpng-1.6.50
        THIRDPARTY_LIBPNG_CONFIG_PATH = $${PWD}/config

        INCLUDEPATH += $${THIRDPARTY_LIBPNG_CONFIG_PATH} $${THIRDPARTY_LIBPNG_PATH}
        DEPENDPATH += $${THIRDPARTY_LIBPNG_CONFIG_PATH} $${THIRDPARTY_LIBPNG_PATH}
        TR_EXCLUDE += $${THIRDPARTY_LIBPNG_PATH}/* $${THIRDPARTY_LIBPNG_CONFIG_PATH}/*

        OUT_LIB_TARGET = tp_libpng
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/libpng
        OUT_LIB_DIR2 = $${OUT_LIB_DIR}
        OUT_LIB_NAME =
        OUT_LIB_LINK =
        win32 {
            CONFIG(release, debug|release) {
                OUT_LIB_DIR2 = $${OUT_LIB_DIR}/release
            } else:CONFIG(debug, debug|release) {
                OUT_LIB_DIR2 = $${OUT_LIB_DIR}/debug
            }
            *g++*|*clang*|*llvm*|*xcode* {
                OUT_LIB_NAME = lib$${OUT_LIB_TARGET}.a
                OUT_LIB_LINK = -l$${OUT_LIB_TARGET}
            } else {
                OUT_LIB_NAME = $${OUT_LIB_TARGET}.lib
                OUT_LIB_LINK = $${OUT_LIB_NAME}
            }
        } else {
            OUT_LIB_NAME = lib$${OUT_LIB_TARGET}.a
            OUT_LIB_LINK = -l$${OUT_LIB_TARGET}
        }
        LIBS += -L$${OUT_LIB_DIR2} -L$${OUT_LIB_DIR} $${OUT_LIB_LINK}
#        PRE_TARGETDEPS += $${OUT_LIB_DIR}/$${OUT_LIB_NAME}

    } else {

        disable_pkgconfig {
            *msvc*: LIBS += libpng.lib
            else: LIBS += -lpng
        } else {
            PKGCONFIG += libpng
        }

    }

}

