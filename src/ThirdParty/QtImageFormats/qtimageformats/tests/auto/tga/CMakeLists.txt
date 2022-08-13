# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from tga.pro.

#####################################################################
## tst_qtga Test:
#####################################################################

qt_internal_add_test(tst_qtga
    SOURCES
        tst_qtga.cpp
    LIBRARIES
        Qt::Gui
)

# Resources:
set(tga_resource_files
    "../../shared/images/tga/test-flag.tga"
)

qt_internal_add_resource(tst_qtga "tga"
    PREFIX
        "/"
    BASE
        "../../shared/images"
    FILES
        ${tga_resource_files}
)
