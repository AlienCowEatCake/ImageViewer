# Generated from webp.pro.

#####################################################################
## QWebpPlugin Plugin:
#####################################################################

qt_add_plugin(QWebpPlugin
    OUTPUT_NAME qwebp
    TYPE imageformats
    SOURCES
        main.cpp
        qwebphandler.cpp qwebphandler_p.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        WrapWebP::WrapWebP
)

#### Keys ignored in scope 1:.:.:webp.pro:<TRUE>:
# OTHER_FILES = "webp.json"
# QT_FOR_CONFIG = "imageformats-private"

## Scopes:
#####################################################################
