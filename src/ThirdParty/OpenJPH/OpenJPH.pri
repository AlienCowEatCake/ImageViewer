# URL: https://github.com/aous72/OpenJPH
# License: 2-Clause BSD License - https://github.com/aous72/OpenJPH/blob/master/LICENSE

include($${PWD}/../../Features.pri)

!disable_openjph {

    DEFINES += HAS_OPENJPH

    !system_openjph {

        THIRDPARTY_OPENJPH_INCLUDE_PATH = $${PWD}/include

        INCLUDEPATH += $${THIRDPARTY_OPENJPH_INCLUDE_PATH} $${THIRDPARTY_OPENJPH_INCLUDE_PATH}/openjph
        DEPENDPATH += $${THIRDPARTY_OPENJPH_INCLUDE_PATH} $${THIRDPARTY_OPENJPH_INCLUDE_PATH}/openjph
        TR_EXCLUDE += $${THIRDPARTY_OPENJPH_INCLUDE_PATH}/*

        OUT_LIB_TARGET = tp_openjph
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/OpenJPH
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
            *msvc*: LIBS += openjph.lib
            else: LIBS += -lopenjph
        } else {
            PKGCONFIG += openjph
        }

    }

}

