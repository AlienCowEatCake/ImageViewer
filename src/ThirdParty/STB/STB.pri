# URL: https://github.com/nothings/stb
# License: public domain or MIT

THIRDPARTY_STB_PATH = $${PWD}/stb

CONFIG += has_thirdparty_stb
INCLUDEPATH += $${THIRDPARTY_STB_PATH}
DEPENDPATH += $${THIRDPARTY_STB_PATH}
DEFINES += HAS_THIRDPARTY_STB

win32 {
    CONFIG(release, debug|release) {
        LIBS += -L$${OUT_PWD}/../ThirdParty/STB/release
    } else:CONFIG(debug, debug|release) {
        LIBS += -L$${OUT_PWD}/../ThirdParty/STB/debug
    }
    *g++*|*clang* {
        LIBS += -ltp_STB
    } else {
        LIBS += tp_STB.lib
    }
} else {
    LIBS += -L$${OUT_PWD}/../ThirdParty/STB -ltp_STB
}

