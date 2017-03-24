# URL: http://ijg.org/
# License: ???

THIRDPARTY_LIBJPEG_PATH = $${PWD}/jpeg-9b
THIRDPARTY_LIBJPEG_CONFIG_PATH = $${PWD}/config

INCLUDEPATH += $${THIRDPARTY_LIBJPEG_PATH} $${THIRDPARTY_LIBJPEG_CONFIG_PATH}
DEPENDPATH += $${THIRDPARTY_LIBJPEG_PATH} $${THIRDPARTY_LIBJPEG_CONFIG_PATH}

win32 {
    CONFIG(release, debug|release) {
        LIBS += -L$${OUT_PWD}/../ThirdParty/libjpeg/release
    } else:CONFIG(debug, debug|release) {
        LIBS += -L$${OUT_PWD}/../ThirdParty/libjpeg/debug
    }
    *g++*|*clang* {
        LIBS += -ltp_libjpeg
    } else {
        LIBS += tp_libjpeg.lib
    }
} else {
    LIBS += -L$${OUT_PWD}/../ThirdParty/libjpeg -ltp_libjpeg
}

