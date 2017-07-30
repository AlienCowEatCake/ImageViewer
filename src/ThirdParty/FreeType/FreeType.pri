# URL: https://www.freetype.org/
# License: FreeType License / GNU GPL v2

include($${PWD}/../../Features.pri)

!disable_freetype {

    DEFINES += HAS_FREETYPE

    !system_freetype {

        THIRDPARTY_FREETYPE_PATH = $${PWD}/freetype-2.8

        INCLUDEPATH = $${THIRDPARTY_FREETYPE_PATH}/include $${INCLUDEPATH}
        DEPENDPATH += $${THIRDPARTY_FREETYPE_PATH}/include

        OUT_LIB_TARGET = tp_freetype
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/FreeType
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

        *msvc*: LIBS += freetype.lib
        else: LIBS += -lfreetype

    }

}

