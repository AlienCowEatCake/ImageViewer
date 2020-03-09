# URL: https://www.cl.cam.ac.uk/~mgk25/jbigkit/
# License: GNU GPL v2 or Commercial - https://www.cl.cam.ac.uk/~mgk25/jbigkit/#licensing

include($${PWD}/../../Features.pri)

!disable_jbigkit {

    DEFINES += HAS_JBIGKIT

    !system_jbigkit {

        THIRDPARTY_JBIGKIT_PATH = $${PWD}/jbigkit-2.1

        INCLUDEPATH += $${THIRDPARTY_JBIGKIT_PATH}/libjbig
        DEPENDPATH += $${THIRDPARTY_JBIGKIT_PATH}/libjbig

        OUT_LIB_TARGET = tp_JBIGKit
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/JBIGKit
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

        *msvc*: LIBS += libjbig.lib
        else: LIBS += -ljbig

    }

}

