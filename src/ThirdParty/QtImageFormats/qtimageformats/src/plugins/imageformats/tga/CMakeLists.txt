# Generated from tga.pro.

#####################################################################
## QTgaPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QTgaPlugin
    OUTPUT_NAME qtga
    TYPE imageformats
    SOURCES
        main.cpp
        qtgafile.cpp qtgafile.h
        qtgahandler.cpp qtgahandler.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
)

#### Keys ignored in scope 1:.:.:tga.pro:<TRUE>:
# OTHER_FILES = "tga.json"
