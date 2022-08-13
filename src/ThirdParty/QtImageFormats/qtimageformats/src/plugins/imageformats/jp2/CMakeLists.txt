# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from jp2.pro.

#####################################################################
## QJp2Plugin Plugin:
#####################################################################

qt_internal_add_plugin(QJp2Plugin
    OUTPUT_NAME qjp2
    PLUGIN_TYPE imageformats
    SOURCES
        main.cpp
        qjp2handler.cpp qjp2handler_p.h
    LIBRARIES
        WrapJasper::WrapJasper
        Qt::Core
        Qt::Gui
)

#### Keys ignored in scope 1:.:.:jp2.pro:<TRUE>:
# OTHER_FILES = "jp2.json"
# QT_FOR_CONFIG = "imageformats-private"
