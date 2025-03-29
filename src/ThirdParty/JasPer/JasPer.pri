# URL: https://github.com/jasper-software/jasper
# License: JasPer License v2.0 - https://github.com/jasper-software/jasper/blob/master/LICENSE.txt

include($${PWD}/../../Features.pri)

!disable_libjasper {

    DEFINES += HAS_LIBJASPER

    !system_libjasper {

        THIRDPARTY_JASPER_PATH = $${PWD}/jasper-6cb475b
        THIRDPARTY_JASPER_CONFIG_PATH = $${PWD}/config

        INCLUDEPATH += $${THIRDPARTY_JASPER_CONFIG_PATH} $${THIRDPARTY_JASPER_PATH}/src/libjasper/include
        DEPENDPATH += $${THIRDPARTY_JASPER_CONFIG_PATH} $${THIRDPARTY_JASPER_PATH}/src/libjasper/include
        TR_EXCLUDE += $${THIRDPARTY_JASPER_PATH}/* $${THIRDPARTY_JASPER_CONFIG_PATH}/*

        DEFINES += __STDC_CONSTANT_MACROS __STDC_LIMIT_MACROS

        OUT_LIB_TARGET = tp_jasper
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/JasPer
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

        *msvc*: LIBS += libjasper.lib
        else: LIBS += -ljasper

    }

}

