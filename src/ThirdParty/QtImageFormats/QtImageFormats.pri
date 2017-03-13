# https://github.com/qt/qtimageformats

lessThan(QT_MAJOR_VERSION, 5): error(This project requires Qt 5 or later)

THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH = $${PWD}/wrapper

CONFIG += has_thirdparty_qtimageformats
INCLUDEPATH += $${THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH}
DEPENDPATH += $${THIRDPARTY_QTIMAGEFORMATS_WRAPPER_PATH}
DEFINES += HAS_THIRDPARTY_QTIMAGEFORMATS

QT += core gui widgets

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

