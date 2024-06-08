# URL: https://github.com/google/highway
# License: Apache-2.0 License - https://github.com/google/highway/blob/master/LICENSE

include($${PWD}/../../Features.pri)

!disable_highway {

    DEFINES += HAS_HIGHWAY

    !system_highway {

        THIRDPARTY_HIGHWAY_PATH = $${PWD}/highway-1.2.0

        INCLUDEPATH += $${THIRDPARTY_HIGHWAY_PATH}
        DEPENDPATH += $${THIRDPARTY_HIGHWAY_PATH}

        OUT_LIB_TARGET = tp_highway
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/highway
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
            *msvc*: LIBS += hwy.lib
            else: LIBS += -lhwy
        } else {
            PKGCONFIG += libhwy
        }

    }

}

