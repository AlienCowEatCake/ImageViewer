# URL: http://wvware.sourceforge.net/libwmf.html + https://github.com/caolanm/libwmf + https://github.com/ArtifexSoftware/urw-base35-fonts
# License: GNU GPL v2 - https://www.gnu.org/licenses/old-licenses/gpl-2.0.html

include($${PWD}/../../Features.pri)

!disable_libwmf {

    DEFINES += HAS_LIBWMF

    !system_libwmf {

        THIRDPARTY_LIBWMF_PATH = $${PWD}/libwmf-0.2.13

        INCLUDEPATH += $${THIRDPARTY_LIBWMF_PATH}/include $${THIRDPARTY_LIBWMF_PATH}
        DEPENDPATH += $${THIRDPARTY_LIBWMF_PATH}/include $${THIRDPARTY_LIBWMF_PATH}
        TR_EXCLUDE += $${THIRDPARTY_LIBWMF_PATH}/*

        OUT_LIB_TARGET = tp_libwmf
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/libwmf
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

        *msvc*: LIBS += libwmf.lib libwmflite.lib
        else: LIBS += -lwmf -lwmflite

    }

}

