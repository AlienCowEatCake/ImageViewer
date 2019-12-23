#
#  Copyright (C) 2017-2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>
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

# ::::: Languages Configuration :::::

# C++11 options:
#    disable_cxx11
#    enable_cxx11
!disable_cxx11 {
    *msvc* {
        isEmpty(QMAKE_MSC_VER) {
            win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 | win32-msvc2013 {
                CONFIG += test_cxx11_incompatible_msvc
            }
        } else {
            !greaterThan(QMAKE_MSC_VER, 1900) { # MSVC2015
                CONFIG += test_cxx11_incompatible_msvc
            }
        }
        !test_cxx11_incompatible_msvc {
            CONFIG += test_cxx11_compatible_msvc
        }
    }
    equals(QT_MAJOR_VERSION, 5) : greaterThan(QT_MINOR_VERSION, 6) {
        CONFIG += test_cxx11_compatible_qt
    }
    greaterThan(QT_MAJOR_VERSION, 5) {
        CONFIG += test_cxx11_compatible_qt
    }
    c++11 | c++1y | c++14 | c++1z | c++17 | c++2a | c++20 {
        CONFIG += test_cxx11_compatible_config
    }
    test_cxx11_compatible_qt | test_cxx11_compatible_config | test_cxx11_compatible_msvc | enable_cxx11 {
        CONFIG -= disable_cxx11
        CONFIG += enable_cxx11
    } else {
        CONFIG += disable_cxx11
        CONFIG -= enable_cxx11
    }
} else {
    CONFIG += disable_cxx11
    CONFIG -= enable_cxx11
}

# ::::: System Libraries Configuration :::::

# pkg-config options:
#    disable_pkgconfig
#    enable_pkgconfig
!enable_pkgconfig {
    !unix | macx {
        CONFIG += disable_pkgconfig
    }
}
!disable_pkgconfig {
    QT_CONFIG -= no-pkg-config
    CONFIG += link_pkgconfig
}

# ZLib options:
#    disable_zlib
#    system_zlib


# Zstandard options:
#    disable_zstd
#    system_zstd


# XZUtils options:
#    disable_xzutils
#    system_xzutils
*msvc* {
    isEmpty(QMAKE_MSC_VER) {
        win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 {
            CONFIG += disable_xzutils # FIXME: C99
        }
    } else {
        !greaterThan(QMAKE_MSC_VER, 1800) { # MSVC2013
            CONFIG += disable_xzutils # FIXME: C99
        }
    }
}

# libexpat options:
#    disable_libexpat
#    system_libexpat


# LCMS options:
#    disable_liblcms2
#    system_liblcms2


# libexif options:
#    disable_libexif
#    system_libexif
*msvc* {
    isEmpty(QMAKE_MSC_VER) {
        win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 | win32-msvc2013 {
            CONFIG += disable_libexif # FIXME: C99
        }
    } else {
        !greaterThan(QMAKE_MSC_VER, 1900) { # MSVC2015
            CONFIG += disable_libexif # FIXME: C99
        }
    }
}

# exiv2 options:
#    disable_exiv2
#    system_exiv2
*msvc* {
    isEmpty(QMAKE_MSC_VER) {
        win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 {
            CONFIG += disable_exiv2 # FIXME: C99
        }
    } else {
        !greaterThan(QMAKE_MSC_VER, 1600) { # MSVC2010
            CONFIG += disable_exiv2 # FIXME: C99
        }
    }
}

# LibJPEG options:
#    disable_libjpeg
#    system_libjpeg


# LibJasPer options:
#    disable_libjasper
#    system_libjasper
*msvc* {
    isEmpty(QMAKE_MSC_VER) {
        win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 {
            CONFIG += disable_libjasper # FIXME: C99
        }
    } else {
        !greaterThan(QMAKE_MSC_VER, 1600) { # MSVC2010
            CONFIG += disable_libjasper # FIXME: C99
        }
    }
}

# libmng options:
#    disable_libmng
#    system_libmng
disable_zlib : !system_libmng {
    CONFIG += disable_libmng
}

