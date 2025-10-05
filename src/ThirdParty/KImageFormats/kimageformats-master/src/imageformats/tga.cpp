/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Dominik Seichter <domseichter@web.de>
    SPDX-FileCopyrightText: 2004 Ignacio Castaño <castano@ludicon.com>
    SPDX-FileCopyrightText: 2025 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/* this code supports:
 * reading:
 *     uncompressed and run length encoded indexed, grey and color tga files.
 *     image types 1, 2, 3, 9, 10 and 11.
 *     only RGB color maps with no more than 256 colors.
 *     pixel formats 8, 15, 16, 24 and 32.
 * writing:
 *     uncompressed rgb color tga files
 *     uncompressed grayscale tga files
 *     uncompressed indexed tga files
 */

#include "microexif_p.h"
#include "scanlineconverter_p.h"
#include "tga_p.h"
#include "util_p.h"

#include <assert.h>

#include <QColorSpace>
#include <QDataStream>
#include <QDateTime>
#include <QImage>
#include <QLoggingCategory>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QVector>
#endif

typedef quint32 uint;
typedef quint16 ushort;
typedef quint8 uchar;

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(LOG_TGAPLUGIN, "kf.imageformats.plugins.tga", QtDebugMsg)
#else
Q_LOGGING_CATEGORY(LOG_TGAPLUGIN, "kf.imageformats.plugins.tga", QtWarningMsg)
#endif

#ifndef TGA_V2E_AS_DEFAULT
/*
 * Uncomment to change the default version of the plugin to `TGAv2E`.
 */
// #define TGA_V2E_AS_DEFAULT
#endif // TGA_V2E_AS_DEFAULT

namespace // Private.
{
// Header format of saved files.
enum TGAType {
    TGA_TYPE_INDEXED = 1,
    TGA_TYPE_RGB = 2,
    TGA_TYPE_GREY = 3,
    TGA_TYPE_RLE_INDEXED = 9,
    TGA_TYPE_RLE_RGB = 10,
    TGA_TYPE_RLE_GREY = 11,
};

#define TGA_INTERLEAVE_MASK 0xc0
#define TGA_INTERLEAVE_NONE 0x00
#define TGA_INTERLEAVE_2WAY 0x40
#define TGA_INTERLEAVE_4WAY 0x80

#define TGA_ORIGIN_MASK 0x30
#define TGA_ORIGIN_LEFT 0x00
#define TGA_ORIGIN_RIGHT 0x10
#define TGA_ORIGIN_LOWER 0x00
#define TGA_ORIGIN_UPPER 0x20

/*
 * Each TAG is a SHORT value in the range of 0 to 65535. Values from 0 - 32767 are available
 * for developer use, while values from 32768 - 65535 are reserved for Truevision.
 * Truevision will maintain a list of tags assigned to companies.
 * In any case, there's no public "list of tag" and Truevision no longer exists.
 */
#define TGA_EXIF_TAGID 0x7001 // Exif data preceded by "eXif" string
#define TGA_XMPP_TAGID 0x7002 // Xmp packet preceded by "xMPP" string
#define TGA_ICCP_TAGID 0x7003 // Icc profile preceded by "iCCP" string

/** Tga Header. */
struct TgaHeader {
    uchar id_length = 0;
    uchar colormap_type = 0;
    uchar image_type = 0;
    ushort colormap_index = 0;
    ushort colormap_length = 0;
    uchar colormap_size = 0;
    ushort x_origin = 0;
    ushort y_origin = 0;
    ushort width = 0;
    ushort height = 0;
    uchar pixel_size = 0;
    uchar flags = 0;

    enum {
        SIZE = 18,
    }; // const static int SIZE = 18;
};

/** Tga 2.0 Footer */
struct TgaFooter {
    TgaFooter()
        : extensionOffset(0)
        , developerOffset(0)
    {
        std::memcpy(signature, "TRUEVISION-XFILE.\0", 18);
    }
    bool isValid() const
    {
        return std::memcmp(signature, "TRUEVISION-XFILE.\0", 18) == 0;
    }

    quint32 extensionOffset; // Extension Area Offset
    quint32 developerOffset; // Developer Directory Offset
    char signature[18]; // TGA Signature
};

/** Tga 2.0 extension area */
struct TgaExtension {
    enum AttributeType : quint16 {
        NoAlpha = 0, // no Alpha data included (bits 3-0 of TgaHeader::flags should also be set to zero).
        IgnoreAlpha = 1, // undefined data in the Alpha field, can be ignored
        RetainAlpha = 2, // undefined data in the Alpha field, but should be retained
        Alpha = 3, // useful Alpha channel data is present
        PremultipliedAlpha = 4 // pre-multiplied Alpha (see description below)
    };

    TgaExtension()
    {
        std::memset(this, 0, sizeof(TgaExtension));
        size = 495; // TGA 2.0 specs

        // If you do not use Software Version field, set the SHORT to binary
        // zero, and the BYTE to a space (' ').
        versionLetter = 0x20;
    }

    bool isValid() const
    {
        return size == 495;
    }

    void setDateTime(const QDateTime &dt)
    {
        if (dt.isValid()) {
            auto date = dt.date();
            stampMonth = date.month();
            stampDay = date.day();
            stampYear = date.year();
            auto time = dt.time();
            stampHour = time.hour();
            stampMinute = time.minute();
            stampSecond = time.second();
        }
    }
    QDateTime dateTime() const
    {
        auto date = QDate(stampYear, stampMonth, stampDay);
        auto time = QTime(stampHour, stampMinute, stampSecond);
        if (!date.isValid() || !time.isValid())
            return {};
        return QDateTime(date, time);
    }

