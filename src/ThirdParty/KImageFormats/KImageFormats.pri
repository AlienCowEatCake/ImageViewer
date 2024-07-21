# URL: https://invent.kde.org/frameworks/kimageformats + https://github.com/KDE/kimageformats
# License: GNU LGPL v2.1 - https://invent.kde.org/frameworks/kimageformats/-/blob/master/README.md

include($${PWD}/../../Features.pri)

!disable_kimageformats {

    lessThan(QT_MAJOR_VERSION, 5): error(This project requires Qt 5.15 or later)
    equals(QT_MAJOR_VERSION, 5) : lessThan(QT_MINOR_VERSION, 15) : error(This project requires Qt 5.15 or later)

    THIRDPARTY_KIMAGEFORMATS_WRAPPER_PATH = $${PWD}/wrapper

    INCLUDEPATH += $${THIRDPARTY_KIMAGEFORMATS_WRAPPER_PATH}
    DEPENDPATH += $${THIRDPARTY_KIMAGEFORMATS_WRAPPER_PATH}
    DEFINES += HAS_KIMAGEFORMATS

    QT += core gui widgets

    OUT_LIB_TARGET = tp_KImageFormats
    OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/KImageFormats
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
#    PRE_TARGETDEPS += $${OUT_LIB_DIR}/$${OUT_LIB_NAME}

#    LIBS += -lKF5Archive
}

