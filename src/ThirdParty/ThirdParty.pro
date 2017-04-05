TEMPLATE = subdirs

include(../../Features.pri)

SUBDIRS =

!disable_zlib : !system_zlib {
    SUBDIRS += zlib/zlib.pro
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

!disable_qtextended {
    SUBDIRS += QtExtended/QtExtended.pro
}

!disable_stb {
    SUBDIRS += STB/STB.pro
}

!disable_qtimageformats {
    SUBDIRS += QtImageFormats/QtImageFormats.pro
}

