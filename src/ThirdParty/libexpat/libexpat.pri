# URL: https://github.com/libexpat/libexpat
# License: MIT License - https://github.com/libexpat/libexpat/blob/master/expat/COPYING

include($${PWD}/../../Features.pri)

!disable_libexpat {

    DEFINES += HAS_LIBEXPAT

    !system_libexpat {

        THIRDPARTY_LIBEXPAT_PATH = $${PWD}/expat-2.2.10

        INCLUDEPATH += $${THIRDPARTY_LIBEXPAT_PATH}/lib
        DEPENDPATH += $${THIRDPARTY_LIBEXPAT_PATH}/lib

        DEFINES += XML_STATIC

        OUT_LIB_TARGET = tp_libexpat
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/libexpat
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
            *msvc*: LIBS += libexpat.lib
            else: LIBS += -lexpat
        } else {
            PKGCONFIG += expat
        }

    }

}

