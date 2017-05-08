# URL: https://tukaani.org/xz/
# License: ???

include($${PWD}/../../Features.pri)

!disable_xzutils {

    DEFINES += HAS_XZUTILS

    !system_xzutils {

        THIRDPARTY_XZUTILS_PATH = $${PWD}/xz-5.2.3

        INCLUDEPATH += $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/api
        DEPENDPATH += $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/api

        DEFINES += LZMA_API_STATIC
        DEFINES += TUKLIB_SYMBOL_PREFIX=tp_lzma_

        OUT_LIB_TARGET = tp_XZUtils
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/XZUtils
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

        *msvc*: LIBS += liblzma.lib
        else: LIBS += -llzma

    }

}

