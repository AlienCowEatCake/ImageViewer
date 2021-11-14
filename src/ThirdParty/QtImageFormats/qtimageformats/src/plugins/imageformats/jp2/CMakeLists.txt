# Generated from jp2.pro.

#####################################################################
## QJp2Plugin Plugin:
#####################################################################

qt_internal_add_plugin(QJp2Plugin
    OUTPUT_NAME qjp2
    PLUGIN_TYPE imageformats
    SOURCES
        main.cpp
        qjp2handler.cpp qjp2handler_p.h
    LIBRARIES
        WrapJasper::WrapJasper
    LIBRARIES
        Qt::Core
        Qt::Gui
)

#### Keys ignored in scope 1:.:.:jp2.pro:<TRUE>:
# OTHER_FILES = "jp2.json"
# QT_FOR_CONFIG = "imageformats-private"
