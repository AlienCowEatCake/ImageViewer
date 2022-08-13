# NB: the desktop files are installed for the benefit of KImageIO in KDELibs4Support.

##################################

function(kimageformats_add_plugin plugin)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(KIF_ADD_PLUGIN "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    if(NOT KIF_ADD_PLUGIN_SOURCES)
        message(FATAL_ERROR "kimageformats_add_plugin called without SOURCES parameter")
    endif()

    add_library(${plugin} MODULE ${KIF_ADD_PLUGIN_SOURCES})
    set_target_properties(${plugin} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/imageformats")
    target_link_libraries(${plugin} Qt${QT_MAJOR_VERSION}::Gui)
    install(TARGETS ${plugin} DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/imageformats)
endfunction()

##################################

kimageformats_add_plugin(kimg_ani SOURCES ani.cpp)
install(FILES ani.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR}/qimageioplugins/)

##################################

if (TARGET avif)
    kimageformats_add_plugin(kimg_avif SOURCES "avif.cpp")
    target_link_libraries(kimg_avif "avif")
    install(FILES avif.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR}/qimageioplugins/)
endif()

##################################

install(FILES dds-qt.desktop RENAME dds.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR}/qimageioplugins/)

##################################

if (BUILD_EPS_PLUGIN)
    if (TARGET Qt${QT_MAJOR_VERSION}::PrintSupport)
        kimageformats_add_plugin(kimg_eps SOURCES eps.cpp)
        target_link_libraries(kimg_eps Qt${QT_MAJOR_VERSION}::PrintSupport)
        install(FILES eps.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR}/qimageioplugins/)
    endif()
endif()

##################################

# need this for Qt's version of the plugin
install(FILES jp2.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR}/qimageioplugins/)

##################################

if(OpenEXR_FOUND)
    kimageformats_add_plugin(kimg_exr SOURCES exr.cpp)
    if(TARGET OpenEXR::OpenEXR)
        target_link_libraries(kimg_exr OpenEXR::OpenEXR)
    else()
        if(OpenEXR_VERSION_STRING VERSION_LESS 2.3.0)
            # Older OpenEXR versions use dynamic exception specifications, so
            # cannot use C++17 with them
            set_target_properties(kimg_exr PROPERTIES CXX_STANDARD 14)
        endif()
        target_link_libraries(kimg_exr OpenEXR::IlmImf)
    endif()
    kde_target_enable_exceptions(kimg_exr PRIVATE)

    install(FILES exr.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR}/qimageioplugins/)
endif()

##################################

kimageformats_add_plugin(kimg_hdr SOURCES hdr.cpp)
install(FILES hdr.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR}/qimageioplugins/)

##################################

if (LibHeif_FOUND)
    kimageformats_add_plugin(kimg_heif SOURCES heif.cpp)
    target_link_libraries(kimg_heif PkgConfig::LibHeif)
    kde_target_enable_exceptions(kimg_heif PRIVATE)
    install(FILES heif.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR}/qimageioplugins/)
endif()

##################################

if (LibJXL_FOUND AND LibJXLThreads_FOUND)
    kimageformats_add_plugin(kimg_jxl SOURCES jxl.cpp)
    target_link_libraries(kimg_jxl PkgConfig::LibJXL PkgConfig::LibJXLThreads)
    if (LibJXL_VERSION VERSION_GREATER_EQUAL "0.7.0")
        target_compile_definitions(kimg_jxl PRIVATE KIMG_JXL_API_VERSION=70)
    endif()
    install(FILES jxl.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR}/qimageioplugins/)
endif()

##################################

kimageformats_add_plugin(kimg_pcx SOURCES pcx.cpp)
install(FILES pcx.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR}/qimageioplugins/)

##################################

kimageformats_add_plugin(kimg_pic SOURCES pic.cpp)
install(FILES pic.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR}/qimageioplugins/)

##################################

kimageformats_add_plugin(kimg_psd SOURCES psd.cpp)
install(FILES psd.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR}/qimageioplugins/)

##################################

kimageformats_add_plugin(kimg_ras SOURCES ras.cpp)
install(FILES ras.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR}/qimageioplugins/)

##################################

kimageformats_add_plugin(kimg_rgb SOURCES rgb.cpp)
install(FILES rgb.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR}/qimageioplugins/)

##################################

kimageformats_add_plugin(kimg_tga SOURCES tga.cpp)
install(FILES tga.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR}/qimageioplugins/)

##################################

kimageformats_add_plugin(kimg_xcf SOURCES xcf.cpp)
install(FILES xcf.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR}/qimageioplugins/)

##################################

if (KF5Archive_FOUND)

    kimageformats_add_plugin(kimg_kra SOURCES kra.cpp)
    target_link_libraries(kimg_kra KF5::Archive)
    install(FILES kra.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR}/qimageioplugins/)

    kimageformats_add_plugin(kimg_ora SOURCES ora.cpp)
    target_link_libraries(kimg_ora KF5::Archive)
    install(FILES ora.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR}/qimageioplugins/)

endif()
