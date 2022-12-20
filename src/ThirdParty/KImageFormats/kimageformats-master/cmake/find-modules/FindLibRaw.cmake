# - Find LibRaw
# Find the LibRaw library <https://www.libraw.org>
# This module defines
#  LibRaw_VERSION, the version string of LibRaw
#  LibRaw_INCLUDE_DIR, where to find libraw/libraw.h
#  LibRaw_LIBRARIES, the libraries needed to use LibRaw (non-thread-safe)
#  LibRaw_r_LIBRARIES, the libraries needed to use LibRaw (thread-safe)
#  LibRaw_DEFINITIONS, the definitions needed to use LibRaw (non-thread-safe)
#  LibRaw_r_DEFINITIONS, the definitions needed to use LibRaw (thread-safe)
#
# SPDX-FileCopyrightText: 2013 Pino Toscano <pino at kde dot org>
# SPDX-FileCopyrightText: 2013 Gilles Caulier <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause

FIND_PACKAGE(PkgConfig)

IF(PKG_CONFIG_FOUND)
   PKG_CHECK_MODULES(PC_LIBRAW libraw)
   SET(LibRaw_DEFINITIONS ${PC_LIBRAW_CFLAGS_OTHER})

   PKG_CHECK_MODULES(PC_LIBRAW_R libraw_r)
   SET(LibRaw_r_DEFINITIONS ${PC_LIBRAW_R_CFLAGS_OTHER})   
ENDIF()

FIND_PATH(LibRaw_INCLUDE_DIR libraw/libraw.h
          HINTS
          ${PC_LIBRAW_INCLUDEDIR}
          ${PC_LibRaw_INCLUDE_DIRS}
         )

FIND_LIBRARY(LibRaw_LIBRARIES NAMES raw
             HINTS
             ${PC_LIBRAW_LIBDIR}
             ${PC_LIBRAW_LIBRARY_DIRS}
            )

FIND_LIBRARY(LibRaw_r_LIBRARIES NAMES raw_r
             HINTS
             ${PC_LIBRAW_R_LIBDIR}
             ${PC_LIBRAW_R_LIBRARY_DIRS}
            )

IF(LibRaw_INCLUDE_DIR)
   FILE(READ ${LibRaw_INCLUDE_DIR}/libraw/libraw_version.h _libraw_version_content)
   
   STRING(REGEX MATCH "#define LIBRAW_MAJOR_VERSION[ \t]*([0-9]*)\n" _version_major_match ${_libraw_version_content})
   SET(_libraw_version_major "${CMAKE_MATCH_1}")
   
   STRING(REGEX MATCH "#define LIBRAW_MINOR_VERSION[ \t]*([0-9]*)\n" _version_minor_match ${_libraw_version_content})
   SET(_libraw_version_minor "${CMAKE_MATCH_1}")
   
   STRING(REGEX MATCH "#define LIBRAW_PATCH_VERSION[ \t]*([0-9]*)\n" _version_patch_match ${_libraw_version_content})
   SET(_libraw_version_patch "${CMAKE_MATCH_1}")
   
   IF(_version_major_match AND _version_minor_match AND _version_patch_match)
      SET(LibRaw_VERSION "${_libraw_version_major}.${_libraw_version_minor}.${_libraw_version_patch}")
   ELSE()
      IF(NOT LibRaw_FIND_QUIETLY)
         MESSAGE(STATUS "Failed to get version information from ${LibRaw_INCLUDE_DIR}/libraw/libraw_version.h")
      ENDIF()
   ENDIF()
ENDIF()

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibRaw
                                  REQUIRED_VARS LibRaw_LIBRARIES LibRaw_INCLUDE_DIR
                                  VERSION_VAR LibRaw_VERSION
                                 )

MARK_AS_ADVANCED(LibRaw_VERSION
                 LibRaw_INCLUDE_DIR
                 LibRaw_LIBRARIES
                 LibRaw_r_LIBRARIES
                 LibRaw_DEFINITIONS
                 LibRaw_r_DEFINITIONS
                 )

if(LibRaw_FOUND AND NOT TARGET LibRaw::LibRaw)
    add_library(LibRaw::LibRaw UNKNOWN IMPORTED)
    set_target_properties(LibRaw::LibRaw PROPERTIES
        IMPORTED_LOCATION "${LibRaw_LIBRARIES}"
        INTERFACE_COMPILE_OPTIONS "${LibRaw_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${LibRaw_INCLUDE_DIR}"
    )
endif()
