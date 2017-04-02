#
#  Copyright (C) 2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>
#
#  This file is part of the `ImageViewer' program.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# ::::: System Libraries Configuration :::::

# ZLib options:
#    disable_zlib
#    system_zlib
macx {
    CONFIG += system_zlib
}

# LibJPEG options:
#    disable_libjpeg
#    system_libjpeg


# LibJasPer options:
#    disable_libjasper
#    system_libjasper
win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 | win32-msvc2013 {
    CONFIG += disable_libjasper # FIXME: C99
}

# LCMS options:
#    disable_liblcms2
#    system_liblcms2


# libexif options:
#    disable_libexif
#    system_libexif



# ::::: Optional Third Party Components Configuration :::::

# QtExtended options:
#    disable_qtextended


# STB options:
#    disable_stb


# QtImageFormats options:
#    disable_qtimageformats
!greaterThan(QT_MAJOR_VERSION, 4) {
    CONFIG += disable_qtimageformats
}

