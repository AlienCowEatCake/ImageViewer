TEMPLATE = subdirs

include(../Features.pri)

SUBDIRS =

!disable_zlib : !system_zlib {
    SUBDIRS += zlib/zlib.pro
}

!disable_xzutils : !system_xzutils {
    SUBDIRS += XZUtils/XZUtils.pro
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

!disable_libmng : !system_libmng {
    SUBDIRS += libmng/libmng.pro
}

!disable_libpng : !system_libpng {
    SUBDIRS += libpng/libpng.pro
}

!disable_jbigkit : !system_jbigkit {
    SUBDIRS += JBIGKit/JBIGKit.pro
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

!disable_qtextended {
    SUBDIRS += QtExtended/QtExtended.pro
}

!disable_stb {
    SUBDIRS += STB/STB.pro
}

!disable_qtimageformats {
    SUBDIRS += QtImageFormats/QtImageFormats.pro
}