    void setAuthor(const QString &str)
    {
        auto ba = str.toLatin1();
        std::memcpy(authorName, ba.data(), std::min(sizeof(authorName) - 1, size_t(ba.size())));
    }
    QString author() const
    {
        if (authorName[sizeof(authorName) - 1] != char(0))
            return {};
        return QString::fromLatin1(authorName);
    }

    void setComment(const QString &str)
    {
        auto ba = str.toLatin1();
        std::memcpy(authorComment, ba.data(), std::min(sizeof(authorComment) - 1, size_t(ba.size())));
    }
    QString comment() const
    {
        if (authorComment[sizeof(authorComment) - 1] != char(0))
            return {};
        return QString::fromLatin1(authorComment);
    }

    void setSoftware(const QString &str)
    {
        auto ba = str.toLatin1();
        std::memcpy(softwareId, ba.data(), std::min(sizeof(softwareId) - 1, size_t(ba.size())));
    }
    QString software() const
    {
        if (softwareId[sizeof(softwareId) - 1] != char(0))
            return {};
        return QString::fromLatin1(softwareId);
    }

    quint16 size; // Extension Size
    char authorName[41]; // Author Name
    char authorComment[324]; // Author Comment
    quint16 stampMonth; // Date/Time Stamp: Month
    quint16 stampDay; // Date/Time Stamp: Day
    quint16 stampYear; // Date/Time Stamp: Year
    quint16 stampHour; // Date/Time Stamp: Hour
    quint16 stampMinute; // Date/Time Stamp: Minute
    quint16 stampSecond; // Date/Time Stamp: Second
    char jobName[41]; // Job Name/ID
    quint16 jobHour; // Job Time: Hours
    quint16 jobMinute; // Job Time: Minutes
    quint16 jobSecond; // Job Time: Seconds
    char softwareId[41]; // Software ID
    quint16 versionNumber; // Software Version Number
    quint8 versionLetter; // Software Version Letter
    quint32 keyColor; // Key Color
    quint16 pixelNumerator; // Pixel Aspect Ratio
    quint16 pixelDenominator; // Pixel Aspect Ratio
    quint16 gammaNumerator; // Gamma Value
    quint16 gammaDenominator; // Gamma Value
    quint32 colorOffset; // Color Correction Offset
    quint32 stampOffset; // Postage Stamp Offset
    quint32 scanOffset; // Scan-Line Table Offset
    quint8 attributesType; // Attributes Types
};

struct TgaDeveloperDirectory {
    struct Field {
        quint16 tagId;
        quint32 offset;
        quint32 size;
    };

    bool isEmpty() const
    {
        return fields.isEmpty();
    }

