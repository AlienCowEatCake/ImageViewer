TEMPLATE = subdirs

include(../Features.pri)

SUBDIRS =

!disable_zlib : !system_zlib {
    SUBDIRS += zlib/zlib.pro
}

!disable_zstd : !system_zstd {
    SUBDIRS += Zstandard/Zstandard.pro
}

!disable_xzutils : !system_xzutils {
    SUBDIRS += XZUtils/XZUtils.pro
}

!disable_brotli : !system_brotli {
    SUBDIRS += brotli/brotli.pro
}

!disable_highway : !system_highway {
    SUBDIRS += highway/highway.pro
}

!disable_libexpat : !system_libexpat {
    SUBDIRS += libexpat/libexpat.pro
}

!disable_libyuv : !system_libyuv {
    SUBDIRS += libyuv/libyuv.pro
}

!disable_libjpeg : !system_libjpeg {
    SUBDIRS += libjpeg/libjpeg.pro
}

!disable_libjasper : !system_libjasper {
    SUBDIRS += JasPer/JasPer.pro
}

!disable_liblcms2 : !system_liblcms2 {
    SUBDIRS += LittleCMS2/LittleCMS2.pro
}

!disable_libexif : !system_libexif {
    SUBDIRS += libexif/libexif.pro
}

!disable_exiv2 : !system_exiv2 {
    SUBDIRS += Exiv2/Exiv2.pro
}

!disable_libmng : !system_libmng {
    SUBDIRS += libmng/libmng.pro
}

!disable_libpng : !system_libpng {
    SUBDIRS += libpng/libpng.pro
}

!disable_jbigkit : !system_jbigkit {
    SUBDIRS += JBIGKit/JBIGKit.pro
}

!disable_lerc : !system_lerc {
    SUBDIRS += LERC/LERC.pro
}

!disable_libtiff : !system_libtiff {
    SUBDIRS += libtiff/libtiff.pro
}

!disable_libwebp : !system_libwebp {
    SUBDIRS += LibWebP/LibWebP.pro
}

!disable_libbpg : !system_libbpg {
    SUBDIRS += libbpg/libbpg.pro
}

!disable_freetype : !system_freetype {
    SUBDIRS += FreeType/FreeType.pro
}

!disable_libwmf : !system_libwmf {
    SUBDIRS += libwmf/libwmf.pro
}

!disable_openjpeg : !system_openjpeg {
    SUBDIRS += OpenJPEG/OpenJPEG.pro
}

!disable_openjph : !system_openjph {
    SUBDIRS += OpenJPH/OpenJPH.pro
}

!disable_giflib : !system_giflib {
    SUBDIRS += giflib/giflib.pro
}

!disable_libraw : !system_libraw {
    SUBDIRS += LibRaw/LibRaw.pro
}

!disable_aom : !system_aom {
    SUBDIRS += aom/aom.pro
}

!disable_openh264 : !system_openh264 {
    SUBDIRS += OpenH264/OpenH264.pro
}

!disable_libde265 : !system_libde265 {
    SUBDIRS += libde265/libde265.pro
}

!disable_libheif : !system_libheif {
    SUBDIRS += libheif/libheif.pro
}

!disable_openexr : !system_openexr {
    SUBDIRS += OpenEXR/OpenEXR.pro
}

!disable_libavif : !system_libavif {
    SUBDIRS += libavif/libavif.pro
}

!disable_flif : !system_flif {
    SUBDIRS += FLIF/FLIF.pro
}

!disable_jxrlib : !system_jxrlib {
    SUBDIRS += jxrlib/jxrlib.pro
}

!disable_libjxl : !system_libjxl {
    SUBDIRS += libjxl/libjxl.pro
}

!disable_stb {
    SUBDIRS += STB/STB.pro
}

!disable_qtimageformats {
    SUBDIRS += QtImageFormats/QtImageFormats.pro
}

!disable_kimageformats {
    SUBDIRS += KImageFormats/KImageFormats.pro
}

