# URL: https://sourceforge.net/projects/libmng/
# License: Zlib License - https://opensource.org/licenses/Zlib

include($${PWD}/../../Features.pri)

!disable_libmng {

    DEFINES += HAS_LIBMNG

    !system_libmng {

        THIRDPARTY_LIBMNG_PATH = $${PWD}/libmng-2.0.3
        THIRDPARTY_LIBMNG_CONFIG_PATH = $${PWD}/config

        INCLUDEPATH += $${THIRDPARTY_LIBMNG_CONFIG_PATH} $${THIRDPARTY_LIBMNG_PATH}
        DEPENDPATH += $${THIRDPARTY_LIBMNG_CONFIG_PATH} $${THIRDPARTY_LIBMNG_PATH}

        DEFINES += MNG_PREFIX

        DEFINES += MNG_BUILD_SO

        disable_libjpeg {
            DEFINES += MNG_NO_INCLUDE_JNG
        }

        OUT_LIB_TARGET = tp_libmng
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/libmng
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

        *msvc*: LIBS += libmng.lib
        else: LIBS += -lmng

    }

}

