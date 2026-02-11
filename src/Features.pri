#
#  Copyright (C) 2017-2026 Peter S. Zhigalov <peter.zhigalov@gmail.com>
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

# Get Qt and GCC/Clang/MSVC versions
isEmpty(QT_VERSION_NUMERIC) {
    QT_VERSION_NUMERIC = "$${QT_MAJOR_VERSION}"
    isEmpty(QT_MINOR_VERSION) {
        QT_VERSION_NUMERIC = "$${QT_VERSION_NUMERIC}00"
    } else : greaterThan(QT_MINOR_VERSION, 9) {
        QT_VERSION_NUMERIC = "$${QT_VERSION_NUMERIC}$${QT_MINOR_VERSION}"
    } else {
        QT_VERSION_NUMERIC = "$${QT_VERSION_NUMERIC}0$${QT_MINOR_VERSION}"
    }
    isEmpty(QT_PATCH_VERSION) {
        QT_VERSION_NUMERIC = "$${QT_VERSION_NUMERIC}00"
    } else : greaterThan(QT_PATCH_VERSION, 9) {
        QT_VERSION_NUMERIC = "$${QT_VERSION_NUMERIC}$${QT_PATCH_VERSION}"
    } else {
        QT_VERSION_NUMERIC = "$${QT_VERSION_NUMERIC}0$${QT_PATCH_VERSION}"
    }
}
*g++* : !*clang* : isEmpty(GCC_VERSION_NUMERIC) {
    GCC_VERSION_STR = $$system("$${QMAKE_CXX} -dumpversion")
    GCC_VERSION_LIST = $$split(GCC_VERSION_STR, -)
    GCC_VERSION_STR = $$member(GCC_VERSION_LIST, 0)
    GCC_VERSION_LIST = $$split(GCC_VERSION_STR, .)
    GCC_MAJOR_VERSION = $$member(GCC_VERSION_LIST, 0)
    # There are no reasons to get minor and patch versions for GCC >= 9 currently
    !lessThan(GCC_MAJOR_VERSION, 7) : lessThan(GCC_MAJOR_VERSION, 9) {
        GCC_VERSION_STR_FV = $$system("$${QMAKE_CXX} -dumpfullversion")
        GCC_VERSION_LIST_FV = $$split(GCC_VERSION_STR_FV, -)
        GCC_VERSION_STR_FV = $$member(GCC_VERSION_LIST_FV, 0)
        GCC_VERSION_LIST_FV = $$split(GCC_VERSION_STR_FV, .)
        GCC_MAJOR_VERSION_FV = $$member(GCC_VERSION_LIST_FV, 0)
        equals(GCC_MAJOR_VERSION, "$${GCC_MAJOR_VERSION_FV}") {
            GCC_VERSION_LIST = $${GCC_VERSION_LIST_FV}
        }
    }
    GCC_MINOR_VERSION = $$member(GCC_VERSION_LIST, 1)
    GCC_PATCH_VERSION = $$member(GCC_VERSION_LIST, 2)
    GCC_VERSION_NUMERIC = "$${GCC_MAJOR_VERSION}"
    isEmpty(GCC_MINOR_VERSION) {
        GCC_VERSION_NUMERIC = "$${GCC_VERSION_NUMERIC}00"
    } else : greaterThan(GCC_MINOR_VERSION, 9) {
        GCC_VERSION_NUMERIC = "$${GCC_VERSION_NUMERIC}$${GCC_MINOR_VERSION}"
    } else {
        GCC_VERSION_NUMERIC = "$${GCC_VERSION_NUMERIC}0$${GCC_MINOR_VERSION}"
    }
    isEmpty(GCC_PATCH_VERSION) {
        GCC_VERSION_NUMERIC = "$${GCC_VERSION_NUMERIC}00"
    } else : greaterThan(GCC_PATCH_VERSION, 9) {
        GCC_VERSION_NUMERIC = "$${GCC_VERSION_NUMERIC}$${GCC_PATCH_VERSION}"
    } else {
        GCC_VERSION_NUMERIC = "$${GCC_VERSION_NUMERIC}0$${GCC_PATCH_VERSION}"
    }
}
*clang* : isEmpty(CLANG_VERSION_NUMERIC) {
    CLANG_VERSION_STR = $$system("$${QMAKE_CXX} -dumpversion")
    CLANG_VERSION_LIST = $$split(CLANG_VERSION_STR, -)
    CLANG_VERSION_STR = $$member(CLANG_VERSION_LIST, 0)
    CLANG_VERSION_LIST = $$split(CLANG_VERSION_STR, .)
    CLANG_MAJOR_VERSION = $$member(CLANG_VERSION_LIST, 0)
    CLANG_MINOR_VERSION = $$member(CLANG_VERSION_LIST, 1)
    CLANG_PATCH_VERSION = $$member(CLANG_VERSION_LIST, 2)
    CLANG_VERSION_NUMERIC = "$${CLANG_MAJOR_VERSION}"
    isEmpty(CLANG_MINOR_VERSION) {
        CLANG_VERSION_NUMERIC = "$${CLANG_VERSION_NUMERIC}00"
    } else : greaterThan(CLANG_MINOR_VERSION, 9) {
        CLANG_VERSION_NUMERIC = "$${CLANG_VERSION_NUMERIC}$${CLANG_MINOR_VERSION}"
    } else {
        CLANG_VERSION_NUMERIC = "$${CLANG_VERSION_NUMERIC}0$${CLANG_MINOR_VERSION}"
    }
    isEmpty(CLANG_PATCH_VERSION) {
        CLANG_VERSION_NUMERIC = "$${CLANG_VERSION_NUMERIC}00"
    } else : greaterThan(CLANG_PATCH_VERSION, 9) {
        CLANG_VERSION_NUMERIC = "$${CLANG_VERSION_NUMERIC}$${CLANG_PATCH_VERSION}"
    } else {
        CLANG_VERSION_NUMERIC = "$${CLANG_VERSION_NUMERIC}0$${CLANG_PATCH_VERSION}"
    }
}
*msvc* : isEmpty(MSVC_VERSION) {
    isEmpty(QMAKE_MSC_VER) {
        win32-msvc2026 {
            QMAKE_MSC_VER = 1950
        } else : win32-msvc2022 {
            QMAKE_MSC_VER = 1930
        } else : win32-msvc2019 {
            QMAKE_MSC_VER = 1920
        } else : win32-msvc2017 {
            QMAKE_MSC_VER = 1910
        } else : win32-msvc2015 {
            QMAKE_MSC_VER = 1900
        } else : win32-msvc2013 {
            QMAKE_MSC_VER = 1800
        } else : win32-msvc2012 {
            QMAKE_MSC_VER = 1700
        } else : win32-msvc2010 {
            QMAKE_MSC_VER = 1600
        } else : win32-msvc2008 {
            QMAKE_MSC_VER = 1500
        } else : win32-msvc2005 {
            QMAKE_MSC_VER = 1400
        } else : win32-msvc2003 {
            QMAKE_MSC_VER = 1310
        } else : win32-msvc2002 {
            QMAKE_MSC_VER = 1300
        } else : win32-msvc.net {
            QMAKE_MSC_VER = 1300
        } else {
            QMAKE_MSC_VER = 1200
        }
    }
    lessThan(QMAKE_MSC_VER, 1300) {
        MSVC_VERSION = 1998
    } else : lessThan(QMAKE_MSC_VER, 1310) {
        MSVC_VERSION = 2002
    } else : lessThan(QMAKE_MSC_VER, 1400) {
        MSVC_VERSION = 2003
    } else : lessThan(QMAKE_MSC_VER, 1500) {
        MSVC_VERSION = 2005
    } else : lessThan(QMAKE_MSC_VER, 1600) {
        MSVC_VERSION = 2008
    } else : lessThan(QMAKE_MSC_VER, 1700) {
        MSVC_VERSION = 2010
    } else : lessThan(QMAKE_MSC_VER, 1800) {
        MSVC_VERSION = 2012
    } else : lessThan(QMAKE_MSC_VER, 1900) {
        MSVC_VERSION = 2013
    } else : lessThan(QMAKE_MSC_VER, 1910) {
        MSVC_VERSION = 2015
    } else : lessThan(QMAKE_MSC_VER, 1920) {
        MSVC_VERSION = 2017
    } else : lessThan(QMAKE_MSC_VER, 1930) {
        MSVC_VERSION = 2019
    } else : lessThan(QMAKE_MSC_VER, 1950) {
        MSVC_VERSION = 2022
    } else {
        MSVC_VERSION = 2026
    }
}

