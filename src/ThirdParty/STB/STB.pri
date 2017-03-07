# https://github.com/nothings/stb

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
        LIBS += -lSTB
    } else {
        LIBS += STB.lib
    }
} else {
    LIBS += -L$${OUT_PWD}/../ThirdParty/STB -lSTB
}

