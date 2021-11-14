# Generated from wbmp.pro.

#####################################################################
## tst_qwbmp Test:
#####################################################################

qt_internal_add_test(tst_qwbmp
    SOURCES
        tst_qwbmp.cpp
    PUBLIC_LIBRARIES
        Qt::Gui
)

# Resources:
set(wbmp_resource_files
    "../../shared/images/wbmp/qt-logo-big.wbmp"
    "../../shared/images/wbmp/qt-logo-small.wbmp"
)

qt_internal_add_resource(tst_qwbmp "wbmp"
    PREFIX
        "/"
    BASE
        "../../shared/images"
    FILES
        ${wbmp_resource_files}
)
