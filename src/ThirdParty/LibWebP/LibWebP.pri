# URL: https://www.webmproject.org/code/#webp-repositories + https://github.com/webmproject/libwebp/
# License: 3-clause BSD License - https://chromium.googlesource.com/webm/libwebp/+/refs/heads/master/COPYING

include($${PWD}/../../Features.pri)

!disable_libwebp {

    DEFINES += HAS_LIBWEBP

    !system_libwebp {

        THIRDPARTY_LIBWEBP_PATH = $${PWD}/libwebp-1.2.1

        INCLUDEPATH += $${THIRDPARTY_LIBWEBP_PATH}/src
        DEPENDPATH += $${THIRDPARTY_LIBWEBP_PATH}/src

        OUT_LIB_TARGET = tp_LibWebP
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/LibWebP
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
            *msvc*: LIBS += libwebp.lib libwebpdemux.lib libwebpmux.lib
            else: LIBS += -lwebp -lwebpdemux -lwebpmux
        } else {
            PKGCONFIG += libwebp libwebpdemux libwebpmux
        }

    }

}

