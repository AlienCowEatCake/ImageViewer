# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from tga.pro.

#####################################################################
## QTgaPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QTgaPlugin
    OUTPUT_NAME qtga
    PLUGIN_TYPE imageformats
    SOURCES
        main.cpp
        qtgafile.cpp qtgafile.h
        qtgahandler.cpp qtgahandler.h
    LIBRARIES
        Qt::Core
        Qt::Gui
)

#### Keys ignored in scope 1:.:.:tga.pro:<TRUE>:
# OTHER_FILES = "tga.json"