    QList<Field> fields;
};

static QDataStream &operator>>(QDataStream &s, TgaHeader &head)
{
    s >> head.id_length;
    s >> head.colormap_type;
    s >> head.image_type;
    s >> head.colormap_index;
    s >> head.colormap_length;
    s >> head.colormap_size;
    s >> head.x_origin;
    s >> head.y_origin;
    s >> head.width;
    s >> head.height;
    s >> head.pixel_size;
    s >> head.flags;
    return s;
}

static QDataStream &operator>>(QDataStream &s, TgaFooter &footer)
{
    s >> footer.extensionOffset;
    s >> footer.developerOffset;
    s.readRawData(footer.signature, sizeof(footer.signature));
    return s;
}

static QDataStream &operator<<(QDataStream &s, const TgaFooter &footer)
{
    s << footer.extensionOffset;
    s << footer.developerOffset;
    s.writeRawData(footer.signature, sizeof(footer.signature));
    return s;
}

static QDataStream &operator>>(QDataStream &s, TgaDeveloperDirectory &dir)
{
    quint16 n;
    s >> n;
    for (auto i = n; i > 0; --i) {
        TgaDeveloperDirectory::Field f;
        s >> f.tagId;
        s >> f.offset;
        s >> f.size;
        dir.fields << f;
    }
    return s;
}

static QDataStream &operator<<(QDataStream &s, const TgaDeveloperDirectory &dir)
{
    s << quint16(dir.fields.size());
    for (auto &&f : dir.fields) {
        s << f.tagId;
        s << f.offset;
        s << f.size;
    }
    return s;
}

static QDataStream &operator>>(QDataStream &s, TgaExtension &ext)
{
    s >> ext.size;
    s.readRawData(ext.authorName, sizeof(ext.authorName));
    s.readRawData(ext.authorComment, sizeof(ext.authorComment));
    s >> ext.stampMonth;
    s >> ext.stampDay;
    s >> ext.stampYear;
    s >> ext.stampHour;
    s >> ext.stampMinute;
    s >> ext.stampSecond;
    s.readRawData(ext.jobName, sizeof(ext.jobName));
    s >> ext.jobHour;
    s >> ext.jobMinute;
    s >> ext.jobSecond;
    s.readRawData(ext.softwareId, sizeof(ext.softwareId));
    s >> ext.versionNumber;
    s >> ext.versionLetter;
    s >> ext.keyColor;
    s >> ext.pixelNumerator;
    s >> ext.pixelDenominator;
    s >> ext.gammaNumerator;
    s >> ext.gammaDenominator;
    s >> ext.colorOffset;
    s >> ext.stampOffset;
    s >> ext.scanOffset;
    s >> ext.attributesType;
    return s;
}

static QDataStream &operator<<(QDataStream &s, const TgaExtension &ext)
{
    s << ext.size;
    s.writeRawData(ext.authorName, sizeof(ext.authorName));
    s.writeRawData(ext.authorComment, sizeof(ext.authorComment));
    s << ext.stampMonth;
    s << ext.stampDay;
    s << ext.stampYear;
    s << ext.stampHour;
    s << ext.stampMinute;
    s << ext.stampSecond;
    s.writeRawData(ext.jobName, sizeof(ext.jobName));
    s << ext.jobHour;
    s << ext.jobMinute;
    s << ext.jobSecond;
    s.writeRawData(ext.softwareId, sizeof(ext.softwareId));
    s << ext.versionNumber;
    s << ext.versionLetter;
    s << ext.keyColor;
    s << ext.pixelNumerator;
    s << ext.pixelDenominator;
    s << ext.gammaNumerator;
    s << ext.gammaDenominator;
    s << ext.colorOffset;
    s << ext.stampOffset;
    s << ext.scanOffset;
    s << ext.attributesType;
    return s;
}

static bool IsSupported(const TgaHeader &head)
{
    if (head.image_type != TGA_TYPE_INDEXED && head.image_type != TGA_TYPE_RGB && head.image_type != TGA_TYPE_GREY && head.image_type != TGA_TYPE_RLE_INDEXED
        && head.image_type != TGA_TYPE_RLE_RGB && head.image_type != TGA_TYPE_RLE_GREY) {
        return false;
    }
    if (head.image_type == TGA_TYPE_INDEXED || head.image_type == TGA_TYPE_RLE_INDEXED) {
        // GIMP saves TGAs with palette size of 257 (but 256 used) so, I need to check the pixel size only.
        if (head.pixel_size > 8 || head.colormap_type != 1) {
            return false;
        }
        if (head.colormap_size != 15 && head.colormap_size != 16 && head.colormap_size != 24 && head.colormap_size != 32) {
            return false;
        }
    }
    if (head.image_type == TGA_TYPE_RGB || head.image_type == TGA_TYPE_GREY || head.image_type == TGA_TYPE_RLE_RGB || head.image_type == TGA_TYPE_RLE_GREY) {
        if (head.colormap_type != 0) {
            return false;
        }
    }
    if (head.width == 0 || head.height == 0) {
        return false;
    }
    if (head.pixel_size != 8 && head.pixel_size != 15 && head.pixel_size != 16 && head.pixel_size != 24 && head.pixel_size != 32) {
        return false;
    }
    // If the colormap_type field is set to zero, indicating that no color map exists, then colormap_index and colormap_length should be set to zero.
    if (head.colormap_type == 0 && (head.colormap_index != 0 || head.colormap_length != 0)) {
        return false;
    }

    return true;
}

/*!
 * \brief imageId
 * Create the TGA imageId from the image TITLE metadata
 */
static QByteArray imageId(const QImage &img)
{
    auto ba = img.text(QStringLiteral(META_KEY_TITLE)).trimmed().toLatin1();
    if (ba.size() > 255)
        ba = ba.left(255);
    return ba;
}

struct TgaHeaderInfo {
    bool rle;
    bool pal;
    bool rgb;
    bool grey;

