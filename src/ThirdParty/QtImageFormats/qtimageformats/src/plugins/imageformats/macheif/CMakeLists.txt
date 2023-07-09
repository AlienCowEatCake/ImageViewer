# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QMacHeifPlugin Plugin:
#####################################################################

find_library(FWCoreFoundation CoreFoundation)
find_library(FWCoreGraphics CoreGraphics)
find_library(FWImageIO ImageIO)

qt_internal_add_plugin(QMacHeifPlugin
    OUTPUT_NAME qmacheif
    PLUGIN_TYPE imageformats
    SOURCES
        ../shared/qiiofhelpers.cpp ../shared/qiiofhelpers_p.h
        main.cpp
        qmacheifhandler.cpp qmacheifhandler.h
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        ${FWCoreFoundation}
        ${FWCoreGraphics}
        ${FWImageIO}
        Qt::Core
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
)
