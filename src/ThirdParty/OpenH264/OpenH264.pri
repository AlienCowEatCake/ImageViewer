# URL: https://github.com/cisco/openh264
# License: 2-Clause BSD License - https://github.com/cisco/openh264/blob/master/LICENSE

include($${PWD}/../../Features.pri)

!disable_openh264 {

    DEFINES += HAS_OPENH264

    !system_openh264 {

        THIRDPARTY_OPENH264_PATH = $${PWD}/openh264

        INCLUDEPATH += $${THIRDPARTY_OPENH264_PATH}/codec/api
        DEPENDPATH += $${THIRDPARTY_OPENH264_PATH}/codec/api
        TR_EXCLUDE += $${THIRDPARTY_OPENH264_PATH}/*

        OUT_LIB_TARGET = tp_openh264
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/OpenH264
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
            *msvc*: LIBS += openh264.lib
            else: LIBS += -lopenh264
        } else {
            PKGCONFIG += openh264
        }

    }

}

