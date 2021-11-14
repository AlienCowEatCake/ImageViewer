# Generated from imageformats.pro.

# special case begin
qt_feature_module_begin(
    NO_MODULE
    PUBLIC_FILE "qtimageformats-config.h"
    PRIVATE_FILE "qtimageformats-config_p.h"
)
include(../../imageformats/configure.cmake)
qt_feature_module_end(NO_MODULE)
# special case end

add_subdirectory(tga)
add_subdirectory(wbmp)
if(QT_FEATURE_tiff)
    add_subdirectory(tiff)
endif()
if(QT_FEATURE_webp)
    add_subdirectory(webp)
endif()
if(APPLE)
    add_subdirectory(macheif)
endif()
if(QT_FEATURE_regularexpression)
    add_subdirectory(icns)
endif()
if(QT_FEATURE_mng)
    add_subdirectory(mng)
endif()
if(QT_FEATURE_jasper)
    add_subdirectory(jp2)
endif()
if(APPLE AND NOT QT_FEATURE_jasper)
    add_subdirectory(macjp2)
endif()
