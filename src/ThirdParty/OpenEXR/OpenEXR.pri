# URL: https://github.com/openexr/openexr
# License: 3-clause BSD License - https://github.com/openexr/openexr/blob/master/LICENSE.md

include($${PWD}/../../Features.pri)

!disable_openexr {

    DEFINES += HAS_OPENEXR

    !system_openexr {

        THIRDPARTY_OPENEXR_INCLUDE_PATH = $${PWD}/include

        INCLUDEPATH = \
            $${THIRDPARTY_OPENEXR_INCLUDE_PATH} \
            $${THIRDPARTY_OPENEXR_INCLUDE_PATH}/OpenEXR \
            $${INCLUDEPATH}
        DEPENDPATH += \
            $${THIRDPARTY_OPENEXR_INCLUDE_PATH}

        OUT_LIB_TARGET = tp_openexr
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/OpenEXR
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
            *msvc*: LIBS += IlmImf.lib IlmImfUtil.lib Iex.lib Half.lib
            else: LIBS += -lIlmImf -lIlmImfUtil -lIex -lHalf
        } else {
            PKGCONFIG += OpenEXR IlmBase
        }

    }

}

