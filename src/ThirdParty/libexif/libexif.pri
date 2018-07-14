# URL: https://libexif.github.io/
# License: GNU LGPL v2.1

include($${PWD}/../../Features.pri)

!disable_libexif {

    DEFINES += HAS_LIBEXIF

    !system_libexif {

        THIRDPARTY_LIBEXIF_PATH = $${PWD}/libexif-5c93e6b
        THIRDPARTY_LIBEXIF_INCLUDE_PATH = $${PWD}/include

        INCLUDEPATH += $${THIRDPARTY_LIBEXIF_INCLUDE_PATH} $${THIRDPARTY_LIBEXIF_PATH}
        DEPENDPATH += $${THIRDPARTY_LIBEXIF_INCLUDE_PATH} $${THIRDPARTY_LIBEXIF_PATH}

        OUT_LIB_TARGET = tp_libexif
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/libexif
        OUT_LIB_NAME =
        OUT_LIB_LINK =
        win32 {
            CONFIG(release, debug|release) {
                OUT_LIB_DIR = $${OUT_LIB_DIR}/release
            } else:CONFIG(debug, debug|release) {
                OUT_LIB_DIR = $${OUT_LIB_DIR}/debug
            }
            *g++*|*clang* {
                OUT_LIB_NAME = lib$${OUT_LIB_TARGET}.a
                OUT_LIB_LINK = -l$${OUT_LIB_TARGET}
            } else {
                OUT_LIB_NAME = $${OUT_LIB_TARGET}.lib
                OUT_LIB_LINK = $${OUT_LIB_NAME}
            }
        } else {
            OUT_LIB_DIR = $${OUT_LIB_DIR}
            OUT_LIB_NAME = lib$${OUT_LIB_TARGET}.a
            OUT_LIB_LINK = -l$${OUT_LIB_TARGET}
        }
        LIBS += -L$${OUT_LIB_DIR} $${OUT_LIB_LINK}
#        PRE_TARGETDEPS += $${OUT_LIB_DIR}/$${OUT_LIB_NAME}

    } else {

        *msvc*: LIBS += libexif.lib
        else: LIBS += -lexif

    }

}

