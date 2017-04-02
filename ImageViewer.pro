#-------------------------------------------------
#
# Project created by QtCreator 2017-02-25T14:20:14
#
#-------------------------------------------------

TEMPLATE = subdirs

CONFIG += ordered

include(Features.pri)

SUBDIRS =

!disable_zlib : !system_zlib {
    SUBDIRS += src/ThirdParty/zlib/zlib.pro
}

!disable_libjpeg : !system_libjpeg {
    SUBDIRS += src/ThirdParty/libjpeg/libjpeg.pro
}

!disable_libjasper : !system_libjasper {
    SUBDIRS += src/ThirdParty/JasPer/JasPer.pro
}

!disable_liblcms2 : !system_liblcms2 {
    SUBDIRS += src/ThirdParty/LittleCMS2/LittleCMS2.pro
}

!disable_libexif : !system_libexif {
    SUBDIRS += src/ThirdParty/libexif/libexif.pro
}

!disable_qtextended {
    SUBDIRS += src/ThirdParty/QtExtended/QtExtended.pro
}

!disable_stb {
    SUBDIRS += src/ThirdParty/STB/STB.pro
}

!disable_qtimageformats {
    SUBDIRS += src/ThirdParty/QtImageFormats/QtImageFormats.pro
}

SUBDIRS += \
    src/QtUtils/QtUtils.pro \
    src/ImageViewer/ImageViewer.pro

