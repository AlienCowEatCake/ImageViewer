# https://github.com/nothings/stb

THIRDPARTY_STB_PATH = $${PWD}/stb

CONFIG += has_thirdparty_qtextended
INCLUDEPATH += $${THIRDPARTY_STB_PATH}
DEPENDPATH += $${THIRDPARTY_STB_PATH}
DEFINES += HAS_THIRDPARTY_STB

HEADERS += \
    $$files($${THIRDPARTY_STB_PATH}/*.h)

SOURCES += \
    $$files($${THIRDPARTY_STB_PATH}/*.cpp)

