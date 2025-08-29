/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2025 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "microexif_p.h"
#include "util_p.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QDataStream>
#include <QHash>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QStringDecoder>
#endif
#include <QTimeZone>

// TIFF 6 specs
#define TIFF_IMAGEWIDTH 0x100
#define TIFF_IMAGEHEIGHT 0x101
#define TIFF_BITSPERSAMPLE 0x102
#define TIFF_IMAGEDESCRIPTION 0x10E
#define TIFF_MAKE 0x10F
#define TIFF_MODEL 0x110
#define TIFF_ORIENT 0x0112
#define TIFF_XRES 0x011A
#define TIFF_YRES 0x011B
#define TIFF_URES 0x0128
#define TIFF_SOFTWARE 0x0131
#define TIFF_ARTIST 0x013B
#define TIFF_DATETIME 0x0132
#define TIFF_COPYRIGHT 0x8298

#define TIFF_VAL_URES_NOABSOLUTE 1
#define TIFF_VAL_URES_INCH 2
#define TIFF_VAL_URES_CENTIMETER 3

// EXIF 3 specs
#define EXIF_EXIFIFD 0x8769
#define EXIF_GPSIFD 0x8825
#define EXIF_EXIFVERSION 0x9000
#define EXIF_DATETIMEORIGINAL 0x9003
#define EXIF_DATETIMEDIGITIZED 0x9004
#define EXIF_OFFSETTIME 0x9010
#define EXIF_OFFSETTIMEORIGINAL 0x9011
#define EXIF_OFFSETTIMEDIGITIZED 0x9012
#define EXIF_COLORSPACE 0xA001
#define EXIF_PIXELXDIM 0xA002
#define EXIF_PIXELYDIM 0xA003
#define EXIF_IMAGEUNIQUEID 0xA420
#define EXIF_BODYSERIALNUMBER 0xA431
#define EXIF_LENSMAKE 0xA433
#define EXIF_LENSMODEL 0xA434
#define EXIF_LENSSERIALNUMBER 0xA435
#define EXIF_IMAGETITLE 0xA436

#define EXIF_VAL_COLORSPACE_SRGB 1
#define EXIF_VAL_COLORSPACE_UNCAL 0xFFFF

#define GPS_GPSVERSION 0
#define GPS_LATITUDEREF 1
#define GPS_LATITUDE 2
#define GPS_LONGITUDEREF 3
#define GPS_LONGITUDE 4
#define GPS_ALTITUDEREF 5
#define GPS_ALTITUDE 6
#define GPS_IMGDIRECTIONREF 16
#define GPS_IMGDIRECTION 17
#define EXIF_TAG_VALUE(n, byteSize) (((n) << 6) | ((byteSize) & 0x3F))
#define EXIF_TAG_SIZEOF(dataType) (quint16(dataType) & 0x3F)
#define EXIF_TAG_DATATYPE(dataType) (quint16(dataType) >> 6)

enum class ExifTagType : quint16 {
    // Base data types
    Byte = EXIF_TAG_VALUE(1, 1),
    Ascii = EXIF_TAG_VALUE(2, 1),
    Short = EXIF_TAG_VALUE(3, 2),
    Long = EXIF_TAG_VALUE(4, 4),
    Rational = EXIF_TAG_VALUE(5, 8),

    // Extended data types
    SByte = EXIF_TAG_VALUE(6, 1),
    Undefined = EXIF_TAG_VALUE(7, 1),
    SShort = EXIF_TAG_VALUE(8, 2),
    SLong = EXIF_TAG_VALUE(9, 4),
    SRational = EXIF_TAG_VALUE(10, 8),

    Float = EXIF_TAG_VALUE(11, 4), // not used in EXIF specs
    Double = EXIF_TAG_VALUE(12, 8), // not used in EXIF specs
    Ifd = EXIF_TAG_VALUE(13, 4), // not used in EXIF specs

    // BigTiff data types (EXIF specs are 32-bits only)
    Long8 = EXIF_TAG_VALUE(16, 8), // not used in EXIF specs
    SLong8 = EXIF_TAG_VALUE(17, 8), // not used in EXIF specs
    Ifd8 = EXIF_TAG_VALUE(18, 8), // not used in EXIF specs

    // Exif 3.0 only
    Utf8 = EXIF_TAG_VALUE(129, 1)
};

using TagPos = QHash<quint16, quint32>;
using KnownTags = QHash<quint16, ExifTagType>;
using TagInfo = std::pair<quint16, ExifTagType>;

/*!
 * \brief staticTagTypes
 * The supported tags.
 * \note EXIF tags are an extension of TIFF tags, so I'm writing them all together.
 */
// clang-format off
static const KnownTags staticTagTypes = {
    TagInfo(TIFF_IMAGEWIDTH, ExifTagType::Long),
    TagInfo(TIFF_IMAGEHEIGHT, ExifTagType::Long),
    TagInfo(TIFF_BITSPERSAMPLE, ExifTagType::Short),
    TagInfo(TIFF_IMAGEDESCRIPTION, ExifTagType::Utf8),
    TagInfo(TIFF_MAKE, ExifTagType::Utf8),
    TagInfo(TIFF_MODEL, ExifTagType::Utf8),
    TagInfo(TIFF_ORIENT, ExifTagType::Short),
    TagInfo(TIFF_XRES, ExifTagType::Rational),
    TagInfo(TIFF_YRES, ExifTagType::Rational),
    TagInfo(TIFF_URES, ExifTagType::Short),
    TagInfo(TIFF_SOFTWARE, ExifTagType::Utf8),
    TagInfo(TIFF_ARTIST, ExifTagType::Utf8),
    TagInfo(TIFF_DATETIME, ExifTagType::Ascii),
    TagInfo(TIFF_COPYRIGHT, ExifTagType::Utf8),
    TagInfo(EXIF_EXIFIFD, ExifTagType::Long),
    TagInfo(EXIF_GPSIFD, ExifTagType::Long),
    TagInfo(EXIF_DATETIMEORIGINAL, ExifTagType::Ascii),
    TagInfo(EXIF_OFFSETTIMEDIGITIZED, ExifTagType::Ascii),
    TagInfo(EXIF_OFFSETTIME, ExifTagType::Ascii),
    TagInfo(EXIF_OFFSETTIMEORIGINAL, ExifTagType::Ascii),
    TagInfo(EXIF_OFFSETTIMEDIGITIZED, ExifTagType::Ascii),
    TagInfo(EXIF_COLORSPACE, ExifTagType::Short),
    TagInfo(EXIF_PIXELXDIM, ExifTagType::Long),
    TagInfo(EXIF_PIXELYDIM, ExifTagType::Long),
    TagInfo(EXIF_IMAGEUNIQUEID, ExifTagType::Ascii),
    TagInfo(EXIF_BODYSERIALNUMBER, ExifTagType::Ascii),
    TagInfo(EXIF_LENSMAKE, ExifTagType::Utf8),
    TagInfo(EXIF_LENSMODEL, ExifTagType::Utf8),
    TagInfo(EXIF_LENSSERIALNUMBER, ExifTagType::Ascii),
    TagInfo(EXIF_IMAGETITLE, ExifTagType::Utf8),
    TagInfo(EXIF_EXIFVERSION, ExifTagType::Undefined)
};
// clang-format on

