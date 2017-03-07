# https://github.com/nothings/stb

THIRDPARTY_STB_PATH = $${PWD}/stb

CONFIG += has_thirdparty_stb
INCLUDEPATH += $${THIRDPARTY_STB_PATH}
DEPENDPATH += $${THIRDPARTY_STB_PATH}
DEFINES += HAS_THIRDPARTY_STB

LIBS += -L$${OUT_PWD}/../ThirdParty/STB
*g++*|*clang* {
    LIBS += -lSTB
} else {
    LIBS += STB.lib
}