# C++11 options:
#    disable_cxx11
#    enable_cxx11
!disable_cxx11 {
    # MSVC 2013 is not a C++11-conformant compiler
    *msvc* : greaterThan(MSVC_VERSION, 2013) : CONFIG *= test_cxx11_compatible_compiler
    # GCC < 4.8.1 is not a C++11-conformant compiler: https://gcc.gnu.org/projects/cxx-status.html#cxx11
    *g++* : !*clang* : !lessThan(GCC_VERSION_NUMERIC, 40801) : CONFIG *= test_cxx11_compatible_compiler
    # Clang < 3.3 is not a C++11-conformant compiler: https://clang.llvm.org/cxx_status.html#cxx11
    *clang* : !mac : !lessThan(CLANG_VERSION_NUMERIC, 30300) : CONFIG *= test_cxx11_compatible_compiler
    # Assume that clang on macOS is compatible if Qt is *definitely* built with libc++ (Qt 5.7+)
    *clang* : mac : !lessThan(QT_VERSION_NUMERIC, 50700) : CONFIG *= test_cxx11_compatible_compiler
    !lessThan(QT_VERSION_NUMERIC, 50700) : CONFIG *= test_cxx11_compatible_qt
    !test_cxx11_compatible_qt : !test_cxx11_compatible_compiler : !enable_cxx11 : !enable_cxx14 : !enable_cxx17 {
        CONFIG *= disable_cxx11
    }
}

