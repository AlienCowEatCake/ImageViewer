# https://github.com/qt/qtimageformats

THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH = $${PWD}/wrapper

CONFIG += has_thirdparty_qtimageformats
INCLUDEPATH += $${THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH}
DEPENDPATH += $${THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH}
DEFINES += HAS_THIRDPARTY_QTIMAGEFORMATS

QT += core gui

win32 {
    CONFIG(release, debug|release) {
        LIBS += -L$${OUT_PWD}/../ThirdParty/QtImageFormats/release
    } else:CONFIG(debug, debug|release) {
        LIBS += -L$${OUT_PWD}/../ThirdParty/QtImageFormats/debug
    }
    *g++*|*clang* {
        LIBS += -lQtImageFormats
    } else {
        LIBS += QtImageFormats.lib
    }
} else {
    LIBS += -L$${OUT_PWD}/../ThirdParty/QtImageFormats -lQtImageFormats
}

