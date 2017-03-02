# https://sourceforge.net/projects/qpe/files/QPE/qtopia/qt-extended-opensource-src-4.4.3.tar.gz/download

THIRDPARTY_QTEXTENDED_PATH = $${PWD}/src

CONFIG += has_thirdparty_qtextended
INCLUDEPATH += $${THIRDPARTY_QTEXTENDED_PATH}
DEPENDPATH += $${THIRDPARTY_QTEXTENDED_PATH}
DEFINES += HAS_THIRDPARTY_QTEXTENDED

HEADERS += \
    $$files($${THIRDPARTY_QTEXTENDED_PATH}/*.h)

SOURCES += \
    $$files($${THIRDPARTY_QTEXTENDED_PATH}/*.cpp)

