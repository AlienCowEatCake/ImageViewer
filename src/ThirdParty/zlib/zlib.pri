# URL: http://www.zlib.net/
# License: Zlib License - http://www.zlib.net/zlib_license.html

include($${PWD}/../../Features.pri)

!disable_zlib {

    DEFINES += HAS_ZLIB

    !system_zlib {

        THIRDPARTY_ZLIB_PATH = $${PWD}/zlib-1.3.1

        INCLUDEPATH += $${THIRDPARTY_ZLIB_PATH}
        DEPENDPATH += $${THIRDPARTY_ZLIB_PATH}

        DEFINES += Z_PREFIX

        OUT_LIB_TARGET = tp_zlib
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/zlib
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
            *msvc*: LIBS += zdll.lib
            else: LIBS += -lz
        } else {
            PKGCONFIG += zlib
        }

    }

}