    TgaHeaderInfo(const TgaHeader &tga)
        : rle(false)
        , pal(false)
        , rgb(false)
        , grey(false)
    {
        switch (tga.image_type) {
        case TGA_TYPE_RLE_INDEXED:
            rle = true;
            Q_FALLTHROUGH();
        // no break is intended!
        case TGA_TYPE_INDEXED:
            pal = true;
            break;

        case TGA_TYPE_RLE_RGB:
            rle = true;
            Q_FALLTHROUGH();
        // no break is intended!
        case TGA_TYPE_RGB:
            rgb = true;
            break;

        case TGA_TYPE_RLE_GREY:
            rle = true;
            Q_FALLTHROUGH();
        // no break is intended!
        case TGA_TYPE_GREY:
            grey = true;
            break;

        default:
            // Error, unknown image type.
            break;
        }
    }
};

static QImage::Format imageFormat(const TgaHeader &head)
{
    auto format = QImage::Format_Invalid;
    if (IsSupported(head)) {
        TgaHeaderInfo info(head);

        // Bits 0-3 are the numbers of alpha bits (can be zero!)
        const int numAlphaBits = head.flags & 0xf;
        // However alpha should exists only in the 32 bit format.
        if ((head.pixel_size == 32) && (numAlphaBits)) {
            if (numAlphaBits <= 8) {
                format = QImage::Format_ARGB32;
            }
        // Anyway, GIMP also saves gray images with alpha in TGA format
        } else if ((info.grey) && (head.pixel_size == 16) && (numAlphaBits)) {
            if (numAlphaBits == 8) {
                format = QImage::Format_ARGB32;
            }
        } else if (info.grey) {
            format = QImage::Format_Grayscale8;
        } else if (info.pal) {
            format = QImage::Format_Indexed8;
        } else if (info.rgb && (head.pixel_size == 15 || head.pixel_size == 16)) {
            format = QImage::Format_RGB555;
        } else {
            format = QImage::Format_RGB32;
        }
    }
    return format;
}

/*!
 * \brief peekHeader
 * Reads the header but does not change the position in the device.
 */
static bool peekHeader(QIODevice *device, TgaHeader &header)
{
    auto head = device->peek(TgaHeader::SIZE);
    if (head.size() < TgaHeader::SIZE) {
        return false;
    }
    QDataStream stream(head);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream >> header;
    return true;
}

/*!
 * \brief readTgaLine
 * Read a scan line from the raw data.
 * \param dev The current device.
 * \param pixel_size The number of bytes per pixel.
 * \param size The size of the uncompressed TGA raw line
 * \param rle True if the stream is RLE compressed, otherwise false.
 * \param cache The cache buffer used to store data (only used when the stream is RLE).
 * \return The uncompressed raw data of a line or an empty array on error.
 */
static QByteArray readTgaLine(QIODevice *dev, qint32 pixel_size, qint32 size, bool rle, QByteArray &cache)
{
    // uncompressed stream
    if (!rle) {
        auto ba = dev->read(size);
        if (ba.size() != size)
            ba.clear();
        return ba;
    }

    // RLE compressed stream
    if (cache.size() < qsizetype(size)) {
        // Decode image.
        qint64 num = size;

        while (num > 0) {
            if (dev->atEnd()) {
                break;
            }

            // Get packet header.
            char cc;
            if (dev->read(&cc, 1) != 1) {
                cache.clear();
                break;
            }
            auto c = uchar(cc);

            uint count = (c & 0x7f) + 1;
            QByteArray tmp(count * pixel_size, char());
            auto dst = tmp.data();
            num -= count * pixel_size;

            if (c & 0x80) { // RLE pixels.
                assert(pixel_size <= 8);
                char pixel[8];
                const int dataRead = dev->read(pixel, pixel_size);
                if (dataRead < (int)pixel_size) {
                    memset(&pixel[dataRead], 0, pixel_size - dataRead);
                }
                do {
                    memcpy(dst, pixel, pixel_size);
                    dst += pixel_size;
                } while (--count);
            } else { // Raw pixels.
                count *= pixel_size;
                const int dataRead = dev->read(dst, count);
                if (dataRead < 0) {
                    cache.clear();
                    break;
                }

                if ((uint)dataRead < count) {
                    const size_t toCopy = count - dataRead;
                    memset(&dst[dataRead], 0, toCopy);
                }
                dst += count;
            }

            cache.append(tmp);
        }
    }

    auto data = cache.left(size);
    cache.remove(0, size);
    if (data.size() != size)
        data.clear();
    return data;
}

inline QRgb rgb555ToRgb(char c0, char c1)
{
    // c0 = GGGBBBBB
    // c1 = IRRRRRGG (I = interrupt control of VDA(D) -> ignore it)
    return qRgb(int((c1 >> 2) & 0x1F) * 255 / 31, int(((c1 & 3) << 3) | ((c0 >> 5) & 7)) * 255 / 31, int(c0 & 0x1F) * 255 / 31);
}

static bool LoadTGA(QIODevice *dev, const TgaHeader &tga, QImage &img)
{
    img = imageAlloc(tga.width, tga.height, imageFormat(tga));
    if (img.isNull()) {
        qCWarning(LOG_TGAPLUGIN) << "LoadTGA: Failed to allocate image, invalid dimensions?" << QSize(tga.width, tga.height);
        return false;
    }

    TgaHeaderInfo info(tga);

    const int numAlphaBits = qBound(0, tga.flags & 0xf, 8);
    bool hasAlpha = img.hasAlphaChannel() && numAlphaBits > 0;
    qint32 pixel_size = (tga.pixel_size == 15 ? 16 : tga.pixel_size) / 8;
    qint32 line_size = qint32(tga.width) * pixel_size;
    qint64 size = qint64(tga.height) * line_size;
    if (size < 1) {
        //          qCDebug(LOG_TGAPLUGIN) << "This TGA file is broken with size " << size;
        return false;
    }

    // Read palette.
    if (info.pal) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QList<QRgb> colorTable;
#else
        QVector<QRgb> colorTable;
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 8, 0)
        colorTable.resize(tga.colormap_length);
#else
        colorTable.resizeForOverwrite(tga.colormap_length);
#endif

        if (tga.colormap_size == 32) {
            char data[4]; // BGRA
            for (QRgb &rgb : colorTable) {
                const auto dataRead = dev->read(data, 4);
                if (dataRead < 4) {
                    return false;
                }
                // BGRA.
                rgb = qRgba(data[2], data[1], data[0], data[3]);
            }
        } else if (tga.colormap_size == 24) {
            char data[3]; // BGR
            for (QRgb &rgb : colorTable) {
                const auto dataRead = dev->read(data, 3);
                if (dataRead < 3) {
                    return false;
                }
                // BGR.
                rgb = qRgb(data[2], data[1], data[0]);
            }
        } else if (tga.colormap_size == 16 || tga.colormap_size == 15) {
            char data[2];
            for (QRgb &rgb : colorTable) {
                const auto dataRead = dev->read(data, 2);
                if (dataRead < 2) {
                    return false;
                }
                rgb = rgb555ToRgb(data[0], data[1]);
            }
        } else {
            return false;
        }

