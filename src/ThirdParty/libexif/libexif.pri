# URL: https://libexif.github.io/ + https://github.com/libexif/libexif
# License: GNU LGPL v2.1 - https://github.com/libexif/libexif/blob/master/COPYING

include($${PWD}/../../Features.pri)

!disable_libexif {

    DEFINES += HAS_LIBEXIF

    !system_libexif {

        THIRDPARTY_LIBEXIF_PATH = $${PWD}/libexif-0f231cd
        THIRDPARTY_LIBEXIF_INCLUDE_PATH = $${PWD}/include

        INCLUDEPATH += $${THIRDPARTY_LIBEXIF_INCLUDE_PATH} $${THIRDPARTY_LIBEXIF_PATH}
        DEPENDPATH += $${THIRDPARTY_LIBEXIF_INCLUDE_PATH} $${THIRDPARTY_LIBEXIF_PATH}

        OUT_LIB_TARGET = tp_libexif
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/libexif
        OUT_LIB_DIR2 = $${OUT_LIB_DIR}
        OUT_LIB_NAME =
        OUT_LIB_LINK =
        win32 {
            CONFIG(release, debug|release) {
                OUT_LIB_DIR2 = $${OUT_LIB_DIR}/release
            } else:CONFIG(debug, debug|release) {
                OUT_LIB_DIR2 = $${OUT_LIB_DIR}/debug
            }
            *g++*|*clang* {
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
            *msvc*: LIBS += libexif.lib
            else: LIBS += -lexif
        } else {
            PKGCONFIG += libexif
        }

    }

}

