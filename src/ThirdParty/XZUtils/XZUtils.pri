# URL: https://tukaani.org/xz/
# License: Public Domain - https://git.tukaani.org/?p=xz.git;a=blob;f=COPYING

include($${PWD}/../../Features.pri)

!disable_xzutils {

    DEFINES += HAS_XZUTILS

    !system_xzutils {

        THIRDPARTY_XZUTILS_PATH = $${PWD}/xz-5.4.3

        INCLUDEPATH += $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/api
        DEPENDPATH += $${THIRDPARTY_XZUTILS_PATH}/src/liblzma/api

        DEFINES += LZMA_API_STATIC
        DEFINES += TUKLIB_SYMBOL_PREFIX=tp_lzma_

        OUT_LIB_TARGET = tp_XZUtils
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/XZUtils
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
            *msvc*: LIBS += liblzma.lib
            else: LIBS += -llzma
        } else {
            PKGCONFIG += liblzma
        }

    }

}

