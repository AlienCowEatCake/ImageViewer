# URL: https://sourceforge.net/projects/qpe/files/QPE/qtopia/qt-extended-opensource-src-4.4.3.tar.gz/download
# License: GNU GPL v2 - https://www.gnu.org/licenses/old-licenses/gpl-2.0.html

include($${PWD}/../../Features.pri)

!disable_qtextended {

    THIRDPARTY_QTEXTENDED_PATH = $${PWD}/src

    INCLUDEPATH += $${THIRDPARTY_QTEXTENDED_PATH}
    DEPENDPATH += $${THIRDPARTY_QTEXTENDED_PATH}
    DEFINES += HAS_QTEXTENDED

    QT += core gui

    OUT_LIB_TARGET = tp_QtExtended
    OUT_LIB_DIR = $${OUT_PWD}/../ThirdParty/QtExtended
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

