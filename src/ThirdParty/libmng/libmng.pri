# URL: https://sourceforge.net/projects/libmng/
# License: zlib/libpng License

include($${PWD}/../../Features.pri)

!disable_libmng {

    DEFINES += HAS_LIBMNG

    !system_libjpeg {

        THIRDPARTY_LIBMNG_PATH = $${PWD}/libmng-1.0.10

        INCLUDEPATH += $${THIRDPARTY_LIBMNG_PATH}
        DEPENDPATH += $${THIRDPARTY_LIBMNG_PATH}

        DEFINES += MNG_BUILD_SO

        disable_libjpeg {
            DEFINES += MNG_NO_INCLUDE_JNG
        }

        !disable_liblcms2 {
            DEFINES += MNG_FULL_CMS
        }

        OUT_LIB_TARGET = tp_libmng
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/libmng
        OUT_LIB_NAME =
        OUT_LIB_LINK =
        win32 {
            CONFIG(release, debug|release) {
                OUT_LIB_DIR = $${OUT_LIB_DIR}/release
            } else:CONFIG(debug, debug|release) {
                OUT_LIB_DIR = $${OUT_LIB_DIR}/debug
            }
            *g++*|*clang* {
                OUT_LIB_NAME = lib$${OUT_LIB_TARGET}.a
                OUT_LIB_LINK = -l$${OUT_LIB_TARGET}
            } else {
                OUT_LIB_NAME = $${OUT_LIB_TARGET}.lib
                OUT_LIB_LINK = $${OUT_LIB_NAME}
            }
        } else {
            OUT_LIB_DIR = $${OUT_LIB_DIR}
            OUT_LIB_NAME = lib$${OUT_LIB_TARGET}.a
            OUT_LIB_LINK = -l$${OUT_LIB_TARGET}
        }
        LIBS += -L$${OUT_LIB_DIR} $${OUT_LIB_LINK}
#        PRE_TARGETDEPS += $${OUT_LIB_DIR}/$${OUT_LIB_NAME}

    } else {

        *msvc*: LIBS += libmng.lib
        else: LIBS += -lmng

    }

}

