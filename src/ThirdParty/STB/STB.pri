# URL: https://github.com/nothings/stb
# License: Public Domain or MIT License - https://github.com/nothings/stb/blob/master/README.md

include($${PWD}/../../Features.pri)

!disable_stb {

    THIRDPARTY_STB_PATH = $${PWD}/stb

    INCLUDEPATH += $${THIRDPARTY_STB_PATH}
    DEPENDPATH += $${THIRDPARTY_STB_PATH}
    DEFINES += HAS_STB

    OUT_LIB_TARGET = tp_STB
    OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/STB
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
#    PRE_TARGETDEPS += $${OUT_LIB_DIR}/$${OUT_LIB_NAME}

}

