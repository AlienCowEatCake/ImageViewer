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
set(mng_resource_files
    "../../shared/images/mng/animation.mng"
    "../../shared/images/mng/ball.mng"
    "../../shared/images/mng/corrupt.mng"
    "../../shared/images/mng/dutch.mng"
    "../../shared/images/mng/fire.mng"
)

qt_internal_add_resource(tst_qmng "mng"
    PREFIX
        "/"
    BASE
        "../../shared/images"
    FILES
        ${mng_resource_files}
)
