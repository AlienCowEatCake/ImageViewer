# URL: https://www.ece.uvic.ca/~frodo/jasper/
# License: https://www.ece.uvic.ca/~frodo/jasper/LICENSE

THIRDPARTY_JASPER_PATH = $${PWD}/jasper-2.0.12
THIRDPARTY_JASPER_CONFIG_PATH = $${PWD}/config

INCLUDEPATH += $${THIRDPARTY_JASPER_PATH}/src/libjasper/include $${THIRDPARTY_JASPER_PATH}/include $${THIRDPARTY_JASPER_CONFIG_PATH}
DEPENDPATH += $${THIRDPARTY_JASPER_PATH}/src/libjasper/include $${THIRDPARTY_JASPER_PATH}/include $${THIRDPARTY_JASPER_CONFIG_PATH}

win32 {
    CONFIG(release, debug|release) {
        LIBS += -L$${OUT_PWD}/../ThirdParty/JasPer/release
    } else:CONFIG(debug, debug|release) {
        LIBS += -L$${OUT_PWD}/../ThirdParty/JasPer/debug
    }
    *g++*|*clang* {
        LIBS += -ltp_jasper
    } else {
        LIBS += tp_jasper.lib
    }
} else {
    LIBS += -L$${OUT_PWD}/../ThirdParty/JasPer -ltp_jasper
}

