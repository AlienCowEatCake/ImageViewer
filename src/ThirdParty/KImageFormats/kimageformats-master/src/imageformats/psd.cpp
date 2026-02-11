/*
    Photoshop File Format support for QImage.

    SPDX-FileCopyrightText: 2003 Ignacio Castaño <castano@ludicon.com>
    SPDX-FileCopyrightText: 2015 Alex Merry <alex.merry@kde.org>
    SPDX-FileCopyrightText: 2022-2025 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/*
 * The early version of this code was based on Thacher Ulrich PSD loading code
 * released into the public domain. See: http://tulrich.com/geekstuff/
 *
 * Documentation on this file format is available at
 * http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/
 *
 * Limitations of the current code:
 * - Color spaces other than RGB/Grayscale cannot be read due to lack of QImage
 *   support. Where possible, a conversion to RGB is done:
 *   - CMYK images are converted using an approximated way that ignores the color
 *     information (ICC profile) with Qt less than 6.8.
 *   - LAB images are converted to sRGB using literature formulas.
 *   - MULICHANNEL images with 1 channel are treat as Grayscale images.
 *   - MULICHANNEL images with more than 1 channels are treat as CMYK images.
 *   - DUOTONE images are treat as Grayscale images.
 */

#include "fastmath_p.h"
#include "microexif_p.h"
#include "packbits_p.h"
#include "psd_p.h"
#include "scanlineconverter_p.h"
#include "util_p.h"

#include <QColorSpace>
#include <QDataStream>
#include <QImage>
#include <QLoggingCategory>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QVector>
#endif

#include <cmath>
#include <cstring>

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(LOG_PSDPLUGIN, "kf.imageformats.plugins.psd", QtDebugMsg)
#else
Q_LOGGING_CATEGORY(LOG_PSDPLUGIN, "kf.imageformats.plugins.psd", QtWarningMsg)
#endif

typedef quint32 uint;
typedef quint16 ushort;
typedef quint8 uchar;

/* The fast LAB conversion converts the image to linear sRgb instead to sRgb.
 * This should not be a problem because the Qt's QColorSpace supports the linear
 * sRgb colorspace.
 *
 * Using linear conversion, the loading speed is slightly improved. Anyway, if you are using
 * an software that discard color info, you should comment it.
 *
 * At the time I'm writing (07/2022), Gwenview and Krita supports linear sRgb but KDE
 * preview creator does not. This is the why, for now, it is disabled.
 */
// #define PSD_FAST_LAB_CONVERSION

/* Since Qt version 6.8, the 8-bit CMYK format is natively supported.
 * If you encounter problems with native CMYK support you can continue to force the plugin to convert
 * to RGB as in previous versions by defining PSD_NATIVE_CMYK_SUPPORT_DISABLED.
 */
// #define PSD_NATIVE_CMYK_SUPPORT_DISABLED

/* The detection of the nature of the extra channel (alpha or not) passes through the reading of
 * the PSD sections.
 * By default, any extra channel is assumed to be non-alpha. If enabled, for RGB images only,
 * any extra channel is assumed as alpha unless refuted by the data in the various sections.
 *
 * Note: this parameter is for debugging only and should not be enabled in releases.
 */
// #define PSD_FORCE_RGBA

/* *** PSD_MAX_IMAGE_WIDTH and PSD_MAX_IMAGE_HEIGHT ***
 * The maximum size in pixel allowed by the plugin.
 */
#ifndef PSD_MAX_IMAGE_WIDTH
#define PSD_MAX_IMAGE_WIDTH KIF_LARGE_IMAGE_PIXEL_LIMIT
#endif
#ifndef PSD_MAX_IMAGE_HEIGHT
#define PSD_MAX_IMAGE_HEIGHT PSD_MAX_IMAGE_WIDTH
#endif

namespace // Private.
{

#if QT_VERSION < QT_VERSION_CHECK(6, 8, 0) || defined(PSD_NATIVE_CMYK_SUPPORT_DISABLED)
#   define CMYK_FORMAT QImage::Format_Invalid
#else
#   define CMYK_FORMAT QImage::Format_CMYK8888
#endif

#define NATIVE_CMYK (CMYK_FORMAT != QImage::Format_Invalid)

enum Signature : quint32 {
    S_8BIM = 0x3842494D, // '8BIM'
    S_8B64 = 0x38423634, // '8B64'

    S_MeSa = 0x4D655361   // 'MeSa'
};

enum ColorMode : quint16 {
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
    IRI_ALPHAIDENTIFIERS = 0x041D,
    IRI_VERSIONINFO = 0x0421,
    IRI_EXIFDATA1 = 0x0422,
    IRI_EXIFDATA3 = 0x0423, // never seen
    IRI_XMPMETADATA = 0x0424
};

enum LayerId : quint32 {
    LI_MT16 = 0x4D743136,   // 'Mt16',
    LI_MT32 = 0x4D743332,   // 'Mt32',
    LI_MTRN = 0x4D74726E    // 'Mtrn'
};

struct PSDHeader {
    PSDHeader() {
        memset(this, 0, sizeof(PSDHeader));
    }

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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QList<QRgb> palette;
#else
    QVector<QRgb> palette;
#endif
};

using PSDImageResourceSection = QHash<quint16, PSDImageResourceBlock>;

struct PSDLayerInfo {
    qint64 size = -1;
    qint16 layerCount = 0;
};

struct PSDGlobalLayerMaskInfo {
    qint64 size = -1;
};

struct PSDAdditionalLayerInfo {
    Signature signature = Signature();
    LayerId id = LayerId();
    qint64 size = -1;
};

struct PSDLayerAndMaskSection {
    qint64 size = -1;
    PSDLayerInfo layerInfo;
    PSDGlobalLayerMaskInfo globalLayerMaskInfo;
    QHash<LayerId, PSDAdditionalLayerInfo> additionalLayerInfo;

    bool isNull() const {
        return (size <= 0);
    }

    bool hasAlpha() const {
        return layerInfo.layerCount < 0 ||
               additionalLayerInfo.contains(LI_MT16) ||
               additionalLayerInfo.contains(LI_MT32) ||
               additionalLayerInfo.contains(LI_MTRN);
    }