/*!
 * \brief staticGpsTagTypes
 */
// clang-format off
static const KnownTags staticGpsTagTypes = {
    TagInfo(GPS_GPSVERSION, ExifTagType::Byte),
    TagInfo(GPS_LATITUDEREF, ExifTagType::Ascii),
    TagInfo(GPS_LATITUDE, ExifTagType::Rational),
    TagInfo(GPS_LONGITUDEREF, ExifTagType::Ascii),
    TagInfo(GPS_LONGITUDE, ExifTagType::Rational),
    TagInfo(GPS_ALTITUDEREF, ExifTagType::Byte),
    TagInfo(GPS_ALTITUDE, ExifTagType::Rational),
    TagInfo(GPS_IMGDIRECTIONREF, ExifTagType::Ascii),
    TagInfo(GPS_IMGDIRECTION, ExifTagType::Rational)
};
// clang-format on

/*!
 * \brief tiffStrMap
 * TIFF string <-> metadata
 */
// clang-format off
static const QList<std::pair<quint16, QString>> tiffStrMap = {
    std::pair<quint16, QString>(TIFF_IMAGEDESCRIPTION, QStringLiteral(META_KEY_DESCRIPTION)),
    std::pair<quint16, QString>(TIFF_ARTIST, QStringLiteral(META_KEY_AUTHOR)),
    std::pair<quint16, QString>(TIFF_SOFTWARE, QStringLiteral(META_KEY_SOFTWARE)),
    std::pair<quint16, QString>(TIFF_COPYRIGHT, QStringLiteral(META_KEY_COPYRIGHT)),
    std::pair<quint16, QString>(TIFF_MAKE, QStringLiteral(META_KEY_MANUFACTURER)),
    std::pair<quint16, QString>(TIFF_MODEL, QStringLiteral(META_KEY_MODEL))
};
// clang-format on

/*!
 * \brief exifStrMap
 * EXIF string <-> metadata
 */
// clang-format off
static const QList<std::pair<quint16, QString>> exifStrMap = {
    std::pair<quint16, QString>(EXIF_BODYSERIALNUMBER, QStringLiteral(META_KEY_SERIALNUMBER)),
    std::pair<quint16, QString>(EXIF_LENSMAKE, QStringLiteral(META_KEY_LENS_MANUFACTURER)),
    std::pair<quint16, QString>(EXIF_LENSMODEL, QStringLiteral(META_KEY_LENS_MODEL)),
    std::pair<quint16, QString>(EXIF_LENSSERIALNUMBER, QStringLiteral(META_KEY_LENS_SERIALNUMBER)),
    std::pair<quint16, QString>(EXIF_IMAGETITLE, QStringLiteral(META_KEY_TITLE)),
};
// clang-format on

/*!
 * \brief timeOffset
 * \param offset The EXIF string of the offset from UTC.
 * \return The offset in minutes.
 */
static qint16 timeOffset(const QString& offset)
{
    if (offset.size() != 6 || offset.at(3) != u':')
        return 0;
    auto ok = false;
    auto hh = offset.left(3).toInt(&ok);
    if (!ok)
        return 0;
    auto mm = offset.mid(4, 2).toInt(&ok) * (hh < 0 ? -1 : 1);
    if (!ok)
        return 0;
    return qint16(hh * 60 + mm);
}

/*!
 * \brief timeOffset
 * \param offset Offset from UTC in minutes.
 * \return The EXIF string of the offset.
 */
static QString timeOffset(qint16 offset)
{
    auto absOff = quint16(std::abs(offset));
    return QStringLiteral("%1%2:%3")
        .arg(offset < 0 ? QStringLiteral("-") : QStringLiteral("+"))
        .arg(absOff / 60, 2, 10, QChar(u'0'))
        .arg(absOff % 60, 2, 10, QChar(u'0'));
}


/*!
 * \brief checkHeader
 * \param ds The data stream
 * \return True if header is a valid EXIF, otherwise false.
 */
static bool checkHeader(QDataStream &ds)
{
    quint16 order;
    ds >> order;
    if (order == 0x4949) {
        ds.setByteOrder(QDataStream::LittleEndian);
    } else if (order == 0x4d4d) {
        ds.setByteOrder(QDataStream::BigEndian);
    } else {
        return false;
    }

    quint16 version;
    ds >> version;
    if (version != 0x002A && version != 0x01BC)
        return false; // not TIFF or JXR

    quint32 offset;
    ds >> offset;
    offset -= 8;
    if (ds.skipRawData(offset) != offset)
        return false;

    return ds.status() == QDataStream::Ok;
}

/*!
 * \brief updatePos
 * Write the current stram position in \a pos position as uint32.
 * \return True on success, otherwise false;
 */
static bool updatePos(QDataStream &ds, quint32 pos)
{
    auto dev = ds.device();
    if (pos != 0) {
        auto p = dev->pos();
        if (!dev->seek(pos))
            return false;
        ds << quint32(p);
        if (!dev->seek(p))
            return false;
    }
    return ds.status() == QDataStream::Ok;
}

static qint32 countBytes(const ExifTagType &dataType, const QVariant &value)
{
    auto count = 1;
    if (dataType == ExifTagType::Ascii) {
        count = value.toString().toLatin1().size() + 1; // ASCIIZ
    } else if (dataType == ExifTagType::Utf8) {
        count = value.toString().toUtf8().size() + 1; // ASCIIZ
    } else if (dataType == ExifTagType::Undefined) {
        count = value.toByteArray().size();
    } else if (dataType == ExifTagType::Byte) {
        count = value.value<QList<quint8>>().size();
    } else if (dataType == ExifTagType::Short) {
        count = value.value<QList<quint16>>().size();
    } else if (dataType == ExifTagType::Long || dataType == ExifTagType::Ifd) {
        count = value.value<QList<quint32>>().size();
    } else if (dataType == ExifTagType::SByte) {
        count = value.value<QList<qint8>>().size();
    } else if (dataType == ExifTagType::SShort) {
        count = value.value<QList<qint16>>().size();
    } else if (dataType == ExifTagType::SLong) {
        count = value.value<QList<qint32>>().size();
    } else if (dataType == ExifTagType::Rational || dataType == ExifTagType::SRational || dataType == ExifTagType::Double) {
        count = value.value<QList<double>>().size();
    } else if (dataType == ExifTagType::Float) {
        count = value.value<QList<float>>().size();
    }
    return std::max(1, count);
}

