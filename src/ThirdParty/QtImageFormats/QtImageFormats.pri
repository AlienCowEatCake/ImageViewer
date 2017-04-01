# URL: https://github.com/qt/qtimageformats

include($${PWD}/../../../Features.pri)

!disable_qtimageformats {

    lessThan(QT_MAJOR_VERSION, 5): error(This project requires Qt 5 or later)

    THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH = $${PWD}/wrapper

    INCLUDEPATH += $${THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH}
    DEPENDPATH += $${THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH}
    DEFINES += HAS_QTIMAGEFORMATS

    QT += core gui widgets

    OUT_LIB_TARGET = tp_QtImageFormats
    OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/QtImageFormats
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
    PRE_TARGETDEPS += $${OUT_LIB_DIR}/$${OUT_LIB_NAME}

}

