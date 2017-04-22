# URL: http://hookatooka.com/poshlib/
# License: ???

include($${PWD}/../../Features.pri)

!disable_poshlib {

    DEFINES += HAS_POSHLIB

        THIRDPARTY_POSHLIB_PATH = $${PWD}/poshlib

        INCLUDEPATH += $${THIRDPARTY_POSHLIB_PATH}
        DEPENDPATH += $${THIRDPARTY_POSHLIB_PATH}

        OUT_LIB_TARGET = tp_poshlib
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/POSH
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

}

