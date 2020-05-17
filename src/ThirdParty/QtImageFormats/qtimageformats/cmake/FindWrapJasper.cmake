set(WrapJasper_FOUND OFF)
find_package(Jasper)

if(Jasper_FOUND)
    set(WrapJasper_FOUND ON)

    # Upstream package does not provide targets, only variables. So define a target.
    add_library(WrapJasper::WrapJasper INTERFACE IMPORTED)
    target_link_libraries(WrapJasper::WrapJasper INTERFACE ${JASPER_LIBRARIES})
    target_include_directories(WrapJasper::WrapJasper INTERFACE ${JASPER_INCLUDE_DIR})
endif()
