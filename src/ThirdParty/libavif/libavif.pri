# URL: https://github.com/AOMediaCodec/libavif
# License: 2-Clause BSD License - https://github.com/AOMediaCodec/libavif/blob/master/LICENSE

include($${PWD}/../../Features.pri)

!disable_libavif {

    DEFINES += HAS_LIBAVIF

    !system_libavif {

        THIRDPARTY_LIBAVIF_PATH = $${PWD}/libavif-1.1.1

        INCLUDEPATH += $${THIRDPARTY_LIBAVIF_PATH}/include
        DEPENDPATH += $${THIRDPARTY_LIBAVIF_PATH}/include

        OUT_LIB_TARGET = tp_libavif
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/libavif
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
            *msvc*: LIBS += avif.lib
            else: LIBS += -lavif
        } else {
            PKGCONFIG += libavif
        }

    }

}
