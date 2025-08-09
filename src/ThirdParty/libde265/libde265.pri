# URL: https://github.com/strukturag/libde265
# License: GNU LGPL v3 - https://github.com/strukturag/libde265/blob/master/COPYING

include($${PWD}/../../Features.pri)

!disable_libde265 {

    DEFINES += HAS_LIBDE256

    !system_libde265 {

        THIRDPARTY_LIBDE265_PATH = $${PWD}/libde265-1.0.16

        INCLUDEPATH += $${THIRDPARTY_LIBDE265_PATH}
        DEPENDPATH += $${THIRDPARTY_LIBDE265_PATH}
        TR_EXCLUDE += $${THIRDPARTY_LIBDE265_PATH}/*

        DEFINES += LIBDE265_STATIC_BUILD

        OUT_LIB_TARGET = tp_libde265
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/libde265
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
            *msvc*: LIBS += libde265.lib
            else: LIBS += -lde265
        } else {
            PKGCONFIG += libde265
        }

    }

}

