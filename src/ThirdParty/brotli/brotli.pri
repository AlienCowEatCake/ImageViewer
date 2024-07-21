# URL: https://github.com/google/brotli
# License: MIT License - https://github.com/google/brotli/blob/master/LICENSE

include($${PWD}/../../Features.pri)

!disable_brotli {

    DEFINES += HAS_BROTLI

    !system_brotli {

        THIRDPARTY_BROTLI_PATH = $${PWD}/brotli-1.1.0

        INCLUDEPATH += $${THIRDPARTY_BROTLI_PATH}/c/include
        DEPENDPATH += $${THIRDPARTY_BROTLI_PATH}/c/include

        OUT_LIB_TARGET = tp_brotli
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/brotli
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
            *msvc*: LIBS += brotlienc.lib brotlidec.lib brotlicommon.lib
            else: LIBS += -lbrotlienc -lbrotlidec -lbrotlicommon
        } else {
            PKGCONFIG += libbrotlienc libbrotlidec libbrotlicommon
        }

    }

}

