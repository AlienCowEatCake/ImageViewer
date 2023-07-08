include(LibFindMacros)
include(CheckSymbolExists)

libfind_pkg_check_modules(AOM_PKGCONF aom)

find_path(AOM_INCLUDE_DIR
    NAMES aom/aom_decoder.h aom/aom_encoder.h
    HINTS ${AOM_PKGCONF_INCLUDE_DIRS} ${AOM_PKGCONF_INCLUDEDIR}
    PATH_SUFFIXES AOM
)

check_symbol_exists(AOM_USAGE_GOOD_QUALITY ${AOM_INCLUDE_DIR}/aom/aom_encoder.h aom_usage_flag_exists)

find_library(AOM_LIBRARY
    NAMES libaom aom
    HINTS ${AOM_PKGCONF_LIBRARY_DIRS} ${AOM_PKGCONF_LIBDIR}
)

if(EXISTS "${AOM_INCLUDE_DIR}/aom/aom_decoder.h")
    set(AOM_DECODER_FOUND YES)
else()
    set(AOM_DECODER_FOUND NO)
endif()

if((EXISTS "${AOM_INCLUDE_DIR}/aom/aom_encoder.h") AND (DEFINED aom_usage_flag_exists))
    set(AOM_ENCODER_FOUND YES)
else()
    set(AOM_ENCODER_FOUND NO)
endif()

set(AOM_PROCESS_LIBS AOM_LIBRARY)
set(AOM_PROCESS_INCLUDES AOM_INCLUDE_DIR)
libfind_process(AOM)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AOM
    REQUIRED_VARS
        AOM_INCLUDE_DIR
        AOM_LIBRARIES
)