# C++14 options:
#    disable_cxx14
#    enable_cxx14
!disable_cxx14 {
    # MSVC 2015 is not a C++14-conformant compiler
    *msvc* : greaterThan(MSVC_VERSION, 2015) : CONFIG *= test_cxx14_compatible_compiler
    # GCC < 5.0 is not a C++14-conformant compiler: https://gcc.gnu.org/projects/cxx-status.html#cxx14
    *g++* : !*clang* : !lessThan(GCC_VERSION_NUMERIC, 50000) : CONFIG *= test_cxx14_compatible_compiler
    # Clang < 3.4 is not a C++14-conformant compiler: https://clang.llvm.org/cxx_status.html#cxx14
    *clang* : !mac : !lessThan(CLANG_VERSION_NUMERIC, 30400) : CONFIG *= test_cxx14_compatible_compiler
    # Assume that clang on macOS is compatible if Qt is *definitely* built with libc++ (Qt 5.7+)
    *clang* : mac : !lessThan(QT_VERSION_NUMERIC, 50700) : CONFIG *= test_cxx14_compatible_compiler
    greaterThan(QT_MAJOR_VERSION, 5) : CONFIG *= test_cxx14_compatible_qt
    !test_cxx14_compatible_qt : !test_cxx14_compatible_compiler : !enable_cxx14 : !enable_cxx17 {
        CONFIG *= disable_cxx14
    }
}

