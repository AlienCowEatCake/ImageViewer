# Generated from wbmp.pro.

#####################################################################
## QWbmpPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWbmpPlugin
    OUTPUT_NAME qwbmp
    TYPE imageformats
    SOURCES
        main.cpp
        qwbmphandler.cpp qwbmphandler_p.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
)

#### Keys ignored in scope 1:.:.:wbmp.pro:<TRUE>:
# OTHER_FILES = "wbmp.json"