        img.setColorTable(colorTable);
    }

    // Convert image to internal format.
    bool valid = true;
    int y_start = tga.height - 1;
    int y_step = -1;
    int y_end = -1;
    if (tga.flags & TGA_ORIGIN_UPPER) {
        y_start = 0;
        y_step = 1;
        y_end = tga.height;
    }
    int x_start = 0;
    int x_step = 1;
    int x_end = tga.width;
    if (tga.flags & TGA_ORIGIN_RIGHT) {
        x_start = tga.width - 1;
        x_step = -1;
        x_end = -1;
    }

    QByteArray cache;
    for (int y = y_start; y != y_end; y += y_step) {
        auto tgaLine = readTgaLine(dev, pixel_size, line_size, info.rle, cache);
        if (tgaLine.size() != qsizetype(line_size)) {
            qCWarning(LOG_TGAPLUGIN) << "LoadTGA: Error while decoding a TGA raw line";
            valid = false;
            break;
        }
        auto src = tgaLine.data();
        if (info.pal && img.depth() == 8) {
            // Paletted.
            auto scanline = img.scanLine(y);
            for (int x = x_start; x != x_end; x += x_step) {
                uchar idx = *src++;
                if (Q_UNLIKELY(idx >= tga.colormap_length)) {
                    valid = false;
                    break;
                }
                scanline[x] = idx;
            }
        } else if (info.grey) {
            if (tga.pixel_size == 16 && img.depth() == 32) { // Greyscale with alpha.
                auto scanline = reinterpret_cast<QRgb *>(img.scanLine(y));
                for (int x = x_start; x != x_end; x += x_step) {
                    scanline[x] = qRgba(*src, *src, *src, *(src + 1));
                    src += 2;
                }
            } else if (tga.pixel_size == 8 && img.depth() == 8) { // Greyscale.
                auto scanline = img.scanLine(y);
                for (int x = x_start; x != x_end; x += x_step) {
                    scanline[x] = *src;
                    src++;
                }
            } else {
                valid = false;
                break;
            }
        } else {
            // True Color.
            if ((tga.pixel_size == 15 || tga.pixel_size == 16) && img.depth() == 16) {
                auto scanline = reinterpret_cast<quint16 *>(img.scanLine(y));
                for (int x = x_start; x != x_end; x += x_step) {
                    scanline[x] = ((quint16(src[1] & 0x7f) << 8) | quint8(src[0]));
                    src += 2;
                }
            } else if (tga.pixel_size == 24 && img.depth() == 32) {
                auto scanline = reinterpret_cast<QRgb *>(img.scanLine(y));
                for (int x = x_start; x != x_end; x += x_step) {
                    scanline[x] = qRgb(src[2], src[1], src[0]);
                    src += 3;
                }
            } else if (tga.pixel_size == 32 && img.depth() == 32) {
                auto scanline = reinterpret_cast<QRgb *>(img.scanLine(y));
                auto div = (1 << numAlphaBits) - 1;
                if (div == 0)
                    hasAlpha = false;
                for (int x = x_start; x != x_end; x += x_step) {
                    const int alpha = hasAlpha ? int((src[3]) << (8 - numAlphaBits)) * 255 / div : 255;
                    scanline[x] = qRgba(src[2], src[1], src[0], alpha);
                    src += 4;
                }
            } else {
                valid = false;
                break;
            }
        }
    }

    if (!cache.isEmpty() && valid) {
        qCDebug(LOG_TGAPLUGIN) << "LoadTGA: Found unused image data";
    }

    return valid;
}

} // namespace

class TGAHandlerPrivate
{
public:
    TGAHandlerPrivate()
#ifdef TGA_V2E_AS_DEFAULT
        : m_subType(subTypeTGA_V2E())
#else
        : m_subType(subTypeTGA_V2S())
#endif
    {
    }
    ~TGAHandlerPrivate() {}

    static QByteArray subTypeTGA_V1()
    {
        return QByteArrayLiteral("TGAv1");
    }
    static QByteArray subTypeTGA_V2S()
    {
        return QByteArrayLiteral("TGAv2");
    }
    static QByteArray subTypeTGA_V2E()
    {
        return QByteArrayLiteral("TGAv2E");
    }

    TgaHeader m_header;

    QByteArray m_subType;
};

TGAHandler::TGAHandler()
    : QImageIOHandler()
    , d(new TGAHandlerPrivate)
{
}

bool TGAHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("tga");
        return true;
    }
    return false;
}

bool TGAHandler::read(QImage *outImage)
{
    // qCDebug(LOG_TGAPLUGIN) << "Loading TGA file!";

    auto dev = device();
    auto&& tga = d->m_header;
    if (!peekHeader(dev, tga) || !IsSupported(tga)) {
        //         qCDebug(LOG_TGAPLUGIN) << "This TGA file is not valid.";
        return false;
    }

    QByteArray imageId;
    if (dev->isSequential()) {
        auto tmp = dev->read(TgaHeader::SIZE);
        if (tmp.size() != TgaHeader::SIZE)
            return false;
    } else {
        if (!dev->seek(TgaHeader::SIZE))
            return false;
    }
    if (tga.id_length > 0) {
        imageId = dev->read(tga.id_length);
    }

    // Check image file format.
    if (dev->atEnd()) {
        //         qCDebug(LOG_TGAPLUGIN) << "This TGA file is not valid.";
        return false;
    }

    QImage img;
    if (!LoadTGA(dev, tga, img)) {
        //         qCDebug(LOG_TGAPLUGIN) << "Error loading TGA file.";
        return false;
    } else if (!imageId.isEmpty()) {
        img.setText(QStringLiteral(META_KEY_TITLE), QString::fromLatin1(imageId));
    }
    if (!readMetadata(img)) {
        qCDebug(LOG_TGAPLUGIN) << "read: error while reading metadata";
    }

    *outImage = img;
    return true;
}

bool TGAHandler::write(const QImage &image)
{
    auto ok = false;
    if (image.format() == QImage::Format_Indexed8) {
        ok = writeIndexed(image);
    } else if (image.format() == QImage::Format_Grayscale8 || image.format() == QImage::Format_Grayscale16) {
        ok = writeGrayscale(image);
    } else if (image.format() == QImage::Format_RGB555 || image.format() == QImage::Format_RGB16 || image.format() == QImage::Format_RGB444) {
        ok = writeRGB555(image);
    } else {
        ok = writeRGBA(image);
    }
    return (ok && writeMetadata(image));
}

