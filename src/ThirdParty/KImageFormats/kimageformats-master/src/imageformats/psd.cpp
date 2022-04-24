/*
    Photoshop File Format support for QImage.

    SPDX-FileCopyrightText: 2003 Ignacio Castaño <castano@ludicon.com>
    SPDX-FileCopyrightText: 2015 Alex Merry <alex.merry@kde.org>
    SPDX-FileCopyrightText: 2022 Mirco Miranda <mirco.miranda@systemceramics.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/*
 * This code is based on Thacher Ulrich PSD loading code released
 * into the public domain. See: http://tulrich.com/geekstuff/
 */

/*
 * Documentation on this file format is available at
 * http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/
 */

/*
 * Limitations of the current code:
 * - 32-bit float image are converted to 16-bit integer image.
 *   NOTE: Qt 6.2 allow 32-bit float images (RGB only)
 * - Other color spaces cannot be read due to lack of QImage support for
 *   color spaces other than RGB (and Grayscale): a conversion to
 *   RGB must be done.
 *   - The best way to convert between different color spaces is to use a
 *     color management engine (e.g. LittleCMS).
 *   - An approximate way is to ignore the color information and use
 *     literature formulas (possible but not recommended).
 */

#include "psd_p.h"

#include "util_p.h"

#include <QDataStream>
#include <QDebug>
#include <QImage>
#include <QColorSpace>

typedef quint32 uint;
typedef quint16 ushort;
typedef quint8 uchar;

