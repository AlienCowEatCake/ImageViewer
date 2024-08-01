#-------------------------------------------------
#
# Project created by QtCreator 2017-02-25T14:20:14
#
#-------------------------------------------------

TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS =

exists(src/ThirdParty/ThirdParty.pro) {
    SUBDIRS += src/ThirdParty/ThirdParty.pro
}

SUBDIRS += \
    src/QtUtils/QtUtils.pro \
    src/ImageViewer/ImageViewer.pro

