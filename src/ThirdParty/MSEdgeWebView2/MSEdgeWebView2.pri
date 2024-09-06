# URL: https://www.nuget.org/packages/Microsoft.Web.WebView2
# License: 3-Clause BSD License - https://www.nuget.org/packages/Microsoft.Web.WebView2/1.0.2739.15/License

include($${PWD}/../../Features.pri)

!disable_msedgewebview2 {

    THIRDPARTY_MSEDGEWEBVIEW2_PATH = $${PWD}/microsoft.web.webview2.1.0.2739.15

    INCLUDEPATH += $${THIRDPARTY_MSEDGEWEBVIEW2_PATH}/build/native/include
    DEPENDPATH += $${THIRDPARTY_MSEDGEWEBVIEW2_PATH}/build/native/include
    TR_EXCLUDE += $${THIRDPARTY_MSEDGEWEBVIEW2_PATH}/*

    DEFINES += HAS_MSEDGEWEBVIEW2

    OUT_LIB_DIR = $${THIRDPARTY_MSEDGEWEBVIEW2_PATH}/build/native/x86
    contains(QMAKE_TARGET.arch, x86_64) {
        OUT_LIB_DIR = $${THIRDPARTY_MSEDGEWEBVIEW2_PATH}/build/native/x64
    }
    contains(QMAKE_TARGET.arch, arm64) {
        OUT_LIB_DIR = $${THIRDPARTY_MSEDGEWEBVIEW2_PATH}/build/native/arm64
    }
    OUT_LIB_NAME = WebView2LoaderStatic.lib

    LIBS += -L$${OUT_LIB_DIR} $${OUT_LIB_NAME}
    DEFINES += LINKED_MSEDGEWEBVIEW2

    *g++*|*clang* {
        LIBS += -ladvapi32
    } else {
        LIBS += Advapi32.lib
    }
}
