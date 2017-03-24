#-------------------------------------------------
#
# Project created by QtCreator 2017-02-25T14:20:14
#
#-------------------------------------------------

TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = \
    src/QtUtils/QtUtils.pro \
    src/ThirdParty/zlib/zlib.pro \
    src/ThirdParty/QtExtended/QtExtended.pro \
    src/ThirdParty/STB/STB.pro

greaterThan(QT_MAJOR_VERSION, 4) {
    SUBDIRS += \
        src/ThirdParty/libjpeg/libjpeg.pro \
        src/ThirdParty/JasPer/JasPer.pro \
        src/ThirdParty/QtImageFormats/QtImageFormats.pro
}

SUBDIRS += \
    src/ImageViewer/ImageViewer.pro

