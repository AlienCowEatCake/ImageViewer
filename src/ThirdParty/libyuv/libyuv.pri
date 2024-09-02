# URL: https://chromium.googlesource.com/libyuv/libyuv/
# License: 3-Clause BSD License - https://chromium.googlesource.com/libyuv/libyuv/+/HEAD/LICENSE

include($${PWD}/../../Features.pri)

!disable_libyuv {

    DEFINES += HAS_LIBYUV

    !system_libyuv {

        THIRDPARTY_LIBYUV_PATH = $${PWD}/libyuv

        INCLUDEPATH += $${THIRDPARTY_LIBYUV_PATH}/include
        DEPENDPATH += $${THIRDPARTY_LIBYUV_PATH}/include
        TR_EXCLUDE += $${THIRDPARTY_LIBYUV_PATH}/*

        OUT_LIB_TARGET = tp_libyuv
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/libyuv
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

        *msvc*: LIBS += libyuv.lib
        else: LIBS += -lyuv

    }

}