template <class T>
static void writeList(QDataStream &ds, const QVariant &value)
{
    auto l = value.value<QList<T>>();
    if (l.isEmpty())
        l.append(value.toInt());
    for (;l.size() < qsizetype(4 / sizeof(T));)
        l.append(T());
    for (auto &&v : l)
        ds << v;
}

inline qint32 rationalPrecision(double v)
{
    v = qAbs(v);
    return 8 - qBound(0, v < 1 ? 8 : int(std::log10(v)), 8);
}

template<class T>
static void writeRationalList(QDataStream &ds, const QVariant &value)
{
    auto l = value.value<QList<double>>();
    if (l.isEmpty())
        l.append(value.toDouble());
    for (auto &&v : l) {
        auto den = std::pow(10, rationalPrecision(v));
        ds << T(qRound(v * den));
        ds << T(den);
    }
}

static void writeByteArray(QDataStream &ds, const QByteArray &ba)
{
    for (auto &&v : ba)
        ds << v;
    for (auto n = ba.size(); n < 4; ++n)
        ds << char();
}

static void writeData(QDataStream &ds, const QVariant &value, const ExifTagType& dataType)
{
    if (dataType == ExifTagType::Ascii) {
        writeByteArray(ds, value.toString().toLatin1().append(char()));
    } else if (dataType == ExifTagType::Utf8) {
        writeByteArray(ds, value.toString().toUtf8().append(char()));
    } else if (dataType == ExifTagType::Undefined) {
        writeByteArray(ds, value.toByteArray());
    } else if (dataType == ExifTagType::Byte) {
        writeList<quint8>(ds, value);
    } else if (dataType == ExifTagType::SByte) {
        writeList<qint8>(ds, value);
    } else if (dataType == ExifTagType::Short) {
        writeList<quint16>(ds, value);
    } else if (dataType == ExifTagType::SShort) {
        writeList<qint16>(ds, value);
    } else if (dataType == ExifTagType::Long || dataType == ExifTagType::Ifd) {
        writeList<quint32>(ds, value);
    } else if (dataType == ExifTagType::SLong) {
        writeList<qint32>(ds, value);
    } else if (dataType == ExifTagType::Rational) {
        writeRationalList<quint32>(ds, value);
    } else if (dataType == ExifTagType::SRational) {
        writeRationalList<qint32>(ds, value);
    }
}

static ExifTagType updateDataType(const ExifTagType &dataType, const QVariant &value, const MicroExif::Version &ver)
{
    if (dataType != ExifTagType::Utf8)
        return dataType;

    if (ver == MicroExif::V2)
        return ExifTagType::Ascii;

    // Note that in EXIF specs, UTF-8 is backward compatible with ASCII: all UTF-8 tags can also be ASCII.
    // To maximize compatibility, I check if the string can be encoded in ASCII.
    auto txt = value.toString();

    // Exif ASCII data type allow only values up to 127 (7-bit ASCII).
    auto u8 = txt.toUtf8();
    for (auto &&c : u8) {
        if (uchar(c) > 127)
            return dataType;
    }

    return ExifTagType::Ascii;
}

/*!
 * \brief writeIfd
 * \param ds The stream.
 * \param tags The list of tags to write.
 * \param pos The position of the TAG value to update with this IFD position.
 * \param knownTags List of known and supported tags.
 * \return True on success, otherwise false.
 */
static bool writeIfd(QDataStream &ds,
                     const MicroExif::Version &ver,
                     const MicroExif::Tags &tags,
                     TagPos &positions,
                     quint32 pos = 0,
                     const KnownTags &knownTags = staticTagTypes)
{
    if (tags.isEmpty())
        return true;
    if (!updatePos(ds, pos))
        return false;

    auto keys = tags.keys();
    auto entries = quint16(keys.size());
    ds << entries;
    for (auto &&key : keys) {
        if (!knownTags.contains(key)) {
            continue;
        }
        auto value = tags.value(key);
        auto dataType = updateDataType(knownTags.value(key), value, ver);
        auto count = countBytes(dataType, value);

        ds << quint16(key);
        ds << quint16(EXIF_TAG_DATATYPE(dataType));
        ds << quint32(count);
        positions.insert(key, quint32(ds.device()->pos()));
        auto valueSize = count * EXIF_TAG_SIZEOF(dataType);
        if (valueSize > 4) {
            ds << quint32();
        } else {
            writeData(ds, value, dataType);
        }
    }
    // no more IFDs
    ds << quint32();

    // write data larger than 4 bytes
    for (auto &&key : keys) {
        if (!knownTags.contains(key)) {
            continue;
        }
        auto value = tags.value(key);
        auto dataType = updateDataType(knownTags.value(key), value, ver);
        auto count = countBytes(dataType, value);
        auto valueSize = count * EXIF_TAG_SIZEOF(dataType);
        if (valueSize <= 4)
            continue;
        if (!updatePos(ds, positions.value(key)))
            return false;
        writeData(ds, value, dataType);
    }

    return ds.status() ==  QDataStream::Ok;
}

template<class T>
static QList<T> readList(QDataStream &ds, quint32 count)
{
    QList<T> l;
    T c;
    for (quint32 i = 0; i < count; ++i) {
        ds >> c;
        l.append(c);
    }
    for (auto n = count; n < quint32(4 / sizeof(T)); ++n) {
        ds >> c;
    }
    return l;
}

template<class T>
static QList<double> readRationalList(QDataStream &ds, quint32 count)
{
    QList<double> l;
    for (quint32 i = 0; i < count; ++i) {
        T num;
        ds >> num;
        T den;
        ds >> den;
        l.append(den == 0 ? 0 : double(num) / double(den));
    }
    return l;
}

static QByteArray readBytes(QDataStream &ds, quint32 count, bool asciiz)
{
    QByteArray l;
    if (count == 0) {
        return l;
    }
    char c;
    for (quint32 i = 0; i < count; ++i) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        ds >> c;
#else
        ds >> reinterpret_cast<qint8&>(c);
#endif
        l.append(c);
    }
    if (asciiz && l.at(l.size() - 1) == 0) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        l.removeLast();
#else
        if(!l.isEmpty())
            l.remove(l.size() - 1, 1);
#endif
    }
    for (auto n = count; n < 4; ++n) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        ds >> c;
#else
        ds >> reinterpret_cast<qint8&>(c);
#endif
    }
    return l;
}

/*!
 * \brief readIfd
 * \param ds The stream.
 * \param tags Where to sotro the read tags.
 * \param pos The position of the IFD.
 * \param knownTags List of known and supported tags.
 * \param nextIfd The position of next IFD (0 if none).
 * \return True on succes, otherwise false.
 */
