# URL: http://www.exiv2.org/
# License: GNU GPL

include($${PWD}/../../Features.pri)

!disable_exiv2 {

    DEFINES += HAS_EXIV2

    !system_exiv2 {

        THIRDPARTY_EXIV2_PATH = $${PWD}/exiv2-0.27.0-Source
        THIRDPARTY_EXIV2_CONFIG_PATH = $${PWD}/config

        INCLUDEPATH += $${THIRDPARTY_EXIV2_PATH}/include $${THIRDPARTY_EXIV2_CONFIG_PATH}
        DEPENDPATH += $${THIRDPARTY_EXIV2_PATH}/include $${THIRDPARTY_EXIV2_CONFIG_PATH}

        OUT_LIB_TARGET = tp_exiv2
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/Exiv2
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
        LIBS += -lexpat #TODO

    } else {

        *msvc*: LIBS += exiv2.lib
        else: LIBS += -lexiv2

    }

}