# C++17 options:
#    disable_cxx17
#    enable_cxx17
!disable_cxx17 {
    # MSVC 2017 is not a C++17-conformant compiler
    *msvc* : greaterThan(MSVC_VERSION, 2017) : CONFIG *= test_cxx17_compatible_compiler
    # GCC < 8.0 is not a C++17-conformant compiler: https://gcc.gnu.org/projects/cxx-status.html#cxx17
    *g++* : !*clang* : !lessThan(GCC_VERSION_NUMERIC, 80000) : CONFIG *= test_cxx17_compatible_compiler
    # Clang < 5.0 is not a C++17-conformant compiler: https://clang.llvm.org/cxx_status.html#cxx17
    *clang* : !mac : !lessThan(CLANG_VERSION_NUMERIC, 50000) : CONFIG *= test_cxx17_compatible_compiler
    # Assume that clang on macOS is compatible if Qt is *definitely* built with C++17-compatible libc++ (macOS 10.13, Qt 5.15+)
    *clang* : mac : !lessThan(QT_VERSION_NUMERIC, 51500) : CONFIG *= test_cxx17_compatible_compiler
    greaterThan(QT_MAJOR_VERSION, 5) : CONFIG *= test_cxx17_compatible_qt
    !test_cxx17_compatible_qt : !test_cxx17_compatible_compiler : !enable_cxx17 {
        CONFIG *= disable_cxx17
    }
}

# Cleanup C++
disable_cxx11 {
    CONFIG *= disable_cxx14
    CONFIG *= disable_cxx17
}
disable_cxx14 {
    CONFIG *= disable_cxx17
}
!disable_cxx17 {
    CONFIG *= enable_cxx11
    CONFIG *= enable_cxx14
    CONFIG *= enable_cxx17
} else {
    CONFIG -= enable_cxx17
}
!disable_cxx14 {
    CONFIG *= enable_cxx11
    CONFIG *= enable_cxx14
} else {
    CONFIG -= enable_cxx14
}
!disable_cxx11 {
    CONFIG *= enable_cxx11
} else {
    CONFIG -= enable_cxx11
}

# Upgrade lower C++ standard if possible
!c++latest : !c++03 : !c++0x : !c++11 : !c++1y : !c++14 : !c++1z : !c++17 : !c++2a : !c++20 : !c++2b : !c++23 {
    CONFIG *= unconfigured_cxx
}
enable_cxx17 {
    unconfigured_cxx | c++03 | c++0x | c++11 | c++1y | c++14 | c++1z | c++17 {
        CONFIG -= c++03
        CONFIG -= c++0x
        CONFIG -= c++11
        CONFIG -= c++1y
        CONFIG -= c++14
        CONFIG -= c++1z
        CONFIG -= c++17
        !lessThan(QT_VERSION_NUMERIC, 60100) {
            CONFIG *= c++17
        } else : !lessThan(QT_VERSION_NUMERIC, 51200) {
            CONFIG *= c++1z
        } else : *msvc* {
            greaterThan(MSVC_VERSION, 2017) {
                QMAKE_CXXFLAGS *= /std:c++17
            } else {
                QMAKE_CXXFLAGS *= /std:c++latest
            }
        } else : *clang* {
            QMAKE_CXXFLAGS *= -std=gnu++1z
        } else : *g++* {
            QMAKE_CXXFLAGS *= -std=gnu++1z
        }
    }
} else : enable_cxx14 {
    unconfigured_cxx | c++03 | c++0x | c++11 | c++1y | c++14 {
        CONFIG -= c++03
        CONFIG -= c++0x
        CONFIG -= c++11
        CONFIG -= c++1y
        CONFIG -= c++14
        !lessThan(QT_VERSION_NUMERIC, 50500) {
            CONFIG *= c++14
        } else : *clang* {
            QMAKE_CXXFLAGS *= -std=gnu++1y
        } else : *g++* {
            QMAKE_CXXFLAGS *= -std=gnu++1y
        }
    }
} else : enable_cxx11 {
    unconfigured_cxx | c++03 | c++0x | c++11 {
        CONFIG -= c++03
        CONFIG -= c++0x
        CONFIG -= c++11
        !lessThan(QT_VERSION_NUMERIC, 50200) {
            CONFIG *= c++11
        } else : *clang* {
            QMAKE_CXXFLAGS *= -std=gnu++0x
        } else : *g++* {
            QMAKE_CXXFLAGS *= -std=gnu++0x
        }
    }
}