static bool readIfd(QDataStream &ds, MicroExif::Tags &tags, quint32 pos = 0, const KnownTags &knownTags = staticTagTypes, quint32 *nextIfd = nullptr)
{
    auto localNextIfd = quint32();
    if (nextIfd == nullptr)
        nextIfd = &localNextIfd;
    *nextIfd = 0;

    auto device = ds.device();
    if (pos && !device->seek(pos))
        return false;

    quint16 entries;
    ds >> entries;
    if (ds.status() != QDataStream::Ok)
        return false;

    for (quint16 i = 0; i < entries; ++i) {
        quint16 tagId;
        ds >> tagId;
        quint16 dataType;
        ds >> dataType;
        quint32 count;
        ds >> count;
        if (ds.status() != QDataStream::Ok)
            return false;

        // search for supported values only
        if (!knownTags.contains(tagId)) {
            quint32 value;
            ds >> value;
            continue;
        }

        // read TAG data
        auto toRead = qint64(count) * EXIF_TAG_SIZEOF(knownTags.value(tagId));
        if (toRead > qint64(device->size()))
            return false;

        auto curPos = qint64();
        if (toRead > 4) {
            quint32 value;
            ds >> value;
            curPos = device->pos();
            if (!device->seek(value))
                return false;
        }

        if (dataType == EXIF_TAG_DATATYPE(ExifTagType::Ascii) || dataType == EXIF_TAG_DATATYPE(ExifTagType::Utf8)) {
            auto l = readBytes(ds, count, true);
            if (!l.isEmpty()) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                // It seems that converting to Latin 1 never detects errors so, using UTF-8.
                // Note that if the dataType is ASCII, by EXIF specification, it must use only the
                // first 128 values ​​so the UTF-8 conversion is correct.
                auto dec = QStringDecoder(QStringDecoder::Utf8);
                // QStringDecoder raise an error only after converting to QString
                auto ut8 = QString(dec(l));
                // If there are errors in the conversion to UTF-8, then I try with latin1 (extended ASCII)
                tags.insert(tagId, dec.hasError() ? QString::fromLatin1(l) : ut8);
#else
                auto ut8 = QString::fromUtf8(l);
                tags.insert(tagId, ut8.isEmpty() ? QString::fromLatin1(l) : ut8);
#endif
            }
        } else if (dataType == EXIF_TAG_DATATYPE(ExifTagType::Undefined)) {
            auto l = readBytes(ds, count, false);
            if (!l.isEmpty())
                tags.insert(tagId, l);
        } else if (dataType == EXIF_TAG_DATATYPE(ExifTagType::Byte)) {
            auto l = readList<quint8>(ds, count);
            tags.insert(tagId, l.size() == 1 ? QVariant(l.first()) : QVariant::fromValue(l));
        } else if (dataType == EXIF_TAG_DATATYPE(ExifTagType::SByte)) {
            auto l = readList<qint8>(ds, count);
            tags.insert(tagId, l.size() == 1 ? QVariant(l.first()) : QVariant::fromValue(l));
        } else if (dataType == EXIF_TAG_DATATYPE(ExifTagType::Short)) {
            auto l = readList<quint16>(ds, count);
            tags.insert(tagId, l.size() == 1 ? QVariant(l.first()) : QVariant::fromValue(l));
        } else if (dataType == EXIF_TAG_DATATYPE(ExifTagType::SShort)) {
            auto l = readList<qint16>(ds, count);
            tags.insert(tagId, l.size() == 1 ? QVariant(l.first()) : QVariant::fromValue(l));
        } else if (dataType == EXIF_TAG_DATATYPE(ExifTagType::Long) || dataType == EXIF_TAG_DATATYPE(ExifTagType::Ifd)) {
            auto l = readList<quint32>(ds, count);
            tags.insert(tagId, l.size() == 1 ? QVariant(l.first()) : QVariant::fromValue(l));
        } else if (dataType == EXIF_TAG_DATATYPE(ExifTagType::SLong)) {
            auto l = readList<qint32>(ds, count);
            tags.insert(tagId, l.size() == 1 ? QVariant(l.first()) : QVariant::fromValue(l));
        } else if (dataType == EXIF_TAG_DATATYPE(ExifTagType::Rational)) {
            auto l = readRationalList<quint32>(ds, count);
            tags.insert(tagId, l.size() == 1 ? QVariant(l.first()) : QVariant::fromValue(l));
        } else if (dataType == EXIF_TAG_DATATYPE(ExifTagType::SRational)) {
            auto l = readRationalList<qint32>(ds, count);
            tags.insert(tagId, l.size() == 1 ? QVariant(l.first()) : QVariant::fromValue(l));
        }

        if (curPos > 0 && !device->seek(curPos))
            return false;
    }
    ds >> *nextIfd;

    return true;
}

MicroExif::MicroExif()
{

}

void MicroExif::clear()
{
    m_tiffTags.clear();
    m_exifTags.clear();
    m_gpsTags.clear();
}

bool MicroExif::isEmpty() const
{
    return m_tiffTags.isEmpty() && m_exifTags.isEmpty() && m_gpsTags.isEmpty();
}

double MicroExif::horizontalResolution() const
{
    auto u = m_tiffTags.value(TIFF_URES).toUInt();
    auto v = m_tiffTags.value(TIFF_XRES).toDouble();
    if (u == TIFF_VAL_URES_CENTIMETER)
        return v * 2.54;
    return v;
}

void MicroExif::setHorizontalResolution(double hres)
{
    auto u = m_tiffTags.value(TIFF_URES).toUInt();
    if (u == TIFF_VAL_URES_CENTIMETER) {
        hres /= 2.54;
    } else if (u < TIFF_VAL_URES_INCH) {
        m_tiffTags.insert(TIFF_URES, TIFF_VAL_URES_INCH);
    }
    m_tiffTags.insert(TIFF_XRES, hres);
}

double MicroExif::verticalResolution() const
{
    auto u = m_tiffTags.value(TIFF_URES).toUInt();
    auto v = m_tiffTags.value(TIFF_YRES).toDouble();
    if (u == TIFF_VAL_URES_CENTIMETER)
        return v * 2.54;
    return v;
}

void MicroExif::setVerticalResolution(double vres)
{
    auto u = m_tiffTags.value(TIFF_URES).toUInt();
    if (u == TIFF_VAL_URES_CENTIMETER) {
        vres /= 2.54;
    } else if (u < TIFF_VAL_URES_INCH) {
        m_tiffTags.insert(TIFF_URES, TIFF_VAL_URES_INCH);
    }
    m_tiffTags.insert(TIFF_YRES, vres);
}

QColorSpace MicroExif::colosSpace() const
{
    if (m_exifTags.value(EXIF_COLORSPACE).toUInt() == EXIF_VAL_COLORSPACE_SRGB)
        return QColorSpace(QColorSpace::SRgb);
    return QColorSpace();
}

