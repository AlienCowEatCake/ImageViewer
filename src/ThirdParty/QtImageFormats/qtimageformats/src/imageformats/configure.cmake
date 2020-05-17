

#### Inputs

# input tiff
set(INPUT_tiff "undefined" CACHE STRING "")
set_property(CACHE INPUT_tiff PROPERTY STRINGS undefined no qt system)

# input webp
set(INPUT_webp "undefined" CACHE STRING "")
set_property(CACHE INPUT_webp PROPERTY STRINGS undefined no qt system)



#### Libraries

qt_find_package(WrapJasper PROVIDED_TARGETS WrapJasper::WrapJasper)
qt_find_package(TIFF PROVIDED_TARGETS TIFF::TIFF)
qt_find_package(WrapWebP PROVIDED_TARGETS WrapWebP::WrapWebP)


#### Tests



#### Features

qt_feature("jasper" PRIVATE
    LABEL "JasPer"
    CONDITION QT_FEATURE_imageformatplugin AND WrapJasper_FOUND
    DISABLE INPUT_jasper STREQUAL 'no'
)
qt_feature_definition("jasper" "QT_NO_IMAGEFORMAT_JASPER" NEGATE)
qt_feature("mng" PRIVATE
    LABEL "MNG"
    CONDITION libs.mng OR FIXME
    DISABLE INPUT_mng STREQUAL 'no'
)
qt_feature("tiff" PRIVATE
    LABEL "TIFF"
    CONDITION QT_FEATURE_imageformatplugin AND TIFF_FOUND
    DISABLE INPUT_tiff STREQUAL 'no'
)
qt_feature("system_tiff" PRIVATE
    LABEL "  Using system libtiff"
    CONDITION QT_FEATURE_tiff AND TIFF_FOUND
    ENABLE INPUT_tiff STREQUAL 'system'
    DISABLE INPUT_tiff STREQUAL 'qt'
)
qt_feature("webp" PRIVATE
    LABEL "WEBP"
    CONDITION QT_FEATURE_imageformatplugin AND WrapWebP_FOUND
    DISABLE INPUT_webp STREQUAL 'no'
)
qt_feature("system_webp" PRIVATE
    LABEL "  Using system libwebp"
    CONDITION QT_FEATURE_webp AND WrapWebP_FOUND
    ENABLE INPUT_webp STREQUAL 'system'
    DISABLE INPUT_webp STREQUAL 'qt'
)
