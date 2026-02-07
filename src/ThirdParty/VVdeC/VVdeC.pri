# URL: https://github.com/fraunhoferhhi/vvdec
# License: 3-Clause Clear BSD License - https://github.com/fraunhoferhhi/vvdec/blob/master/LICENSE.txt

include($${PWD}/../../Features.pri)

!disable_vvdec {

    DEFINES += HAS_VVDEC

    !system_vvdec {

        THIRDPARTY_VVDEC_PATH = $${PWD}/vvdec-3.1.0
        THIRDPARTY_VVDEC_CONFIG_PATH = $${PWD}/config

        INCLUDEPATH += $${THIRDPARTY_VVDEC_CONFIG_PATH} $${THIRDPARTY_VVDEC_PATH}/include
        DEPENDPATH += $${THIRDPARTY_VVDEC_CONFIG_PATH} $${THIRDPARTY_VVDEC_PATH}/include
        TR_EXCLUDE += $${THIRDPARTY_VVDEC_CONFIG_PATH}/* $${THIRDPARTY_VVDEC_PATH}/*

        OUT_LIB_TARGET = tp_vvdec
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/VVdeC
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
            *msvc*: LIBS += vvdec.lib
            else: LIBS += -lvvdec
        } else {
            PKGCONFIG += libvvdec
        }

    }

}