void MicroExif::setColorSpace(const QColorSpace &cs)
{
    auto srgb = cs.transferFunction() == QColorSpace::TransferFunction::SRgb && cs.primaries() == QColorSpace::Primaries::SRgb;
    m_exifTags.insert(EXIF_COLORSPACE, srgb ? EXIF_VAL_COLORSPACE_SRGB : EXIF_VAL_COLORSPACE_UNCAL);
}

void MicroExif::setColorSpace(const QColorSpace::NamedColorSpace &csName)
{
    auto srgb = csName == QColorSpace::SRgb;
    m_exifTags.insert(EXIF_COLORSPACE, srgb ? EXIF_VAL_COLORSPACE_SRGB : EXIF_VAL_COLORSPACE_UNCAL);
}

qint32 MicroExif::width() const
{
    return m_tiffTags.value(TIFF_IMAGEWIDTH).toUInt();
}

void MicroExif::setWidth(qint32 w)
{
    m_tiffTags.insert(TIFF_IMAGEWIDTH, w);
    m_exifTags.insert(EXIF_PIXELXDIM, w);
}

qint32 MicroExif::height() const
{
    return m_tiffTags.value(TIFF_IMAGEHEIGHT).toUInt();
}

void MicroExif::setHeight(qint32 h)
{
    m_tiffTags.insert(TIFF_IMAGEHEIGHT, h);
    m_exifTags.insert(EXIF_PIXELYDIM, h);
}

quint16 MicroExif::orientation() const
{
    return m_tiffTags.value(TIFF_ORIENT).toUInt();
}

void MicroExif::setOrientation(quint16 orient)
{
    if (orient < 1 || orient > 8)
        m_tiffTags.remove(TIFF_ORIENT);
    else
        m_tiffTags.insert(TIFF_ORIENT, orient);
}

QImageIOHandler::Transformation MicroExif::transformation() const
{
    switch (orientation()) {
    case 1:
        return QImageIOHandler::TransformationNone;
    case 2:
        return QImageIOHandler::TransformationMirror;
    case 3:
        return QImageIOHandler::TransformationRotate180;
    case 4:
        return QImageIOHandler::TransformationFlip;
    case 5:
        return QImageIOHandler::TransformationFlipAndRotate90;
    case 6:
        return QImageIOHandler::TransformationRotate90;
    case 7:
        return QImageIOHandler::TransformationMirrorAndRotate90;
    case 8:
        return QImageIOHandler::TransformationRotate270;
    default:
        break;
    };
    return QImageIOHandler::TransformationNone;
}

void MicroExif::setTransformation(const QImageIOHandler::Transformation &t)
{
    switch (t) {
    case QImageIOHandler::TransformationNone:
        setOrientation(1);
        break;
    case QImageIOHandler::TransformationMirror:
        setOrientation(2);
        break;
    case QImageIOHandler::TransformationRotate180:
        setOrientation(3);
        break;
    case QImageIOHandler::TransformationFlip:
        setOrientation(4);
        break;
    case QImageIOHandler::TransformationFlipAndRotate90:
        setOrientation(5);
        break;
    case QImageIOHandler::TransformationRotate90:
        setOrientation(6);
        break;
    case QImageIOHandler::TransformationMirrorAndRotate90:
        setOrientation(7);
        break;
    case QImageIOHandler::TransformationRotate270:
        setOrientation(8);
        break;
    default:
        break;
    }
    setOrientation(0); // no orientation set
}

QString MicroExif::software() const
{
    return tiffString(TIFF_SOFTWARE);
}

void MicroExif::setSoftware(const QString &s)
{
    setTiffString(TIFF_SOFTWARE, s);
}

QString MicroExif::description() const
{
    return tiffString(TIFF_IMAGEDESCRIPTION);
}

void MicroExif::setDescription(const QString &s)
{
    setTiffString(TIFF_IMAGEDESCRIPTION, s);
}

QString MicroExif::artist() const
{
    return tiffString(TIFF_ARTIST);
}

void MicroExif::setArtist(const QString &s)
{
    setTiffString(TIFF_ARTIST, s);
}

QString MicroExif::copyright() const
{
    return tiffString(TIFF_COPYRIGHT);
}

void MicroExif::setCopyright(const QString &s)
{
    setTiffString(TIFF_COPYRIGHT, s);
}

QString MicroExif::make() const
{
    return tiffString(TIFF_MAKE);
}

void MicroExif::setMake(const QString &s)
{
    setTiffString(TIFF_MAKE, s);
}

QString MicroExif::model() const
{
    return tiffString(TIFF_MODEL);
}

void MicroExif::setModel(const QString &s)
{
    setTiffString(TIFF_MODEL, s);
}

QString MicroExif::serialNumber() const
{
    return tiffString(EXIF_BODYSERIALNUMBER);
}

void MicroExif::setSerialNumber(const QString &s)
{
    setTiffString(EXIF_BODYSERIALNUMBER, s);
}

QString MicroExif::lensMake() const
{
    return tiffString(EXIF_LENSMAKE);
}

void MicroExif::setLensMake(const QString &s)
{
    setTiffString(EXIF_LENSMAKE, s);
}

QString MicroExif::lensModel() const
{
    return tiffString(EXIF_LENSMODEL);
}

void MicroExif::setLensModel(const QString &s)
{
    setTiffString(EXIF_LENSMODEL, s);
}

QString MicroExif::lensSerialNumber() const
{
    return tiffString(EXIF_LENSSERIALNUMBER);
}

void MicroExif::setLensSerialNumber(const QString &s)
{
    setTiffString(EXIF_LENSSERIALNUMBER, s);
}

QDateTime MicroExif::dateTime() const
{
    auto dt = QDateTime::fromString(tiffString(TIFF_DATETIME), QStringLiteral("yyyy:MM:dd HH:mm:ss"));
    auto ofTag = exifString(EXIF_OFFSETTIME);
    if (dt.isValid() && !ofTag.isEmpty())
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        dt.setTimeZone(QTimeZone::fromSecondsAheadOfUtc(timeOffset(ofTag) * 60));
#else
        dt.setTimeZone(QTimeZone(static_cast<int>(timeOffset(ofTag) * 60)));
#endif
    return(dt);
}

void MicroExif::setDateTime(const QDateTime &dt)
{
    if (!dt.isValid()) {
        m_tiffTags.remove(TIFF_DATETIME);
        m_exifTags.remove(EXIF_OFFSETTIME);
        return;
    }
    setTiffString(TIFF_DATETIME, dt.toString(QStringLiteral("yyyy:MM:dd HH:mm:ss")));
    setExifString(EXIF_OFFSETTIME, timeOffset(dt.offsetFromUtc() / 60));
}

