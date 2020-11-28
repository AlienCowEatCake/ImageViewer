# Generated from tga.pro.

#####################################################################
## tst_qtga Test:
#####################################################################

qt_internal_add_test(tst_qtga
    SOURCES
        tst_qtga.cpp
    PUBLIC_LIBRARIES
        Qt::Gui
)

# Resources:
set_source_files_properties("../../shared/images/tga/test-flag.tga"
    PROPERTIES QT_RESOURCE_ALIAS "tga/test-flag.tga"
)
set(tga_resource_files
    "tga/test-flag.tga"
)

qt_internal_add_resource(tst_qtga "tga"
    PREFIX
        "/"
    BASE
        "../../shared/images"
    FILES
        ${tga_resource_files}
)

