# URL: https://aomedia.googlesource.com/aom/
# License: 2-clause BSD License - https://aomedia.googlesource.com/aom/+/refs/heads/master/LICENSE

include($${PWD}/../../Features.pri)

!disable_aom {

    DEFINES += HAS_AOM

    !system_aom {

        THIRDPARTY_AOM_PATH = $${PWD}/aom-v2.0.1

        INCLUDEPATH += $${THIRDPARTY_AOM_PATH}
        DEPENDPATH += $${THIRDPARTY_AOM_PATH}

        OUT_LIB_TARGET = tp_aom
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/aom
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
            *msvc*: LIBS += aom.lib
            else: LIBS += -laom
        } else {
            PKGCONFIG += aom
        }

    }

}
