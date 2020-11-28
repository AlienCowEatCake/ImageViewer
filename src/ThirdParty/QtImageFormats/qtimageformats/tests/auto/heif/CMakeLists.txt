# Generated from heif.pro.

#####################################################################
## tst_qheif Test:
#####################################################################

qt_internal_add_test(tst_qheif
    SOURCES
        tst_qheif.cpp
    PUBLIC_LIBRARIES
        Qt::Gui
)

# Resources:
set_source_files_properties("../../shared/images/heif/col320x480.heic"
    PROPERTIES QT_RESOURCE_ALIAS "heif/col320x480.heic"
)
set_source_files_properties("../../shared/images/heif/newlogoCCW.heic"
    PROPERTIES QT_RESOURCE_ALIAS "heif/newlogoCCW.heic"
)
set(heif_resource_files
    "heif/col320x480.heic"
    "heif/newlogoCCW.heic"
)

qt_internal_add_resource(tst_qheif "heif"
    PREFIX
        "/"
    BASE
        "../../shared/images"
    FILES
        ${heif_resource_files}
)

