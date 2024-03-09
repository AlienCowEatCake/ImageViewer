# URL: https://sourceforge.net/projects/giflib/files/
# License: MIT License - https://sourceforge.net/p/giflib/code/ci/master/tree/COPYING

include($${PWD}/../../Features.pri)

!disable_giflib {

    DEFINES += HAS_GIFLIB

    !system_giflib {

        THIRDPARTY_GIFLIB_PATH = $${PWD}/giflib-5.2.2

        INCLUDEPATH += $${THIRDPARTY_GIFLIB_PATH}
        DEPENDPATH += $${THIRDPARTY_GIFLIB_PATH}

        OUT_LIB_TARGET = tp_giflib
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/giflib
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

        *msvc*: LIBS += giflib.lib
        else: LIBS += -lgif

    }

}
