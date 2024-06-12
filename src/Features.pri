#
#  Copyright (C) 2017-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>
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

# Autodetect C++ standard if not specified in CONFIG
!c++latest : !c++03 : !c++0x : !c++11 : !c++1y : !c++14 : !c++1z : !c++17 : !c++2a : !c++20 : !c++2b : !c++23 {
    contains(QT_CONFIG, c++17) {
        CONFIG += c++17
    } else : contains(QT_CONFIG, c++1z) {
        CONFIG += c++1z
    } else : contains(QT_CONFIG, c++14) {
        CONFIG += c++14
    } else : contains(QT_CONFIG, c++1y) {
        CONFIG += c++1y
    } else : contains(QT_CONFIG, c++11) {
        CONFIG += c++11
    } else : contains(QT_CONFIG, c++0x) {
        CONFIG += c++0x
    }
}

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
            !greaterThan(QMAKE_MSC_VER, 1800) { # MSVC2013
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
    c++latest | c++11 | c++1y | c++14 | c++1z | c++17 | c++2a | c++20 | c++2b | c++23 {
        CONFIG += test_cxx11_compatible_config
    }
    # Workaround: MSVC 2013 is not a C++11-conformant compiler
    test_cxx11_incompatible_msvc {
        CONFIG -= test_cxx11_compatible_qt
        CONFIG -= test_cxx11_compatible_config
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

# C++14 options:
#    disable_cxx14
#    enable_cxx14
!disable_cxx14 {
    *msvc* {
        isEmpty(QMAKE_MSC_VER) {
            win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 | win32-msvc2013 | win32-msvc2015 {
                CONFIG += test_cxx14_incompatible_msvc
            }
        } else {
            !greaterThan(QMAKE_MSC_VER, 1900) { # MSVC2015
                CONFIG += test_cxx14_incompatible_msvc
            }
        }
        !test_cxx14_incompatible_msvc {
            CONFIG += test_cxx14_compatible_msvc
        }
    }
    greaterThan(QT_MAJOR_VERSION, 5) {
        CONFIG += test_cxx14_compatible_qt
    }
    c++latest | c++14 | c++1z | c++17 | c++2a | c++20 | c++2b | c++23 {
        CONFIG += test_cxx14_compatible_config
    }
    # Workaround: MSVC 2015 is not a C++14-conformant compiler
    test_cxx14_incompatible_msvc {
        CONFIG -= test_cxx14_compatible_qt
        CONFIG -= test_cxx14_compatible_config
    }
    test_cxx14_compatible_qt | test_cxx14_compatible_config | test_cxx14_compatible_msvc | enable_cxx14 {
        CONFIG -= disable_cxx14
        CONFIG += enable_cxx14
    } else {
        CONFIG += disable_cxx14
        CONFIG -= enable_cxx14
    }
} else {
    CONFIG += disable_cxx14
    CONFIG -= enable_cxx14
}

# C++17 options:
#    disable_cxx17
#    enable_cxx17
!disable_cxx17 {
    *msvc* {
        isEmpty(QMAKE_MSC_VER) {
            win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 | win32-msvc2013 | win32-msvc2015 | win32-msvc2017 {
                CONFIG += test_cxx17_incompatible_msvc
            }
        } else {
            !greaterThan(QMAKE_MSC_VER, 1919) { # MSVC2017
                CONFIG += test_cxx17_incompatible_msvc
            }
        }
        !test_cxx17_incompatible_msvc {
            CONFIG += test_cxx17_compatible_msvc
        }
    }
    greaterThan(QT_MAJOR_VERSION, 5) {
        CONFIG += test_cxx17_compatible_qt
    }
    c++latest | c++17 | c++2a | c++20 | c++2b | c++23 {
        CONFIG += test_cxx17_compatible_config
    }
    # Workaround: MSVC 2017 is not a C++17-conformant compiler
    test_cxx17_incompatible_msvc {
        CONFIG -= test_cxx17_compatible_qt
        CONFIG -= test_cxx17_compatible_config
    }
    test_cxx17_compatible_qt | test_cxx17_compatible_config | test_cxx17_compatible_msvc | enable_cxx17 {
        CONFIG -= disable_cxx17
        CONFIG += enable_cxx17
    } else {
        CONFIG += disable_cxx17
        CONFIG -= enable_cxx17
    }
} else {
    CONFIG += disable_cxx17
    CONFIG -= enable_cxx17
}

# Cleanup C++
disable_cxx11 {
    enable_cxx14 {
        CONFIG -= enable_cxx14
    }
    !disable_cxx14 {
        CONFIG += disable_cxx14
    }
    enable_cxx17 {
        CONFIG -= enable_cxx17
    }
    !disable_cxx17 {
        CONFIG += disable_cxx17
    }
}
enable_cxx14 {
    disable_cxx11 {
        CONFIG -= disable_cxx11
    }
    !enable_cxx11 {
        CONFIG += enable_cxx11
    }
}
disable_cxx14 {
    enable_cxx17 {
        CONFIG -= enable_cxx17
    }
    !disable_cxx17 {
        CONFIG += disable_cxx17
    }
}
enable_cxx17 {
    disable_cxx11 {
        CONFIG -= disable_cxx11
    }
    !enable_cxx11 {
        CONFIG += enable_cxx11
    }
    disable_cxx14 {
        CONFIG -= disable_cxx14
    }
    !enable_cxx14 {
        CONFIG += enable_cxx14
    }
}

# Upgrade lower C++ standard if possible
enable_cxx17 {
    c++0x | c++11 | c++1y | c++14 | c++1z {
        contains(QT_CONFIG, c++17) {
            CONFIG -= c++0x
            CONFIG -= c++11
            CONFIG -= c++1y
            CONFIG -= c++14
            CONFIG -= c++1z
            CONFIG *= c++17
        } else : contains(QT_CONFIG, c++1z) {
            CONFIG -= c++0x
            CONFIG -= c++11
            CONFIG -= c++1y
            CONFIG -= c++14
            CONFIG *= c++1z
        }
    }
} else : enable_cxx14 {
    c++0x | c++11 | c++1y {
        contains(QT_CONFIG, c++14) {
            CONFIG -= c++0x
            CONFIG -= c++11
            CONFIG -= c++1y
            CONFIG *= c++14
        } else : contains(QT_CONFIG, c++1y) {
            CONFIG -= c++0x
            CONFIG -= c++11
            CONFIG *= c++1y
        }
    }
} else : enable_cxx11 {
    c++0x {
        contains(QT_CONFIG, c++11) {
            CONFIG -= c++0x
            CONFIG *= c++11
        } else : contains(QT_CONFIG, c++0x) {
            CONFIG *= c++0x
        }
    }
}

# ::::: Misc Configuration :::::

# ThirdParty options:
#    disable_thirdparty
disable_thirdparty {
    CONFIG += disable_aom
    CONFIG += disable_brotli
    CONFIG += disable_exiv2
    CONFIG += disable_flif
    CONFIG += disable_freetype
    CONFIG += disable_giflib
    CONFIG += disable_graphicsmagick
    CONFIG += disable_graphicsmagickwand
    CONFIG += disable_highway
    CONFIG += disable_jbigkit
    CONFIG += disable_jxrlib
    CONFIG += disable_kimageformats
    CONFIG += disable_lerc
    CONFIG += disable_libavif
    CONFIG += disable_libbpg
    CONFIG += disable_libde265
    CONFIG += disable_libexif
    CONFIG += disable_libexpat
    CONFIG += disable_libheif
    CONFIG += disable_libjasper
    CONFIG += disable_libjpeg
    CONFIG += disable_libjxl
    CONFIG += disable_liblcms2
    CONFIG += disable_libmng
    CONFIG += disable_libpng
    CONFIG += disable_libraw
    CONFIG += disable_librsvg
    CONFIG += disable_libtiff
    CONFIG += disable_libwebp
    CONFIG += disable_libwmf
    CONFIG += disable_macwebview
    CONFIG += disable_macwkwebview
    CONFIG += disable_magickcore
    CONFIG += disable_magickwand
    CONFIG += disable_msedgewebview2
    CONFIG += disable_mshtml
    CONFIG += disable_nanosvg
    CONFIG += disable_nsimage
    CONFIG += disable_openexr
    CONFIG += disable_openjpeg
    CONFIG += disable_pkgconfig
    CONFIG += disable_qmlwebengine
    CONFIG += disable_qtcore5compat
    CONFIG += disable_qtextended
    CONFIG += disable_qtimageformats
    CONFIG += disable_qtsvg
    CONFIG += disable_qtwebengine
    CONFIG += disable_qtwebkit
    CONFIG += disable_resvg
    CONFIG += disable_stb
    CONFIG += disable_wic
    CONFIG += disable_xzutils
    CONFIG += disable_zlib
    CONFIG += disable_zstd
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
        !greaterThan(QMAKE_MSC_VER, 1700) { # MSVC2012
            CONFIG += disable_xzutils # FIXME: C99
        }
    }
}

# brotli options:
#    disable_brotli
#    system_brotli
*msvc* {
    isEmpty(QMAKE_MSC_VER) {
        win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 {
            CONFIG += disable_brotli # FIXME: C99
        }
    } else {
        !greaterThan(QMAKE_MSC_VER, 1700) { # MSVC2012
            CONFIG += disable_brotli # FIXME: C99
        }
    }
}

# highway options:
#    disable_highway
#    system_highway
disable_cxx11 : !system_highway {
    CONFIG += disable_highway
}
*msvc* : !system_highway {
    isEmpty(QMAKE_MSC_VER) {
        win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 | win32-msvc2013 | win32-msvc2015 {
            CONFIG += disable_highway
        }
    } else {
        !greaterThan(QMAKE_MSC_VER, 1900) { # MSVC2015
            CONFIG += disable_highway
        }
    }
}
*g++*|*clang* {
    win32 : !system_highway {
        CONFIG += disable_highway # FIXME: FTBFS
    }
    haiku : !system_highway {
        CONFIG += disable_highway # FIXME: FTBFS
    }
}

# libexpat options:
#    disable_libexpat
#    system_libexpat
*msvc* {
    isEmpty(QMAKE_MSC_VER) {
        win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 {
            CONFIG += disable_libexpat # FIXME: C99
        }
    } else {
        !greaterThan(QMAKE_MSC_VER, 1700) { # MSVC2012
            CONFIG += disable_libexpat # FIXME: C99
        }
    }
}

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
        !greaterThan(QMAKE_MSC_VER, 1800) { # MSVC2013
            CONFIG += disable_libexif # FIXME: C99
        }
    }
}

