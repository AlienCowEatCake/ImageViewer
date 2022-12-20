cmake_minimum_required(VERSION 3.16)

project(KImageFormats)

include(FeatureSummary)
find_package(ECM 5.101.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)


set(CMAKE_MODULE_PATH ${KImageFormats_SOURCE_DIR}/cmake/find-modules ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

include(KDEGitCommitHooks)
include(ECMDeprecationSettings)
include(CheckIncludeFiles)
include(FindPkgConfig)

set(REQUIRED_QT_VERSION 5.15.2)
find_package(Qt${QT_MAJOR_VERSION}Gui ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)

find_package(KF5Archive)
set_package_properties(KF5Archive PROPERTIES
    TYPE OPTIONAL
    PURPOSE "Required for the QImage plugin for Krita and OpenRaster images"
)

# EPS support depends on the gs utility; non-UNIX systems are unlikely to have
# this available in PATH
set(BUILD_EPS_PLUGIN FALSE)
if (UNIX)
    find_package(Qt${QT_MAJOR_VERSION}PrintSupport ${REQUIRED_QT_VERSION} NO_MODULE)
    set_package_properties(Qt${QT_MAJOR_VERSION}PrintSupport PROPERTIES
        PURPOSE "Required for the QImage plugin for EPS images"
        TYPE OPTIONAL
    )
    if (TARGET Qt${QT_MAJOR_VERSION}::PrintSupport)
        set(BUILD_EPS_PLUGIN TRUE)
    endif()
endif()

find_package(OpenEXR 3.0 CONFIG QUIET)
if(NOT OpenEXR_FOUND)
    find_package(OpenEXR)
endif()
set_package_properties(OpenEXR PROPERTIES
    TYPE OPTIONAL
    PURPOSE "Required for the QImage plugin for OpenEXR images"
)

find_package(libavif 0.8.2 CONFIG)
set_package_properties(libavif PROPERTIES
    TYPE OPTIONAL
    PURPOSE "Required for the QImage plugin for AVIF images"
)

option(KIMAGEFORMATS_HEIF "Enable plugin for HEIF format" OFF)
if(KIMAGEFORMATS_HEIF)
    pkg_check_modules(LibHeif IMPORTED_TARGET libheif>=1.10.0)
endif()
add_feature_info(LibHeif LibHeif_FOUND "required for the QImage plugin for HEIF/HEIC images")

option(KIMAGEFORMATS_JXL "Enable plugin for JPEG XL format" ON)
if(KIMAGEFORMATS_JXL)
    pkg_check_modules(LibJXL IMPORTED_TARGET libjxl>=0.6.1)
    pkg_check_modules(LibJXLThreads IMPORTED_TARGET libjxl_threads>=0.6.1)
endif()
add_feature_info(LibJXL LibJXL_FOUND "required for the QImage plugin for JPEG XL images")

# note: module FindLibRaw missing from https://invent.kde.org/frameworks/extra-cmake-modules
find_package(LibRaw 0.20.2)
set_package_properties(LibRaw PROPERTIES
    TYPE OPTIONAL
    PURPOSE "Required for the QImage plugin for RAW images"
)

ecm_set_disabled_deprecation_versions(
    QT 5.15.2
    KF 5.95
)

add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