# ::::: Misc Configuration :::::

# ThirdParty options:
#    disable_thirdparty
#    system_thirdparty
!exists($${PWD}/ThirdParty/ThirdParty.pro) {
    CONFIG *= system_thirdparty
}
disable_thirdparty {
    CONFIG *= disable_aom
    CONFIG *= disable_brotli
    CONFIG *= disable_exiv2
    CONFIG *= disable_fallback_iccprofiles
    CONFIG *= disable_flif
    CONFIG *= disable_freetype
    CONFIG *= disable_giflib
    CONFIG *= disable_highway
    CONFIG *= disable_j40
    CONFIG *= disable_jbigkit
    CONFIG *= disable_jxrlib
    CONFIG *= disable_kimageformats
    CONFIG *= disable_lerc
    CONFIG *= disable_libavif
    CONFIG *= disable_libbpg
    CONFIG *= disable_openh264
    CONFIG *= disable_libde265
    CONFIG *= disable_vvdec
    CONFIG *= disable_libexif
    CONFIG *= disable_libexpat
    CONFIG *= disable_libheif
    CONFIG *= disable_libjasper
    CONFIG *= disable_libjpeg
    CONFIG *= disable_libjxl
    CONFIG *= disable_liblcms2
    CONFIG *= disable_libmng
    CONFIG *= disable_libpng
    CONFIG *= disable_libraw
    CONFIG *= disable_librsvg
    CONFIG *= disable_libtiff
    CONFIG *= disable_libwebp
    CONFIG *= disable_libwmf
    CONFIG *= disable_libyuv
    CONFIG *= disable_openexr
    CONFIG *= disable_openjpeg
    CONFIG *= disable_openjph
    CONFIG *= disable_pkgconfig
    CONFIG *= disable_qtcore5compat
    CONFIG *= disable_qtimageformats
    CONFIG *= disable_resvg
    CONFIG *= disable_stb
    CONFIG *= disable_xzutils
    CONFIG *= disable_zlib
    CONFIG *= disable_zstd
}
system_thirdparty : !disable_thirdparty {
    # Use pkg-config if not disabled explicitly
    !disable_pkgconfig : CONFIG *= enable_pkgconfig
    # Unused in "system" configuration
    CONFIG *= disable_aom
    CONFIG *= disable_brotli
    CONFIG *= disable_freetype
    CONFIG *= disable_highway
    CONFIG *= disable_openh264
    CONFIG *= disable_libde265
    CONFIG *= disable_vvdec
    CONFIG *= disable_libexpat
    CONFIG *= disable_libyuv
    CONFIG *= disable_openjph
    CONFIG *= disable_xzutils
    CONFIG *= disable_zstd
    # No rules for build as system packages
    CONFIG *= disable_j40
    CONFIG *= disable_stb
    # Should be installed as Qt plugins in "system" configuration
    CONFIG *= disable_kimageformats
    CONFIG *= disable_qtimageformats
    # System ICC profiles should be used for "system" configuration
    CONFIG *= disable_fallback_iccprofiles
    # System packages
    CONFIG *= system_exiv2
    CONFIG *= system_flif
    CONFIG *= system_giflib
    CONFIG *= system_jbigkit
    CONFIG *= system_jxrlib
    CONFIG *= system_lerc
    CONFIG *= system_libavif
    CONFIG *= system_libbpg
    CONFIG *= system_libexif
    CONFIG *= system_libheif
    CONFIG *= system_libjasper
    CONFIG *= system_libjpeg
    CONFIG *= system_libjxl
    CONFIG *= system_liblcms2
    CONFIG *= system_libmng
    CONFIG *= system_libpng
    CONFIG *= system_libraw
    CONFIG *= system_librsvg
    CONFIG *= system_libtiff
    CONFIG *= system_libwebp
    CONFIG *= system_libwmf
    CONFIG *= system_openexr
    CONFIG *= system_openjpeg
    CONFIG *= system_zlib
    # Runtime loaders are inappropriate for "system" configuration
    enable_resvg              : CONFIG *= system_resvg
}

