#
#  Copyright (C) 2017-2018 Peter S. Zhigalov <peter.zhigalov@gmail.com>
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


# XZUtils options:
#    disable_xzutils
#    system_xzutils
win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 {
    CONFIG += disable_xzutils # FIXME: C99
}

# LCMS options:
#    disable_liblcms2
#    system_liblcms2


# libexif options:
#    disable_libexif
#    system_libexif
win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 | win32-msvc2013 {
    CONFIG += disable_libexif # FIXME: C99
}

# LibJPEG options:
#    disable_libjpeg
#    system_libjpeg


# LibJasPer options:
#    disable_libjasper
#    system_libjasper


# libmng options:
#    disable_libmng
#    system_libmng
disable_zlib {
    CONFIG += disable_libmng
}

# libpng options:
#    disable_libpng
#    system_libpng
disable_zlib {
    CONFIG += disable_libpng
}

# jbigkit options:
#    disable_jbigkit
#    system_jbigkit


# libtiff options:
#    disable_libtiff
#    system_libtiff


# LibWebP options:
#    disable_libwebp
#    system_libwebp


# libbpg options:
#    disable_libbpg
#    system_libbpg
win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 {
    CONFIG += disable_libbpg # FIXME: C99
}

# FreeType options:
#    disable_freetype
#    system_freetype
disable_zlib | disable_libpng {
    CONFIG += disable_freetype
}

# libwmf options:
#    disable_libwmf
#    system_libwmf
disable_zlib | disable_libpng | disable_freetype | disable_libjpeg {
    CONFIG += disable_libwmf
}

# OpenJPEG options:
#    disable_openjpeg
#    system_openjpeg


# GIFLIB options:
#    disable_giflib
#    system_giflib
win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 {
    CONFIG += disable_giflib # FIXME: C99
}

# librsvg options:
#    disable_librsvg


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

# ::::: Optional Built-in Components Configuration :::::

# DecoderQtSVG options:
#    disable_qtsvg


# DecoderQtWebKit options:
#    disable_qtwebkit
#    enable_qtwebkit
!enable_qtwebkit {
    CONFIG += disable_qtwebkit
}

# DecoderQtWebEngine options:
#    disable_qtwebengine
#    enable_qtwebengine
!enable_qtwebengine {
    CONFIG += disable_qtwebengine
}
!greaterThan(QT_MAJOR_VERSION, 4) {
    CONFIG += disable_qtwebengine
}
equals(QT_MAJOR_VERSION, 5) : lessThan(QT_MINOR_VERSION, 4) {
    CONFIG += disable_qtwebengine
}

# DecoderNSImage options:
#    disable_nsimage


# DecoderMacWebKit options:
#    disable_macwebkit
!macx {
    CONFIG += disable_macwebkit
}

# MacToolBar options:
#    disable_mactoolbar
!macx {
    CONFIG += disable_mactoolbar
}