# libpng options:
#    disable_libpng
#    system_libpng
disable_zlib : !system_libpng {
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
*msvc* {
    isEmpty(QMAKE_MSC_VER) {
        win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 {
            CONFIG += disable_libbpg # FIXME: C99
        }
    } else {
        !greaterThan(QMAKE_MSC_VER, 1800) { # MSVC2013
            CONFIG += disable_libbpg # FIXME: C99
        }
    }
}

# FreeType options:
#    disable_freetype
#    system_freetype
disable_zlib : !system_freetype {
    CONFIG += disable_freetype
}
disable_zlib : !system_freetype {
    CONFIG += disable_freetype
}

# libwmf options:
#    disable_libwmf
#    system_libwmf
*msvc* {
    isEmpty(QMAKE_MSC_VER) {
        win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 {
            CONFIG += disable_libwmf # FIXME: C99
        }
    } else {
        !greaterThan(QMAKE_MSC_VER, 1600) { # MSVC2010
            CONFIG += disable_libwmf # FIXME: C99
        }
    }
}
disable_zlib : !system_libwmf {
    CONFIG += disable_libwmf
}
disable_libpng : !system_libwmf {
    CONFIG += disable_libwmf
}
disable_freetype : !system_libwmf {
    CONFIG += disable_libwmf
}
disable_libjpeg : !system_libwmf {
    CONFIG += disable_libwmf
}

# OpenJPEG options:
#    disable_openjpeg
#    system_openjpeg


# GIFLIB options:
#    disable_giflib
#    system_giflib
*msvc* {
    isEmpty(QMAKE_MSC_VER) {
        win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 {
            CONFIG += disable_giflib # FIXME: C99
        }
    } else {
        !greaterThan(QMAKE_MSC_VER, 1800) { # MSVC2013
            CONFIG += disable_giflib # FIXME: C99
        }
    }
}

# LibRaw options:
#    disable_libraw
#    system_libraw


# librsvg options:
#    disable_librsvg


# resvg options:
#    disable_resvg
#    enable_resvg
!enable_resvg {
    CONFIG += disable_resvg
}

# libde265 options:
#    disable_libde265
#    system_libde265
disable_cxx11 : !system_libde265 {
    CONFIG += disable_libde265
}

# libheif options:
#    disable_libheif
#    system_libheif
disable_libde265 : !system_libheif {
    CONFIG += disable_libheif
}
disable_cxx11 : !system_libheif {
    CONFIG += disable_libheif
}

# OpenEXR options:
#    disable_openexr
#    system_openexr
disable_zlib : !system_openexr {
    CONFIG += disable_openexr
}
*msvc* : !system_openexr {
    isEmpty(QMAKE_MSC_VER) {
        win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 {
            CONFIG += disable_openexr # FIXME: C99/C++11
        }
    } else {
        !greaterThan(QMAKE_MSC_VER, 1800) { # MSVC2013
            CONFIG += disable_openexr # FIXME: C99/C++11
        }
    }
}

# ::::: Optional Third Party Components Configuration :::::

# QtExtended options:
#    disable_qtextended


# STB options:
#    disable_stb


# NanoSVG options:
#    disable_nanosvg
*msvc* {
    isEmpty(QMAKE_MSC_VER) {
        win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 {
            CONFIG += disable_nanosvg # FIXME: C99
        }
    } else {
        !greaterThan(QMAKE_MSC_VER, 1800) { # MSVC2013
            CONFIG += disable_nanosvg # FIXME: C99
        }
    }
}

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

# DecoderQMLWebEngine options:
#    disable_qmlwebengine
#    enable_qmlwebengine
!enable_qmlwebengine {
    CONFIG += disable_qmlwebengine
}
!greaterThan(QT_MAJOR_VERSION, 4) {
    CONFIG += disable_qmlwebengine
}
equals(QT_MAJOR_VERSION, 5) : lessThan(QT_MINOR_VERSION, 4) {
    CONFIG += disable_qmlwebengine
}

# DecoderMSHTML options:
#    disable_mshtml
!win32 {
    CONFIG += disable_mshtml
}

# DecoderNSImage options:
#    disable_nsimage
!macx {
    CONFIG += disable_nsimage
}

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

# ::::: Cleanup Unised :::::

disable_libtiff | system_libtiff {
    CONFIG += disable_zstd
    CONFIG += disable_xzutils
}

disable_libwmf | system_libwmf {
    CONFIG += disable_freetype
}

system_libheif | disable_libheif {
    CONFIG += disable_libde265
}

disable_exiv2 | system_exiv2 {
    CONFIG += disable_libexpat
}
