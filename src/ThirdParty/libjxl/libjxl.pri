# URL: https://github.com/libjxl/libjxl
# License: BSD-style License - https://github.com/libjxl/libjxl/blob/main/LICENSE

include($${PWD}/../../Features.pri)

!disable_libjxl {

    DEFINES += HAS_LIBJXL

    !system_libjxl {

        THIRDPARTY_LIBJXL_PATH = $${PWD}/libjxl-0.9.0
        THIRDPARTY_LIBJXL_CONFIG_PATH = $${PWD}/config

        INCLUDEPATH += $${THIRDPARTY_LIBJXL_PATH}/lib/include $${THIRDPARTY_LIBJXL_CONFIG_PATH}
        DEPENDPATH += $${THIRDPARTY_LIBJXL_PATH}/lib/include $${THIRDPARTY_LIBJXL_CONFIG_PATH}

        OUT_LIB_TARGET = tp_libjxl
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/libjxl
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

        disable_pkgconfig {
            *msvc*: LIBS += libjxl.lib
            else: LIBS += -ljxl
        } else {
            PKGCONFIG += libjxl
        }

    }

}

