# Generated from mng.pro.

#####################################################################
## tst_qmng Test:
#####################################################################

qt_internal_add_test(tst_qmng
    SOURCES
        tst_qmng.cpp
    PUBLIC_LIBRARIES
        Qt::Gui
)

# Resources:
set_source_files_properties("../../shared/images/mng/animation.mng"
    PROPERTIES QT_RESOURCE_ALIAS "mng/animation.mng"
)
set_source_files_properties("../../shared/images/mng/ball.mng"
    PROPERTIES QT_RESOURCE_ALIAS "mng/ball.mng"
)
set_source_files_properties("../../shared/images/mng/corrupt.mng"
    PROPERTIES QT_RESOURCE_ALIAS "mng/corrupt.mng"
)
set_source_files_properties("../../shared/images/mng/dutch.mng"
    PROPERTIES QT_RESOURCE_ALIAS "mng/dutch.mng"
)
set_source_files_properties("../../shared/images/mng/fire.mng"
    PROPERTIES QT_RESOURCE_ALIAS "mng/fire.mng"
)
set(mng_resource_files
    "mng/animation.mng"
    "mng/ball.mng"
    "mng/corrupt.mng"
    "mng/dutch.mng"
    "mng/fire.mng"
)

qt_internal_add_resource(tst_qmng "mng"
    PREFIX
        "/"
    BASE
        "../../shared/images"
    FILES
        ${mng_resource_files}
)

