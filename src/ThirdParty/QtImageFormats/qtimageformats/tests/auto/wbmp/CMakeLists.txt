# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from wbmp.pro.

#####################################################################
## tst_qwbmp Test:
#####################################################################

qt_internal_add_test(tst_qwbmp
    SOURCES
        tst_qwbmp.cpp
    LIBRARIES
        Qt::Gui
)

# Resources:
set(wbmp_resource_files
    "../../shared/images/wbmp/qt-logo-big.wbmp"
    "../../shared/images/wbmp/qt-logo-small.wbmp"
)

qt_internal_add_resource(tst_qwbmp "wbmp"
    PREFIX
        "/"
    BASE
        "../../shared/images"
    FILES
        ${wbmp_resource_files}
)