# exiv2 options:
#    disable_exiv2
#    system_exiv2
disable_cxx17 {
    CONFIG += disable_exiv2
}

# LibJPEG options:
#    disable_libjpeg
#    system_libjpeg


# LibJasPer options:
#    disable_libjasper
#    system_libjasper
*msvc* {
    isEmpty(QMAKE_MSC_VER) {
        win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 {
            CONFIG += disable_libjasper # FIXME: C99
        }
    } else {
        !greaterThan(QMAKE_MSC_VER, 1700) { # MSVC2012
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


# LERC options:
#    disable_lerc
#    system_lerc
*msvc* : !system_lerc {
    isEmpty(QMAKE_MSC_VER) {
        win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 {
            CONFIG += disable_lerc # FIXME: C99/C++11
        }
    } else {
        !greaterThan(QMAKE_MSC_VER, 1700) { # MSVC2012
            CONFIG += disable_lerc # FIXME: C99/C++11
        }
    }
}

# libtiff options:
#    disable_libtiff
#    system_libtiff
*msvc* : !system_libtiff {
    isEmpty(QMAKE_MSC_VER) {
        win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 | win32-msvc2013 {
            CONFIG += disable_libtiff # FIXME: C99
        }
    } else {
        !greaterThan(QMAKE_MSC_VER, 1800) { # MSVC2013
            CONFIG += disable_libtiff # FIXME: C99
        }
    }
}

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
        !greaterThan(QMAKE_MSC_VER, 1700) { # MSVC2012
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
        !greaterThan(QMAKE_MSC_VER, 1500) { # MSVC2008
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
*msvc* {
    isEmpty(QMAKE_MSC_VER) {
        win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 | win32-msvc2013 {
            CONFIG += disable_openjpeg # FIXME: C99
        }
    } else {
        !greaterThan(QMAKE_MSC_VER, 1800) { # MSVC2013
            CONFIG += disable_openjpeg # FIXME: C99
        }
    }
}

# GIFLIB options:
#    disable_giflib
#    system_giflib
*msvc* {
    isEmpty(QMAKE_MSC_VER) {
        win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 {
            CONFIG += disable_giflib # FIXME: C99
        }
    } else {
        !greaterThan(QMAKE_MSC_VER, 1700) { # MSVC2012
            CONFIG += disable_giflib # FIXME: C99
        }
    }
}

# LibRaw options:
#    disable_libraw
#    system_libraw


# libRSVG options:
#    disable_librsvg
#    enable_librsvg
#    system_librsvg
!enable_librsvg : !system_librsvg {
    CONFIG += disable_librsvg
}

# resvg options:
#    disable_resvg
#    enable_resvg
#    system_resvg
!enable_resvg : !system_resvg {
    CONFIG += disable_resvg
}

# aom options:
#    disable_aom
#    system_aom
*msvc* : !system_aom {
    isEmpty(QMAKE_MSC_VER) {
        win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 {
            CONFIG += disable_aom # FIXME: C99
        }
    } else {
        !greaterThan(QMAKE_MSC_VER, 1700) { # MSVC2012
            CONFIG += disable_aom # FIXME: C99
        }
    }
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
        !greaterThan(QMAKE_MSC_VER, 1700) { # MSVC2012
            CONFIG += disable_openexr # FIXME: C99/C++11
        }
    }
}

# libavif options:
#    disable_libavif
#    system_libavif
disable_aom : !system_libavif {
    CONFIG += disable_libavif
}
*msvc* : !system_libavif {
    isEmpty(QMAKE_MSC_VER) {
        win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 {
            CONFIG += disable_libavif # FIXME: C99
        }
    } else {
        !greaterThan(QMAKE_MSC_VER, 1700) { # MSVC2012
            CONFIG += disable_libavif # FIXME: C99
        }
    }
}

# FLIF options:
#    disable_flif
#    system_flif
disable_cxx11 : !system_flif {
    CONFIG += disable_flif
}

# jxrlib options:
#    disable_jxrlib
#    system_jxrlib


# libjxl options:
#    disable_libjxl
#    system_libjxl
!system_libjxl {
    disable_cxx11 | disable_brotli | disable_highway | disable_liblcms2 {
        CONFIG += disable_libjxl
    }
}

# MagickCore options:
#    disable_magickcore
#    enable_magickcore
!enable_magickcore {
    CONFIG += disable_magickcore
}

# MagickWand options:
#    disable_magickwand
#    enable_magickwand
#    system_magickwand
!enable_magickwand {
    CONFIG += disable_magickwand
}

# GraphicsMagick options:
#    disable_graphicsmagick
#    enable_graphicsmagick
!enable_graphicsmagick {
    CONFIG += disable_graphicsmagick
}

# GraphicsMagickWand options:
#    disable_graphicsmagickwand
#    enable_graphicsmagickwand
#    system_graphicsmagickwand
!enable_graphicsmagickwand {
    CONFIG += disable_graphicsmagickwand
}

# ::::: Optional Third Party Components Configuration :::::

# QtExtended options:
#    disable_qtextended
#    enable_qtextended
!enable_qtextended {
    CONFIG += disable_qtextended
}
greaterThan(QT_MAJOR_VERSION, 5) {
    CONFIG += disable_qtextended
}

# STB options:
#    disable_stb


# NanoSVG options:
#    disable_nanosvg
#    enable_nanosvg
!enable_nanosvg {
    CONFIG += disable_nanosvg
}
*msvc* {
    isEmpty(QMAKE_MSC_VER) {
        win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 | win32-msvc2012 {
            CONFIG += disable_nanosvg # FIXME: C99
        }
    } else {
        !greaterThan(QMAKE_MSC_VER, 1700) { # MSVC2012
            CONFIG += disable_nanosvg # FIXME: C99
        }
    }
}

# QtImageFormats options:
#    disable_qtimageformats
!greaterThan(QT_MAJOR_VERSION, 4) {
    CONFIG += disable_qtimageformats
}

# KImageFormats options:
#    disable_kimageformats
!greaterThan(QT_MAJOR_VERSION, 4) {
    CONFIG += disable_kimageformats
}
equals(QT_MAJOR_VERSION, 5) : lessThan(QT_MINOR_VERSION, 15) {
    CONFIG += disable_kimageformats
}

# MSEdgeWebView2 options:
#    disable_msedgewebview2
#    enable_msedgewebview2
!enable_msedgewebview2 {
    CONFIG += disable_msedgewebview2
}
!win32 {
    CONFIG += disable_msedgewebview2
}
!*msvc* {
    !enable_msedgewebview2 {
        CONFIG += disable_msedgewebview2
    }
}
*msvc* {
    isEmpty(QMAKE_MSC_VER) {
        win32-msvc | win32-msvc.net | win32-msvc2002 | win32-msvc2003 | win32-msvc2005 | win32-msvc2008 | win32-msvc2010 {
            CONFIG += disable_msedgewebview2
        }
    } else {
        !greaterThan(QMAKE_MSC_VER, 1600) { # MSVC2010
            CONFIG += disable_msedgewebview2
        }
    }
}

# ghc::filesystem options:
#    disable_ghc_filesystem
disable_cxx11 {
    CONFIG += disable_ghc_filesystem
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
#    enable_mshtml
!enable_mshtml {
    CONFIG += disable_mshtml
}
!win32 {
    CONFIG += disable_mshtml
}

# DecoderWIC options
#    disable_wic
!win32 {
    CONFIG += disable_wic
}
*g++*|*clang* {
    disable_cxx11 {
        CONFIG += disable_wic
    }
}

# DecoderNSImage options:
#    disable_nsimage
!macx {
    CONFIG += disable_nsimage
}

# DecoderMacWebView options:
#    disable_macwebview
#    enable_macwebview
!enable_macwebview {
    CONFIG += disable_macwebview
}
!macx {
    CONFIG += disable_macwebview
}

# DecoderMacWKWebView options:
#    disable_macwkwebview
#    enable_macwkwebview
!enable_macwkwebview {
    CONFIG += disable_macwkwebview
}
!macx {
    CONFIG += disable_macwkwebview
}

# MacToolBar options:
#    disable_mactoolbar
!macx {
    CONFIG += disable_mactoolbar
}

# MacTouchBar options:
#    disable_mactouchbar
!macx {
    CONFIG += disable_mactouchbar
}

# Print Support options:
#    disable_printsupport


# QtCore5Compat options:
#    disable_qtcore5compat
#    enable_qtcore5compat
!enable_qtcore5compat {
    CONFIG += disable_qtcore5compat
}
!equals(QT_MAJOR_VERSION, 6) {
    CONFIG += disable_qtcore5compat
}

# ::::: Cleanup Unused :::::

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
    disable_libwmf | system_libwmf {
        CONFIG += disable_libexpat
    }
}

disable_exiv2 | system_exiv2 {
    CONFIG += disable_ghc_filesystem
}

disable_libavif | system_libavif {
    disable_libheif | system_libheif {
        CONFIG += disable_aom
    }
}

disable_libjxl | system_libjxl {
    disable_exiv2 | system_exiv2 {
        CONFIG += disable_brotli
    }
}

disable_libjxl | system_libjxl {
    CONFIG += disable_highway
}