QDateTime MicroExif::dateTimeOriginal() const
{
    auto dt = QDateTime::fromString(exifString(EXIF_DATETIMEORIGINAL), QStringLiteral("yyyy:MM:dd HH:mm:ss"));
    auto ofTag = exifString(EXIF_OFFSETTIMEORIGINAL);
    if (dt.isValid() && !ofTag.isEmpty())
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        dt.setTimeZone(QTimeZone::fromSecondsAheadOfUtc(timeOffset(ofTag) * 60));
#else
        dt.setTimeZone(QTimeZone(static_cast<int>(timeOffset(ofTag) * 60)));
#endif
    return(dt);
}

void MicroExif::setDateTimeOriginal(const QDateTime &dt)
{
    if (!dt.isValid()) {
        m_exifTags.remove(EXIF_DATETIMEORIGINAL);
        m_exifTags.remove(EXIF_OFFSETTIMEORIGINAL);
        return;
    }
    setExifString(EXIF_DATETIMEORIGINAL, dt.toString(QStringLiteral("yyyy:MM:dd HH:mm:ss")));
    setExifString(EXIF_OFFSETTIMEORIGINAL, timeOffset(dt.offsetFromUtc() / 60));
}

QDateTime MicroExif::dateTimeDigitized() const
{
    auto dt = QDateTime::fromString(exifString(EXIF_DATETIMEDIGITIZED), QStringLiteral("yyyy:MM:dd HH:mm:ss"));
    auto ofTag = exifString(EXIF_OFFSETTIMEDIGITIZED);
    if (dt.isValid() && !ofTag.isEmpty())
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        dt.setTimeZone(QTimeZone::fromSecondsAheadOfUtc(timeOffset(ofTag) * 60));
#else
        dt.setTimeZone(QTimeZone(static_cast<int>(timeOffset(ofTag) * 60)));
#endif
    return(dt);
}

void MicroExif::setDateTimeDigitized(const QDateTime &dt)
{
    if (!dt.isValid()) {
        m_exifTags.remove(EXIF_DATETIMEDIGITIZED);
        m_exifTags.remove(EXIF_OFFSETTIMEDIGITIZED);
        return;
    }
    setExifString(EXIF_DATETIMEDIGITIZED, dt.toString(QStringLiteral("yyyy:MM:dd HH:mm:ss")));
    setExifString(EXIF_OFFSETTIMEDIGITIZED, timeOffset(dt.offsetFromUtc() / 60));
}

QString MicroExif::title() const
{
    return exifString(EXIF_IMAGETITLE);
}

void MicroExif::setImageTitle(const QString &s)
{
    setExifString(EXIF_IMAGETITLE, s);
}

QUuid MicroExif::uniqueId() const
{
    auto s = exifString(EXIF_IMAGEUNIQUEID);
    if (s.length() == 32) {
        auto tmp = QStringLiteral("%1-%2-%3-%4-%5").arg(s.left(8), s.mid(8, 4), s.mid(12, 4), s.mid(16, 4), s.mid(20));
        return QUuid(tmp);
    }
    return {};
}

void MicroExif::setUniqueId(const QUuid &uuid)
{
    if (uuid.isNull())
        setExifString(EXIF_IMAGEUNIQUEID, QString());
    else
        setExifString(EXIF_IMAGEUNIQUEID, uuid.toString(QUuid::WithoutBraces).replace(QStringLiteral("-"), QString()));
}

double MicroExif::latitude() const
{
    auto ref = gpsString(GPS_LATITUDEREF).toUpper();
    if (ref != QStringLiteral("N") && ref != QStringLiteral("S"))
        return qQNaN();
    auto lat = m_gpsTags.value(GPS_LATITUDE).value<QList<double>>();
    if (lat.size() != 3)
        return qQNaN();
    auto degree = lat.at(0) + lat.at(1) / 60 + lat.at(2) / 3600;
    if (degree < -90.0 || degree > 90.0)
        return qQNaN();
    return ref == QStringLiteral("N") ? degree : -degree;
}

void MicroExif::setLatitude(double degree)
{
    if (qIsNaN(degree)) {
        m_gpsTags.remove(GPS_LATITUDEREF);
        m_gpsTags.remove(GPS_LATITUDE);
    }
    if (degree < -90.0 || degree > 90.0)
        return; // invalid latitude
    auto adeg = qAbs(degree);
    auto min = (adeg - int(adeg)) * 60;
    auto sec = (min - int(min)) * 60;
    m_gpsTags.insert(GPS_LATITUDEREF, degree < 0 ? QStringLiteral("S") : QStringLiteral("N"));
    m_gpsTags.insert(GPS_LATITUDE, QVariant::fromValue(QList<double>() << int(adeg) << int(min) << sec));
}

double MicroExif::longitude() const
{
    auto ref = gpsString(GPS_LONGITUDEREF).toUpper();
    if (ref != QStringLiteral("E") && ref != QStringLiteral("W"))
        return qQNaN();
    auto lon = m_gpsTags.value(GPS_LONGITUDE).value<QList<double>>();
    if (lon.size() != 3)
        return qQNaN();
    auto degree = lon.at(0) + lon.at(1) / 60 + lon.at(2) / 3600;
    if (degree < -180.0 || degree > 180.0)
        return qQNaN();
    return ref == QStringLiteral("E") ? degree : -degree;
}

void MicroExif::setLongitude(double degree)
{
    if (qIsNaN(degree)) {
        m_gpsTags.remove(GPS_LONGITUDEREF);
        m_gpsTags.remove(GPS_LONGITUDE);
    }
    if (degree < -180.0 || degree > 180.0)
        return; // invalid longitude
    auto adeg = qAbs(degree);
    auto min = (adeg - int(adeg)) * 60;
    auto sec = (min - int(min)) * 60;
    m_gpsTags.insert(GPS_LONGITUDEREF, degree < 0 ? QStringLiteral("W") : QStringLiteral("E"));
    m_gpsTags.insert(GPS_LONGITUDE, QVariant::fromValue(QList<double>() << int(adeg) << int(min) << sec));
}

double MicroExif::altitude() const
{
    auto ref = m_gpsTags.value(GPS_ALTITUDEREF);
    if (ref.isNull())
        return qQNaN();
    if (!m_gpsTags.contains(GPS_ALTITUDE))
        return qQNaN();
    auto alt = m_gpsTags.value(GPS_ALTITUDE).toDouble();
    return (ref.toInt() == 0 || ref.toInt() == 2) ? alt : -alt;
}

void MicroExif::setAltitude(double meters)
{
    if (qIsNaN(meters)) {
        m_gpsTags.remove(GPS_ALTITUDEREF);
        m_gpsTags.remove(GPS_ALTITUDE);
    }
    m_gpsTags.insert(GPS_ALTITUDEREF, quint8(meters < 0 ? 1 : 0));
    m_gpsTags.insert(GPS_ALTITUDE, meters);
}

