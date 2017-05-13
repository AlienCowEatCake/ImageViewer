# URL: https://www.webmproject.org/code/#webp-repositories
# License: ???

include($${PWD}/../../Features.pri)

!disable_libwebp {

    DEFINES += HAS_LIBWEBP

    !system_libwebp {

        THIRDPARTY_LIBWEBP_PATH = $${PWD}/libwebp-0.6.0

        INCLUDEPATH += $${THIRDPARTY_LIBWEBP_PATH}/src
        DEPENDPATH += $${THIRDPARTY_LIBWEBP_PATH}/src

        OUT_LIB_TARGET = tp_LibWebP
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/LibWebP
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

        *msvc*: LIBS += libwebp.lib
        else: LIBS += -lwebp

    }

}