namespace // Private.
{
enum ColorMode {
    CM_BITMAP = 0,
    CM_GRAYSCALE = 1,
    CM_INDEXED = 2,
    CM_RGB = 3,
    CM_CMYK = 4,
    CM_MULTICHANNEL = 7,
    CM_DUOTONE = 8,
    CM_LABCOLOR = 9,
};

enum ImageResourceId : quint16 {
    IRI_RESOLUTIONINFO = 0x03ED,
    IRI_ICCPROFILE = 0x040F,
    IRI_TRANSPARENCYINDEX = 0x0417,
    IRI_VERSIONINFO = 0x0421,
    IRI_XMPMETADATA = 0x0424
};

struct PSDHeader {
    uint signature;
    ushort version;
    uchar reserved[6];
    ushort channel_count;
    uint height;
    uint width;
    ushort depth;
    ushort color_mode;
};

struct PSDImageResourceBlock {
    QString name;
    QByteArray data;
};

/*!
 * \brief The PSDDuotoneOptions struct
 * \note You can decode the duotone data using the "Duotone Options"
 * file format found in the "Photoshop File Format" specs.
 */
struct PSDDuotoneOptions {
    QByteArray data;
};

/*!
 * \brief The PSDColorModeDataSection struct
 * Only indexed color and duotone have color mode data.
 */
struct PSDColorModeDataSection {
    PSDDuotoneOptions duotone;
    QVector<QRgb> palette;
};

using PSDImageResourceSection = QHash<quint16, PSDImageResourceBlock>;

/*!
 * \brief fixedPointToDouble
 * Converts a fixed point number to floating point one.
 */
static double fixedPointToDouble(qint32 fixedPoint)
{
    auto i = double(fixedPoint >> 16);
    auto d = double((fixedPoint & 0x0000FFFF) / 65536.0);
    return (i+d);
}

/*!
 * \brief readPascalString
 * Reads the Pascal string as defined in the PSD specification.
 * \param s The stream.
 * \param alignBytes Alignment of the string.
 * \param size Number of stream bytes used.
 * \return The string read.
 */
static QString readPascalString(QDataStream &s, qint32 alignBytes = 1, qint32 *size = nullptr)
{
    qint32 tmp = 0;
    if (size == nullptr)
        size = &tmp;

    quint8 stringSize;
    s >> stringSize;
    *size = sizeof(stringSize);

    QString str;
    if (stringSize > 0) {
        QByteArray ba;
        ba.resize(stringSize);
        auto read = s.readRawData(ba.data(), ba.size());
        if (read > 0) {
            *size += read;
            str = QString::fromLatin1(ba);
        }
    }

    // align
    if (alignBytes > 1)
        if (auto pad = *size % alignBytes)
            *size += s.skipRawData(alignBytes - pad);

    return str;
}

/*!
 * \brief readImageResourceSection
 * Reads the image resource section.
 * \param s The stream.
 * \param ok Pointer to the operation result variable.
 * \return The image resource section raw data.
 */
static PSDImageResourceSection readImageResourceSection(QDataStream &s, bool *ok = nullptr)
{
    PSDImageResourceSection irs;

    bool tmp = true;
    if (ok == nullptr)
        ok = &tmp;
    *ok = true;

    // Section size
    qint32 sectioSize;
    s >> sectioSize;

#ifdef QT_DEBUG
    auto pos = qint64();
    if (auto dev = s.device())
        pos = dev->pos();
#endif

    // Reading Image resource block
    for (auto size = sectioSize; size > 0;) {
        // Length      Description
        // -------------------------------------------------------------------
        // 4           Signature: '8BIM'
        // 2           Unique identifier for the resource. Image resource IDs
        //             contains a list of resource IDs used by Photoshop.
        // Variable    Name: Pascal string, padded to make the size even
        //             (a null name consists of two bytes of 0)
        // 4           Actual size of resource data that follows
        // Variable    The resource data, described in the sections on the
        //             individual resource types. It is padded to make the size
        //             even.

        quint32 signature;
        s >> signature;
        size -= sizeof(signature);
        // NOTE: MeSa signature is not documented but found in some old PSD take from Photoshop 7.0 CD.
        if (signature != 0x3842494D && signature != 0x4D655361) { // 8BIM and MeSa
            qDebug() << "Invalid Image Resource Block Signature!";
            *ok = false;
            break;
        }

        // id
        quint16 id;
        s >> id;
        size -= sizeof(id);

        // getting data
        PSDImageResourceBlock irb;

        // name
        qint32 bytes = 0;
        irb.name = readPascalString(s, 2, &bytes);
        size -= bytes;

        // data read
        quint32 dataSize;
        s >> dataSize;
        size -= sizeof(dataSize);
        // NOTE: Qt device::read() and QDataStream::readRawData() could read less data than specified.
        //       The read code should be improved.
        if(auto dev = s.device())
            irb.data = dev->read(dataSize);
        auto read = irb.data.size();
        if (read > 0)
            size -= read;
        if (read != dataSize) {
            qDebug() << "Image Resource Block Read Error!";
            *ok = false;
            break;
        }

        if (auto pad = dataSize % 2) {
            auto skipped = s.skipRawData(pad);
            if (skipped > 0)
                size -= skipped;
        }

        // insert IRB
        irs.insert(id, irb);
    }

#ifdef QT_DEBUG
    if (auto dev = s.device()) {
        if ((dev->pos() - pos) != sectioSize) {
            *ok = false;
        }
    }
#endif

    return irs;
}

/*!
 * \brief readColorModeDataSection
 * Read the color mode section
 * \param s The stream.
 * \param ok Pointer to the operation result variable.
 * \return The color mode section.
 */
PSDColorModeDataSection readColorModeDataSection(QDataStream &s, bool *ok = nullptr)
{
    PSDColorModeDataSection cms;

    bool tmp = false;
    if (ok == nullptr)
        ok = &tmp;
    *ok = true;

    qint32 size;
    s >> size;
    if (size != 768) {  // read the duotone data (524 bytes)
        // NOTE: A RGB/Gray float image has a 112 bytes ColorModeData that could be
        //       the "32-bit Toning Options" of Photoshop (starts with 'hdrt').
        //       Official Adobe specification tells "Only indexed color and duotone
        //       (see the mode field in the File header section) have color mode data.".
        //       See test case images 32bit_grayscale.psd and 32bit-rgb.psd
        cms.duotone.data = s.device()->read(size);
        if (cms.duotone.data.size() != size)
            *ok = false;
    }
    else {              // read the palette (768 bytes)
        auto&& palette = cms.palette;
        QVector<quint8> vect(size);
        for (auto&& v : vect)
            s >> v;
        for (qsizetype i = 0, n = vect.size()/3; i < n; ++i)
            palette.append(qRgb(vect.at(i), vect.at(n+i), vect.at(n+n+i)));
    }

    return cms;
}

/*!
 * \brief setColorSpace
 * Set the color space to the image.
 * \param img The image.
 * \param irs The image resource section.
 * \return True on success, otherwise false.
 */
static bool setColorSpace(QImage& img, const PSDImageResourceSection& irs)
{
    if (!irs.contains(IRI_ICCPROFILE))
        return false;
    auto irb = irs.value(IRI_ICCPROFILE);
    auto cs = QColorSpace::fromIccProfile(irb.data);
    if (!cs.isValid())
        return false;
    img.setColorSpace(cs);
    return true;
}

/*!
 * \brief setXmpData
 * Adds XMP metadata to QImage.
 * \param img The image.
 * \param irs The image resource section.
 * \return True on success, otherwise false.
 */
static bool setXmpData(QImage& img, const PSDImageResourceSection& irs)
{
    if (!irs.contains(IRI_XMPMETADATA))
        return false;
    auto irb = irs.value(IRI_XMPMETADATA);
    auto xmp = QString::fromUtf8(irb.data);
    if (xmp.isEmpty())
        return false;
    // NOTE: "XML:com.adobe.xmp" is the meta set by Qt reader when an
    //       XMP packet is found (e.g. when reading a PNG saved by Photoshop).
    //       I'm reusing the same key because a programs could search for it.
    img.setText(QStringLiteral("XML:com.adobe.xmp"), xmp);
    return true;
}

/*!
 * \brief hasMergedData
 * Checks if merged image data are available.
 * \param irs The image resource section.
 * \return True on success or if the block does not exist, otherwise false.
 */
static bool hasMergedData(const PSDImageResourceSection& irs)
{
    if (!irs.contains(IRI_VERSIONINFO))
        return true;
    auto irb = irs.value(IRI_VERSIONINFO);
    if (irb.data.size() > 4)
        return irb.data.at(4) != 0;
    return false;
}

/*!
 * \brief setResolution
 * Set the image resolution.
 * \param img The image.
 * \param irs The image resource section.
 * \return True on success, otherwise false.
 */
static bool setResolution(QImage& img, const PSDImageResourceSection& irs)
{
    if (!irs.contains(IRI_RESOLUTIONINFO))
        return false;
    auto irb = irs.value(IRI_RESOLUTIONINFO);

    QDataStream s(irb.data);
    s.setByteOrder(QDataStream::BigEndian);

    qint32 i32;
    s >> i32;                               // Horizontal resolution in pixels per inch.
    if (i32 <= 0)
        return false;
    auto hres = fixedPointToDouble(i32);

    s.skipRawData(4);                       // Display data (not used here)

    s >> i32;                               // Vertial resolution in pixels per inch.
    if (i32 <= 0)
        return false;
    auto vres = fixedPointToDouble(i32);

    img.setDotsPerMeterX(hres * 1000 / 25.4);
    img.setDotsPerMeterY(vres * 1000 / 25.4);
    return true;
}

/*!
 * \brief setTransparencyIndex
 * Search for transparency index block and, if found, changes the alpha of the value at the given index.
 * \param img The image.
 * \param irs The image resource section.
 * \return True on success, otherwise false.
 */
static bool setTransparencyIndex(QImage& img, const PSDImageResourceSection& irs)
{
    if (!irs.contains(IRI_TRANSPARENCYINDEX))
        return false;
    auto irb = irs.value(IRI_TRANSPARENCYINDEX);
    QDataStream s(irb.data);
    s.setByteOrder(QDataStream::BigEndian);
    quint16 idx;
    s >> idx;

    auto palette = img.colorTable();
    if (idx < palette.size()) {
        auto&& v = palette[idx];
        v = QRgb(v & ~0xFF000000);
        img.setColorTable(palette);
        return true;
    }

    return false;
}

static QDataStream &operator>>(QDataStream &s, PSDHeader &header)
{
    s >> header.signature;
    s >> header.version;
    for (int i = 0; i < 6; i++) {
        s >> header.reserved[i];
    }
    s >> header.channel_count;
    s >> header.height;
    s >> header.width;
    s >> header.depth;
    s >> header.color_mode;
    return s;
}

// Check that the header is a valid PSD.
static bool IsValid(const PSDHeader &header)
{
    if (header.signature != 0x38425053) { // '8BPS'
        return false;
    }
    return true;
}

// Check that the header is supported.
static bool IsSupported(const PSDHeader &header)
{
    if (header.version != 1 && header.version != 2) {
        return false;
    }
    if (header.depth != 8 &&
        header.depth != 16 &&
        header.depth != 32 &&
        header.depth != 1) {
        return false;
    }
    if (header.color_mode != CM_RGB &&
        header.color_mode != CM_GRAYSCALE &&
        header.color_mode != CM_INDEXED &&
        header.color_mode != CM_DUOTONE &&
        header.color_mode != CM_BITMAP) {
        return false;
    }
    return true;
}

static bool skip_section(QDataStream &s, bool psb = false)
{
    qint64 section_length;
    if (!psb) {
        quint32 tmp;
        s >> tmp;
        section_length = tmp;
    }
    else {
        s >> section_length;
    }

    // Skip mode data.
    for (qint32 i32 = 0; section_length; section_length -= i32) {
        i32 = std::min(section_length, qint64(std::numeric_limits<qint32>::max()));
        i32 = s.skipRawData(i32);
        if (i32 < 1)
            return false;
    }
    return true;
}

/*!
 * \brief decompress
 * Fast PackBits decompression.
 * \param input The compressed input buffer.
 * \param ilen The input buffer size.
 * \param output The uncompressed target buffer.
 * \param olen The target buffer size.
 * \return The number of valid bytes in the target buffer.
 */
qint64 decompress(const char *input, qint64 ilen, char *output, qint64 olen)
{
    qint64  j = 0;
    for (qint64 ip = 0, rr = 0, available = olen; j < olen && ip < ilen; available = olen - j) {
        char n = input[ip++];
        if (static_cast<signed char>(n) == -128)
            continue;

        if (static_cast<signed char>(n) >= 0) {
            rr = qint64(n) + 1;
            if (available < rr) {
                ip--;
                break;
            }

            if (ip + rr > ilen)
                return -1;
            memcpy(output + j, input + ip, size_t(rr));
            ip += rr;
        }
        else if (ip < ilen) {
            rr = qint64(1-n);
            if (available < rr) {
                ip--;
                break;
            }
            memset(output + j, input[ip++], size_t(rr));
        }

        j += rr;
    }
    return j;
}

/*!
 * \brief imageFormat
 * \param header The PSD header.
 * \return The Qt image format.
 */
static QImage::Format imageFormat(const PSDHeader &header)
{
    if (header.channel_count == 0) {
        return QImage::Format_Invalid;
    }

    auto format = QImage::Format_Invalid;
    switch(header.color_mode) {
    case CM_RGB:
        if (header.depth == 16 || header.depth == 32)
            format = header.channel_count < 4 ? QImage::Format_RGBX64 : QImage::Format_RGBA64;
        else
            format = header.channel_count < 4 ? QImage::Format_RGB888 : QImage::Format_RGBA8888;
        break;
    case CM_GRAYSCALE:
    case CM_DUOTONE:
        format = header.depth == 8 ? QImage::Format_Grayscale8 : QImage::Format_Grayscale16;
        break;
    case CM_INDEXED:
        format = header.depth == 8 ? QImage::Format_Indexed8 : QImage::Format_Invalid;
        break;
    case CM_BITMAP:
        format = header.depth == 1 ? QImage::Format_Mono : QImage::Format_Invalid;
        break;
    }
    return format;
}

/*!
 * \brief imageChannels
 * \param format The Qt image format.
 * \return The number of channels of the image format.
 */
static qint32 imageChannels(const QImage::Format& format)
{
    qint32 c = 4;
    switch(format) {
    case QImage::Format_RGB888:
        c = 3;
        break;
    case QImage::Format_Grayscale8:
    case QImage::Format_Grayscale16:
    case QImage::Format_Indexed8:
    case QImage::Format_Mono:
        c = 1;
        break;
    default:
        break;
    }
    return c;
}

inline quint8 xchg(quint8 v) {
    return v;
}

inline quint16 xchg(quint16 v) {
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    return quint16( (v>>8) | (v<<8) );
#else
    return v;   // never tested
#endif
}

inline quint32 xchg(quint32 v) {
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    return quint32( (v>>24) | ((v & 0x00FF0000)>>8) | ((v & 0x0000FF00)<<8) | (v<<24) );
#else
    return v;  // never tested
#endif
}

template<class T>
inline void planarToChunchy(uchar *target, const char* source, qint32 width, qint32 c, qint32 cn)
{
    auto s = reinterpret_cast<const T*>(source);
    auto t = reinterpret_cast<T*>(target);
    for (qint32 x = 0; x < width; ++x)
        t[x*cn+c] = xchg(s[x]);
}

template<class T>
inline void planarToChunchyFloat(uchar *target, const char* source, qint32 width, qint32 c, qint32 cn)
{
    auto s = reinterpret_cast<const T*>(source);
    auto t = reinterpret_cast<quint16*>(target);
    for (qint32 x = 0; x < width; ++x) {
        auto tmp = xchg(s[x]);
        t[x*cn+c] = std::min(quint16(*reinterpret_cast<float*>(&tmp) * std::numeric_limits<quint16>::max() + 0.5),
                             std::numeric_limits<quint16>::max());
    }
}

inline void monoInvert(uchar *target, const char* source, qint32 bytes)
{
    auto s = reinterpret_cast<const quint8*>(source);
    auto t = reinterpret_cast<quint8*>(target);
    for (qint32 x = 0; x < bytes; ++x)
        t[x] = ~s[x];
}

// Load the PSD image.
static bool LoadPSD(QDataStream &stream, const PSDHeader &header, QImage &img)
{
    // Checking for PSB
    auto isPsb = header.version == 2;
    bool ok = false;

    // Color Mode Data section
    auto cmds = readColorModeDataSection(stream, &ok);
    if (!ok) {
        qDebug() << "Error while skipping Color Mode Data section";
        return false;
    }

    // Image Resources Section
    auto irs = readImageResourceSection(stream, &ok);
    if (!ok) {
        qDebug() << "Error while reading Image Resources Section";
        return false;
    }
    // Checking for merged image (Photoshop compatibility data)
    if (!hasMergedData(irs)) {
        qDebug() << "No merged data found";
        return false;
    }

    // Layer and Mask section
    if (!skip_section(stream, isPsb)) {
        qDebug() << "Error while skipping Layer and Mask section";
        return false;
    }

    // Find out if the data is compressed.
    // Known values:
    //   0: no compression
    //   1: RLE compressed
    quint16 compression;
    stream >> compression;
    if (compression > 1) {
        qDebug() << "Unknown compression type";
        return false;
    }

    const QImage::Format format = imageFormat(header);
    if (format == QImage::Format_Invalid) {
        qWarning() << "Unsupported image format. color_mode:" << header.color_mode << "depth:" << header.depth << "channel_count:" << header.channel_count;
        return false;
    }

    img = QImage(header.width, header.height, format);
    if (img.isNull()) {
        qWarning() << "Failed to allocate image, invalid dimensions?" << QSize(header.width, header.height);
        return false;
    }
    img.fill(qRgb(0, 0, 0));
    if (!cmds.palette.isEmpty()) {
        img.setColorTable(cmds.palette);
        setTransparencyIndex(img, irs);
    }

    auto imgChannels = imageChannels(img.format());
    auto channel_num = std::min(qint32(header.channel_count), imgChannels);
    auto raw_count = qsizetype(header.width * header.depth + 7) / 8;

    if (header.height > kMaxQVectorSize / header.channel_count / sizeof(quint32)) {
        qWarning() << "LoadPSD() header height/channel_count too big" << header.height << header.channel_count;
        return false;
    }

    QVector<quint32> strides(header.height * header.channel_count, raw_count);
    // Read the compressed stride sizes
    if (compression)
        for (auto&& v : strides) {
            if (isPsb) {
                stream >> v;
                continue;
            }
            quint16 tmp;
            stream >> tmp;
            v = tmp;
        }

    // Read the image
    QByteArray rawStride;
    rawStride.resize(raw_count);
    for (qint32 c = 0; c < channel_num; ++c) {
        for(qint32 y = 0, h = header.height; y < h; ++y) {
            auto&& strideSize = strides.at(c*qsizetype(h)+y);
            if (compression) {
                QByteArray tmp;
                tmp.resize(strideSize);
                if (stream.readRawData(tmp.data(), tmp.size()) != tmp.size()) {
                    qDebug() << "Error while reading the stream of channel" << c << "line" << y;
                    return false;
                }
                if (decompress(tmp.data(), tmp.size(), rawStride.data(), rawStride.size()) < 0) {
                    qDebug() << "Error while decompressing the channel" << c << "line" << y;
                    return false;
                }
            }
            else {
                if (stream.readRawData(rawStride.data(), rawStride.size()) != rawStride.size()) {
                    qDebug() << "Error while reading the stream of channel" << c << "line" << y;
                    return false;
                }
            }

            if (stream.status() != QDataStream::Ok) {
                qDebug() << "Stream read error" << stream.status();
                return false;
            }

            auto scanLine = img.scanLine(y);
            if (header.depth == 1)          // Bitmap
                monoInvert(scanLine, rawStride.data(), std::min(rawStride.size(), img.bytesPerLine()));
            else if (header.depth == 8)     // 8-bits images: Indexed, Grayscale, RGB/RGBA
                planarToChunchy<quint8>(scanLine, rawStride.data(), header.width, c, imgChannels);
            else if (header.depth == 16)    // 16-bits integer images: Grayscale, RGB/RGBA
                planarToChunchy<quint16>(scanLine, rawStride.data(), header.width, c, imgChannels);
            else if (header.depth == 32)    // 32-bits float images: Grayscale, RGB/RGBA (coverted to equivalent integer 16-bits)
                planarToChunchyFloat<quint32>(scanLine, rawStride.data(), header.width, c, imgChannels);
        }
    }

    // Resolution info
    if (!setResolution(img, irs)) {
        // qDebug() << "No resolution info found!";
    }

    // ICC profile
    if (!setColorSpace(img, irs)) {
        // qDebug() << "No colorspace info set!";
    }

    // XMP data
    if (!setXmpData(img, irs)) {
        // qDebug() << "No XMP data found!";
    }

    // Duotone images: color data contains the duotone specification (not documented).
    // Other applications that read Photoshop files can treat a duotone image as a gray image,
    // and just preserve the contents of the duotone information when reading and writing the file.
    if (!cmds.duotone.data.isEmpty()) {
        img.setText(QStringLiteral("PSDDuotoneOptions"), QString::fromUtf8(cmds.duotone.data.toHex()));
    }

    return true;
}

} // Private