bool TGAHandler::writeIndexed(const QImage &image)
{
    auto dev = device();
    { // write header and palette
        QDataStream s(dev);
        s.setByteOrder(QDataStream::LittleEndian);

        auto ct = image.colorTable();
        auto iid = imageId(image);
        s << quint8(iid.size()); // ID Length
        s << quint8(1); // Color Map Type
        s << quint8(TGA_TYPE_INDEXED); // Image Type
        s << quint16(0); // First Entry Index
        s << quint16(ct.size()); // Color Map Length
        s << quint8(32); // Color map Entry Size
        s << quint16(0); // X-origin of Image
        s << quint16(0); // Y-origin of Image

        s << quint16(image.width()); // Image Width
        s << quint16(image.height()); // Image Height
        s << quint8(8); // Pixel Depth
        s << quint8(TGA_ORIGIN_UPPER + TGA_ORIGIN_LEFT); // Image Descriptor

        if (!iid.isEmpty())
            s.writeRawData(iid.data(), iid.size());

        for (auto &&rgb : ct) {
            s << quint8(qBlue(rgb));
            s << quint8(qGreen(rgb));
            s << quint8(qRed(rgb));
            s << quint8(qAlpha(rgb));
        }

        if (s.status() != QDataStream::Ok) {
            return false;
        }
    }

    for (int y = 0, h = image.height(), w = image.width(); y < h; y++) {
        auto ptr = reinterpret_cast<const char *>(image.constScanLine(y));
        if (dev->write(ptr, w) != w) {
            return false;
        }
    }

    return true;
}

bool TGAHandler::writeGrayscale(const QImage &image)
{
    auto dev = device();
    { // write header
        QDataStream s(dev);
        s.setByteOrder(QDataStream::LittleEndian);

        auto iid = imageId(image);
        s << quint8(iid.size()); // ID Length
        s << quint8(0); // Color Map Type
        s << quint8(TGA_TYPE_GREY); // Image Type
        s << quint16(0); // First Entry Index
        s << quint16(0); // Color Map Length
        s << quint8(0); // Color map Entry Size
        s << quint16(0); // X-origin of Image
        s << quint16(0); // Y-origin of Image

        s << quint16(image.width()); // Image Width
        s << quint16(image.height()); // Image Height
        s << quint8(8); // Pixel Depth
        s << quint8(TGA_ORIGIN_UPPER + TGA_ORIGIN_LEFT); // Image Descriptor

        if (!iid.isEmpty())
            s.writeRawData(iid.data(), iid.size());

        if (s.status() != QDataStream::Ok) {
            return false;
        }
    }

    ScanLineConverter scl(QImage::Format_Grayscale8);
    for (int y = 0, h = image.height(), w = image.width(); y < h; y++) {
        auto ptr = reinterpret_cast<const char *>(scl.convertedScanLine(image, y));
        if (dev->write(ptr, w) != w) {
            return false;
        }
    }

    return true;
}

bool TGAHandler::writeRGB555(const QImage &image)
{
    auto dev = device();
    { // write header
        QDataStream s(dev);
        s.setByteOrder(QDataStream::LittleEndian);

        auto iid = imageId(image);
        for (char c : {int(iid.size()), 0, int(TGA_TYPE_RGB), 0, 0, 0, 0, 0, 0, 0, 0, 0}) {
            s << c;
        }
        s << quint16(image.width()); // width
        s << quint16(image.height()); // height
        s << quint8(16); // depth
        s << quint8(TGA_ORIGIN_UPPER + TGA_ORIGIN_LEFT);

        if (!iid.isEmpty())
            s.writeRawData(iid.data(), iid.size());

        if (s.status() != QDataStream::Ok) {
            return false;
        }
    }

    ScanLineConverter scl(QImage::Format_RGB555);
    QByteArray ba(image.width() * 2, char());
    for (int y = 0, h = image.height(); y < h; y++) {
        auto ptr = reinterpret_cast<const quint16 *>(scl.convertedScanLine(image, y));
        for (int x = 0, w = image.width(); x < w; x++) {
            auto color = *(ptr + x);
            ba[x * 2] = char(color);
            ba[x * 2 + 1] = char(color >> 8);
        }
        if (dev->write(ba.data(), ba.size()) != qint64(ba.size())) {
            return false;
        }
    }

    return true;
}