# ::::: System Libraries Configuration :::::

# pkg-config options:
#    disable_pkgconfig
#    enable_pkgconfig
!enable_pkgconfig {
    !unix | macx {
        CONFIG *= disable_pkgconfig
    }
}
!disable_pkgconfig {
    QT_CONFIG -= no-pkg-config
    CONFIG *= link_pkgconfig
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
*msvc* : !system_xzutils : lessThan(MSVC_VERSION, 2015) {
    CONFIG *= disable_xzutils
}

# brotli options:
#    disable_brotli
#    system_brotli
*msvc* : !system_brotli : lessThan(MSVC_VERSION, 2013) {
    CONFIG *= disable_brotli
}

# highway options:
#    disable_highway
#    system_highway
disable_cxx11 : !system_highway {
    CONFIG *= disable_highway
}
*msvc* : !system_highway : lessThan(MSVC_VERSION, 2017) {
    CONFIG *= disable_highway
}
*g++* : !*clang* : !system_highway : lessThan(GCC_VERSION_NUMERIC, 80400) {
    CONFIG *= disable_highway
}
!disable_highway : !system_highway {
    DEFINES *= HWY_COMPILE_ONLY_SCALAR
    DEFINES *= HWY_BROKEN_EMU128=1
    DEFINES *= HWY_HAVE_SCALAR_F16_TYPE=0
}

# libexpat options:
#    disable_libexpat
#    system_libexpat
*msvc* : !system_libexpat : lessThan(MSVC_VERSION, 2013) {
    CONFIG *= disable_libexpat
}

# libyuv options:
#    disable_libyuv
#    system_libyuv


# LCMS options:
#    disable_liblcms2
#    system_liblcms2


# libexif options:
#    disable_libexif
#    system_libexif
*msvc* : !system_libexif : lessThan(MSVC_VERSION, 2015) {
    CONFIG *= disable_libexif
}

# exiv2 options:
#    disable_exiv2
#    system_exiv2
*msvc* : !system_exiv2 : lessThan(MSVC_VERSION, 2010) {
    CONFIG *= disable_exiv2
}
*msvc* : !system_exiv2 : lessThan(MSVC_VERSION, 2019) {
    CONFIG *= disable_exiv2
}

# LibJPEG options:
#    disable_libjpeg
#    system_libjpeg


# LibJasPer options:
#    disable_libjasper
#    system_libjasper
*msvc* : !system_libjasper : lessThan(MSVC_VERSION, 2013) {
    CONFIG *= disable_libjasper
}

# libmng options:
#    disable_libmng
#    system_libmng
disable_zlib : !system_libmng {
    CONFIG *= disable_libmng
}

# libpng options:
#    disable_libpng
#    system_libpng
disable_zlib : !system_libpng {
    CONFIG *= disable_libpng
}

# jbigkit options:
#    disable_jbigkit
#    system_jbigkit


# LERC options:
#    disable_lerc
#    system_lerc
*msvc* : !system_lerc : lessThan(MSVC_VERSION, 2013) {
    CONFIG *= disable_lerc
}

# libtiff options:
#    disable_libtiff
#    system_libtiff
*msvc* : !system_libtiff : lessThan(MSVC_VERSION, 2015) {
    CONFIG *= disable_libtiff
}

# LibWebP options:
#    disable_libwebp
#    system_libwebp


# libbpg options:
#    disable_libbpg
#    system_libbpg
*msvc* : !system_libbpg : lessThan(MSVC_VERSION, 2013) {
    CONFIG *= disable_libbpg
}

# FreeType options:
#    disable_freetype
#    system_freetype
*msvc* : !system_freetype : lessThan(MSVC_VERSION, 2015) {
    CONFIG *= disable_freetype
}
disable_zlib : !system_freetype {
    CONFIG *= disable_freetype
}
disable_zlib : !system_freetype {
    CONFIG *= disable_freetype
}

# libwmf options:
#    disable_libwmf
#    system_libwmf
*msvc* : !system_libwmf : lessThan(MSVC_VERSION, 2010) {
    CONFIG *= disable_libwmf
}
disable_zlib : !system_libwmf {
    CONFIG *= disable_libwmf
}
disable_libpng : !system_libwmf {
    CONFIG *= disable_libwmf
}
disable_freetype : !system_libwmf {
    CONFIG *= disable_libwmf
}
disable_libjpeg : !system_libwmf {
    CONFIG *= disable_libwmf
}

# OpenJPEG options:
#    disable_openjpeg
#    system_openjpeg
*msvc* : !system_openjpeg : lessThan(MSVC_VERSION, 2015) {
    CONFIG *= disable_openjpeg
}

# OpenJPH options:
#    disable_openjph
#    system_openjph
disable_cxx14 : !system_openjph {
    CONFIG *= disable_openjph
}

# GIFLIB options:
#    disable_giflib
#    system_giflib
*msvc* : !system_giflib : lessThan(MSVC_VERSION, 2013) {
    CONFIG *= disable_giflib
}

# LibRaw options:
#    disable_libraw
#    system_libraw


# libRSVG options:
#    disable_librsvg
#    enable_librsvg
#    system_librsvg
!enable_librsvg : !system_librsvg {
    CONFIG *= disable_librsvg
}

# resvg options:
#    disable_resvg
#    enable_resvg
#    system_resvg
!enable_resvg : !system_resvg {
    CONFIG *= disable_resvg
}

# aom options:
#    disable_aom
#    system_aom
*msvc* : !system_aom : lessThan(MSVC_VERSION, 2015) {
    CONFIG *= disable_aom
}

# OpenH264 options:
#    disable_openh264
#    system_openh264


# libde265 options:
#    disable_libde265
#    system_libde265
*msvc* : !system_libde265 : lessThan(MSVC_VERSION, 2012) {
    CONFIG *= disable_libde265
}
win32 : *g++* : !*clang* : lessThan(GCC_VERSION_NUMERIC, 50100) { # FIXME: Find exact version
    CONFIG *= disable_libde265
}
disable_cxx11 : !system_libde265 {
    CONFIG *= disable_libde265
}

# VVdeC options:
#    disable_vvdec
#    system_vvdec
*msvc* : !system_vvdec : lessThan(MSVC_VERSION, 2017) {
    CONFIG *= disable_vvdec
}
*g++* : !*clang* : !system_vvdec : lessThan(GCC_VERSION_NUMERIC, 80100) { # FIXME: Find exact version
    CONFIG *= disable_vvdec
}
disable_cxx14 : !system_vvdec {
    CONFIG *= disable_vvdec
}

# libheif options:
#    disable_libheif
#    system_libheif
*msvc* : !system_libheif : lessThan(MSVC_VERSION, 2015) {
    CONFIG *= disable_libheif
}
disable_cxx11 : !system_libheif {
    CONFIG *= disable_libheif
}

# OpenEXR options:
#    disable_openexr
#    system_openexr
disable_zlib : !system_openexr {
    CONFIG *= disable_openexr
}
*msvc* : !system_openexr : lessThan(MSVC_VERSION, 2013) {
    CONFIG *= disable_openexr
}

# libavif options:
#    disable_libavif
#    system_libavif
disable_aom : !system_libavif {
    CONFIG *= disable_libavif
}
disable_libyuv : !system_libavif {
    CONFIG *= disable_libavif
}
*msvc* : !system_libavif : lessThan(MSVC_VERSION, 2015) {
    CONFIG *= disable_libavif
}

# FLIF options:
#    disable_flif
#    system_flif
*msvc* : !system_flif : lessThan(MSVC_VERSION, 2015) {
    CONFIG *= disable_flif
}
disable_cxx11 : !system_flif {
    CONFIG *= disable_flif
}

# jxrlib options:
#    disable_jxrlib
#    system_jxrlib
*msvc* : !system_jxrlib : lessThan(MSVC_VERSION, 2010) {
    CONFIG *= disable_jxrlib
}

# libjxl options:
#    disable_libjxl
#    system_libjxl
!system_libjxl {
    disable_cxx11 | disable_brotli | disable_highway | disable_liblcms2 {
        CONFIG *= disable_libjxl
    }
}
disable_cxx17 : !system_libjxl {
    CONFIG *= disable_libjxl
}

# ::::: Optional Third Party Components Configuration :::::

# STB options:
#    disable_stb


# J40 options:
#    disable_j40
*msvc* : lessThan(MSVC_VERSION, 2010) {
    CONFIG *= disable_j40
}

# QtImageFormats options:
#    disable_qtimageformats
!greaterThan(QT_MAJOR_VERSION, 4) {
    CONFIG *= disable_qtimageformats
}

# KImageFormats options:
#    disable_kimageformats
lessThan(QT_VERSION_NUMERIC, 51500) {
    CONFIG *= disable_kimageformats
}
disable_cxx11 {
    CONFIG *= disable_kimageformats
}

# ::::: Optional Built-in Components Configuration :::::

# DecoderQtSVG options:
#    disable_qtsvg


# DecoderWIC options
#    disable_wic
!win32 {
    CONFIG *= disable_wic
}

# DecoderNSImage options:
#    disable_nsimage
!macx {
    CONFIG *= disable_nsimage
}

# MacToolBar options:
#    disable_mactoolbar
!macx {
    CONFIG *= disable_mactoolbar
}

# MacTouchBar options:
#    disable_mactouchbar
!macx {
    CONFIG *= disable_mactouchbar
}

# Print Support options:
#    disable_printsupport


# Fallback ICC Profiles options:
#    disable_fallback_iccprofiles


# QtCore5Compat options:
#    disable_qtcore5compat
#    enable_qtcore5compat
!enable_qtcore5compat {
    CONFIG *= disable_qtcore5compat
}
!equals(QT_MAJOR_VERSION, 6) {
    CONFIG *= disable_qtcore5compat
}

# ::::: Cleanup Unused :::::

disable_libtiff | system_libtiff {
    CONFIG *= disable_zstd
    CONFIG *= disable_xzutils
}

disable_libwmf | system_libwmf {
    CONFIG *= disable_freetype
}

system_libheif | disable_libheif {
    CONFIG *= disable_libde265
    CONFIG *= disable_vvdec
}

disable_exiv2 | system_exiv2 {
    disable_libwmf | system_libwmf {
        CONFIG *= disable_libexpat
    }
}

disable_libavif | system_libavif {
    disable_libheif | system_libheif {
        CONFIG *= disable_aom
    }
}

disable_libjxl | system_libjxl {
    disable_libheif | system_libheif {
        CONFIG *= disable_brotli
    }
}

disable_libjxl | system_libjxl {
    CONFIG *= disable_highway
}

disable_libavif | system_libavif {
    disable_aom | system_aom {
        CONFIG *= disable_libyuv
    }
}

!disable_libjxl {
    CONFIG *= disable_j40
}

CONFIG *= disable_openjph
CONFIG *= disable_openh264
