/*
   Copyright (C) 2017-2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

   This file is part of the `ImageViewer' program.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ImageMetaData.h"

#include <cstdio>

#include <QImage>
#include <QTransform>
#include <QBuffer>
#include <QDebug>

//#undef HAS_LIBEXIF
//#undef HAS_QTEXTENDED

#if defined (HAS_LIBEXIF)
#include <libexif/exif-data.h>
#elif defined (HAS_QTEXTENDED)
#include "qexifimageheader.h"
#endif

namespace {

#if defined (HAS_LIBEXIF)

IImageMetaData::MetaDataType getMetaDataType(ExifIfd ifd)
{
    return QString::fromLatin1("EXIF IFD %1").arg(QString::fromUtf8(exif_ifd_get_name(ifd)));
}

#endif

} // namespace

struct ImageMetaData::Impl
{
    IImageMetaData::MetaDataEntryListMap entryListMap;
#if defined (HAS_LIBEXIF)
    ExifData *exifData;
#elif defined (HAS_QTEXTENDED)
    QExifImageHeader exifHeader;
#endif

    Impl()
    {
#if defined (HAS_LIBEXIF)
        exifData = NULL;
#endif
    }

    ~Impl()
    {
#if defined (HAS_LIBEXIF)
        if(exifData)
            exif_data_unref(exifData);
#endif
    }

    quint16 getExifOrientation() const
    {
        quint16 orientation = 1;
#if defined (HAS_LIBEXIF)
        if(!exifData)
            return orientation;
        ExifEntry *entry = exif_content_get_entry(exifData->ifd[EXIF_IFD_0], EXIF_TAG_ORIENTATION);
        if(entry && entry->parent && entry->parent->parent && entry->format == EXIF_FORMAT_SHORT && entry->components == 1)
        {
            orientation = exif_get_short(entry->data, exif_data_get_byte_order(entry->parent->parent));
            qDebug() << "EXIF orientation =" << orientation;
        }
#elif defined (HAS_QTEXTENDED)
        if(exifHeader.contains(QExifImageHeader::Orientation))
        {
            orientation = exifHeader.value(QExifImageHeader::Orientation).toShort();
            qDebug() << "EXIF orientation =" << orientation;
        }
#endif
        return orientation;
    }

#if defined (HAS_LIBEXIF)

    void FillMetadata()
    {
        if(!exifData)
            return;
        for(int i = 0; i < EXIF_IFD_COUNT; i++)
        {
            if(!exifData->ifd[i] || !exifData->ifd[i]->count)
                continue;
            const IImageMetaData::MetaDataType type = getMetaDataType(static_cast<ExifIfd>(i));
            IImageMetaData::MetaDataEntryList list = entryListMap[type];
            for(unsigned int j = 0, count = exifData->ifd[i]->count; j < count; j++)
            {
                ExifEntry *e = exifData->ifd[i]->entries[j];
                char value[8192];
                list.append(IImageMetaData::MetaDataEntry(
                                QString::fromUtf8(exif_tag_get_name_in_ifd(e->tag, static_cast<ExifIfd>(i))),
                                QString::fromUtf8(exif_tag_get_title_in_ifd(e->tag, static_cast<ExifIfd>(i))),
                                QString::fromUtf8(exif_tag_get_description_in_ifd(e->tag, static_cast<ExifIfd>(i))),
                                QString::fromUtf8(exif_entry_get_value(e, value, sizeof(value)))
                                ));
            }
            entryListMap[type] = list;
        }
    }

#elif defined (HAS_QTEXTENDED)

    void FillMetadata()
    {
        /// @todo
    }

#endif
};

ImageMetaData *ImageMetaData::createExifMetaData(const QString &filePath)
{
    ImageMetaData *metaData = new ImageMetaData();
    if(metaData->readExifData(filePath))
        return metaData;
    delete metaData;
    return NULL;
}

ImageMetaData *ImageMetaData::createExifMetaData(const QByteArray &rawExifData)
{
    ImageMetaData *metaData = new ImageMetaData();
    if(metaData->readExifData(rawExifData))
        return metaData;
    delete metaData;
    return NULL;
}

ImageMetaData::ImageMetaData()
    : m_impl(new Impl())
{}

ImageMetaData::~ImageMetaData()
{}

bool ImageMetaData::readExifData(const QString &filePath)
{
    m_impl->entryListMap.clear();
#if defined (HAS_LIBEXIF)
    if(m_impl->exifData)
        exif_data_unref(m_impl->exifData);
    m_impl->exifData = exif_data_new_from_file(filePath.toLocal8Bit());
    if(!m_impl->exifData)
        return false;
//#if defined (QT_DEBUG)
//    fflush(stdout);
//    fflush(stderr);
//    exif_data_dump(m_impl->exifData);
//    fflush(stdout);
//    fflush(stderr);
//#endif
    qDebug() << "EXIF header detected";
    m_impl->FillMetadata();
    return true;
#elif defined (HAS_QTEXTENDED)
    m_impl->exifHeader.clear();
    if(!m_impl->exifHeader.loadFromJpeg(filePath))
        return false;
    qDebug() << "EXIF header detected";
    m_impl->FillMetadata();
    return true;
#else
    Q_UNUSED(filePath);
    return false;
#endif
}

bool ImageMetaData::readExifData(const QByteArray &rawExifData)
{
    m_impl->entryListMap.clear();
#if defined (HAS_LIBEXIF)
    if(m_impl->exifData)
        exif_data_unref(m_impl->exifData);
    const QByteArray rawExifDataWithHeader = QByteArray("Exif\0\0", 6) + rawExifData;
    const unsigned char* data = reinterpret_cast<const unsigned char*>(rawExifDataWithHeader.data());
    const unsigned int dataSize = static_cast<unsigned int>(rawExifDataWithHeader.size());
    m_impl->exifData = exif_data_new_from_data(data, dataSize);
    if(!m_impl->exifData)
        return false;
//#if defined (QT_DEBUG)
//    fflush(stdout);
//    fflush(stderr);
//    exif_data_dump(m_impl->exifData);
//    fflush(stdout);
//    fflush(stderr);
//#endif
    qDebug() << "EXIF header detected";
    m_impl->FillMetadata();
    return true;
#elif defined (HAS_QTEXTENDED)
    m_impl->exifHeader.clear();
    QBuffer buffer(const_cast<QByteArray*>(&rawExifData));
    if(!buffer.open(QIODevice::ReadOnly))
        return false;
    if(!m_impl->exifHeader.read(&buffer))
        return false;
    qDebug() << "EXIF header detected";
    m_impl->FillMetadata();
    return true;
#else
    Q_UNUSED(rawExifData);
    return false;
#endif
}

// https://bugreports.qt.io/browse/QTBUG-37946
// https://codereview.qt-project.org/#/c/110668/2
// https://github.com/qt/qtbase/blob/v5.4.0/src/gui/image/qjpeghandler.cpp
void ImageMetaData::applyExifOrientation(QImage *image) const
{
    if(!image || image->isNull())
        return;

    // This is not an optimized implementation, but easiest to maintain
    QTransform transform;

    switch(m_impl->getExifOrientation())
    {
        case 1: // normal
            break;
        case 2: // mirror horizontal
            *image = image->mirrored(true, false);
            break;
        case 3: // rotate 180
            transform.rotate(180);
            *image = image->transformed(transform);
            break;
        case 4: // mirror vertical
            *image = image->mirrored(false, true);
            break;
        case 5: // mirror horizontal and rotate 270 CCW
            *image = image->mirrored(true, false);
            transform.rotate(270);
            *image = image->transformed(transform);
            break;
        case 6: // rotate 90 CW
            transform.rotate(90);
            *image = image->transformed(transform);
            break;
        case 7: // mirror horizontal and rotate 90 CW
            *image = image->mirrored(true, false);
            transform.rotate(90);
            *image = image->transformed(transform);
            break;
        case 8: // rotate 270 CW
            transform.rotate(-90);
            *image = image->transformed(transform);
            break;
        default:
            qWarning("This should never happen");
    }
}

QList<IImageMetaData::MetaDataType> ImageMetaData::types() const
{
    return m_impl->entryListMap.keys();
}

IImageMetaData::MetaDataEntryList ImageMetaData::metaData(IImageMetaData::MetaDataType type) const
{
    return m_impl->entryListMap.value(type);
}
