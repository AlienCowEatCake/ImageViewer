# URL: http://ijg.org/
# License: Libjpeg License - https://jpegclub.org/reference/libjpeg-license/

include($${PWD}/../../Features.pri)

!disable_libjpeg {

    DEFINES += HAS_LIBJPEG

    !system_libjpeg {

        THIRDPARTY_LIBJPEG_PATH = $${PWD}/jpeg-9f
        THIRDPARTY_LIBJPEG_CONFIG_PATH = $${PWD}/config

        INCLUDEPATH += $${THIRDPARTY_LIBJPEG_CONFIG_PATH} $${THIRDPARTY_LIBJPEG_PATH}
        DEPENDPATH += $${THIRDPARTY_LIBJPEG_CONFIG_PATH} $${THIRDPARTY_LIBJPEG_PATH}

        DEFINES += JPEG_PREFIX

        OUT_LIB_TARGET = tp_libjpeg
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/libjpeg
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
            *msvc*: LIBS += libjpeg.lib
            else: LIBS += -ljpeg
        } else {
            PKGCONFIG += libjpeg
        }

    }

}