bool TGAHandler::writeRGBA(const QImage &image)
{
    auto format = image.format();
    const bool hasAlpha = image.hasAlphaChannel();
    auto tcs = QColorSpace();
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    auto cs = image.colorSpace();
    if (cs.isValid() && cs.colorModel() == QColorSpace::ColorModel::Cmyk && image.format() == QImage::Format_CMYK8888) {
        format = QImage::Format_RGB32;
        tcs = QColorSpace(QColorSpace::SRgb);
    } else if (hasAlpha && image.format() != QImage::Format_ARGB32) {
#else
    if (hasAlpha && image.format() != QImage::Format_ARGB32) {
#endif
        format = QImage::Format_ARGB32;
    } else if (!hasAlpha && image.format() != QImage::Format_RGB32) {
        format = QImage::Format_RGB32;
    }

    auto dev = device();
    { // write header
        QDataStream s(dev);
        s.setByteOrder(QDataStream::LittleEndian);

        const quint8 originTopLeft = TGA_ORIGIN_UPPER + TGA_ORIGIN_LEFT; // 0x20
        const quint8 alphaChannel8Bits = 0x08;

        auto iid = imageId(image);
        for (char c : {int(iid.size()), 0, int(TGA_TYPE_RGB), 0, 0, 0, 0, 0, 0, 0, 0, 0}) {
            s << c;
        }
        s << quint16(image.width()); // width
        s << quint16(image.height()); // height
        s << quint8(hasAlpha ? 32 : 24); // depth (24 bit RGB + 8 bit alpha)
        s << quint8(hasAlpha ? originTopLeft + alphaChannel8Bits : originTopLeft); // top left image (0x20) + 8 bit alpha (0x8)

        if (!iid.isEmpty())
            s.writeRawData(iid.data(), iid.size());

        if (s.status() != QDataStream::Ok) {
            return false;
        }
    }

    ScanLineConverter scl(format);
    if (tcs.isValid()) {
        scl.setTargetColorSpace(tcs);
    }
    auto mul = hasAlpha ? 4 : 3;
    QByteArray ba(image.width() * mul, char());
    for (int y = 0, h = image.height(); y < h; y++) {
        auto ptr = reinterpret_cast<const QRgb *>(scl.convertedScanLine(image, y));
        for (int x = 0, w = image.width(); x < w; x++) {
            auto color = *(ptr + x);
            auto xmul = x * mul;
            ba[xmul] = char(qBlue(color));
            ba[xmul + 1] = char(qGreen(color));
            ba[xmul + 2] = char(qRed(color));
            if (hasAlpha) {
                ba[xmul + 3] = char(qAlpha(color));
            }
        }
        if (dev->write(ba.data(), ba.size()) != qint64(ba.size())) {
            return false;
        }
    }

    return true;
}

bool TGAHandler::writeMetadata(const QImage &image)
{
    if (d->m_subType == TGAHandlerPrivate::subTypeTGA_V1()) {
        return true; // TGA V1 does not have these data
    }

    auto dev = device();
    if (dev == nullptr) {
        return false;
    }
    if (dev->isSequential()) {
        qCInfo(LOG_TGAPLUGIN) << "writeMetadata: unable to save metadata on a sequential device";
        return true;
    }

    QDataStream s(dev);
    s.setByteOrder(QDataStream::LittleEndian);

    // TGA 2.0 footer
    TgaFooter foot;

    // 32-bit overflow check (rough check)
    // I need at least 495 (extension) + 26 (footer) bytes -> 1024 bytes.
    // for the development area I roughly estimate 4096 KiB (profile, exif and xmp) they should always be less.
    auto reqBytes = qint64(d->m_subType == TGAHandlerPrivate::subTypeTGA_V2E() ? 4096 * 1024 : 1024);
    if (dev->pos() > std::numeric_limits<quint32>::max() - reqBytes) {
        qCInfo(LOG_TGAPLUGIN) << "writeMetadata: there is no enough space for metadata";
        return true;
    }

    // TGA 2.0 developer area
    TgaDeveloperDirectory dir;
    if (d->m_subType == TGAHandlerPrivate::subTypeTGA_V2E()) {
        auto exif = MicroExif::fromImage(image);
        if (!exif.isEmpty()) {
            auto ba = QByteArray("eXif").append(exif.toByteArray(s.byteOrder()));
            TgaDeveloperDirectory::Field f;
            f.tagId = TGA_EXIF_TAGID;
            f.offset = dev->pos();
            f.size = ba.size();
            if (s.writeRawData(ba.data(), ba.size()) != ba.size()) {
                return false;
            }
            dir.fields << f;
        }
        auto icc = image.colorSpace().iccProfile();
        if (!icc.isEmpty()) {
            auto ba = QByteArray("iCCP").append(icc);
            TgaDeveloperDirectory::Field f;
            f.tagId = TGA_ICCP_TAGID;
            f.offset = dev->pos();
            f.size = ba.size();
            if (s.writeRawData(ba.data(), ba.size()) != ba.size()) {
                return false;
            }
            dir.fields << f;
        }
        auto xmp = image.text(QStringLiteral(META_KEY_XMP_ADOBE)).trimmed();
        if (!xmp.isEmpty()) {
            auto ba = QByteArray("xMPP").append(xmp.toUtf8());
            TgaDeveloperDirectory::Field f;
            f.tagId = TGA_XMPP_TAGID;
            f.offset = dev->pos();
            f.size = ba.size();
            if (s.writeRawData(ba.data(), ba.size()) != ba.size()) {
                return false;
            }
            dir.fields << f;
        }
    }

    // TGA 2.0 extension area
    TgaExtension ext;
    ext.setDateTime(QDateTime::currentDateTimeUtc());
    if (image.hasAlphaChannel()) {
        ext.attributesType = TgaExtension::Alpha;
    }
    auto keys = image.textKeys();
    for (auto &&key : keys) {
        if (!key.compare(QStringLiteral(META_KEY_AUTHOR), Qt::CaseInsensitive)) {
            ext.setAuthor(image.text(key));
            continue;
        }
        if (!key.compare(QStringLiteral(META_KEY_COMMENT), Qt::CaseInsensitive)) {
            ext.setComment(image.text(key));
            continue;
        }
        if (!key.compare(QStringLiteral(META_KEY_DESCRIPTION), Qt::CaseInsensitive)) {
            if (ext.comment().isEmpty())
                ext.setComment(image.text(key));
            continue;
        }
        if (!key.compare(QStringLiteral(META_KEY_SOFTWARE), Qt::CaseInsensitive)) {
            ext.setSoftware(image.text(key));
            continue;
        }
    }

    // write developer area
    if (!dir.isEmpty()) {
        foot.developerOffset = dev->pos();
        s << dir;
    }

    // write extension area (date time is always set)
    foot.extensionOffset = dev->pos();
    s << ext;
    s << foot;

    return s.status() == QDataStream::Ok;
}

bool TGAHandler::readMetadata(QImage &image)
{
    auto dev = device();
    if (dev == nullptr) {
        return false;
    }
    if (dev->isSequential()) {
        qCInfo(LOG_TGAPLUGIN) << "readMetadata: unable to load metadata on a sequential device";
        return true;
    }

    // read TGA footer
    if (!dev->seek(dev->size() - 26)) {
        return false;
    }

    QDataStream s(dev);
    s.setByteOrder(QDataStream::LittleEndian);

    TgaFooter foot;
    s >> foot;
    if (s.status() != QDataStream::Ok) {
        return false;
    }
    if (!foot.isValid()) {
        return true; // not a TGA 2.0 -> no metadata are present
    }

    if (foot.extensionOffset > 0) {
        // read the extension area
        if (!dev->seek(foot.extensionOffset)) {
            return false;
        }

        TgaExtension ext;
        s >> ext;
        if (s.status() != QDataStream::Ok || !ext.isValid()) {
            return false;
        }

        auto dt = ext.dateTime();
        if (dt.isValid()) {
            image.setText(QStringLiteral(META_KEY_MODIFICATIONDATE), dt.toString(Qt::ISODate));
        }
        auto au = ext.author();
        if (!au.isEmpty()) {
            image.setText(QStringLiteral(META_KEY_AUTHOR), au);
        }
        auto cm = ext.comment();
        if (!cm.isEmpty()) {
            image.setText(QStringLiteral(META_KEY_COMMENT), cm);
        }
        auto sw = ext.software();
        if (!sw.isEmpty()) {
            image.setText(QStringLiteral(META_KEY_SOFTWARE), sw);
        }
    }

    if (foot.developerOffset > 0) {
        // read developer area
        if (!dev->seek(foot.developerOffset)) {
            return false;
        }

        TgaDeveloperDirectory dir;
        s >> dir;
        if (s.status() != QDataStream::Ok) {
            return false;
        }

        for (auto &&f : dir.fields) {
            if (!dev->seek(f.offset)) {
                return false;
            }
            if (f.tagId == TGA_EXIF_TAGID) {
                auto ba = dev->read(f.size);
                if (ba.startsWith(QByteArray("eXif"))) {
                    auto exif = MicroExif::fromByteArray(ba.mid(4));
                    exif.updateImageMetadata(image, true);
                    exif.updateImageResolution(image);
                }
                continue;
            }
            if (f.tagId == TGA_ICCP_TAGID) {
                auto ba = dev->read(f.size);
                if (ba.startsWith(QByteArray("iCCP"))) {
                    image.setColorSpace(QColorSpace::fromIccProfile(ba.mid(4)));
                }
                continue;
            }
            if (f.tagId == TGA_XMPP_TAGID) {
                auto ba = dev->read(f.size);
                if (ba.startsWith(QByteArray("xMPP"))) {
                    image.setText(QStringLiteral(META_KEY_XMP_ADOBE), QString::fromUtf8(ba.mid(4)));
                }
                continue;
            }
        }
    }

    return s.status() == QDataStream::Ok;
}

bool TGAHandler::supportsOption(ImageOption option) const
{
    if (option == QImageIOHandler::Size) {
        return true;
    }
    if (option == QImageIOHandler::ImageFormat) {
        return true;
    }
    if (option == QImageIOHandler::SubType) {
        return true;
    }
    if (option == QImageIOHandler::SupportedSubTypes) {
        return true;
    }
    return false;
}

void TGAHandler::setOption(ImageOption option, const QVariant &value)
{
    if (option == QImageIOHandler::SubType) {
        auto subType = value.toByteArray();
        auto list = TGAHandler::option(QImageIOHandler::SupportedSubTypes).value<QList<QByteArray>>();
        if (list.contains(subType)) {
            d->m_subType = subType;
        } else {
            d->m_subType = TGAHandlerPrivate::subTypeTGA_V2S();
        }
    }
}

QVariant TGAHandler::option(ImageOption option) const
{
    if (!supportsOption(option)) {
        return {};
    }

    if (option == QImageIOHandler::SupportedSubTypes) {
        return QVariant::fromValue(QList<QByteArray>()
                                   << TGAHandlerPrivate::subTypeTGA_V1() << TGAHandlerPrivate::subTypeTGA_V2S() << TGAHandlerPrivate::subTypeTGA_V2E());
    }

    if (option == QImageIOHandler::SubType) {
        return QVariant::fromValue(d->m_subType);
    }

    auto &&header = d->m_header;
    if (!IsSupported(header)) {
        if (auto dev = device())
            if (!peekHeader(dev, header) && IsSupported(header))
                return {};
        if (!IsSupported(header)) {
            return {};
        }
    }

    if (option == QImageIOHandler::Size) {
        return QVariant::fromValue(QSize(header.width, header.height));
    }

    if (option == QImageIOHandler::ImageFormat) {
        return QVariant::fromValue(imageFormat(header));
    }

    return {};
}

bool TGAHandler::canRead(QIODevice *device)
{
    if (!device) {
        qCWarning(LOG_TGAPLUGIN) << "TGAHandler::canRead() called with no device";
        return false;
    }

    TgaHeader tga;
    if (!peekHeader(device, tga)) {
        qCWarning(LOG_TGAPLUGIN) << "TGAHandler::canRead() error while reading the header";
        return false;
    }

    return IsSupported(tga);
}

QImageIOPlugin::Capabilities TGAPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "tga") {
        return Capabilities(CanRead | CanWrite);
    }
    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && TGAHandler::canRead(device)) {
        cap |= CanRead;
    }
    if (device->isWritable()) {
        cap |= CanWrite;
    }
    return cap;
}

QImageIOHandler *TGAPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new TGAHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#include "moc_tga_p.cpp"