    bool atEnd(bool isPsb) const {
        qint64 currentSize = 0;
        if (layerInfo.size > -1) {
            currentSize += layerInfo.size + 4;
            if (isPsb)
                currentSize += 4;
        }
        if (globalLayerMaskInfo.size > -1) {
            currentSize += globalLayerMaskInfo.size + 4;
        }
        auto aliv = additionalLayerInfo.values();
        for (auto &&v : aliv) {
            currentSize += (12 + v.size);
            if (v.signature == S_8B64)
                currentSize += 4;
        }
        return (size <= currentSize);
    }
};

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

static qint64 readSize(QDataStream &s, bool psb = false)
{
    qint64 size = 0;
    if (!psb) {
        quint32 tmp;
        s >> tmp;
        size = tmp;
    } else {
        s >> size;
    }
    if (s.status() != QDataStream::Ok) {
        size = -1;
    }
    return size;
}

static bool skip_data(QDataStream &s, qint64 size)
{
    // Skip mode data.
    for (qint32 i32 = 0; size; size -= i32) {
        i32 = std::min(size, qint64(std::numeric_limits<qint32>::max()));
        i32 = s.skipRawData(i32);
        if (i32 < 1)
            return false;
    }
    return true;
}

static bool skip_section(QDataStream &s, bool psb = false)
{
    auto section_length = readSize(s, psb);
    if (section_length < 0)
        return false;
    return skip_data(s, section_length);
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
    quint32 tmpSize;
    s >> tmpSize;
    qint64 sectioSize = tmpSize;

    // Reading Image resource block
    for (auto size = sectioSize; size > 0;) {

#define DEC_SIZE(value) \
        if ((size -= qint64(value)) < 0) { \
            *ok = false; \
            break; }

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
        DEC_SIZE(sizeof(signature))
        // NOTE: MeSa signature is not documented but found in some old PSD take from Photoshop 7.0 CD.
        if (signature != S_8BIM && signature != S_MeSa) { // 8BIM and MeSa
            qCDebug(LOG_PSDPLUGIN) << "Invalid Image Resource Block Signature!";
            *ok = false;
            break;
        }

        // id
        quint16 id;
        s >> id;
        DEC_SIZE(sizeof(id))

        // getting data
        PSDImageResourceBlock irb;

        // name
        qint32 bytes = 0;
        irb.name = readPascalString(s, 2, &bytes);
        DEC_SIZE(bytes)

        // data read
        quint32 dataSize;
        s >> dataSize;
        DEC_SIZE(sizeof(dataSize))
        if (auto dev = s.device()) {
            if (dataSize > size) {
                qCDebug(LOG_PSDPLUGIN) << "Invalid Image Resource Block Data Size!";
                *ok = false;
                break;
            }
            irb.data = deviceRead(dev, dataSize);
        }
        auto read = irb.data.size();
        if (read > 0) {
            DEC_SIZE(read)
        }
        if (read != qint64(dataSize)) {
            qCDebug(LOG_PSDPLUGIN) << "Image Resource Block Read Error!";
            *ok = false;
            break;
        }

        if (auto pad = dataSize % 2) {
            auto skipped = s.skipRawData(pad);
            if (skipped > 0) {
                DEC_SIZE(skipped);
            }
        }

        // insert IRB
        irs.insert(id, irb);

#undef DEC_SIZE
    }

    return irs;
}

PSDAdditionalLayerInfo readAdditionalLayer(QDataStream &s, bool *ok = nullptr)
{
    PSDAdditionalLayerInfo li;

    bool tmp = true;
    if (ok == nullptr)
        ok = &tmp;

    s >> li.signature;
    *ok = li.signature == S_8BIM || li.signature == S_8B64;
    if (!*ok)
        return li;

    s >> li.id;
    *ok = s.status() == QDataStream::Ok;
    if (!*ok)
        return li;

    li.size = readSize(s, li.signature == S_8B64);
    *ok = li.size >= 0;
    if (!*ok)
        return li;

    *ok = skip_data(s, li.size);

    return li;
}

PSDLayerAndMaskSection readLayerAndMaskSection(QDataStream &s, bool isPsb, bool *ok = nullptr)
{
    PSDLayerAndMaskSection lms;

    bool tmp = true;
    if (ok == nullptr)
        ok = &tmp;
    *ok = true;

    auto device = s.device();
    device->startTransaction();

    lms.size = readSize(s, isPsb);

    // read layer info
    if (s.status() == QDataStream::Ok && !lms.atEnd(isPsb)) {
        lms.layerInfo.size = readSize(s, isPsb);
        if (lms.layerInfo.size > 0) {
            s >> lms.layerInfo.layerCount;
            skip_data(s, lms.layerInfo.size - sizeof(lms.layerInfo.layerCount));
        }
    }

    // read global layer mask info
    if (s.status() == QDataStream::Ok && !lms.atEnd(isPsb)) {
        lms.globalLayerMaskInfo.size = readSize(s, false); // always 32-bits
        if (lms.globalLayerMaskInfo.size > 0) {
            skip_data(s, lms.globalLayerMaskInfo.size);
        }
    }

    // read additional layer info
    if (s.status() == QDataStream::Ok) {
        for (bool ok = true; ok && !lms.atEnd(isPsb);) {
            auto al = readAdditionalLayer(s, &ok);
            if (ok) {
                lms.additionalLayerInfo.insert(al.id, al);
            }
        }
    }

    device->rollbackTransaction();
    *ok = skip_section(s, isPsb);
    return lms;
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
    if (size < 0) {
        *ok = false;
    } else if (size > 8 * 1024 * 1024) {
        // The known color sections are all in the order of a few hundred bytes.
        // I skip the ones that are too big, I don't know what to do with them.
        *ok = s.skipRawData(size) == size;
    } else if (size != 768) {  // read the duotone data (524 bytes)
        // NOTE: A RGB/Gray float image has a 112 bytes ColorModeData that could be
        //       the "32-bit Toning Options" of Photoshop (starts with 'hdrt').
        //       Official Adobe specification tells "Only indexed color and duotone
        //       (see the mode field in the File header section) have color mode data.".
        //       See test case images 32bit_grayscale.psd and 32bit-rgb.psd
        cms.duotone.data = s.device()->read(size);
        if (cms.duotone.data.size() != size)
            *ok = false;
    } else { // read the palette (768 bytes)
        auto &&palette = cms.palette;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QList<quint8> vect(size);
#else
        QVector<quint8> vect(size);
#endif
        for (auto &&v : vect)
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
static bool setColorSpace(QImage &img, const PSDImageResourceSection &irs)
{
    if (!irs.contains(IRI_ICCPROFILE) || img.isNull())
        return false;
    auto irb = irs.value(IRI_ICCPROFILE);
    auto cs = QColorSpace::fromIccProfile(irb.data);
    if (!cs.isValid())
        return false;

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    if (cs.colorModel() == QColorSpace::ColorModel::Gray && img.pixelFormat().colorModel() != QPixelFormat::Grayscale) {
        // I created an RGB from a grayscale without using color profile conversion (fast).
        // I'll try to create an RGB profile that looks the same.
        if (cs.transferFunction() != QColorSpace::TransferFunction::Custom) {
            auto tmp = QColorSpace(QColorSpace::Primaries::SRgb, cs.transferFunction(), cs.gamma());
            tmp.setWhitePoint(cs.whitePoint());
            tmp.setDescription(QStringLiteral("RGB emulation of \"%1\"").arg(cs.description()));
            if (tmp.isValid())
                cs = tmp;
        }
    }
#endif

    img.setColorSpace(cs);
    return img.colorSpace().isValid();
}

/*!
 * \brief setXmpData
 * Adds XMP metadata to QImage.
 * \param img The image.
 * \param irs The image resource section.
 * \return True on success, otherwise false.
 */
static bool setXmpData(QImage &img, const PSDImageResourceSection &irs)
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
    img.setText(QStringLiteral(META_KEY_XMP_ADOBE), xmp);
    return true;
}

/*!
 * \brief setExifData
 * Adds EXIF metadata to QImage.
 * \param img The image.
 * \param exif The decoded EXIF data.
 * \return True on success, otherwise false.
 */
static bool setExifData(QImage &img, const MicroExif &exif)
{
    if (exif.isEmpty())
        return false;
    exif.updateImageMetadata(img);
    return true;
}

/*!
 * \brief HasMergedData
 * Checks if merged image data are available.
 * \param irs The image resource section.
 * \return True on success or if the block does not exist, otherwise false.
 */
static bool HasMergedData(const PSDImageResourceSection &irs)
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
static bool setResolution(QImage &img, const PSDImageResourceSection &irs)
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
    auto hres = dpi2ppm(fixedPointToDouble(i32));

    s.skipRawData(4);                       // Display data (not used here)

    s >> i32;                               // Vertical resolution in pixels per inch.
    if (i32 <= 0)
        return false;
    auto vres = dpi2ppm(fixedPointToDouble(i32));

    if (hres > 0) {
        img.setDotsPerMeterX(hres);
    }
    if (vres > 0) {
        img.setDotsPerMeterY(vres);
    }
    return true;
}

/*!
 * \brief setTransparencyIndex
 * Search for transparency index block and, if found, changes the alpha of the value at the given index.
 * \param img The image.
 * \param irs The image resource section.
 * \return True on success, otherwise false.
 */
static bool setTransparencyIndex(QImage &img, const PSDImageResourceSection &irs)
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
        auto &&v = palette[idx];
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

// Check that the header is a valid PSD (as written in the PSD specification).
static bool IsValid(const PSDHeader &header)
{
    if (header.signature != 0x38425053) { // '8BPS'
        // qCDebug(LOG_PSDPLUGIN) << "PSD header: invalid signature" << header.signature;
        return false;
    }
    if (header.version != 1 && header.version != 2) {
        qCDebug(LOG_PSDPLUGIN) << "PSD header: invalid version" << header.version;
        return false;
    }
    if (header.depth != 8 &&
        header.depth != 16 &&
        header.depth != 32 &&
        header.depth != 1) {
        qCDebug(LOG_PSDPLUGIN) << "PSD header: invalid depth" << header.depth;
        return false;
    }
    if (header.color_mode != CM_RGB &&
        header.color_mode != CM_GRAYSCALE &&
        header.color_mode != CM_INDEXED &&
        header.color_mode != CM_DUOTONE &&
        header.color_mode != CM_CMYK &&
        header.color_mode != CM_LABCOLOR &&
        header.color_mode != CM_MULTICHANNEL &&
        header.color_mode != CM_BITMAP) {
        qCDebug(LOG_PSDPLUGIN) << "PSD header: invalid color mode" << header.color_mode;
        return false;
    }
    // Specs tells: "Supported range is 1 to 56" but when the alpha channel is present the limit is 57:
    // Photoshop does not make you add more (see also 53alphas.psd test case).
    if (header.channel_count < 1 || header.channel_count > 57) {
        qCDebug(LOG_PSDPLUGIN) << "PSD header: invalid number of channels" << header.channel_count;
        return false;
    }
    if (header.width > uint(std::min(300000, PSD_MAX_IMAGE_WIDTH)) || header.height > uint(std::min(300000, PSD_MAX_IMAGE_HEIGHT))) {
        qCDebug(LOG_PSDPLUGIN) << "PSD header: invalid image size" << header.width << "x" << header.height;
        return false;
    }
    return true;
}

// Check that the header is supported by this plugin.
static bool IsSupported(const PSDHeader &header)
{
    if (!IsValid(header)) {
        return false;
    }
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
        header.color_mode != CM_CMYK &&
        header.color_mode != CM_MULTICHANNEL &&
        header.color_mode != CM_LABCOLOR &&
        header.color_mode != CM_BITMAP) {
        return false;
    }
    return true;
}

