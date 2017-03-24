# URL: http://www.zlib.net/
# License: http://www.zlib.net/zlib_license.html

THIRDPARTY_ZLIB_PATH = $${PWD}/zlib-1.2.11

INCLUDEPATH += $${THIRDPARTY_ZLIB_PATH}
DEPENDPATH += $${THIRDPARTY_ZLIB_PATH}

DEFINES += Z_PREFIX

win32 {
    CONFIG(release, debug|release) {
        LIBS += -L$${OUT_PWD}/../ThirdParty/zlib/release
    } else:CONFIG(debug, debug|release) {
        LIBS += -L$${OUT_PWD}/../ThirdParty/zlib/debug
    }
    *g++*|*clang* {
        LIBS += -ltp_zlib
    } else {
        LIBS += tp_zlib.lib
    }
} else {
    LIBS += -L$${OUT_PWD}/../ThirdParty/zlib -ltp_zlib
}

