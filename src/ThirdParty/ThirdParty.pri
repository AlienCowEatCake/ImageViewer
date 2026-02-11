# ATTENTION! Dependency modules should be lower

# Dependencies:
#  * libmng
#  * JasPer
#  * libtiff
#  * libjpeg
#  * LibWebP
#  * LittleCMS2
#  * libexif
#  * zlib
include(QtImageFormats/QtImageFormats.pri)

# Dependencies:
#  * libavif
#  * OpenEXR
#  * libheif
#  * libjxl
#  * LibRaw
#  * jxrlib
#  * OpenJPEG
#  * zlib
include(KImageFormats/KImageFormats.pri)

# Dependencies:
#  * zlib
include(OpenEXR/OpenEXR.pri)

# Dependencies:
#  * aom
#  * libyuv
#  * LibWebP
include(libavif/libavif.pri)

# Dependencies:
#  * brotli
#  * highway
#  * LittleCMS2
include(libjxl/libjxl.pri)

# Dependencies:
#  * libjpeg
#  * LittleCMS2
#  * zlib
include(LibRaw/LibRaw.pri)

# Dependencies:
#  * FreeType
#  * libjpeg
#  * libpng
#  * zlib
#  * libexpat
include(libwmf/libwmf.pri)

# Dependencies:
#  * libjpeg
#  * LittleCMS2
#  * zlib
include(libmng/libmng.pri)

# Dependencies:
#  * libjpeg
#  * libheif
include(JasPer/JasPer.pri)

# Dependencies:
#  * zlib
#  * Zstandard
#  * JBIGKit
#  * LERC
#  * libjpeg
#  * XZUtils
#  * LibWebP
include(libtiff/libtiff.pri)

# Dependencies:
#  * zlib
#  * libexpat
#  * brotli
include(Exiv2/Exiv2.pri)

# Dependencies:
#  * libpng
#  * zlib
include(FreeType/FreeType.pri)

# Dependencies:
#  * zlib
include(libpng/libpng.pri)

# Dependencies:
#  * zlib
include(libexpat/libexpat.pri)

# Dependencies:
#  * aom
#  * libjpeg
#  * OpenJPEG
#  * OpenJPH
#  * libde265
#  * VVdeC
#  * zlib
#  * brotli
#  * LibWebP
include(libheif/libheif.pri)

# Dependencies:
#  * libyuv
include(aom/aom.pri)

# Dependencies:
#  * libjpeg
include(libyuv/libyuv.pri)

# Modules without dependencies
include(STB/STB.pri)
include(J40/J40.pri)
include(libRSVG/libRSVG.pri)
include(resvg/resvg.pri)
include(FLIF/FLIF.pri)
include(jxrlib/jxrlib.pri)
include(libjpeg/libjpeg.pri)
include(LibWebP/LibWebP.pri)
include(libbpg/libbpg.pri)
include(OpenJPEG/OpenJPEG.pri)
include(OpenJPH/OpenJPH.pri)
include(giflib/giflib.pri)
include(LittleCMS2/LittleCMS2.pri)
include(libexif/libexif.pri)
include(LERC/LERC.pri)
include(JBIGKit/JBIGKit.pri)
include(XZUtils/XZUtils.pri)
include(zlib/zlib.pri)
include(Zstandard/Zstandard.pri)
include(brotli/brotli.pri)
include(highway/highway.pri)
include(OpenH264/OpenH264.pri)
include(libde265/libde265.pri)
include(VVdeC/VVdeC.pri)
