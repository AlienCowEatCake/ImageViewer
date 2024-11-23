/*
   Copyright (C) 2017-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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
#include <string>

#include <QApplication>
#include <QImage>
#include <QTransform>
#include <QBuffer>
#include <QFile>
#include <QString>
#include <QStringList>

//#undef HAS_EXIV2
//#undef HAS_LIBEXIF
//#undef HAS_QTEXTENDED

#if defined (HAS_EXIV2)
#include "Workarounds/BeginIgnoreDeprecated.h"
#include <exiv2/types.hpp>
#if (QT_VERSION_CHECK(EXIV2_MAJOR_VERSION, EXIV2_MINOR_VERSION, EXIV2_PATCH_VERSION) >= QT_VERSION_CHECK(0, 21, 0))
#include <exiv2/exiv2.hpp>
#else
#if (QT_VERSION_CHECK(EXIV2_MAJOR_VERSION, EXIV2_MINOR_VERSION, EXIV2_PATCH_VERSION) >= QT_VERSION_CHECK(0, 18, 1))
#include <exiv2/easyaccess.hpp>
#endif
#include <exiv2/image.hpp>
#endif
#include "Workarounds/EndIgnoreDeprecated.h"
#endif
#if defined (HAS_LIBEXIF)
#include <libexif/exif-data.h>
#include <libexif/exif-loader.h>
#endif
#if defined (HAS_QTEXTENDED)
#include "qexifimageheader.h"
#endif

#include "Utils/Global.h"
#include "Utils/Logging.h"

#include "Utils/MappedBuffer.h"

namespace {

const QString TYPE_CUSTOM = QString::fromLatin1("Custom");
const QString TAG_CUSTOM_ORIENTATION = QString::fromLatin1("Orientation");
const QString TAG_CUSTOM_X_RESOLUTION = QString::fromLatin1("XResolution");
const QString TAG_CUSTOM_Y_RESOLUTION = QString::fromLatin1("YResolution");

#if defined (HAS_EXIV2)

#if (QT_VERSION_CHECK(EXIV2_MAJOR_VERSION, EXIV2_MINOR_VERSION, EXIV2_PATCH_VERSION) < QT_VERSION_CHECK(0, 28, 0))
typedef Exiv2::Image::AutoPtr Exiv2ImagePtr;
long Exiv2ValueToLong(const Exiv2::Value &value, long n = 0) { return value.toLong(n); }
#else
typedef Exiv2::Image::UniquePtr Exiv2ImagePtr;
long Exiv2ValueToLong(const Exiv2::Value &value, long n = 0) { return static_cast<long>(value.toInt64(static_cast<size_t>(n))); }
#endif

class Exiv2Initializer : public QObject
{
public:
    explicit Exiv2Initializer(QObject *parent = Q_NULLPTR)
        : QObject(parent)
    {
        Exiv2::XmpParser::initialize();
#if defined (EXV_ENABLE_BMFF) && (QT_VERSION_CHECK(EXIV2_MAJOR_VERSION, EXIV2_MINOR_VERSION, EXIV2_PATCH_VERSION) < QT_VERSION_CHECK(0, 28, 3))
        Exiv2::enableBMFF(true);
#endif
    }

    ~Exiv2Initializer()
    {
        Exiv2::XmpParser::terminate();
    }
};

void exiv2Initialize()
{
    static bool initialized = false;
    if(!initialized)
    {
        new Exiv2Initializer(qApp);
        initialized = true;
    }
}

#endif
#if defined (HAS_QTEXTENDED)

template<typename T>
QString qtExtendedToStringValue(const T &value)
{
    return QString::fromLatin1("%1").arg(value);
}

template<>
QString qtExtendedToStringValue(const QString &value)
{
    return value;
}

template<>
QString qtExtendedToStringValue(const QByteArray &value)
{
    return QString::fromLatin1(value.toHex());
}

template<>
QString qtExtendedToStringValue(const QExifURational &value)
{
    return QString::fromLatin1("%1/%2").arg(value.first).arg(value.second);
}

template<>
QString qtExtendedToStringValue(const QExifSRational &value)
{
    return QString::fromLatin1("%1/%2").arg(value.first).arg(value.second);
}

template<typename T>
QString qtExtendedToStringValue(const QVector<T> &value)
{
    QStringList result;
    for(typename QVector<T>::ConstIterator it = value.begin(), itEnd = value.end(); it != itEnd; ++it)
        result.append(qtExtendedToStringValue(*it));
    return QString::fromLatin1("{%1}").arg(result.join(QString::fromLatin1(", ")));
}

template<>
QString qtExtendedToStringValue(const QExifValue &value)
{
    const bool isVector = value.count() > 1;
    switch(value.type())
    {
    case QExifValue::Byte:
        return isVector ? qtExtendedToStringValue(value.toByteVector()) : qtExtendedToStringValue(value.toByte());
    case QExifValue::Ascii:
        return qtExtendedToStringValue(value.toString());
    case QExifValue::Short:
        return isVector ? qtExtendedToStringValue(value.toShortVector()) : qtExtendedToStringValue(value.toShort());
    case QExifValue::Long:
        return isVector ? qtExtendedToStringValue(value.toLongVector()) : qtExtendedToStringValue(value.toLong());
    case QExifValue::Rational:
        return isVector ? qtExtendedToStringValue(value.toRationalVector()) : qtExtendedToStringValue(value.toRational());
    case QExifValue::Undefined:
        return qtExtendedToStringValue(value.toByteArray());
    case QExifValue::SignedLong:
        return isVector ? qtExtendedToStringValue(value.toSignedLongVector()) : qtExtendedToStringValue(value.toSignedLong());
    case QExifValue::SignedRational:
        return isVector ? qtExtendedToStringValue(value.toSignedRationalVector()) : qtExtendedToStringValue(value.toSignedRational());
    default:
        break;
    }
    return QString::fromLatin1("-");
}

/// @see http://www.cipa.jp/std/documents/e/DC-008-Translation-2016-E.pdf
IImageMetaData::MetaDataEntry qtExtendedMakeEntry(int tagID, const QExifValue &value)
{
    const QString stringValue = qtExtendedToStringValue(value);
    switch(tagID)
    {
#define ADD_CASE(X) case QExifImageHeader::X: return IImageMetaData::MetaDataEntry(QString::fromLatin1(#X), stringValue);
    // ImageTag
    ADD_CASE(ImageWidth);                   // 0x0100
    ADD_CASE(ImageLength);                  // 0x0101
    ADD_CASE(BitsPerSample);                // 0x0102
    ADD_CASE(Compression);                  // 0x0103
    ADD_CASE(PhotometricInterpretation);    // 0x0106
    ADD_CASE(Orientation);                  // 0x0112
    ADD_CASE(SamplesPerPixel);              // 0x0115
    ADD_CASE(PlanarConfiguration);          // 0x011C
    ADD_CASE(YCbCrSubSampling);             // 0x0212
    ADD_CASE(XResolution);                  // 0x011A
    ADD_CASE(YResolution);                  // 0x011B
    ADD_CASE(ResolutionUnit);               // 0x0128
    ADD_CASE(StripOffsets);                 // 0x0111
    ADD_CASE(RowsPerStrip);                 // 0x0116
    ADD_CASE(StripByteCounts);              // 0x0117
    ADD_CASE(TransferFunction);             // 0x012D
    ADD_CASE(WhitePoint);                   // 0x013E
    ADD_CASE(PrimaryChromaciticies);        // 0x013F
    ADD_CASE(YCbCrCoefficients);            // 0x0211
    ADD_CASE(ReferenceBlackWhite);          // 0x0214
    ADD_CASE(DateTime);                     // 0x0132
    ADD_CASE(ImageDescription);             // 0x010E
    ADD_CASE(Make);                         // 0x010F
    ADD_CASE(Model);                        // 0x0110
    ADD_CASE(Software);                     // 0x0131
    ADD_CASE(Artist);                       // 0x013B
    ADD_CASE(Copyright);                    // 0x8298
    // ExifExtendedTag
    ADD_CASE(ExifVersion);                  // 0x9000
    ADD_CASE(FlashPixVersion);              // 0xA000
    ADD_CASE(ColorSpace);                   // 0xA001
    ADD_CASE(ComponentsConfiguration);      // 0x9101
    ADD_CASE(CompressedBitsPerPixel);       // 0x9102
    ADD_CASE(PixelXDimension);              // 0xA002
    ADD_CASE(PixelYDimension);              // 0xA003
    ADD_CASE(MakerNote);                    // 0x927C
    ADD_CASE(UserComment);                  // 0x9286
    ADD_CASE(RelatedSoundFile);             // 0xA004
    ADD_CASE(DateTimeOriginal);             // 0x9003
    ADD_CASE(DateTimeDigitized);            // 0x9004
    ADD_CASE(SubSecTime);                   // 0x9290
    ADD_CASE(SubSecTimeOriginal);           // 0x9291
    ADD_CASE(SubSecTimeDigitized);          // 0x9292
    ADD_CASE(ImageUniqueId);                // 0xA420
    ADD_CASE(ExposureTime);                 // 0x829A
    ADD_CASE(FNumber);                      // 0x829D
    ADD_CASE(ExposureProgram);              // 0x8822
    ADD_CASE(SpectralSensitivity);          // 0x8824
    ADD_CASE(ISOSpeedRatings);              // 0x8827
    ADD_CASE(Oecf);                         // 0x8828
    ADD_CASE(ShutterSpeedValue);            // 0x9201
    ADD_CASE(ApertureValue);                // 0x9202
    ADD_CASE(BrightnessValue);              // 0x9203
    ADD_CASE(ExposureBiasValue);            // 0x9204
    ADD_CASE(MaxApertureValue);             // 0x9205
    ADD_CASE(SubjectDistance);              // 0x9206
    ADD_CASE(MeteringMode);                 // 0x9207
    ADD_CASE(LightSource);                  // 0x9208
    ADD_CASE(Flash);                        // 0x9209
    ADD_CASE(FocalLength);                  // 0x920A
    ADD_CASE(SubjectArea);                  // 0x9214
    ADD_CASE(FlashEnergy);                  // 0xA20B
    ADD_CASE(SpatialFrequencyResponse);     // 0xA20C
    ADD_CASE(FocalPlaneXResolution);        // 0xA20E
    ADD_CASE(FocalPlaneYResolution);        // 0xA20F
    ADD_CASE(FocalPlaneResolutionUnit);     // 0xA210
    ADD_CASE(SubjectLocation);              // 0xA214
    ADD_CASE(ExposureIndex);                // 0xA215
    ADD_CASE(SensingMethod);                // 0xA217
    ADD_CASE(FileSource);                   // 0xA300
    ADD_CASE(SceneType);                    // 0xA301
    ADD_CASE(CfaPattern);                   // 0xA302
    ADD_CASE(CustomRendered);               // 0xA401
    ADD_CASE(ExposureMode);                 // 0xA402
    ADD_CASE(WhiteBalance);                 // 0xA403
    ADD_CASE(DigitalZoomRatio);             // 0xA404
    ADD_CASE(FocalLengthIn35mmFilm);        // 0xA405
    ADD_CASE(SceneCaptureType);             // 0xA406
    ADD_CASE(GainControl);                  // 0xA407
    ADD_CASE(Contrast);                     // 0xA408
    ADD_CASE(Saturation);                   // 0xA409
    ADD_CASE(Sharpness);                    // 0xA40A
    ADD_CASE(DeviceSettingDescription);     // 0xA40B
    ADD_CASE(SubjectDistanceRange);         // 0x40C
    // GpsTag
    ADD_CASE(GpsVersionId);                 // 0x0000
    ADD_CASE(GpsLatitudeRef);               // 0x0001
    ADD_CASE(GpsLatitude);                  // 0x0002
    ADD_CASE(GpsLongitudeRef);              // 0x0003
    ADD_CASE(GpsLongitude);                 // 0x0004
    ADD_CASE(GpsAltitudeRef);               // 0x0005
    ADD_CASE(GpsAltitude);                  // 0x0006
    ADD_CASE(GpsTimeStamp);                 // 0x0007
    ADD_CASE(GpsSatellites);                // 0x0008
    ADD_CASE(GpsStatus);                    // 0x0009
    ADD_CASE(GpsMeasureMode);               // 0x000A
    ADD_CASE(GpsDop);                       // 0x000B
    ADD_CASE(GpsSpeedRef);                  // 0x000C
    ADD_CASE(GpsSpeed);                     // 0x000D
    ADD_CASE(GpsTrackRef);                  // 0x000E
    ADD_CASE(GpsTrack);                     // 0x000F
    ADD_CASE(GpsImageDirectionRef);         // 0x0010
    ADD_CASE(GpsImageDirection);            // 0x0011
    ADD_CASE(GpsMapDatum);                  // 0x0012
    ADD_CASE(GpsDestLatitudeRef);           // 0x0013
    ADD_CASE(GpsDestLatitude);              // 0x0014
    ADD_CASE(GpsDestLongitudeRef);          // 0x0015
    ADD_CASE(GpsDestLongitude);             // 0x0016
    ADD_CASE(GpsDestBearingRef);            // 0x0017
    ADD_CASE(GpsDestBearing);               // 0x0018
    ADD_CASE(GpsDestDistanceRef);           // 0x0019
    ADD_CASE(GpsDestDistance);              // 0x001A
    ADD_CASE(GpsProcessingMethod);          // 0x001B
    ADD_CASE(GpsAreaInformation);           // 0x001C
    ADD_CASE(GpsDateStamp);                 // 0x001D
    ADD_CASE(GpsDifferential);              // 0x001E
#undef ADD_CASE
    default:
        break;
    }
    return IImageMetaData::MetaDataEntry(QString::fromLatin1("Unknown (Dec: %1, Hex: 0x%2)").arg(tagID).arg(QString::number(tagID, 16)), stringValue);
}

#endif

} // namespace

struct ImageMetaData::Impl
{
    IImageMetaData::MetaDataEntryListMap entryListMap;
    bool isEntryListLoaded;
#if defined (HAS_EXIV2)
    Exiv2ImagePtr exiv2Image;
    Exiv2::ExifData exiv2ExifData;
    Exiv2::XmpData exiv2XmpData;
#endif
#if defined (HAS_LIBEXIF)
    ExifData *libexifExifData;
#endif
#if defined (HAS_QTEXTENDED)
    QExifImageHeader qtExtendedExifHeader;
#endif

    Impl()
        : isEntryListLoaded(false)
    {
#if defined (HAS_LIBEXIF)
        libexifExifData = Q_NULLPTR;
#endif
    }

    ~Impl()
    {
#if defined (HAS_LIBEXIF)
        if(libexifExifData)
            exif_data_unref(libexifExifData);
#endif
    }

#if defined (HAS_EXIV2)

    bool exiv2FillExifMetaData()
    {
        exiv2Initialize();
        bool result = false;
        QList<const Exiv2::ExifData*> exiv2ExifDataList;
        if(exiv2Image.get())
            exiv2ExifDataList.append(&exiv2Image->exifData());
        if(!exiv2ExifData.empty())
            exiv2ExifDataList.append(&exiv2ExifData);
        for(QList<const Exiv2::ExifData*>::ConstIterator itList = exiv2ExifDataList.begin(); itList != exiv2ExifDataList.end(); ++itList)
        {
            try
            {
                const Exiv2::ExifData &data = *(*itList);
                for(Exiv2::ExifData::const_iterator it = data.begin(), end = data.end(); it != end; ++it)
                {
                    try
                    {
#if (QT_VERSION_CHECK(EXIV2_MAJOR_VERSION, EXIV2_MINOR_VERSION, EXIV2_PATCH_VERSION) > QT_VERSION_CHECK(0, 18, 0))
                        const QString familyName = QString::fromUtf8(it->familyName());
#else
                        const QString familyName = QString::fromLatin1("EXIF");
#endif
                        const IImageMetaData::MetaDataType type = familyName + QString::fromLatin1(" ") + QString::fromUtf8(it->ifdName());
                        IImageMetaData::MetaDataEntryList list = entryListMap[type];
                        QString tagName;
                        QString tagTitle;
                        QString tagDescription;
#if defined (HAS_LIBEXIF)
                        tagName = QString::fromUtf8(exif_tag_get_name(static_cast<ExifTag>(it->tag())));
                        tagTitle = QString::fromUtf8(exif_tag_get_title(static_cast<ExifTag>(it->tag())));
                        tagDescription = QString::fromUtf8(exif_tag_get_description(static_cast<ExifTag>(it->tag())));
#endif
                        if(tagName.isEmpty())
                            tagName = QString::fromUtf8(it->tagName().c_str());
                        if(tagTitle.isEmpty())
                            tagTitle = QString::fromUtf8(it->tagLabel().c_str());
                        list.append(IImageMetaData::MetaDataEntry(
                                        tagName,
                                        tagTitle,
                                        tagDescription,
                                        QString::fromUtf8(it->print().c_str())
                                        ));
                        entryListMap[type] = list;
                        result |= true;
                    }
                    catch(...)
                    {}
                }
            }
            catch(...)
            {}
        }
        return result;
    }

    bool exiv2FillIptcMetaData()
    {
        exiv2Initialize();
        bool result = false;
        QList<const Exiv2::IptcData*> exiv2IptcDataList;
        if(exiv2Image.get())
            exiv2IptcDataList.append(&exiv2Image->iptcData());
        for(QList<const Exiv2::IptcData*>::ConstIterator itList = exiv2IptcDataList.begin(); itList != exiv2IptcDataList.end(); ++itList)
        {
            try
            {
                const Exiv2::IptcData &data = *(*itList);
                for(Exiv2::IptcData::const_iterator it = data.begin(), end = data.end(); it != end; ++it)
                {
                    try
                    {
#if (QT_VERSION_CHECK(EXIV2_MAJOR_VERSION, EXIV2_MINOR_VERSION, EXIV2_PATCH_VERSION) > QT_VERSION_CHECK(0, 18, 0))
                        const IImageMetaData::MetaDataType type = QString::fromUtf8(it->familyName());
#else
                        const IImageMetaData::MetaDataType type = QString::fromLatin1("IPTC");
#endif
                        IImageMetaData::MetaDataEntryList list = entryListMap[type];
                        list.append(IImageMetaData::MetaDataEntry(
                                        QString::fromUtf8(it->tagName().c_str()),
                                        QString::fromUtf8(it->tagLabel().c_str()),
                                        QString(),
                                        QString::fromUtf8(it->print().c_str())
                                        ));
                        entryListMap[type] = list;
                        result |= true;
                    }
                    catch(...)
                    {}
                }
            }
            catch(...)
            {}
        }
        return result;
    }

    bool exiv2FillXmpMetaData()
    {
        exiv2Initialize();
        bool result = false;
        QList<const Exiv2::XmpData*> exiv2XmpDataList;
        if(exiv2Image.get())
            exiv2XmpDataList.append(&exiv2Image->xmpData());
        if(!exiv2XmpData.empty())
            exiv2XmpDataList.append(&exiv2XmpData);
        for(QList<const Exiv2::XmpData*>::ConstIterator itList = exiv2XmpDataList.begin(); itList != exiv2XmpDataList.end(); ++itList)
        {
            try
            {
                const Exiv2::XmpData &data = *(*itList);
                for(Exiv2::XmpData::const_iterator it = data.begin(), end = data.end(); it != end; ++it)
                {
                    try
                    {
#if (QT_VERSION_CHECK(EXIV2_MAJOR_VERSION, EXIV2_MINOR_VERSION, EXIV2_PATCH_VERSION) > QT_VERSION_CHECK(0, 18, 0))
                        const IImageMetaData::MetaDataType type = QString::fromUtf8(it->familyName());
#else
                        const IImageMetaData::MetaDataType type = QString::fromLatin1("XMP");
#endif
                        IImageMetaData::MetaDataEntryList list = entryListMap[type];
                        list.append(IImageMetaData::MetaDataEntry(
                                        QString::fromUtf8(it->tagName().c_str()),
                                        QString::fromUtf8(it->tagLabel().c_str()),
                                        QString(),
                                        QString::fromUtf8(it->print().c_str())
                                        ));
                        entryListMap[type] = list;
                        result |= true;
                    }
                    catch(...)
                    {}
                }
            }
            catch(...)
            {}
        }
        return result;
    }

    bool exiv2FillCommentMetaData()
    {
        exiv2Initialize();
        if(!exiv2Image.get())
            return false;
        const std::string data = exiv2Image->comment();
        if(data.empty())
            return false;
        const IImageMetaData::MetaDataType type = QString::fromLatin1("Comment");
        IImageMetaData::MetaDataEntryList list = entryListMap[type];
        list.append(IImageMetaData::MetaDataEntry(type, QString::fromUtf8(data.c_str())));
        entryListMap[type] = list;
        return true;
    }

#endif
#if defined (HAS_LIBEXIF)

    bool libexifFillExifMetaData()
    {
        bool result = false;
        if(!libexifExifData)
            return result;
        for(int i = 0; i < EXIF_IFD_COUNT; i++)
        {
            if(!libexifExifData->ifd[i] || !libexifExifData->ifd[i]->count)
                continue;
            const IImageMetaData::MetaDataType type = QString::fromLatin1("EXIF IFD %1").arg(QString::fromUtf8(exif_ifd_get_name(static_cast<ExifIfd>(i))));
            IImageMetaData::MetaDataEntryList list = entryListMap[type];
            for(unsigned int j = 0, count = libexifExifData->ifd[i]->count; j < count; j++)
            {
                ExifEntry *e = libexifExifData->ifd[i]->entries[j];
                QByteArray value(qMax(static_cast<int>(8192), static_cast<int>(e->size)), '\0');
                list.append(IImageMetaData::MetaDataEntry(
                                QString::fromUtf8(exif_tag_get_name_in_ifd(e->tag, static_cast<ExifIfd>(i))),
                                QString::fromUtf8(exif_tag_get_title_in_ifd(e->tag, static_cast<ExifIfd>(i))),
                                QString::fromUtf8(exif_tag_get_description_in_ifd(e->tag, static_cast<ExifIfd>(i))),
                                QString::fromUtf8(exif_entry_get_value(e, value.data(), static_cast<unsigned int>(value.size())))
                                ));
            }
            entryListMap[type] = list;
            result |= true;
        }
        return result;
    }

#endif
#if defined (HAS_QTEXTENDED)

    bool qtExtendedFillExifMetaData()
    {
        bool result = false;

        const QList<QExifImageHeader::ImageTag> imageTags = qtExtendedExifHeader.imageTags();
        if(!imageTags.empty())
        {
            const IImageMetaData::MetaDataType imageTagsType = QString::fromLatin1("EXIF IFD 0");
            IImageMetaData::MetaDataEntryList imageTagsData = entryListMap[imageTagsType];
            for(QList<QExifImageHeader::ImageTag>::ConstIterator it = imageTags.constBegin(); it != imageTags.constEnd(); ++it)
                imageTagsData.append(qtExtendedMakeEntry(*it, qtExtendedExifHeader.value(*it)));
            entryListMap[imageTagsType] = imageTagsData;
            result |= true;
        }

        const QList<QExifImageHeader::ExifExtendedTag> extendedTags = qtExtendedExifHeader.extendedTags();
        if(!extendedTags.empty())
        {
            const IImageMetaData::MetaDataType extendedTagsType = QString::fromLatin1("EXIF IFD EXIF");
            IImageMetaData::MetaDataEntryList extendedTagsData = entryListMap[extendedTagsType];
            for(QList<QExifImageHeader::ExifExtendedTag>::ConstIterator it = extendedTags.constBegin(); it != extendedTags.constEnd(); ++it)
                extendedTagsData.append(qtExtendedMakeEntry(*it, qtExtendedExifHeader.value(*it)));
            entryListMap[extendedTagsType] = extendedTagsData;
            result |= true;
        }

        const QList<QExifImageHeader::GpsTag> gpsTags = qtExtendedExifHeader.gpsTags();
        if(!gpsTags.empty())
        {
            const IImageMetaData::MetaDataType gpsTagsType = QString::fromLatin1("EXIF IFD GPS");
            IImageMetaData::MetaDataEntryList gpsTagsData = entryListMap[gpsTagsType];
            for(QList<QExifImageHeader::GpsTag>::ConstIterator it = gpsTags.constBegin(); it != gpsTags.constEnd(); ++it)
                gpsTagsData.append(qtExtendedMakeEntry(*it, qtExtendedExifHeader.value(*it)));
            entryListMap[gpsTagsType] = gpsTagsData;
            result |= true;
        }

        return result;
    }

#endif

    void ensureMetaDataFilled()
    {
        if(isEntryListLoaded)
            return;
        const bool exifLoaded = false
#if defined (HAS_LIBEXIF)
                || libexifFillExifMetaData()
#endif
#if defined (HAS_EXIV2)
                || exiv2FillExifMetaData()
#endif
#if defined (HAS_QTEXTENDED)
                || qtExtendedFillExifMetaData()
#endif
                ;
        const bool iptcLoaded = false
#if defined (HAS_EXIV2)
                || exiv2FillIptcMetaData()
#endif
                ;
        const bool xmpLoaded = false
#if defined (HAS_EXIV2)
                || exiv2FillXmpMetaData()
#endif
                ;
        const bool commentLoaded = false
#if defined (HAS_EXIV2)
                || exiv2FillCommentMetaData()
#endif
                ;
        LOG_DEBUG() << LOGGING_CTX << "Fill MetaData:"
                << "EXIF =" << exifLoaded
                << "IPTC =" << iptcLoaded
                << "XMP =" << xmpLoaded
                << "COMMENT =" << commentLoaded
                ;
        isEntryListLoaded = true;
    }
};

ImageMetaData *ImageMetaData::createMetaData(const QString &filePath)
{
    ImageMetaData *metaData = new ImageMetaData();
    if(metaData->readFile(filePath))
        return metaData;
    delete metaData;
    return Q_NULLPTR;
}

ImageMetaData *ImageMetaData::createMetaData(const QByteArray &fileData)
{
    ImageMetaData *metaData = new ImageMetaData();
    if(metaData->readFile(fileData))
        return metaData;
    delete metaData;
    return Q_NULLPTR;
}

ImageMetaData *ImageMetaData::createExifMetaData(const QByteArray &rawExifData)
{
    ImageMetaData *metaData = new ImageMetaData();
    if(metaData->readExifData(rawExifData))
        return metaData;
    delete metaData;
    return Q_NULLPTR;
}

ImageMetaData *ImageMetaData::createXmpMetaData(const QByteArray &rawXmpData)
{
    ImageMetaData *metaData = new ImageMetaData();
    if(metaData->readXmpData(rawXmpData))
        return metaData;
    delete metaData;
    return Q_NULLPTR;
}

ImageMetaData *ImageMetaData::createQImageMetaData(const QImage &image)
{
    ImageMetaData *metaData = Q_NULLPTR;
    const QStringList keys = image.textKeys();
    for(QStringList::ConstIterator it = keys.constBegin(); it != keys.constEnd(); ++it)
    {
        if(it->isEmpty())
            continue;

        static const QString xmpKey = QString::fromLatin1("XML:com.adobe.xmp");
        if(*it == xmpKey)
        {
            metaData = joinMetaData(metaData, createXmpMetaData(image.text(xmpKey).toUtf8()));
            continue;
        }

        if(!metaData)
            metaData = new ImageMetaData();
        static const QString type = QString::fromLatin1("QImage Text");
        metaData->addCustomEntry(type, *it, image.text(*it));
    }
    return metaData;
}

ImageMetaData *ImageMetaData::joinMetaData(ImageMetaData *first, ImageMetaData *second)
{
    if(!first)
        return second;
    if(!second)
        return first;

    first->m_impl->ensureMetaDataFilled();
    const QList<MetaDataType> secondTypes = second->types();
    for(QList<MetaDataType>::ConstIterator it = secondTypes.constBegin(), itEnd = secondTypes.constEnd(); it != itEnd; ++it)
    {
        const MetaDataEntryList secondMetaData = second->metaData(*it);
        for(MetaDataEntryList::ConstIterator jt = secondMetaData.constBegin(), jtEnd = secondMetaData.constEnd(); jt != jtEnd; ++jt)
            first->addCustomEntry(*it, *jt);
    }

    delete second;
    return first;
}

// https://bugreports.qt.io/browse/QTBUG-37946
// https://codereview.qt-project.org/#/c/110668/2
// https://github.com/qt/qtbase/blob/v5.4.0/src/gui/image/qjpeghandler.cpp
void ImageMetaData::applyExifOrientation(QImage *image, quint16 orientation)
{
    if(!image || image->isNull())
        return;

    // This is not an optimized implementation, but easiest to maintain
    QTransform transform;

    switch(orientation)
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
            LOG_WARNING() << LOGGING_CTX << "This should never happen";
    }
}

ImageMetaData::ImageMetaData()
    : m_impl(new Impl())
{}

ImageMetaData::~ImageMetaData()
{}

void ImageMetaData::applyExifOrientation(QImage *image) const
{
    applyExifOrientation(image, orientation());
}

void ImageMetaData::addExifEntry(const QString &type, int tag, const QString &tagString, const QString &value)
{
#if defined (HAS_LIBEXIF)
    const char *name = exif_tag_get_name(static_cast<ExifTag>(tag));
    const char *title = exif_tag_get_title(static_cast<ExifTag>(tag));
    const char *description = exif_tag_get_description(static_cast<ExifTag>(tag));
    if(name && title)
    {
        addCustomEntry(type, IImageMetaData::MetaDataEntry(QString::fromUtf8(name), QString::fromUtf8(title), QString::fromUtf8(description), value));
        return;
    }
#endif
    Q_UNUSED(tag);
    addCustomEntry(type, tagString, value);
}

void ImageMetaData::addCustomEntry(const QString &type, const QString &tag, const QString &value)
{
    addCustomEntry(type, IImageMetaData::MetaDataEntry(tag, value));
}

void ImageMetaData::addCustomEntry(const QString &type, const IImageMetaData::MetaDataEntry &entry)
{
    IImageMetaData::MetaDataEntryList &list = m_impl->entryListMap[type];
    if(!list.contains(entry))
        list.append(entry);
}

void ImageMetaData::addCustomOrientation(quint16 orientation)
{
    addCustomEntry(TYPE_CUSTOM, TAG_CUSTOM_ORIENTATION, QString::number(orientation));
}

void ImageMetaData::addCustomDpi(qreal dpiX, qreal dpiY)
{
    addCustomEntry(TYPE_CUSTOM, TAG_CUSTOM_X_RESOLUTION, QString::number(dpiX));
    addCustomEntry(TYPE_CUSTOM, TAG_CUSTOM_Y_RESOLUTION, QString::number(dpiY));
}

QList<IImageMetaData::MetaDataType> ImageMetaData::types()
{
    m_impl->ensureMetaDataFilled();
    return m_impl->entryListMap.keys();
}

IImageMetaData::MetaDataEntryList ImageMetaData::metaData(const IImageMetaData::MetaDataType &type)
{
    m_impl->ensureMetaDataFilled();
    return m_impl->entryListMap.value(type);
}

quint16 ImageMetaData::orientation() const
{
    const IImageMetaData::MetaDataEntryListMap::ConstIterator customEntryList = m_impl->entryListMap.find(TYPE_CUSTOM);
    if(customEntryList != m_impl->entryListMap.constEnd())
    {
        for(IImageMetaData::MetaDataEntryList::ConstIterator it = customEntryList->constBegin(); it != customEntryList->constEnd(); ++it)
        {
            if(it->tagName == TAG_CUSTOM_ORIENTATION)
            {
                bool ok = false;
                uint result = it->value.toUInt(&ok);
                if(ok && result >= 1 && result <= 8)
                {
                    LOG_DEBUG() << LOGGING_CTX << "CUSTOM: orientation =" << result;
                    return static_cast<quint16>(result);
                }
            }
        }
    }

#if defined (HAS_EXIV2)
#if (QT_VERSION_CHECK(EXIV2_MAJOR_VERSION, EXIV2_MINOR_VERSION, EXIV2_PATCH_VERSION) >= QT_VERSION_CHECK(0, 18, 1))
    exiv2Initialize();
    QList<const Exiv2::ExifData*> exiv2ExifDataList;
    if(m_impl->exiv2Image.get())
        exiv2ExifDataList.append(&m_impl->exiv2Image->exifData());
    if(!m_impl->exiv2ExifData.empty())
        exiv2ExifDataList.append(&m_impl->exiv2ExifData);
    for(QList<const Exiv2::ExifData*>::ConstIterator itList = exiv2ExifDataList.begin(); itList != exiv2ExifDataList.end(); ++itList)
    {
        try
        {
            const Exiv2::ExifData &data = *(*itList);
            Exiv2::ExifData::const_iterator itData = Exiv2::orientation(data);
            if(itData == data.end())
                continue;
            const quint16 orientation = static_cast<quint16>(Exiv2ValueToLong(itData->value()));
            LOG_DEBUG() << LOGGING_CTX << "EXIV2: EXIF orientation =" << orientation;
            return orientation;
        }
        catch(...)
        {}
    }
#else
    /// @todo Exiv2 less 0.18.1: Add implementation?
#endif
#endif
#if defined (HAS_LIBEXIF)
    if(m_impl->libexifExifData)
    {
        ExifEntry *entry = exif_content_get_entry(m_impl->libexifExifData->ifd[EXIF_IFD_0], EXIF_TAG_ORIENTATION);
        if(entry && entry->parent && entry->parent->parent && entry->format == EXIF_FORMAT_SHORT && entry->components == 1)
        {
            const quint16 orientation = exif_get_short(entry->data, exif_data_get_byte_order(entry->parent->parent));
            LOG_DEBUG() << LOGGING_CTX << "LIBEXIF: EXIF orientation =" << orientation;
            return orientation;
        }
    }
#endif
#if defined (HAS_QTEXTENDED)
    if(m_impl->qtExtendedExifHeader.contains(QExifImageHeader::Orientation))
    {
        const quint16 orientation = m_impl->qtExtendedExifHeader.value(QExifImageHeader::Orientation).toShort();
        LOG_DEBUG() << LOGGING_CTX << "QTEXTENDED: EXIF orientation =" << orientation;
        return orientation;
    }
#endif
    return 1;
}

QPair<qreal, qreal> ImageMetaData::dpi() const
{
    const IImageMetaData::MetaDataEntryListMap::ConstIterator customEntryList = m_impl->entryListMap.find(TYPE_CUSTOM);
    if(customEntryList != m_impl->entryListMap.constEnd())
    {
        double resX = -1;
        double resY = -1;
        for(IImageMetaData::MetaDataEntryList::ConstIterator it = customEntryList->constBegin(); it != customEntryList->constEnd(); ++it)
        {
            if(it->tagName == TAG_CUSTOM_X_RESOLUTION)
            {
                bool ok = false;
                resX = it->value.toDouble(&ok);
                if(!ok)
                    resX = -1;
            }
            else if(it->tagName == TAG_CUSTOM_Y_RESOLUTION)
            {
                bool ok = false;
                resY = it->value.toDouble(&ok);
                if(!ok)
                    resY = -1;
            }
        }
        if(resX > 0 && resY > 0)
        {
            const QPair<qreal, qreal> dpi = qMakePair<qreal, qreal>(static_cast<qreal>(resX), static_cast<qreal>(resY));
            LOG_DEBUG() << LOGGING_CTX << "CUSTOM: dpi =" << dpi.first << dpi.second;
            return dpi;
        }
    }

#if defined (HAS_EXIV2)
    exiv2Initialize();
    QList<const Exiv2::ExifData*> exiv2ExifDataList;
    if(m_impl->exiv2Image.get())
        exiv2ExifDataList.append(&m_impl->exiv2Image->exifData());
    if(!m_impl->exiv2ExifData.empty())
        exiv2ExifDataList.append(&m_impl->exiv2ExifData);
    for(QList<const Exiv2::ExifData*>::ConstIterator itList = exiv2ExifDataList.begin(); itList != exiv2ExifDataList.end(); ++itList)
    {
        try
        {
            const Exiv2::ExifData &data = *(*itList);
            Exiv2::ExifData::const_iterator itX = data.findKey(Exiv2::ExifKey("Exif.Image.XResolution"));
            Exiv2::ExifData::const_iterator itY = data.findKey(Exiv2::ExifKey("Exif.Image.YResolution"));
            Exiv2::ExifData::const_iterator itU = data.findKey(Exiv2::ExifKey("Exif.Image.ResolutionUnit"));
            if(itX == data.end() || itY == data.end())
                continue;
            const Exiv2::Rational vX = itX->value().toRational();
            const Exiv2::Rational vY = itY->value().toRational();
            const qreal multiplier = (itU != data.end() && Exiv2ValueToLong(itU->value()) == 3)
                    ? static_cast<qreal>(2.54) // centimeter to inch conversion
                    : static_cast<qreal>(1.0); // no conversion
            if(vX.first > 0 && vX.second > 0 && vY.first > 0 && vY.second > 0)
            {
                const QPair<qreal, qreal> dpi = qMakePair<qreal, qreal>(multiplier * vX.first / vX.second, multiplier * vY.first / vY.second);
                LOG_DEBUG() << LOGGING_CTX << "EXIV2: EXIF dpi =" << dpi.first << dpi.second;
                return dpi;
            }
        }
        catch(...)
        {}
    }
#endif
#if defined (HAS_LIBEXIF)
    if(m_impl->libexifExifData)
    {
        ExifEntry *eX = exif_content_get_entry(m_impl->libexifExifData->ifd[EXIF_IFD_0], EXIF_TAG_X_RESOLUTION);
        ExifEntry *eY = exif_content_get_entry(m_impl->libexifExifData->ifd[EXIF_IFD_0], EXIF_TAG_Y_RESOLUTION);
        if(eX && eY && eX->parent && eY->parent && eX->parent->parent && eY->parent->parent && eX->format == EXIF_FORMAT_RATIONAL && eY->format == EXIF_FORMAT_RATIONAL && eX->components == 1 && eY->components == 1)
        {
            const ExifRational vX = exif_get_rational(eX->data, exif_data_get_byte_order(eX->parent->parent));
            const ExifRational vY = exif_get_rational(eY->data, exif_data_get_byte_order(eY->parent->parent));
            ExifEntry *eU = exif_content_get_entry(m_impl->libexifExifData->ifd[EXIF_IFD_0], EXIF_TAG_RESOLUTION_UNIT);
            const qreal multiplier = (eU && eU->parent && eU->parent->parent && eU->format == EXIF_FORMAT_SHORT && eU->components == 1 && exif_get_short(eU->data, exif_data_get_byte_order(eU->parent->parent)) == 3)
                    ? static_cast<qreal>(2.54) // centimeter to inch conversion
                    : static_cast<qreal>(1.0); // no conversion
            if(vX.numerator > 0 && vX.denominator > 0 && vY.numerator > 0 && vY.denominator > 0)
            {
                const QPair<qreal, qreal> dpi = qMakePair<qreal, qreal>(multiplier * vX.numerator / vX.denominator, multiplier * vY.numerator / vY.denominator);
                LOG_DEBUG() << LOGGING_CTX << "LIBEXIF: EXIF dpi =" << dpi.first << dpi.second;
                return dpi;
            }
        }
    }
#endif
#if defined (HAS_QTEXTENDED)
    if(m_impl->qtExtendedExifHeader.contains(QExifImageHeader::XResolution) && m_impl->qtExtendedExifHeader.contains(QExifImageHeader::YResolution))
    {
        const QExifURational vX = m_impl->qtExtendedExifHeader.value(QExifImageHeader::XResolution).toRational();
        const QExifURational vY = m_impl->qtExtendedExifHeader.value(QExifImageHeader::YResolution).toRational();
        const qreal multiplier = (m_impl->qtExtendedExifHeader.contains(QExifImageHeader::ResolutionUnit) && m_impl->qtExtendedExifHeader.value(QExifImageHeader::ResolutionUnit).toShort() == 3)
                ? static_cast<qreal>(2.54) // centimeter to inch conversion
                : static_cast<qreal>(1.0); // no conversion
        if(vX.first > 0 && vX.second > 0 && vY.first > 0 && vY.second > 0)
        {
            const QPair<qreal, qreal> dpi = qMakePair<qreal, qreal>(multiplier * vX.first / vX.second, multiplier * vY.first / vY.second);
            LOG_DEBUG() << LOGGING_CTX << "QTEXTENDED: EXIF dpi =" << dpi.first << dpi.second;
            return dpi;
        }
    }
#endif

    return qMakePair<qreal, qreal>(72, 72);
}

bool ImageMetaData::readFile(const QString &filePath)
{
    const MappedBuffer inBuffer(filePath);
    if(!inBuffer.isValid())
        return false;
    return readFile(inBuffer.dataAsByteArray());
}

bool ImageMetaData::readFile(const QByteArray &fileData)
{
    m_impl->entryListMap.clear();
    bool status = false;
#if defined (HAS_EXIV2)
    exiv2Initialize();
    try
    {
        m_impl->exiv2Image.reset();
        m_impl->exiv2ExifData.clear();
        m_impl->exiv2XmpData.clear();
        const Exiv2::byte *data = reinterpret_cast<const Exiv2::byte*>(fileData.constData());
        const long dataSize = static_cast<long>(fileData.size());
        m_impl->exiv2Image = Exiv2::ImageFactory::open(data, dataSize);
        if(!m_impl->exiv2Image.get())
            throw std::exception();
        m_impl->exiv2Image->readMetadata();
        if(m_impl->exiv2Image->exifData().empty() && m_impl->exiv2Image->iptcData().empty() && m_impl->exiv2Image->xmpData().empty() && m_impl->exiv2Image->comment().empty())
            throw std::exception();
        if(!m_impl->exiv2Image->exifData().empty())
        {
            LOG_DEBUG() << LOGGING_CTX << "EXIV2: EXIF data detected";
        }
        if(!m_impl->exiv2Image->iptcData().empty())
        {
            LOG_DEBUG() << LOGGING_CTX << "EXIV2: IPTC data detected";
        }
        if(!m_impl->exiv2Image->xmpData().empty())
        {
            LOG_DEBUG() << LOGGING_CTX << "EXIV2: XMP data detected";
        }
        if(!m_impl->exiv2Image->comment().empty())
        {
            LOG_DEBUG() << LOGGING_CTX << "EXIV2: Comment detected";
        }
        status |= true;
    }
    catch(...)
    {
        m_impl->exiv2Image.reset();
    }
#endif
#if defined (HAS_LIBEXIF)
    if(m_impl->libexifExifData)
        exif_data_unref(m_impl->libexifExifData);
    ExifLoader *loader = exif_loader_new();
    unsigned char *buf = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(fileData.constData()));
    unsigned int len = static_cast<unsigned int>(fileData.size());
    exif_loader_write(loader, buf, len);
    m_impl->libexifExifData = exif_loader_get_data(loader);
    exif_loader_unref(loader);
    if(m_impl->libexifExifData)
    {
//#if defined (QT_DEBUG)
//        fflush(stdout);
//        fflush(stderr);
//        exif_data_dump(m_impl->libexifExifData);
//        fflush(stdout);
//        fflush(stderr);
//#endif
        LOG_DEBUG() << LOGGING_CTX << "LIBEXIF: EXIF data detected";
        status |= true;
    }
#endif
#if defined (HAS_QTEXTENDED)
    m_impl->qtExtendedExifHeader.clear();
    QBuffer buffer(const_cast<QByteArray*>(&fileData));
    if(buffer.open(QIODevice::ReadOnly) && m_impl->qtExtendedExifHeader.loadFromJpeg(&buffer))
    {
        LOG_DEBUG() << LOGGING_CTX << "QTEXTENDED: EXIF data detected";
        status |= true;
    }
#endif
    Q_UNUSED(fileData);
    return status;
}

bool ImageMetaData::readExifData(const QByteArray &rawExifData)
{
    m_impl->entryListMap.clear();
    bool status = false;
#if defined (HAS_EXIV2)
    exiv2Initialize();
    try
    {
        m_impl->exiv2Image.reset();
        m_impl->exiv2ExifData.clear();
        m_impl->exiv2XmpData.clear();
#if (QT_VERSION_CHECK(EXIV2_MAJOR_VERSION, EXIV2_MINOR_VERSION, EXIV2_PATCH_VERSION) >= QT_VERSION_CHECK(0, 18, 0))
        const Exiv2::byte *data = reinterpret_cast<const Exiv2::byte*>(rawExifData.constData());
        const uint32_t dataSize = static_cast<uint32_t>(rawExifData.size());
        Exiv2::ExifParser::decode(m_impl->exiv2ExifData, data, dataSize);
        if(m_impl->exiv2ExifData.empty())
            throw std::exception();
        LOG_DEBUG() << LOGGING_CTX << "EXIV2: EXIF data detected";
        status |= true;
#else
        /// @todo Exiv2 less 0.18.0: Add implementation?
#endif
    }
    catch(...)
    {
        m_impl->exiv2ExifData.clear();
    }
#endif
#if defined (HAS_LIBEXIF)
    if(m_impl->libexifExifData)
        exif_data_unref(m_impl->libexifExifData);
    const QByteArray rawExifDataWithHeader = QByteArray("Exif\0\0", 6) + rawExifData;
    const unsigned char *data = reinterpret_cast<const unsigned char*>(rawExifDataWithHeader.data());
    const unsigned int dataSize = static_cast<unsigned int>(rawExifDataWithHeader.size());
    m_impl->libexifExifData = exif_data_new_from_data(data, dataSize);
    if(m_impl->libexifExifData)
    {
//#if defined (QT_DEBUG)
//        fflush(stdout);
//        fflush(stderr);
//        exif_data_dump(m_impl->libexifExifData);
//        fflush(stdout);
//        fflush(stderr);
//#endif
        LOG_DEBUG() << LOGGING_CTX << "LIBEXIF: EXIF data detected";
        status |= true;
    }
#endif
#if defined (HAS_QTEXTENDED)
    m_impl->qtExtendedExifHeader.clear();
    QBuffer buffer(const_cast<QByteArray*>(&rawExifData));
    if(buffer.open(QIODevice::ReadOnly) && m_impl->qtExtendedExifHeader.read(&buffer))
    {
        LOG_DEBUG() << LOGGING_CTX << "QTEXTENDED: EXIF data detected";
        status |= true;
    }
#endif
    Q_UNUSED(rawExifData);
    return status;
}

bool ImageMetaData::readXmpData(const QByteArray &rawXmpData)
{
    m_impl->entryListMap.clear();
    bool status = false;
#if defined (HAS_EXIV2)
    exiv2Initialize();
    try
    {
        m_impl->exiv2Image.reset();
        m_impl->exiv2ExifData.clear();
        m_impl->exiv2XmpData.clear();
        const std::string packet = std::string(reinterpret_cast<const char*>(rawXmpData.constData()), static_cast<size_t>(rawXmpData.size()));
        Exiv2::XmpParser::decode(m_impl->exiv2XmpData, packet);
        if(m_impl->exiv2XmpData.empty())
            throw std::exception();
        LOG_DEBUG() << LOGGING_CTX << "EXIV2: XMP data detected";
        status |= true;
    }
    catch(...)
    {
        m_impl->exiv2XmpData.clear();
    }
#endif
    Q_UNUSED(rawXmpData);
    return status;
}
