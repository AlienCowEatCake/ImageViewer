# URL: http://www.openjpeg.org/
# License: 2-Clause BSD License - https://github.com/uclouvain/openjpeg/blob/master/LICENSE

include($${PWD}/../../Features.pri)

!disable_openjpeg {

    DEFINES += HAS_OPENJPEG

    !system_openjpeg {

        THIRDPARTY_OPENJPEG_PATH = $${PWD}/openjpeg-2.5.2
        THIRDPARTY_OPENJPEG_CONFIG_PATH = $${PWD}/config
        THIRDPARTY_OPENJPEG_INCLUDE_PATH = $${PWD}/include

        INCLUDEPATH += $${THIRDPARTY_OPENJPEG_CONFIG_PATH} $${THIRDPARTY_OPENJPEG_PATH} $${THIRDPARTY_OPENJPEG_INCLUDE_PATH}
        DEPENDPATH += $${THIRDPARTY_OPENJPEG_CONFIG_PATH} $${THIRDPARTY_OPENJPEG_PATH}/src/lib/openjp2 $${THIRDPARTY_OPENJPEG_INCLUDE_PATH}

        DEFINES += OPJ_STATIC

        OUT_LIB_TARGET = tp_openjp2
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/OpenJPEG
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
            *msvc*: LIBS += libopenjp2.lib
            else: LIBS += -lopenjp2
        } else {
            PKGCONFIG += libopenjp2
        }

    }

}