PSDHandler::PSDHandler()
{
}

bool PSDHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("psd");
        return true;
    }
    return false;
}

bool PSDHandler::read(QImage *image)
{
    QDataStream s(device());
    s.setByteOrder(QDataStream::BigEndian);

    PSDHeader header;
    s >> header;

    // Check image file format.
    if (s.atEnd() || !IsValid(header)) {
        //         qDebug() << "This PSD file is not valid.";
        return false;
    }

    // Check if it's a supported format.
    if (!IsSupported(header)) {
        //         qDebug() << "This PSD file is not supported.";
        return false;
    }

    QImage img;
    if (!LoadPSD(s, header, img)) {
        //         qDebug() << "Error loading PSD file.";
        return false;
    }

    *image = img;
    return true;
}

bool PSDHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("PSDHandler::canRead() called with no device");
        return false;
    }

    qint64 oldPos = device->pos();

    char head[4];
    qint64 readBytes = device->read(head, sizeof(head));
    if (readBytes < 0) {
        qWarning() << "Read failed" << device->errorString();
        return false;
    }

    if (readBytes != sizeof(head)) {
        if (device->isSequential()) {
            while (readBytes > 0) {
                device->ungetChar(head[readBytes-- - 1]);
            }
        } else {
            device->seek(oldPos);
        }
        return false;
    }

    if (device->isSequential()) {
        while (readBytes > 0) {
            device->ungetChar(head[readBytes-- - 1]);
        }
    } else {
        device->seek(oldPos);
    }

    return qstrncmp(head, "8BPS", 4) == 0;
}

QImageIOPlugin::Capabilities PSDPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "psd" || format == "psb" || format == "pdd" || format == "psdt") {
        return Capabilities(CanRead);
    }
    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && PSDHandler::canRead(device)) {
        cap |= CanRead;
    }
    return cap;
}

QImageIOHandler *PSDPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new PSDHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}
