# URL: http://www.exiv2.org/
# License: GNU GPL v2 or later - https://github.com/Exiv2/exiv2/blob/main/LICENSE.txt

include($${PWD}/../../Features.pri)

!disable_exiv2 {

    DEFINES += HAS_EXIV2

    *clang* {
        DEFINES *= _LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR _LIBCPP_DISABLE_DEPRECATION_WARNINGS
    }
    *msvc* {
        DEFINES *= _HAS_AUTO_PTR_ETC=1
    }

    !system_exiv2 {

        THIRDPARTY_EXIV2_PATH = $${PWD}/exiv2-0.28.2
        THIRDPARTY_EXIV2_CONFIG_PATH = $${PWD}/config

        INCLUDEPATH += $${THIRDPARTY_EXIV2_PATH}/include $${THIRDPARTY_EXIV2_CONFIG_PATH}
        DEPENDPATH += $${THIRDPARTY_EXIV2_PATH}/include $${THIRDPARTY_EXIV2_CONFIG_PATH}

        OUT_LIB_TARGET = tp_exiv2
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/Exiv2
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

        win32 {
            *msvc*: LIBS += ws2_32.lib
            else: LIBS += -lws2_32
        }
        haiku {
            LIBS += -lnetwork
        }

    } else {

        disable_pkgconfig {
            *msvc*: LIBS += exiv2.lib
            else: LIBS += -lexiv2
        } else {
            PKGCONFIG += exiv2
        }

    }

}

