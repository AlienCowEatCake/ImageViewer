/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the config.tests of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <tiffio.h>

#if !defined(TIFF_VERSION) && defined(TIFF_VERSION_CLASSIC)
// libtiff 4.0 splits it into TIFF_VERSION_CLASSIC and TIFF_VERSION_BIG
#    define TIFF_VERSION TIFF_VERSION_CLASSIC
#endif

#if !defined(TIFF_VERSION)
#    error "Required libtiff not found"
#elif TIFF_VERSION < 42
#    error "unsupported tiff version"
#endif

int main(int, char **)
{
    tdata_t buffer = _TIFFmalloc(128);
    _TIFFfree(buffer);

    // some libtiff implementations where TIFF_VERSION >= 42 do not
    // have TIFFReadRGBAImageOriented(), so let's check for it
    TIFFReadRGBAImageOriented(0, 0, 0, 0, 0, 0);

    return 0;
}
