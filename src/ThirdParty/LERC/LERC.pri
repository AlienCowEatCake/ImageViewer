# URL: https://github.com/Esri/lerc
# License: Apache License, Version 2.0 - https://github.com/Esri/lerc/blob/master/LICENSE

include($${PWD}/../../Features.pri)

!disable_lerc {

    DEFINES += HAS_LERC

    !system_lerc {

        THIRDPARTY_LERC_PATH = $${PWD}/lerc-3.0

        INCLUDEPATH += $${THIRDPARTY_LERC_PATH}/src/LercLib/include
        DEPENDPATH += $${THIRDPARTY_LERC_PATH}/src/LercLib/include

        OUT_LIB_TARGET = tp_LERC
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/LERC
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

        *msvc*: LIBS += Lerc.lib
        else: LIBS += -lLerc

    }

}

