# Generated from qtimageformats.pro.

cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtImageFormats # special case
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt ImageFormats Libraries" # special case
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core) # special case
find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG OPTIONAL_COMPONENTS Gui) # special case
if(NOT TARGET Qt::Gui)
    message(NOTICE "Skipping the build as the condition \"TARGET Qt::Gui\" is not met.")
    return()
endif()
qt_build_repo()
