# Generated from wbmp.pro.

#####################################################################
## tst_qwbmp Test:
#####################################################################

qt_add_test(tst_qwbmp
    SOURCES
        tst_qwbmp.cpp
    PUBLIC_LIBRARIES
        Qt::Gui
)

# Resources:
set_source_files_properties("../../shared/images/wbmp/qt-logo-big.wbmp"
    PROPERTIES QT_RESOURCE_ALIAS "wbmp/qt-logo-big.wbmp"
)
set_source_files_properties("../../shared/images/wbmp/qt-logo-small.wbmp"
    PROPERTIES QT_RESOURCE_ALIAS "wbmp/qt-logo-small.wbmp"
)
set(wbmp_resource_files
    "wbmp/qt-logo-big.wbmp"
    "wbmp/qt-logo-small.wbmp"
)

qt_add_resource(tst_qwbmp "wbmp"
    PREFIX
        "/"
    BASE
        "../../shared/images"
    FILES
        ${wbmp_resource_files}
)