/*!
 * \brief imageFormat
 * \param header The PSD header.
 * \return The Qt image format.
 */
static QImage::Format imageFormat(const PSDHeader &header, bool alpha)
{
    if (header.channel_count == 0) {
        return QImage::Format_Invalid;
    }

    auto format = QImage::Format_Invalid;
    switch(header.color_mode) {
    case CM_RGB:
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
        if (header.depth == 32) {
            format = header.channel_count < 4 || !alpha ? QImage::Format_RGBX32FPx4 : QImage::Format_RGBA32FPx4_Premultiplied;
        } else if (header.depth == 16) {
#else
        if (header.depth == 16 || header.depth == 32) {
#endif
            format = header.channel_count < 4 || !alpha ? QImage::Format_RGBX64 : QImage::Format_RGBA64_Premultiplied;
        } else {
            format = header.channel_count < 4 || !alpha ? QImage::Format_RGB888 : QImage::Format_RGBA8888_Premultiplied;
        }
        break;
    case CM_MULTICHANNEL: // Treat MCH as CMYK or Grayscale
    case CM_CMYK: // Photoshop supports CMYK/MCH 8-bits and 16-bits only
        if (NATIVE_CMYK && header.channel_count == 4 && (header.depth == 16 || header.depth == 8)) {
            format = CMYK_FORMAT;
        } else if (header.depth == 16) {
            if (header.channel_count == 1)
                format = QImage::Format_Grayscale16;
            else
                format = header.channel_count < 5 || !alpha ? QImage::Format_RGBX64 : QImage::Format_RGBA64;
        } else if (header.depth == 8) {
            if (header.channel_count == 1)
                format = QImage::Format_Grayscale8;
            else
                format = header.channel_count < 5 || !alpha ? QImage::Format_RGB888 : QImage::Format_RGBA8888;
        }
        break;
    case CM_LABCOLOR: // Photoshop supports LAB 8-bits and 16-bits only
        if (header.depth == 16) {
            format = header.channel_count < 4 || !alpha ? QImage::Format_RGBX64 : QImage::Format_RGBA64;
        } else if (header.depth == 8) {
            format = header.channel_count < 4 || !alpha ? QImage::Format_RGB888 : QImage::Format_RGBA8888;
        }
        break;
    case CM_GRAYSCALE:
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
        if (header.depth == 32) {
            format = !alpha ? QImage::Format_RGBX32FPx4 : QImage::Format_RGBA32FPx4_Premultiplied;
        } else if (header.depth == 16) {
#else
        if (header.depth == 32 || header.depth == 16) {
#endif
            format = !alpha ? QImage::Format_Grayscale16 : QImage::Format_RGBA64_Premultiplied;
        } else {
            format = !alpha ? QImage::Format_Grayscale8 : QImage::Format_RGBA8888_Premultiplied;
        }
        break;
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
static qint32 imageChannels(const QImage::Format &format)
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

inline quint8 xchg(quint8 v)
{
    return v;
}

inline quint16 xchg(quint16 v)
{
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    return quint16( (v>>8) | (v<<8) );
#else
    return v;   // never tested
#endif
}

inline quint32 xchg(quint32 v)
{
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    return quint32( (v>>24) | ((v & 0x00FF0000)>>8) | ((v & 0x0000FF00)<<8) | (v<<24) );
#else
    return v;  // never tested
#endif
}

inline float xchg(float v)
{
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
#   ifdef Q_CC_MSVC
    float *pf = &v;
    quint32 f = xchg(*reinterpret_cast<quint32*>(pf));
    quint32 *pi = &f;
    return *reinterpret_cast<float*>(pi);
#   else
    quint32 t;
    std::memcpy(&t, &v, sizeof(quint32));
    t = xchg(t);
    std::memcpy(&v, &t, sizeof(quint32));
    return v;
#   endif
#else
    return v;  // never tested
#endif
}

template<class T>
inline void planarToChunchy(uchar *target, const char *source, qint32 width, qint32 c, qint32 cn)
{
    auto s = reinterpret_cast<const T*>(source);
    auto t = reinterpret_cast<T*>(target);
    for (qint32 x = 0; x < width; ++x) {
        t[x * cn + c] = xchg(s[x]);
    }
}

template<class T>
inline void planarToChunchyCMYK(uchar *target, const char *source, qint32 width, qint32 c, qint32 cn)
{
    auto s = reinterpret_cast<const T*>(source);
    auto t = reinterpret_cast<quint8*>(target);
    const T d = std::numeric_limits<T>::max() / std::numeric_limits<quint8>::max();
    for (qint32 x = 0; x < width; ++x) {
        t[x * cn + c] = quint8((std::numeric_limits<T>::max() - xchg(s[x])) / d);
    }
}


template<class T>
inline void planarToChunchyFloatToUInt16(uchar *target, const char *source, qint32 width, qint32 c, qint32 cn)
{
    auto s = reinterpret_cast<const T*>(source);
    auto t = reinterpret_cast<quint16*>(target);
    for (qint32 x = 0; x < width; ++x) {
        t[x * cn + c] = quint16(std::min(xchg(s[x]) * std::numeric_limits<quint16>::max() + 0.5, double(std::numeric_limits<quint16>::max())));
    }
}

enum class PremulConversion {
    PS2P, // Photoshop premul to qimage premul (required by RGB)
    PS2A, // Photoshop premul to unassociated alpha (required by RGB, CMYK and L* components of LAB)
    PSLab2A // Photoshop premul to unassociated alpha (required by a* and b* components of LAB)
};

template<class T>
inline void premulConversion(char *stride, qint32 width, qint32 ac, qint32 cn, const PremulConversion &conv)
{
    auto s = reinterpret_cast<T*>(stride);
    // NOTE: to avoid overflows, max is casted to qint64: that is possible because max is always an integer (even if T is float)
    auto max = qint64(std::numeric_limits<T>::is_integer ? std::numeric_limits<T>::max() : 1);

    for (qint32 c = 0; c < ac; ++c) {
        if (conv == PremulConversion::PS2P) {
            for (qint32 x = 0; x < width; ++x) {
                auto xcn = x * cn;
                auto alpha = *(s + xcn + ac);
                *(s + xcn + c) = *(s + xcn + c) + alpha - max;
            }
        } else if (conv == PremulConversion::PS2A || (conv == PremulConversion::PSLab2A && c == 0)) {
            for (qint32 x = 0; x < width; ++x) {
                auto xcn = x * cn;
                auto alpha = *(s + xcn + ac);
                if (alpha > 0)
                    *(s + xcn + c) = ((*(s + xcn + c) + alpha - max) * max + alpha / 2) / alpha;
            }
        } else if (conv == PremulConversion::PSLab2A) {
            for (qint32 x = 0; x < width; ++x) {
                auto xcn = x * cn;
                auto alpha = *(s + xcn + ac);
                if (alpha > 0)
                    *(s + xcn + c) = ((*(s + xcn + c) + (alpha - max + 1) / 2) * max + alpha / 2) / alpha;
            }
        }
    }
}

inline void monoInvert(uchar *target, const char* source, qint32 bytes)
{
    auto s = reinterpret_cast<const quint8*>(source);
    auto t = reinterpret_cast<quint8*>(target);
    for (qint32 x = 0; x < bytes; ++x) {
        t[x] = ~s[x];
    }
}

template<class T>
inline void rawChannelsCopyToCMYK(uchar *target, qint32 targetChannels, const char *source, qint32 sourceChannels, qint32 width)
{
    auto s = reinterpret_cast<const T*>(source);
    auto t = reinterpret_cast<quint8*>(target);
    const T d = std::numeric_limits<T>::max() / std::numeric_limits<quint8>::max();
    for (qint32 c = 0, cs = std::min(targetChannels, sourceChannels); c < cs; ++c) {
        for (qint32 x = 0; x < width; ++x) {
            t[x * targetChannels + c] = (std::numeric_limits<T>::max() - s[x * sourceChannels + c]) / d;
        }
    }
}

template<class T>
inline void rawChannelsCopy(uchar *target, qint32 targetChannels, const char *source, qint32 sourceChannels, qint32 width)
{
    auto s = reinterpret_cast<const T*>(source);
    auto t = reinterpret_cast<T*>(target);
    for (qint32 c = 0, cs = std::min(targetChannels, sourceChannels); c < cs; ++c) {
        for (qint32 x = 0; x < width; ++x) {
            t[x * targetChannels + c] = s[x * sourceChannels + c];
        }
    }
}

template<class T>
inline void rawChannelCopy(uchar *target, qint32 targetChannels, qint32 targetChannel, const char *source, qint32 sourceChannels, qint32 sourceChannel, qint32 width)
{
    auto s = reinterpret_cast<const T*>(source);
    auto t = reinterpret_cast<T*>(target);
    for (qint32 x = 0; x < width; ++x) {
        t[x * targetChannels + targetChannel] = s[x * sourceChannels + sourceChannel];
    }
}


template<class T>
inline bool cmykToRgb(uchar *target, qint32 targetChannels, const char *source, qint32 sourceChannels, qint32 width, bool alpha = false)
{
    auto s = reinterpret_cast<const T*>(source);
    auto t = reinterpret_cast<T*>(target);
    auto max = double(std::numeric_limits<T>::max());
    auto invmax = 1.0 / max; // speed improvements by ~10%

    if (sourceChannels < 2) {
        qCDebug(LOG_PSDPLUGIN) << "cmykToRgb: image is not a valid MCH/CMYK!";
        return false;
    }

    for (qint32 w = 0; w < width; ++w) {
        auto ps = s + sourceChannels * w;
        auto C = 1 - *(ps + 0) * invmax;
        auto M = sourceChannels > 1 ? 1 - *(ps + 1) * invmax : 0.0;
        auto Y = sourceChannels > 2 ? 1 - *(ps + 2) * invmax : 0.0;
        auto K = sourceChannels > 3 ? 1 - *(ps + 3) * invmax : 0.0;

        auto pt = t + targetChannels * w;
        *(pt + 0) = T(std::min(max - (C * (1 - K) + K) * max + 0.5, max));
        *(pt + 1) = targetChannels > 1 ? T(std::min(max - (M * (1 - K) + K) * max + 0.5, max)) : std::numeric_limits<T>::max();
        *(pt + 2) = targetChannels > 2 ? T(std::min(max - (Y * (1 - K) + K) * max + 0.5, max)) : std::numeric_limits<T>::max();
        if (targetChannels == 4) {
            if (sourceChannels >= 5 && alpha)
                *(pt + 3) = *(ps + 4);
            else
                *(pt + 3) = std::numeric_limits<T>::max();
        }
    }
    return true;
}

inline double finv(double v)
{
    return (v > 6.0 / 29.0 ? v * v * v : (v - 16.0 / 116.0) / 7.787);
}

inline double gammaCorrection(double linear)
{
#ifdef PSD_FAST_LAB_CONVERSION
    return linear;
#else
    // Replacing fastPow with std::pow the conversion time is 2/3 times longer: using fastPow
    // there are minimal differences in the conversion that are not visually noticeable.
    return (linear > 0.0031308 ? 1.055 * fastPow(linear, 1.0 / 2.4) - 0.055 : 12.92 * linear);
#endif
}

template<class T>
inline bool labToRgb(uchar *target, qint32 targetChannels, const char *source, qint32 sourceChannels, qint32 width, bool alpha = false)
{
    auto s = reinterpret_cast<const T*>(source);
    auto t = reinterpret_cast<T*>(target);
    auto max = double(std::numeric_limits<T>::max());
    auto invmax = 1.0 / max;

    if (sourceChannels < 3) {
        qCDebug(LOG_PSDPLUGIN) << "labToRgb: image is not a valid LAB!";
        return false;
    }

    for (qint32 w = 0; w < width; ++w) {
        auto ps = s + sourceChannels * w;
        auto L = (*(ps + 0) * invmax) * 100.0;
        auto A = (*(ps + 1) * invmax) * 255.0 - 128.0;
        auto B = (*(ps + 2) * invmax) * 255.0 - 128.0;

        // converting LAB to XYZ (D65 illuminant)
        auto Y = (L + 16.0) * (1.0 / 116.0);
        auto X = A * (1.0 / 500.0) + Y;
        auto Z = Y - B * (1.0 / 200.0);

        // NOTE: use the constants of the illuminant of the target RGB color space
        X = finv(X) * 0.9504;   // D50: * 0.9642
        Y = finv(Y) * 1.0000;   // D50: * 1.0000
        Z = finv(Z) * 1.0888;   // D50: * 0.8251

        // converting XYZ to sRGB (sRGB illuminant is D65)
        auto r = gammaCorrection(  3.24071   * X - 1.53726  * Y - 0.498571  * Z);
        auto g = gammaCorrection(- 0.969258  * X + 1.87599  * Y + 0.0415557 * Z);
        auto b = gammaCorrection(  0.0556352 * X - 0.203996 * Y + 1.05707   * Z);

        auto pt = t + targetChannels * w;
        *(pt + 0) = T(std::max(std::min(r * max + 0.5, max), 0.0));
        *(pt + 1) = T(std::max(std::min(g * max + 0.5, max), 0.0));
        *(pt + 2) = T(std::max(std::min(b * max + 0.5, max), 0.0));
        if (targetChannels == 4) {
            if (sourceChannels >= 4 && alpha)
                *(pt + 3) = *(ps + 3);
            else
                *(pt + 3) = std::numeric_limits<T>::max();
        }
    }
    return true;
}

bool readChannel(QByteArray &target, QDataStream &stream, quint32 compressedSize, quint16 compression)
{
    if (compression) {
        if (compressedSize > kMaxQVectorSize) {
            return false;
        }
        QByteArray tmp;
        tmp.resize(compressedSize);
        if (stream.readRawData(tmp.data(), tmp.size()) != tmp.size()) {
            return false;
        }
        if (packbitsDecompress(tmp.data(), tmp.size(), target.data(), target.size()) != target.size()) {
            return false;
        }
    } else if (stream.readRawData(target.data(), target.size()) != target.size()) {
        return false;
    }

    return stream.status() == QDataStream::Ok;
}

} // Private

class PSDHandlerPrivate
{
public:
    PSDHandlerPrivate()
    {
    }
    ~PSDHandlerPrivate()
    {
    }

    bool isPsb() const
    {
        return m_header.version == 2;
    }

    bool isValid() const
    {
        return IsValid(m_header);
    }

    bool isSupported() const
    {
        return IsSupported(m_header);
    }

    bool hasAlpha() const
    {
        // Try to identify the nature of spots: note that this is just one of many ways to identify the presence
        // of alpha channels: should work in most cases where colorspaces != RGB/Gray
#ifdef PSD_FORCE_RGBA
        auto alpha = m_header.color_mode == CM_RGB;
#else
        auto alpha = false;
#endif
        if (m_irs.contains(IRI_ALPHAIDENTIFIERS)) {
            auto irb = m_irs.value(IRI_ALPHAIDENTIFIERS);
            if (irb.data.size() >= 4) {
                QDataStream s(irb.data);
                s.setByteOrder(QDataStream::BigEndian);
                qint32 v;
                s >> v;
                alpha = v == 0;
            }
        } else if (!m_lms.isNull()) {
            alpha = m_lms.hasAlpha();
        }
        return alpha;
    }

    bool hasMergedData() const
    {
        return HasMergedData(m_irs);
    }

    QSize size() const
    {
        if (isValid())
            return QSize(m_header.width, m_header.height);
        return {};
    }

    QImage::Format format() const
    {
        return imageFormat(m_header, hasAlpha());
    }

    QImageIOHandler::Transformations transformation() const
    {
        return m_exif.transformation();
    }

    bool readInfo(QDataStream &stream)
    {
        auto ok = false;

        // Header
        stream >> m_header;

        // Check image file format.
        if (stream.atEnd() || !IsValid(m_header)) {
            // qCDebug(LOG_PSDPLUGIN) << "This PSD file is not valid.";
            return false;
        }

        // Check if it's a supported format.
        if (!IsSupported(m_header)) {
            // qCDebug(LOG_PSDPLUGIN) << "This PSD file is not supported.";
            return false;
        }

        // Color Mode Data section
        m_cmds = readColorModeDataSection(stream, &ok);
        if (!ok) {
            qCDebug(LOG_PSDPLUGIN) << "Error while skipping Color Mode Data section";
            return false;
        }

        // Image Resources Section
        m_irs = readImageResourceSection(stream, &ok);
        if (!ok) {
            qCDebug(LOG_PSDPLUGIN) << "Error while reading Image Resources Section";
            return false;
        }
        // Checking for merged image (Photoshop compatibility data)
        if (!hasMergedData()) {
            qCDebug(LOG_PSDPLUGIN) << "No merged data found";
            return false;
        }

        // Layer and Mask section
        m_lms = readLayerAndMaskSection(stream, isPsb(), &ok);
        if (!ok) {
            qCDebug(LOG_PSDPLUGIN) << "Error while skipping Layer and Mask section";
            return false;
        }

        // storing decoded EXIF
        if (m_irs.contains(IRI_EXIFDATA1)) {
            m_exif = MicroExif::fromByteArray(m_irs.value(IRI_EXIFDATA1).data);
        }

        return ok;
    }

    PSDHeader m_header;
    PSDColorModeDataSection m_cmds;
    PSDImageResourceSection m_irs;
    PSDLayerAndMaskSection m_lms;

    // cache to avoid decoding exif multiple times
    MicroExif m_exif;
};

PSDHandler::PSDHandler()
    : QImageIOHandler()
    , d(new PSDHandlerPrivate)
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
    QDataStream stream(device());
    stream.setByteOrder(QDataStream::BigEndian);

    if (!d->isValid()) {
        if (!d->readInfo(stream))
            return false;
    }

    auto &&header = d->m_header;
    auto &&cmds = d->m_cmds;
    auto &&irs = d->m_irs;
    // auto &&lms = d->m_lms;
    auto isPsb = d->isPsb();
    auto alpha = d->hasAlpha();

    QImage img;
    // Find out if the data is compressed.
    // Known values:
    //   0: no compression
    //   1: RLE compressed
    quint16 compression;
    stream >> compression;
    if (compression > 1) {
        qCDebug(LOG_PSDPLUGIN) << "Unknown compression type";
        return false;
    }

    const QImage::Format format = d->format();
    if (format == QImage::Format_Invalid) {
        qCWarning(LOG_PSDPLUGIN) << "Unsupported image format. color_mode:" << header.color_mode << "depth:" << header.depth << "channel_count:" << header.channel_count;
        return false;
    }

    img = imageAlloc(d->size(), format);
    if (img.isNull()) {
        qCWarning(LOG_PSDPLUGIN) << "Failed to allocate image, invalid dimensions?" << QSize(header.width, header.height);
        return false;
    }
    img.fill(qRgb(0, 0, 0));
    if (!cmds.palette.isEmpty()) {
        img.setColorTable(cmds.palette);
        setTransparencyIndex(img, irs);
    }

    auto imgChannels = imageChannels(img.format());
    auto raw_count = qsizetype(header.width * header.depth + 7) / 8;
    auto native_cmyk = img.format() == CMYK_FORMAT;

    if (header.height > kMaxQVectorSize / header.channel_count / sizeof(quint32)) {
        qCWarning(LOG_PSDPLUGIN) << "LoadPSD() header height/channel_count too big" << header.height << header.channel_count;
        return false;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QList<quint32> strides(header.height * header.channel_count, raw_count);
#else
    QVector<quint32> strides(header.height * header.channel_count, raw_count);
#endif
    // Read the compressed stride sizes
    if (compression) {
        for (auto &&v : strides) {
            if (isPsb) {
                stream >> v;
                continue;
            }
            quint16 tmp;
            stream >> tmp;
            v = tmp;
        }
    }
    // calculate the absolute file positions of each stride (required when a colorspace conversion should be done)
    auto device = stream.device();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QList<quint64> stridePositions(strides.size());
#else
    QVector<quint64> stridePositions(strides.size());
#endif
    if (!stridePositions.isEmpty()) {
        stridePositions[0] = device->pos();
    }
    for (qsizetype i = 1, n = stridePositions.size(); i < n; ++i) {
        stridePositions[i] = stridePositions[i-1] + strides.at(i-1);
    }

    // Read the image
    QByteArray rawStride;
    rawStride.resize(raw_count);

    // clang-format off
    // checks the need of color conversion (that requires random access to the image)
    auto randomAccess = (header.color_mode == CM_CMYK && !native_cmyk) ||
                        (header.color_mode == CM_MULTICHANNEL && header.channel_count != 1 && !native_cmyk) ||
                        (header.color_mode == CM_LABCOLOR) ||
                        (header.color_mode != CM_INDEXED && img.hasAlphaChannel());
    // clang-format on

    if (randomAccess) {
        // CMYK with spots (e.g. CMYKA) ICC conversion to RGBA/RGBX
        QImage tmpCmyk;
        ScanLineConverter iccConv(img.format());
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0) && !defined(PSD_NATIVE_CMYK_SUPPORT_DISABLED)
        if (header.color_mode == CM_CMYK && img.format() != QImage::Format_CMYK8888) {
            auto tmpi = QImage(header.width, 1, QImage::Format_CMYK8888);
            if (setColorSpace(tmpi, irs)) {
                tmpi.fill(0);
                tmpCmyk = tmpi;
            }
            iccConv.setTargetColorSpace(QColorSpace(QColorSpace::SRgb));
        }
#endif

        // In order to make a colorspace transformation, we need all channels of a scanline
        QByteArray psdScanline;
        psdScanline.resize(qsizetype(header.width * header.depth * header.channel_count + 7) / 8);
        for (qint32 y = 0, h = header.height; y < h; ++y) {
            for (qint32 c = 0; c < header.channel_count; ++c) {
                auto strideNumber = c * qsizetype(h) + y;
                if (!device->seek(stridePositions.at(strideNumber))) {
                    qCDebug(LOG_PSDPLUGIN) << "Error while seeking the stream of channel" << c << "line" << y;
                    return false;
                }
                auto &&strideSize = strides.at(strideNumber);
                if (!readChannel(rawStride, stream, strideSize, compression)) {
                    qCDebug(LOG_PSDPLUGIN) << "Error while reading the stream of channel" << c << "line" << y;
                    return false;
                }

                auto scanLine = reinterpret_cast<unsigned char*>(psdScanline.data());
                if (header.depth == 8) {
                    planarToChunchy<quint8>(scanLine, rawStride.data(), header.width, c, header.channel_count);
                } else if (header.depth == 16) {
                    planarToChunchy<quint16>(scanLine, rawStride.data(), header.width, c, header.channel_count);
                } else if (header.depth == 32) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
                    planarToChunchy<float>(scanLine, rawStride.data(), header.width, c, header.channel_count);
#else
                    planarToChunchyFloatToUInt16<float>(scanLine, rawStride.data(), header.width, c, header.channel_count);
#endif
                }
            }

            // Convert premultiplied data to unassociated data
            if (img.hasAlphaChannel()) {
                auto scanLine = reinterpret_cast<char*>(psdScanline.data());
                if (header.color_mode == CM_CMYK) {
                    if (header.depth == 8)
                        premulConversion<quint8>(scanLine, header.width, 4, header.channel_count, PremulConversion::PS2A);
                    else if (header.depth == 16)
                        premulConversion<quint16>(scanLine, header.width, 4, header.channel_count, PremulConversion::PS2A);
                }
                if (header.color_mode == CM_LABCOLOR) {
                    if (header.depth == 8)
                        premulConversion<quint8>(scanLine, header.width, 3, header.channel_count, PremulConversion::PSLab2A);
                    else if (header.depth == 16)
                        premulConversion<quint16>(scanLine, header.width, 3, header.channel_count, PremulConversion::PSLab2A);
                }
                if (header.color_mode == CM_RGB) {
                    if (header.depth == 8)
                        premulConversion<quint8>(scanLine, header.width, 3, header.channel_count, PremulConversion::PS2P);
                    else if (header.depth == 16)
                        premulConversion<quint16>(scanLine, header.width, 3, header.channel_count, PremulConversion::PS2P);
                    else if (header.depth == 32)
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
                        premulConversion<float>(scanLine, header.width, 3, header.channel_count, PremulConversion::PS2P);
#else
                        premulConversion<quint16>(scanLine, header.width, 3, header.channel_count, PremulConversion::PS2P);
#endif
                }
                if (header.color_mode == CM_GRAYSCALE) {
                    if (header.depth == 8)
                        premulConversion<quint8>(scanLine, header.width, 1, header.channel_count, PremulConversion::PS2P);
                    else if (header.depth == 16)
                        premulConversion<quint16>(scanLine, header.width, 1, header.channel_count, PremulConversion::PS2P);
                    else if (header.depth == 32)
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
                        premulConversion<float>(scanLine, header.width, 1, header.channel_count, PremulConversion::PS2P);
#else
                        premulConversion<quint16>(scanLine, header.width, 1, header.channel_count, PremulConversion::PS2P);
#endif
                }
            }

            // Conversion to RGB
            if (header.color_mode == CM_CMYK || header.color_mode == CM_MULTICHANNEL) {
                if (tmpCmyk.isNull()) {
                    if (header.depth == 8) {
                        if (!cmykToRgb<quint8>(img.scanLine(y), imgChannels, psdScanline.data(), header.channel_count, header.width, alpha))
                            return false;
                    } else if (header.depth == 16) {
                        if (!cmykToRgb<quint16>(img.scanLine(y), imgChannels, psdScanline.data(), header.channel_count, header.width, alpha))
                            return false;
                    }
                } else if (header.depth == 8) {
                    rawChannelsCopyToCMYK<quint8>(tmpCmyk.bits(), 4, psdScanline.data(), header.channel_count, header.width);
                    if (auto rgbPtr = iccConv.convertedScanLine(tmpCmyk, 0))
                        std::memcpy(img.scanLine(y), rgbPtr, img.bytesPerLine());
                    if (imgChannels == 4 && header.channel_count >= 5)
                        rawChannelCopy<quint8>(img.scanLine(y), imgChannels, 3, psdScanline.data(), header.channel_count, 4, header.width);
                } else if (header.depth == 16) {
                    rawChannelsCopyToCMYK<quint16>(tmpCmyk.bits(), 4, psdScanline.data(), header.channel_count, header.width);
                    if (auto rgbPtr = iccConv.convertedScanLine(tmpCmyk, 0))
                        std::memcpy(img.scanLine(y), rgbPtr, img.bytesPerLine());
                    if (imgChannels == 4 && header.channel_count >= 5)
                        rawChannelCopy<quint16>(img.scanLine(y), imgChannels, 3, psdScanline.data(), header.channel_count, 4, header.width);
                }
            }
            if (header.color_mode == CM_LABCOLOR) {
                if (header.depth == 8) {
                    if (!labToRgb<quint8>(img.scanLine(y), imgChannels, psdScanline.data(), header.channel_count, header.width, alpha))
                        return false;
                } else if (header.depth == 16) {
                    if (!labToRgb<quint16>(img.scanLine(y), imgChannels, psdScanline.data(), header.channel_count, header.width, alpha))
                        return false;
                }
            }
            if (header.color_mode == CM_RGB) {
                if (header.depth == 8)
                    rawChannelsCopy<quint8>(img.scanLine(y), imgChannels, psdScanline.data(), header.channel_count, header.width);
                else if (header.depth == 16)
                    rawChannelsCopy<quint16>(img.scanLine(y), imgChannels, psdScanline.data(), header.channel_count, header.width);
                else if (header.depth == 32)
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
                    rawChannelsCopy<float>(img.scanLine(y), imgChannels, psdScanline.data(), header.channel_count, header.width);
#else
                    rawChannelsCopy<quint16>(img.scanLine(y), imgChannels, psdScanline.data(), header.channel_count, header.width);
#endif
            }
            if (header.color_mode == CM_GRAYSCALE) {
                for (auto c = 0; c < imgChannels; ++c) { // GRAYA to RGBA
                    auto sc = qBound(0, c - 2, int(header.channel_count));
                    if (header.depth == 8)
                        rawChannelCopy<quint8>(img.scanLine(y), imgChannels, c, psdScanline.data(), header.channel_count, sc, header.width);
                    else if (header.depth == 16)
                        rawChannelCopy<quint16>(img.scanLine(y), imgChannels, c, psdScanline.data(), header.channel_count, sc, header.width);
                    else if (header.depth == 32)
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
                        rawChannelCopy<float>(img.scanLine(y), imgChannels, c, psdScanline.data(), header.channel_count, sc, header.width);
#else
                        rawChannelCopy<quint16>(img.scanLine(y), imgChannels, c, psdScanline.data(), header.channel_count, sc, header.width);
#endif
                }
            }
        }
    } else {
        // Linear read (no position jumps): optimized code usable only for the colorspaces supported by QImage
        auto channel_num = std::min(qint32(header.channel_count), header.color_mode == CM_GRAYSCALE ? 1 : imgChannels);
        for (qint32 c = 0; c < channel_num; ++c) {
            for (qint32 y = 0, h = header.height; y < h; ++y) {
                auto&& strideSize = strides.at(c * qsizetype(h) + y);
                if (!readChannel(rawStride, stream, strideSize, compression)) {
                    qCDebug(LOG_PSDPLUGIN) << "Error while reading the stream of channel" << c << "line" << y;
                    return false;
                }

                auto scanLine = img.scanLine(y);
                if (header.depth == 1) {
                    // Bitmap
                    monoInvert(scanLine, rawStride.data(), std::min(rawStride.size(), img.bytesPerLine()));
                } else if (header.depth == 8) {
                    // 8-bits images: Indexed, Grayscale, RGB/RGBA, CMYK, MCH1, MCH4
                    if (native_cmyk)
                        planarToChunchyCMYK<quint8>(scanLine, rawStride.data(), header.width, c, imgChannels);
                    else
                        planarToChunchy<quint8>(scanLine, rawStride.data(), header.width, c, imgChannels);
                } else if (header.depth == 16) {
                    // 16-bits integer images: Grayscale, RGB/RGBA, CMYK, MCH1, MCH4
                    if (native_cmyk)
                        planarToChunchyCMYK<quint16>(scanLine, rawStride.data(), header.width, c, imgChannels);
                    else
                        planarToChunchy<quint16>(scanLine, rawStride.data(), header.width, c, imgChannels);
                } else if (header.depth == 32 && header.color_mode == CM_RGB) {
                    // 32-bits float images: RGB/RGBA
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
                    planarToChunchy<float>(scanLine, rawStride.data(), header.width, c, imgChannels);
#else
                    planarToChunchyFloatToUInt16<float>(scanLine, rawStride.data(), header.width, c, imgChannels);
#endif
                } else if (header.depth == 32 && header.color_mode == CM_GRAYSCALE) {
                    if (imgChannels >= 3) { // GRAY to RGB
                        planarToChunchy<float>(scanLine, rawStride.data(), header.width, 0, imgChannels);
                        planarToChunchy<float>(scanLine, rawStride.data(), header.width, 1, imgChannels);
                        planarToChunchy<float>(scanLine, rawStride.data(), header.width, 2, imgChannels);
                    } else { // 32-bits float images: Grayscale (converted to equivalent integer 16-bits)
                        planarToChunchyFloatToUInt16<float>(scanLine, rawStride.data(), header.width, c, imgChannels);
                    }
                }
            }
        }
    }

    // Resolution info
    if (!setResolution(img, irs)) {
        // qCDebug(LOG_PSDPLUGIN) << "No resolution info found!";
    }

    // ICC profile
    if (header.color_mode == CM_LABCOLOR) {
        // LAB conversion generates a sRGB image
#ifdef PSD_FAST_LAB_CONVERSION
        img.setColorSpace(QColorSpace(QColorSpace::SRgbLinear));
#else
        img.setColorSpace(QColorSpace(QColorSpace::SRgb));
#endif
    } else if (!setColorSpace(img, irs)) {
        // Float images are used by Photoshop as linear: if no color space
        // is present, a linear one should be chosen.
        if (header.color_mode == CM_RGB && header.depth == 32) {
            img.setColorSpace(QColorSpace(QColorSpace::SRgbLinear));
        }
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
        if (header.color_mode == CM_GRAYSCALE && header.depth == 32) {
            auto qs = QColorSpace(QPointF(0.3127, 0.3291), QColorSpace::TransferFunction::Linear);
            qs.setDescription(QStringLiteral("Linear grayscale"));
            img.setColorSpace(qs);
        }
#endif
    }

    // XMP data
    if (!setXmpData(img, irs)) {
        // qCDebug(LOG_PSDPLUGIN) << "No XMP data found!";
    }

    // EXIF data
    if (!setExifData(img, d->m_exif)) {
        // qCDebug(LOG_PSDPLUGIN) << "No EXIF data found!";
    }

    // Duotone images: color data contains the duotone specification (not documented).
    // Other applications that read Photoshop files can treat a duotone image as a gray image,
    // and just preserve the contents of the duotone information when reading and writing the file.
    if (!cmds.duotone.data.isEmpty()) {
        img.setText(QStringLiteral("PSDDuotoneOptions"), QString::fromUtf8(cmds.duotone.data.toHex()));
    }

    *image = img;
    return true;
}

bool PSDHandler::supportsOption(ImageOption option) const
{
    if (option == QImageIOHandler::Size)
        return true;
    if (option == QImageIOHandler::ImageFormat)
        return true;
    if (option == QImageIOHandler::ImageTransformation)
        return true;
    if (option == QImageIOHandler::Description)
        return true;
    return false;
}

QVariant PSDHandler::option(ImageOption option) const
{
    QVariant v;

    if (auto dev = device()) {
        if (!d->isValid()) {
            QDataStream s(dev);
            s.setByteOrder(QDataStream::BigEndian);
            d->readInfo(s);
        }
    }

    if (option == QImageIOHandler::Size) {
        if (d->isValid()) {
            v = QVariant::fromValue(d->size());
        }
    }

    if (option == QImageIOHandler::ImageFormat) {
        if (d->isValid()) {
            v = QVariant::fromValue(d->format());
        }
    }

    if (option == QImageIOHandler::ImageTransformation) {
        if (d->isValid()) {
            v = QVariant::fromValue(int(d->transformation()));
        }
    }

    if (option == QImageIOHandler::Description) {
        if (d->isValid()) {
            auto descr = d->m_exif.description();
            if (!descr.isEmpty())
                v = QVariant::fromValue(descr);
        }
    }

    return v;
}

bool PSDHandler::canRead(QIODevice *device)
{
    if (!device) {
        qCWarning(LOG_PSDPLUGIN) << "PSDHandler::canRead() called with no device";
        return false;
    }

    auto ba = device->peek(sizeof(PSDHeader));
    QDataStream s(ba);
    s.setByteOrder(QDataStream::BigEndian);

    PSDHeader header;
    s >> header;

    if (s.status() != QDataStream::Ok) {
        return false;
    }

    if (device->isSequential()) {
        if (header.color_mode == CM_CMYK || header.color_mode == CM_MULTICHANNEL) {
            if (header.channel_count != 4 || !NATIVE_CMYK)
                return false;
        }
        if (header.color_mode == CM_LABCOLOR) {
            return false;
        }
        if (header.color_mode == CM_RGB && header.channel_count > 3) {
            return false; // supposing extra channel as alpha
        }
        if (header.color_mode == CM_GRAYSCALE && (header.channel_count > 1 || header.depth == 32)) {
            return false; // supposing extra channel as alpha
        }
    }

    return IsSupported(header);
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

#include "moc_psd_p.cpp"
