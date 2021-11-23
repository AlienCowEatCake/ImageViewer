# URL: http://web.archive.org/web/20180116001029/http://jxrlib.codeplex.com/
# License: 2-clause BSD License

include($${PWD}/../../Features.pri)

!disable_jxrlib {

    DEFINES += HAS_JXRLIB

    !system_jxrlib {

        THIRDPARTY_JXRLIB_PATH = $${PWD}/jxrlib-e922fa5

        INCLUDEPATH += $${THIRDPARTY_JXRLIB_PATH}/jxrgluelib $${THIRDPARTY_JXRLIB_PATH}/common/include $${THIRDPARTY_JXRLIB_PATH}/image/sys
        DEPENDPATH += $${THIRDPARTY_JXRLIB_PATH}/jxrgluelib $${THIRDPARTY_JXRLIB_PATH}/common/include $${THIRDPARTY_JXRLIB_PATH}/image/sys

        OUT_LIB_TARGET = tp_jxrlib
        OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/jxrlib
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

        *msvc*: LIBS += jpegxr.lib jxrglue.lib
        else: LIBS += -ljxrglue -ljpegxr

    }

}

