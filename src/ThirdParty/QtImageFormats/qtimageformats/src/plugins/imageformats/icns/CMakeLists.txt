# Generated from icns.pro.

#####################################################################
## QICNSPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QICNSPlugin
    OUTPUT_NAME qicns
    PLUGIN_TYPE imageformats
    SOURCES
        main.cpp
        qicnshandler.cpp qicnshandler_p.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
    LIBRARIES
        Qt::Core
        Qt::Gui
)

#### Keys ignored in scope 1:.:.:icns.pro:<TRUE>:
# OTHER_FILES = "icns.json"
