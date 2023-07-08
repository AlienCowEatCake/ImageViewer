# URL: http://www.littlecms.com/
# License: MIT License - https://opensource.org/licenses/mit-license

include($${PWD}/../../Features.pri)

!disable_liblcms2 {

    DEFINES += HAS_LCMS2

    !system_liblcms2 {

        THIRDPARTY_LIBLCMS2_PATH = $${PWD}/lcms2-2.15

        INCLUDEPATH += $${THIRDPARTY_LIBLCMS2_PATH}/include
        DEPENDPATH += $${THIRDPARTY_LIBLCMS2_PATH}/include

        OUT_LIB_TARGET = tp_liblcms2
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/LittleCMS2
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
            *msvc*: LIBS += liblcms2.lib
            else: LIBS += -llcms2
        } else {
            PKGCONFIG += lcms2
        }

    }

}

