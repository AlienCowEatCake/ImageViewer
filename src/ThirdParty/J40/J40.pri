# URL: https://github.com/lifthrasiir/j40
# License: MIT No Attribution License - https://github.com/lifthrasiir/j40/blob/main/LICENSE.txt

include($${PWD}/../../Features.pri)

!disable_j40 {

    THIRDPARTY_J40_PATH = $${PWD}/j40-252e798

    INCLUDEPATH += $${THIRDPARTY_J40_PATH}
    DEPENDPATH += $${THIRDPARTY_J40_PATH}
    TR_EXCLUDE += $${THIRDPARTY_J40_PATH}/*

    DEFINES += HAS_J40

    OUT_LIB_TARGET = tp_J40
    OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/J40
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
#    PRE_TARGETDEPS += $${OUT_LIB_DIR}/$${OUT_LIB_NAME}

}

