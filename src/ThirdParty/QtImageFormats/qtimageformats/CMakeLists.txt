# Generated from qtimageformats.pro.

cmake_minimum_required(VERSION 3.15.0)

project(QtImageFormats # special case
    VERSION 6.0.0
    DESCRIPTION "Qt ImageFormats Libraries" # special case
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Gui) # special case

if(NOT TARGET Qt::Gui)
    message(NOTICE "Skipping the build as the condition \"TARGET Qt::Gui\" is not met.")
    return()
endif()
qt_build_repo()
