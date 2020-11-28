# Generated from icns.pro.

#####################################################################
## tst_qicns Test:
#####################################################################

qt_internal_add_test(tst_qicns
    SOURCES
        tst_qicns.cpp
    PUBLIC_LIBRARIES
        Qt::Gui
)

# Resources:
set_source_files_properties("../../shared/images/icns/test-32bit.icns"
    PROPERTIES QT_RESOURCE_ALIAS "icns/test-32bit.icns"
)
set_source_files_properties("../../shared/images/icns/test-jp2.icns"
    PROPERTIES QT_RESOURCE_ALIAS "icns/test-jp2.icns"
)
set_source_files_properties("../../shared/images/icns/test-legacy.icns"
    PROPERTIES QT_RESOURCE_ALIAS "icns/test-legacy.icns"
)
set_source_files_properties("../../shared/images/icns/test-png.icns"
    PROPERTIES QT_RESOURCE_ALIAS "icns/test-png.icns"
)
set_source_files_properties("../../shared/images/icns/test-variants.icns"
    PROPERTIES QT_RESOURCE_ALIAS "icns/test-variants.icns"
)
set_source_files_properties("../../shared/images/icns/test-write-1024.png"
    PROPERTIES QT_RESOURCE_ALIAS "icns/test-write-1024.png"
)
set_source_files_properties("../../shared/images/icns/test-write-128.png"
    PROPERTIES QT_RESOURCE_ALIAS "icns/test-write-128.png"
)
set_source_files_properties("../../shared/images/icns/test-write-16.png"
    PROPERTIES QT_RESOURCE_ALIAS "icns/test-write-16.png"
)
set_source_files_properties("../../shared/images/icns/test-write-256.png"
    PROPERTIES QT_RESOURCE_ALIAS "icns/test-write-256.png"
)
set_source_files_properties("../../shared/images/icns/test-write-32.png"
    PROPERTIES QT_RESOURCE_ALIAS "icns/test-write-32.png"
)
set_source_files_properties("../../shared/images/icns/test-write-512.png"
    PROPERTIES QT_RESOURCE_ALIAS "icns/test-write-512.png"
)
set_source_files_properties("../../shared/images/icns/test-write-64.png"
    PROPERTIES QT_RESOURCE_ALIAS "icns/test-write-64.png"
)
set(icns_resource_files
    "icns/test-32bit.icns"
    "icns/test-jp2.icns"
    "icns/test-legacy.icns"
    "icns/test-png.icns"
    "icns/test-variants.icns"
    "icns/test-write-1024.png"
    "icns/test-write-128.png"
    "icns/test-write-16.png"
    "icns/test-write-256.png"
    "icns/test-write-32.png"
    "icns/test-write-512.png"
    "icns/test-write-64.png"
)

qt_internal_add_resource(tst_qicns "icns"
    PREFIX
        "/"
    BASE
        "../../shared/images"
    FILES
        ${icns_resource_files}
)

