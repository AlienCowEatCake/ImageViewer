# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from wbmp.pro.

#####################################################################
## QWbmpPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWbmpPlugin
    OUTPUT_NAME qwbmp
    PLUGIN_TYPE imageformats
    SOURCES
        main.cpp
        qwbmphandler.cpp qwbmphandler_p.h
    LIBRARIES
        Qt::Core
        Qt::Gui
)

#### Keys ignored in scope 1:.:.:wbmp.pro:<TRUE>:
# OTHER_FILES = "wbmp.json"
