# URL: https://www.libraw.org/download
# License: GNU LGPL v2.1 or CDDL v1.0 - https://github.com/LibRaw/LibRaw/blob/master/COPYRIGHT

include($${PWD}/../../Features.pri)

!disable_libraw {

    DEFINES += HAS_LIBRAW

    *clang* {
        DEFINES *= _LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR
    }

    !system_libraw {

        THIRDPARTY_LIBRAW_PATH = $${PWD}/LibRaw-0.19.5

        INCLUDEPATH += $${THIRDPARTY_LIBRAW_PATH}
        DEPENDPATH += $${THIRDPARTY_LIBRAW_PATH}

        win32 {
            DEFINES += LIBRAW_NODLL
        }

        OUT_LIB_TARGET = tp_LibRaw
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/LibRaw
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

    } else {

        disable_pkgconfig {
            *msvc*: LIBS += libraw.lib
            else: LIBS += -lraw
        } else {
            PKGCONFIG += libraw
        }

    }

}