double MicroExif::imageDirection(bool *isMagnetic) const
{
    auto tmp = false;
    if (isMagnetic == nullptr)
        isMagnetic = &tmp;
    if (!m_gpsTags.contains(GPS_IMGDIRECTION))
        return qQNaN();
    auto ref = gpsString(GPS_IMGDIRECTIONREF).toUpper();
    *isMagnetic = (ref == QStringLiteral("M"));
    return m_gpsTags.value(GPS_IMGDIRECTION).toDouble();
}

void MicroExif::setImageDirection(double degree, bool isMagnetic)
{
    if (qIsNaN(degree)) {
        m_gpsTags.remove(GPS_IMGDIRECTIONREF);
        m_gpsTags.remove(GPS_IMGDIRECTION);
    }
    m_gpsTags.insert(GPS_IMGDIRECTIONREF, isMagnetic ? QStringLiteral("M") : QStringLiteral("T"));
    m_gpsTags.insert(GPS_IMGDIRECTION, degree);
}

QByteArray MicroExif::toByteArray(const QDataStream::ByteOrder &byteOrder, const Version &version) const
{
    QByteArray ba;
    {
        QBuffer buf(&ba);
        if (!write(&buf, byteOrder, version))
            return {};
    }
    return ba;
}

QByteArray MicroExif::exifIfdByteArray(const QDataStream::ByteOrder &byteOrder, const Version &version) const
{
    QByteArray ba;
    {
        QDataStream ds(&ba, QIODevice::WriteOnly);
        ds.setByteOrder(byteOrder);
        auto exifTags = m_exifTags;
        exifTags.insert(EXIF_EXIFVERSION, version == Version::V3 ? QByteArray("0300") : QByteArray("0232"));
        TagPos positions;
        if (!writeIfd(ds, version, exifTags, positions))
            return {};
    }
    return ba;
}

bool MicroExif::setExifIfdByteArray(const QByteArray &ba, const QDataStream::ByteOrder &byteOrder)
{
    QDataStream ds(ba);
    ds.setByteOrder(byteOrder);
    return readIfd(ds, m_exifTags, 0, staticTagTypes);
}

QByteArray MicroExif::gpsIfdByteArray(const QDataStream::ByteOrder &byteOrder, const Version &version) const
{
    QByteArray ba;
    {
        QDataStream ds(&ba, QIODevice::WriteOnly);
        ds.setByteOrder(byteOrder);
        auto gpsTags = m_gpsTags;
        gpsTags.insert(GPS_GPSVERSION, QByteArray("2400"));
        TagPos positions;
        if (!writeIfd(ds, version, gpsTags, positions, 0, staticGpsTagTypes))
            return {};
        return ba;
    }
}

bool MicroExif::setGpsIfdByteArray(const QByteArray &ba, const QDataStream::ByteOrder &byteOrder)
{
    QDataStream ds(ba);
    ds.setByteOrder(byteOrder);
    return readIfd(ds, m_gpsTags, 0, staticGpsTagTypes);
}

bool MicroExif::write(QIODevice *device, const QDataStream::ByteOrder &byteOrder, const Version &version) const
{
    if (device == nullptr || device->isSequential() || isEmpty())
        return false;
    if (device->open(QBuffer::WriteOnly)) {
        QDataStream ds(device);
        ds.setByteOrder(byteOrder);
        if (!writeHeader(ds))
            return false;
        if (!writeIfds(ds, version))
            return false;
    }
    device->close();
    return true;
}

void MicroExif::updateImageMetadata(QImage &targetImage, bool replaceExisting) const
{
    // set TIFF strings
    for (auto &&p : tiffStrMap) {
        if (!replaceExisting && !targetImage.text(p.second).isEmpty())
            continue;
        auto s = tiffString(p.first);
        if (!s.isEmpty())
            targetImage.setText(p.second, s);
    }

    // set EXIF strings
    for (auto &&p : exifStrMap) {
        if (!replaceExisting && !targetImage.text(p.second).isEmpty())
            continue;
        auto s = exifString(p.first);
        if (!s.isEmpty())
            targetImage.setText(p.second, s);
    }

    // set date and time
    if (replaceExisting || targetImage.text(QStringLiteral(META_KEY_MODIFICATIONDATE)).isEmpty()) {
        auto dt = dateTime();
        if (dt.isValid())
            targetImage.setText(QStringLiteral(META_KEY_MODIFICATIONDATE), dt.toString(Qt::ISODate));
    }
    if (replaceExisting || targetImage.text(QStringLiteral(META_KEY_CREATIONDATE)).isEmpty()) {
        auto dt = dateTimeOriginal();
        if (dt.isValid())
            targetImage.setText(QStringLiteral(META_KEY_CREATIONDATE), dt.toString(Qt::ISODate));
    }

    // set GPS info
    if (replaceExisting || targetImage.text(QStringLiteral(META_KEY_ALTITUDE)).isEmpty()) {
        auto v = altitude();
        if (!qIsNaN(v))
            targetImage.setText(QStringLiteral(META_KEY_ALTITUDE), QStringLiteral("%1").arg(v, 0, 'g', 9));
    }
    if (replaceExisting || targetImage.text(QStringLiteral(META_KEY_LATITUDE)).isEmpty()) {
        auto v = latitude();
        if (!qIsNaN(v))
            targetImage.setText(QStringLiteral(META_KEY_LATITUDE), QStringLiteral("%1").arg(v, 0, 'g', 9));
    }
    if (replaceExisting || targetImage.text(QStringLiteral(META_KEY_LONGITUDE)).isEmpty()) {
        auto v = longitude();
        if (!qIsNaN(v))
            targetImage.setText(QStringLiteral(META_KEY_LONGITUDE), QStringLiteral("%1").arg(v, 0, 'g', 9));
    }
    if (replaceExisting || targetImage.text(QStringLiteral(META_KEY_DIRECTION)).isEmpty()) {
        auto v = imageDirection();
        if (!qIsNaN(v))
            targetImage.setText(QStringLiteral(META_KEY_DIRECTION), QStringLiteral("%1").arg(v, 0, 'g', 9));
    }
}

bool MicroExif::updateImageResolution(QImage &targetImage)
{
    if (horizontalResolution() > 0)
        targetImage.setDotsPerMeterX(qRound(horizontalResolution() / 25.4 * 1000));
    if (verticalResolution() > 0)
        targetImage.setDotsPerMeterY(qRound(verticalResolution() / 25.4 * 1000));
    return (horizontalResolution() > 0) || (verticalResolution() > 0);
}

