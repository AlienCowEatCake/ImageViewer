# Generated from macjp2.pro.

#####################################################################
## QMacJp2Plugin Plugin:
#####################################################################

qt_internal_add_plugin(QMacJp2Plugin
    OUTPUT_NAME qmacjp2
    PLUGIN_TYPE imageformats
    SOURCES
        ../shared/qiiofhelpers.cpp ../shared/qiiofhelpers_p.h
        main.cpp
        qmacjp2handler.cpp qmacjp2handler.h
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        ${FWCoreFoundation}
        ${FWCoreGraphics}
        ${FWImageIO}
        Qt::Core
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
)

#### Keys ignored in scope 1:.:.:macjp2.pro:<TRUE>:
# OTHER_FILES = "macjp2.json"