MicroExif MicroExif::fromByteArray(const QByteArray &ba, bool searchHeader)
{
    auto ba0(ba);
    if (searchHeader) {
        auto idxLE = ba0.indexOf(QByteArray("II"));
        auto idxBE = ba0.indexOf(QByteArray("MM"));
        auto idx = -1;
        if (idxLE > -1 && idxBE > -1)
            idx = std::min(idxLE, idxBE);
        else
            idx = idxLE > -1 ? idxLE : idxBE;
        if(idx > 0)
            ba0 = ba0.mid(idx);
    }
    QBuffer buf;
    buf.setData(ba0);
    return fromDevice(&buf);
}

MicroExif MicroExif::fromRawData(const char *data, size_t size, bool searchHeader)
{
    if (data == nullptr || size == 0)
        return {};
    return fromByteArray(QByteArray::fromRawData(data, size), searchHeader);
}

MicroExif MicroExif::fromDevice(QIODevice *device)
{
    if (device == nullptr || device->isSequential())
        return {};
    if (!device->open(QBuffer::ReadOnly))
        return {};

    QDataStream ds(device);
    if (!checkHeader(ds))
        return {};

    MicroExif exif;

    // read TIFF ifd
    if (!readIfd(ds, exif.m_tiffTags))
        return {};

    // read EXIF ifd
    if (auto pos = exif.m_tiffTags.value(EXIF_EXIFIFD).toUInt()) {
        if (!readIfd(ds, exif.m_exifTags, pos))
            return {};
    }

    // read GPS ifd
    if (auto pos = exif.m_tiffTags.value(EXIF_GPSIFD).toUInt()) {
        if (!readIfd(ds, exif.m_gpsTags, pos, staticGpsTagTypes))
            return {};
    }

    return exif;
}

MicroExif MicroExif::fromImage(const QImage &image)
{
    MicroExif exif;
    if (image.isNull())
        return exif;

    // Image properties
    exif.setWidth(image.width());
    exif.setHeight(image.height());
    exif.setHorizontalResolution(image.dotsPerMeterX() * 25.4 / 1000);
    exif.setVerticalResolution(image.dotsPerMeterY() * 25.4 / 1000);
    exif.setColorSpace(image.colorSpace());

    // TIFF strings
    for (auto &&p : tiffStrMap) {
        exif.setTiffString(p.first, image.text(p.second));
    }

    // EXIF strings
    for (auto &&p : exifStrMap) {
        exif.setExifString(p.first, image.text(p.second));
    }

    // TIFF Software
    if (exif.software().isEmpty()) {
        auto sw = QCoreApplication::applicationName();
        auto ver = sw = QCoreApplication::applicationVersion();
        if (!sw.isEmpty() && !ver.isEmpty())
            sw.append(QStringLiteral(" %1").arg(ver));
        exif.setSoftware(sw.trimmed());
    }

    // TIFF date and time
    auto dt = QDateTime::fromString(image.text(QStringLiteral(META_KEY_MODIFICATIONDATE)), Qt::ISODate);
    if (!dt.isValid())
        dt = QDateTime::currentDateTime();
    exif.setDateTime(dt);

    // EXIF original date and time
    dt = QDateTime::fromString(image.text(QStringLiteral(META_KEY_CREATIONDATE)), Qt::ISODate);
    if (!dt.isValid())
        dt = QDateTime::currentDateTime();
    exif.setDateTimeOriginal(dt);

    // GPS Info
    auto ok = false;
    auto alt = image.text(QStringLiteral(META_KEY_ALTITUDE)).toDouble(&ok);
    if (ok)
        exif.setAltitude(alt);
    auto lat = image.text(QStringLiteral(META_KEY_LATITUDE)).toDouble(&ok);
    if (ok)
        exif.setLatitude(lat);
    auto lon = image.text(QStringLiteral(META_KEY_LONGITUDE)).toDouble(&ok);
    if (ok)
        exif.setLongitude(lon);
    auto dir = image.text(QStringLiteral(META_KEY_DIRECTION)).toDouble(&ok);
    if (ok)
        exif.setImageDirection(dir);

    return exif;
}

void MicroExif::setTiffString(quint16 tagId, const QString &s)
{
    MicroExif::setString(m_tiffTags, tagId, s);
}

QString MicroExif::tiffString(quint16 tagId) const
{
    return MicroExif::string(m_tiffTags, tagId);
}

void MicroExif::setExifString(quint16 tagId, const QString &s)
{
    MicroExif::setString(m_exifTags, tagId, s);
}

QString MicroExif::exifString(quint16 tagId) const
{
    return MicroExif::string(m_exifTags, tagId);
}

void MicroExif::setGpsString(quint16 tagId, const QString &s)
{
    MicroExif::setString(m_gpsTags, tagId, s);
}

QString MicroExif::gpsString(quint16 tagId) const
{
    return MicroExif::string(m_gpsTags, tagId);
}

bool MicroExif::writeHeader(QDataStream &ds) const
{
    if (ds.byteOrder() == QDataStream::LittleEndian)
        ds << quint16(0x4949); // II
    else
        ds << quint16(0x4d4d); // MM
    ds << quint16(0x002a); // Tiff V6
    ds << quint32(8); // IFD offset
    return ds.status() ==  QDataStream::Ok;
}

bool MicroExif::writeIfds(QDataStream &ds, const Version &version) const
{
    auto tiffTags = m_tiffTags;
    auto exifTags = m_exifTags;
    auto gpsTags = m_gpsTags;
    updateTags(tiffTags, exifTags, gpsTags, version);

    TagPos positions;
    if (!writeIfd(ds, version, tiffTags, positions))
        return false;
    if (!writeIfd(ds, version, exifTags, positions, positions.value(EXIF_EXIFIFD)))
        return false;
    if (!writeIfd(ds, version, gpsTags, positions, positions.value(EXIF_GPSIFD), staticGpsTagTypes))
        return false;
    return true;
}

void MicroExif::updateTags(Tags &tiffTags, Tags &exifTags, Tags &gpsTags, const Version &version) const
{
    if (exifTags.isEmpty()) {
        tiffTags.remove(EXIF_EXIFIFD);
    } else {
        tiffTags.insert(EXIF_EXIFIFD, quint32());
        exifTags.insert(EXIF_EXIFVERSION, version == Version::V3 ? QByteArray("0300") : QByteArray("0232"));
    }
    if (gpsTags.isEmpty()) {
        tiffTags.remove(EXIF_GPSIFD);
    } else {
        tiffTags.insert(EXIF_GPSIFD, quint32());
        gpsTags.insert(GPS_GPSVERSION, QByteArray("2400"));
    }
}

void MicroExif::setString(Tags &tags, quint16 tagId, const QString &s)
{
    if (s.isEmpty())
        tags.remove(tagId);
    else
        tags.insert(tagId, s);
}

QString MicroExif::string(const Tags &tags, quint16 tagId)
{
    return tags.value(tagId).toString();
}
