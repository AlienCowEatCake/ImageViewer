/*
    The high dynamic range EXR format support for QImage.

    SPDX-FileCopyrightText: 2003 Brad Hards <bradh@frogmouth.net>
    SPDX-FileCopyrightText: 2023 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/* *** EXR_USE_LEGACY_CONVERSIONS ***
 * If defined, the result image is an 8-bit RGB(A) converted
 * without icc profiles. Otherwise, a 16-bit images is generated.
 * NOTE: The use of legacy conversions are discouraged due to
 *       imprecise image result.
 */
//#define EXR_USE_LEGACY_CONVERSIONS // default commented -> you should define it in your cmake file

/* *** EXR_CONVERT_TO_SRGB ***
 * If defined, the linear data is converted to sRGB on read to accommodate
 * programs that do not support color profiles.
 * Otherwise the data are kept as is and it is the display program that
 * must convert to the monitor profile.
 * NOTE: If EXR_USE_LEGACY_CONVERSIONS is active, this is ignored.
 */
//#define EXR_CONVERT_TO_SRGB // default: commented -> you should define it in your cmake file

/* *** EXR_STORE_XMP_ATTRIBUTE ***
 * If defined, disables the stores XMP values in a non-standard attribute named "xmp".
 * The QImage metadata used is "XML:com.adobe.xmp".
 * NOTE: The use of non-standard attributes is possible but discouraged by the specification. However,
 *       metadata is essential for good image management and programs like darktable also set this
 *       attribute. Gimp reads the "xmp" attribute and Darktable writes it as well.
 */
//#define EXR_DISABLE_XMP_ATTRIBUTE // default: commented -> you should define it in your cmake file

/* *** EXR_MAX_IMAGE_WIDTH and EXR_MAX_IMAGE_HEIGHT ***
 * The maximum size in pixel allowed by the plugin.
 */
#ifndef EXR_MAX_IMAGE_WIDTH
#define EXR_MAX_IMAGE_WIDTH 300000
#endif
#ifndef EXR_MAX_IMAGE_HEIGHT
#define EXR_MAX_IMAGE_HEIGHT EXR_MAX_IMAGE_WIDTH
#endif

/* *** EXR_LINES_PER_BLOCK ***
 * Allows certain compression schemes to work in multithreading
 * Requires up to "LINES_PER_BLOCK * MAX_IMAGE_WIDTH * 8"
 * additional RAM (e.g. if 128, up to 307MiB of RAM).
 * There is a performance gain with the following parameters (based on empirical tests):
 * - PIZ compression needs 64+ lines
 * - ZIPS compression needs 8+ lines
 * - ZIP compression needs 32+ lines
 * - Others not tested
 *
 * NOTE: The OpenEXR documentation states that the higher the better :)
 */
#ifndef EXR_LINES_PER_BLOCK
#define EXR_LINES_PER_BLOCK 128
#endif

#include "exr_p.h"
#include "scanlineconverter_p.h"
#include "util_p.h"

#include <IexThrowErrnoExc.h>
#include <ImathBox.h>
#include <ImfArray.h>
#include <ImfBoxAttribute.h>
#include <ImfChannelListAttribute.h>
#include <ImfCompressionAttribute.h>
#include <ImfConvert.h>
#include <ImfFloatAttribute.h>
#include <ImfInputFile.h>
#include <ImfInt64.h>
#include <ImfIntAttribute.h>
#include <ImfLineOrderAttribute.h>
#include <ImfPreviewImage.h>
#include <ImfRgbaFile.h>
#include <ImfStandardAttributes.h>
#include <ImfVersion.h>

#include <iostream>

#include <QColorSpace>
#include <QDataStream>
#include <QDebug>
#include <QFloat16>
#include <QImage>
#include <QImageIOPlugin>
#include <QLocale>
#include <QThread>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QTimeZone>
#endif

// Allow the code to works on all QT versions supported by KDE
// project (Qt 5.15 and Qt 6.x) to easy backports fixes.
#if (QT_VERSION_MAJOR >= 6) && !defined(EXR_USE_LEGACY_CONVERSIONS)
// If uncommented, the image is rendered in a float16 format, the result is very precise
#define EXR_USE_QT6_FLOAT_IMAGE // default uncommented
#endif

class K_IStream : public Imf::IStream
{
public:
    K_IStream(QIODevice *dev, const QByteArray &fileName)
        : IStream(fileName.data())
        , m_dev(dev)
    {
    }

    bool read(char c[], int n) override;
#if OPENEXR_VERSION_MAJOR > 2
    uint64_t tellg() override;
    void seekg(uint64_t pos) override;
#else
    Imf::Int64 tellg() override;
    void seekg(Imf::Int64 pos) override;
#endif
    void clear() override;

private:
    QIODevice *m_dev;
};

bool K_IStream::read(char c[], int n)
{
    qint64 result = m_dev->read(c, n);
    if (result > 0) {
        return true;
    } else if (result == 0) {
        throw Iex::InputExc("Unexpected end of file");
    } else { // negative value {
        Iex::throwErrnoExc("Error in read", result);
    }
    return false;
}

#if OPENEXR_VERSION_MAJOR > 2
uint64_t K_IStream::tellg()
#else
Imf::Int64 K_IStream::tellg()
#endif
{
    return m_dev->pos();
}

#if OPENEXR_VERSION_MAJOR > 2
void K_IStream::seekg(uint64_t pos)
#else
void K_IStream::seekg(Imf::Int64 pos)
#endif
{
    m_dev->seek(pos);
}

void K_IStream::clear()
{
    // TODO
}

class K_OStream : public Imf::OStream
{
public:
    K_OStream(QIODevice *dev, const QByteArray &fileName)
        : OStream(fileName.data())
        , m_dev(dev)
    {
    }

    void write(const char c[/*n*/], int n) override;
#if OPENEXR_VERSION_MAJOR > 2
    uint64_t tellp() override;
    void seekp(uint64_t pos) override;
#else
    Imf::Int64 tellp() override;
    void seekp(Imf::Int64 pos) override;
#endif

private:
    QIODevice *m_dev;
};

void K_OStream::write(const char c[], int n)
{
    qint64 result = m_dev->write(c, n);
    if (result > 0) {
        return;
    } else { // negative value {
        Iex::throwErrnoExc("Error in write", result);
    }
    return;
}

#if OPENEXR_VERSION_MAJOR > 2
uint64_t K_OStream::tellp()
#else
Imf::Int64 K_OStream::tellp()
#endif
{
    return m_dev->pos();
}

#if OPENEXR_VERSION_MAJOR > 2
void K_OStream::seekp(uint64_t pos)
#else
void K_OStream::seekp(Imf::Int64 pos)
#endif
{
    m_dev->seek(pos);
}

#ifdef EXR_USE_LEGACY_CONVERSIONS
// source: https://openexr.com/en/latest/ReadingAndWritingImageFiles.html
inline unsigned char gamma(float x)
{
    x = std::pow(5.5555f * std::max(0.f, x), 0.4545f) * 84.66f;
    return (unsigned char)qBound(0.f, x, 255.f);
}
inline QRgb RgbaToQrgba(struct Imf::Rgba &imagePixel)
{
    return qRgba(gamma(float(imagePixel.r)),
                 gamma(float(imagePixel.g)),
                 gamma(float(imagePixel.b)),
                 (unsigned char)(qBound(0.f, imagePixel.a * 255.f, 255.f) + 0.5f));
}
#endif

EXRHandler::EXRHandler()
    : m_compressionRatio(-1)
    , m_quality(-1)
    , m_imageNumber(0)
    , m_imageCount(0)
    , m_startPos(-1)
{
    // Set the number of threads to use (0 is allowed)
    Imf::setGlobalThreadCount(QThread::idealThreadCount() / 2);
}

bool EXRHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("exr");
        return true;
    }
    return false;
}

static QImage::Format imageFormat(const Imf::RgbaInputFile &file)
{
    auto isRgba = file.channels() & Imf::RgbaChannels::WRITE_A;
#if defined(EXR_USE_LEGACY_CONVERSIONS)
    return (isRgba ? QImage::Format_ARGB32 : QImage::Format_RGB32);
#elif defined(EXR_USE_QT6_FLOAT_IMAGE)
    return (isRgba ? QImage::Format_RGBA16FPx4 : QImage::Format_RGBX16FPx4);
#else
    return (isRgba ? QImage::Format_RGBA64 : QImage::Format_RGBX64);
#endif
}

/*!
 * \brief viewList
 * \param header
 * \return The list of available views.
 */
static QStringList viewList(const Imf::Header &h)
{
    QStringList l;
    if (auto views = h.findTypedAttribute<Imf::StringVectorAttribute>("multiView")) {
        for (auto &&v : views->value()) {
            l << QString::fromStdString(v);
        }
    }
    return l;
}

#ifdef QT_DEBUG
void printAttributes(const Imf::Header &h)
{
    for (auto i = h.begin(); i != h.end(); ++i) {
        qDebug() << i.name();
    }
}
#endif

/*!
 * \brief readMetadata
 * Reads EXR attributes from the \a header and set its as metadata in the \a image.
 */
static void readMetadata(const Imf::Header &header, QImage &image)
{
    // set some useful metadata
    if (auto comments = header.findTypedAttribute<Imf::StringAttribute>("comments")) {
        image.setText(QStringLiteral(META_KEY_COMMENT), QString::fromStdString(comments->value()));
    }

    if (auto owner = header.findTypedAttribute<Imf::StringAttribute>("owner")) {
        image.setText(QStringLiteral(META_KEY_OWNER), QString::fromStdString(owner->value()));
    }

    if (auto lat = header.findTypedAttribute<Imf::FloatAttribute>("latitude")) {
        image.setText(QStringLiteral(META_KEY_LATITUDE), QLocale::c().toString(lat->value()));
    }

    if (auto lon = header.findTypedAttribute<Imf::FloatAttribute>("longitude")) {
        image.setText(QStringLiteral(META_KEY_LONGITUDE), QLocale::c().toString(lon->value()));
    }

    if (auto alt = header.findTypedAttribute<Imf::FloatAttribute>("altitude")) {
        image.setText(QStringLiteral(META_KEY_ALTITUDE), QLocale::c().toString(alt->value()));
    }

    if (auto capDate = header.findTypedAttribute<Imf::StringAttribute>("capDate")) {
        float off = 0;
        if (auto utcOffset = header.findTypedAttribute<Imf::FloatAttribute>("utcOffset")) {
            off = utcOffset->value();
        }
        auto dateTime = QDateTime::fromString(QString::fromStdString(capDate->value()), QStringLiteral("yyyy:MM:dd HH:mm:ss"));
        if (dateTime.isValid()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
            dateTime.setTimeZone(QTimeZone::fromSecondsAheadOfUtc(off));
#else
            dateTime.setOffsetFromUtc(off);
#endif
            image.setText(QStringLiteral(META_KEY_CREATIONDATE), dateTime.toString(Qt::ISODate));
        }
    }

    if (auto xDensity = header.findTypedAttribute<Imf::FloatAttribute>("xDensity")) {
        float par = 1;
        if (auto pixelAspectRatio = header.findTypedAttribute<Imf::FloatAttribute>("pixelAspectRatio")) {
            par = pixelAspectRatio->value();
        }
        image.setDotsPerMeterX(qRound(xDensity->value() * 100.0 / 2.54));
        image.setDotsPerMeterY(qRound(xDensity->value() * par * 100.0 / 2.54));
    }

    // Non-standard attribute
    if (auto xmp = header.findTypedAttribute<Imf::StringAttribute>("xmp")) {
        image.setText(QStringLiteral(META_KEY_XMP_ADOBE), QString::fromStdString(xmp->value()));
    }

    /* TODO: OpenEXR 3.2 metadata
     *
     * New Optional Standard Attributes:
     * - Support automated editorial workflow:
     *   reelName, imageCounter, ascFramingDecisionList
     *
     * - Support forensics (“which other shots used that camera and lens before the camera firmware was updated?”):
     *   cameraMake, cameraModel, cameraSerialNumber, cameraFirmware, cameraUuid, cameraLabel, lensMake, lensModel,
     *   lensSerialNumber, lensFirmware, cameraColorBalance
     *
     * -Support pickup shots (reproduce critical camera settings):
     *   shutterAngle, cameraCCTSetting, cameraTintSetting
     *
     * - Support metadata-driven match move:
     *   sensorCenterOffset, sensorOverallDimensions, sensorPhotositePitch, sensorAcquisitionRectanglenominalFocalLength,
     *   effectiveFocalLength, pinholeFocalLength, entrancePupilOffset, tStop(complementing existing 'aperture')
     */
}

/*!
 * \brief readColorSpace
 * Reads EXR chromaticities from the \a header and set its as color profile in the \a image.
 */
static void readColorSpace(const Imf::Header &header, QImage &image)
{
    // final color operations
#ifndef EXR_USE_LEGACY_CONVERSIONS

    QColorSpace cs;
    if (auto chroma = header.findTypedAttribute<Imf::ChromaticitiesAttribute>("chromaticities")) {
        auto &&v = chroma->value();
        cs = QColorSpace(QPointF(v.white.x, v.white.y),
                         QPointF(v.red.x, v.red.y),
                         QPointF(v.green.x, v.green.y),
                         QPointF(v.blue.x, v.blue.y),
                         QColorSpace::TransferFunction::Linear);
    }
    if (!cs.isValid()) {
        cs = QColorSpace(QColorSpace::SRgbLinear);
    }
    image.setColorSpace(cs);

#ifdef EXR_CONVERT_TO_SRGB
    image.convertToColorSpace(QColorSpace(QColorSpace::SRgb));
#endif

#endif // !EXR_USE_LEGACY_CONVERSIONS
}

bool EXRHandler::read(QImage *outImage)
{
    try {
        auto d = device();

        // set the image position after the first run.
        if (!d->isSequential()) {
            if (m_startPos < 0) {
                m_startPos = d->pos();
            } else {
                d->seek(m_startPos);
            }
        }

        K_IStream istr(d, QByteArray());
        Imf::RgbaInputFile file(istr);
        auto &&header = file.header();

        // set the image to load
        if (m_imageNumber > -1) {
            auto views = viewList(header);
            if (m_imageNumber < views.count()) {
                file.setLayerName(views.at(m_imageNumber).toStdString());
            }
        }

        // get image info
        Imath::Box2i dw = file.dataWindow();
        qint32 width = dw.max.x - dw.min.x + 1;
        qint32 height = dw.max.y - dw.min.y + 1;

        // limiting the maximum image size on a reasonable size (as done in other plugins)
        if (width > EXR_MAX_IMAGE_WIDTH || height > EXR_MAX_IMAGE_HEIGHT) {
            qWarning() << "The maximum image size is limited to" << EXR_MAX_IMAGE_WIDTH << "x" << EXR_MAX_IMAGE_HEIGHT << "px";
            return false;
        }

        // creating the image
        QImage image = imageAlloc(width, height, imageFormat(file));
        if (image.isNull()) {
            qWarning() << "Failed to allocate image, invalid size?" << QSize(width, height);
            return false;
        }

        Imf::Array2D<Imf::Rgba> pixels;
        pixels.resizeErase(EXR_LINES_PER_BLOCK, width);
        bool isRgba = image.hasAlphaChannel();

        // somehow copy pixels into image
        for (int y = 0, n = 0; y < height; y += n) {
            auto my = dw.min.y + y;
            if (my > dw.max.y) { // paranoia check
                break;
            }

            file.setFrameBuffer(&pixels[0][0] - dw.min.x - qint64(my) * width, 1, width);
            file.readPixels(my, std::min(my + EXR_LINES_PER_BLOCK - 1, dw.max.y));

            for (n = 0; n < std::min(EXR_LINES_PER_BLOCK, height - y); ++n) {
#if defined(EXR_USE_LEGACY_CONVERSIONS)
                Q_UNUSED(isRgba)
                auto scanLine = reinterpret_cast<QRgb *>(image.scanLine(y + n));
                for (int x = 0; x < width; ++x) {
                    *(scanLine + x) = RgbaToQrgba(pixels[n][x]);
                }
#elif defined(EXR_USE_QT6_FLOAT_IMAGE)
                auto scanLine = reinterpret_cast<qfloat16 *>(image.scanLine(y + n));
                for (int x = 0; x < width; ++x) {
                    auto xcs = x * 4;
                    *(scanLine + xcs) = qfloat16(qBound(0.f, float(pixels[n][x].r), 1.f));
                    *(scanLine + xcs + 1) = qfloat16(qBound(0.f, float(pixels[n][x].g), 1.f));
                    *(scanLine + xcs + 2) = qfloat16(qBound(0.f, float(pixels[n][x].b), 1.f));
                    *(scanLine + xcs + 3) = qfloat16(isRgba ? qBound(0.f, float(pixels[n][x].a), 1.f) : 1.f);
                }
#else
                auto scanLine = reinterpret_cast<QRgba64 *>(image.scanLine(y + n));
                for (int x = 0; x < width; ++x) {
                    *(scanLine + x) = QRgba64::fromRgba64(quint16(qBound(0.f, float(pixels[n][x].r) * 65535.f + 0.5f, 65535.f)),
                                                          quint16(qBound(0.f, float(pixels[n][x].g) * 65535.f + 0.5f, 65535.f)),
                                                          quint16(qBound(0.f, float(pixels[n][x].b) * 65535.f + 0.5f, 65535.f)),
                                                          isRgba ? quint16(qBound(0.f, float(pixels[n][x].a) * 65535.f + 0.5f, 65535.f)) : quint16(65535));
                }
#endif
            }
        }

        // set some useful metadata
        readMetadata(header, image);
        // final color operations
        readColorSpace(header, image);

        *outImage = image;

        return true;
    } catch (const std::exception &) {
        return false;
    }
}

/*!
 * \brief makePreview
 * Creates a preview of maximum 256 x 256 pixels from the \a image.
 */
bool makePreview(const QImage &image, Imf::Array2D<Imf::PreviewRgba> &pixels)
{
    auto w = image.width();
    auto h = image.height();

    QImage preview;
    if (w > h) {
        preview = image.scaledToWidth(256).convertToFormat(QImage::Format_ARGB32);
    } else {
        preview = image.scaledToHeight(256).convertToFormat(QImage::Format_ARGB32);
    }
    if (preview.isNull()) {
        return false;
    }

    w = preview.width();
    h = preview.height();
    pixels.resizeErase(h, w);
    preview.convertToColorSpace(QColorSpace(QColorSpace::SRgb));

    for (int y = 0; y < h; ++y) {
        auto scanLine = reinterpret_cast<const QRgb *>(preview.constScanLine(y));
        for (int x = 0; x < w; ++x) {
            auto &&out = pixels[y][x];
            out.r = qRed(*(scanLine + x));
            out.g = qGreen(*(scanLine + x));
            out.b = qBlue(*(scanLine + x));
            out.a = qAlpha(*(scanLine + x));
        }
    }

    return true;
}

/*!
 * \brief setMetadata
 * Reades the metadata from \a image and set its as attributes in the \a header.
 */
static void setMetadata(const QImage &image, Imf::Header &header)
{
    auto dateTime = QDateTime::currentDateTime();
    for (auto &&key : image.textKeys()) {
        auto text = image.text(key);
        if (!key.compare(QStringLiteral(META_KEY_COMMENT), Qt::CaseInsensitive)) {
            header.insert("comments", Imf::StringAttribute(text.toStdString()));
        }

        if (!key.compare(QStringLiteral(META_KEY_OWNER), Qt::CaseInsensitive)) {
            header.insert("owner", Imf::StringAttribute(text.toStdString()));
        }

        // clang-format off
        if (!key.compare(QStringLiteral(META_KEY_LATITUDE), Qt::CaseInsensitive) ||
            !key.compare(QStringLiteral(META_KEY_LONGITUDE), Qt::CaseInsensitive) ||
            !key.compare(QStringLiteral(META_KEY_ALTITUDE), Qt::CaseInsensitive)) {
            // clang-format on
            auto ok = false;
            auto value = QLocale::c().toFloat(text, &ok);
            if (ok) {
                header.insert(qPrintable(key.toLower()), Imf::FloatAttribute(value));
            }
        }

        if (!key.compare(QStringLiteral(META_KEY_CREATIONDATE), Qt::CaseInsensitive)) {
            auto dt = QDateTime::fromString(text, Qt::ISODate);
            if (dt.isValid()) {
                dateTime = dt;
            }
        }

#ifndef EXR_DISABLE_XMP_ATTRIBUTE // warning: Non-standard attribute!
        if (!key.compare(QStringLiteral(META_KEY_XMP_ADOBE), Qt::CaseInsensitive)) {
            header.insert("xmp", Imf::StringAttribute(text.toStdString()));
        }
#endif
    }
    if (dateTime.isValid()) {
        header.insert("capDate", Imf::StringAttribute(dateTime.toString(QStringLiteral("yyyy:MM:dd HH:mm:ss")).toStdString()));
        header.insert("utcOffset", Imf::FloatAttribute(dateTime.offsetFromUtc()));
    }

    if (image.dotsPerMeterX() && image.dotsPerMeterY()) {
        header.insert("xDensity", Imf::FloatAttribute(image.dotsPerMeterX() * 2.54f / 100.f));
        header.insert("pixelAspectRatio", Imf::FloatAttribute(float(image.dotsPerMeterX()) / float(image.dotsPerMeterY())));
    }

    // set default chroma (default constructor ITU-R BT.709-3 -> sRGB)
    // The image is converted to Linear sRGB so, the chroma is the default EXR value.
    // If a file doesn’t have a chromaticities attribute, display software should assume that the
    // file’s primaries and the white point match Rec. ITU-R BT.709-3.
    // header.insert("chromaticities", Imf::ChromaticitiesAttribute(Imf::Chromaticities()));

    // TODO: EXR 3.2 attributes (see readMetadata())
}

bool EXRHandler::write(const QImage &image)
{
    try {
        // create EXR header
        qint32 width = image.width();
        qint32 height = image.height();

        // limiting the maximum image size on a reasonable size (as done in other plugins)
        if (width > EXR_MAX_IMAGE_WIDTH || height > EXR_MAX_IMAGE_HEIGHT) {
            qWarning() << "The maximum image size is limited to" << EXR_MAX_IMAGE_WIDTH << "x" << EXR_MAX_IMAGE_HEIGHT << "px";
            return false;
        }

        Imf::Header header(width, height);
        // set compression scheme (forcing PIZ as default)
        header.compression() = Imf::Compression::PIZ_COMPRESSION;
        if (m_compressionRatio >= qint32(Imf::Compression::NO_COMPRESSION) && m_compressionRatio < qint32(Imf::Compression::NUM_COMPRESSION_METHODS)) {
            header.compression() = Imf::Compression(m_compressionRatio);
        }
#if OPENEXR_VERSION_MAJOR > 2
        // set the DCT quality (used by DCT compressions only)
        if (m_quality > -1 && m_quality <= 100) {
            header.dwaCompressionLevel() = float(m_quality);
        }
        // make ZIP compression fast (used by ZIP compressions)
        header.zipCompressionLevel() = 1;
#endif

        // set preview (don't set it for small images)
        if (width > 1024 || height > 1024) {
            Imf::Array2D<Imf::PreviewRgba> previewPixels;
            if (makePreview(image, previewPixels)) {
                header.setPreviewImage(Imf::PreviewImage(previewPixels.width(), previewPixels.height(), &previewPixels[0][0]));
            }
        }

        // set metadata (EXR attributes)
        setMetadata(image, header);

        // write the EXR
        K_OStream ostr(device(), QByteArray());
        auto channelsType = image.hasAlphaChannel() ? Imf::RgbaChannels::WRITE_RGBA : Imf::RgbaChannels::WRITE_RGB;
        if (image.format() == QImage::Format_Mono ||
            image.format() == QImage::Format_MonoLSB ||
            image.format() == QImage::Format_Grayscale16 ||
            image.format() == QImage::Format_Grayscale8) {
            channelsType = Imf::RgbaChannels::WRITE_Y;
        }
        Imf::RgbaOutputFile file(ostr, header, channelsType);
        Imf::Array2D<Imf::Rgba> pixels;
        pixels.resizeErase(EXR_LINES_PER_BLOCK, width);

        // convert the image and write into the stream
#if defined(EXR_USE_QT6_FLOAT_IMAGE)
        auto convFormat = image.hasAlphaChannel() ? QImage::Format_RGBA16FPx4 : QImage::Format_RGBX16FPx4;
#else
        auto convFormat = image.hasAlphaChannel() ? QImage::Format_RGBA64 : QImage::Format_RGBX64;
#endif
        ScanLineConverter slc(convFormat);
        slc.setDefaultSourceColorSpace(QColorSpace(QColorSpace::SRgb));
        slc.setTargetColorSpace(QColorSpace(QColorSpace::SRgbLinear));
        for (int y = 0, n = 0; y < height; y += n) {
            for (n = 0; n < std::min(EXR_LINES_PER_BLOCK, height - y); ++n) {
#if defined(EXR_USE_QT6_FLOAT_IMAGE)
                auto scanLine = reinterpret_cast<const qfloat16 *>(slc.convertedScanLine(image, y + n));
                if (scanLine == nullptr) {
                    return false;
                }
                for (int x = 0; x < width; ++x) {
                    auto xcs = x * 4;
                    pixels[n][x].r = float(*(scanLine + xcs));
                    pixels[n][x].g = float(*(scanLine + xcs + 1));
                    pixels[n][x].b = float(*(scanLine + xcs + 2));
                    pixels[n][x].a = float(*(scanLine + xcs + 3));
                }
#else
                auto scanLine = reinterpret_cast<const QRgba64 *>(slc.convertedScanLine(image, y + n));
                if (scanLine == nullptr) {
                    return false;
                }
                for (int x = 0; x < width; ++x) {
                    pixels[n][x].r = float((scanLine + x)->red() / 65535.f);
                    pixels[n][x].g = float((scanLine + x)->green() / 65535.f);
                    pixels[n][x].b = float((scanLine + x)->blue() / 65535.f);
                    pixels[n][x].a = float((scanLine + x)->alpha() / 65535.f);
                }
#endif
            }
            file.setFrameBuffer(&pixels[0][0] - qint64(y) * width, 1, width);
            file.writePixels(n);
        }
    } catch (const std::exception &) {
        return false;
    }

    return true;
}

void EXRHandler::setOption(ImageOption option, const QVariant &value)
{
    if (option == QImageIOHandler::CompressionRatio) {
        auto ok = false;
        auto cr = value.toInt(&ok);
        if (ok) {
            m_compressionRatio = cr;
        }
    }
    if (option == QImageIOHandler::Quality) {
        auto ok = false;
        auto q = value.toInt(&ok);
        if (ok) {
            m_quality = q;
        }
    }
}

bool EXRHandler::supportsOption(ImageOption option) const
{
    if (option == QImageIOHandler::Size) {
        return true;
    }
    if (option == QImageIOHandler::ImageFormat) {
        return true;
    }
    if (option == QImageIOHandler::CompressionRatio) {
        return true;
    }
    if (option == QImageIOHandler::Quality) {
        return true;
    }
    return false;
}

QVariant EXRHandler::option(ImageOption option) const
{
    QVariant v;

    if (option == QImageIOHandler::Size) {
        if (auto d = device()) {
            // transactions works on both random and sequential devices
            d->startTransaction();
            try {
                K_IStream istr(d, QByteArray());
                Imf::RgbaInputFile file(istr);
                if (m_imageNumber > -1) { // set the image to read
                    auto views = viewList(file.header());
                    if (m_imageNumber < views.count()) {
                        file.setLayerName(views.at(m_imageNumber).toStdString());
                    }
                }
                Imath::Box2i dw = file.dataWindow();
                v = QVariant(QSize(dw.max.x - dw.min.x + 1, dw.max.y - dw.min.y + 1));
            } catch (const std::exception &) {
                // broken file or unsupported version
            }
            d->rollbackTransaction();
        }
    }

    if (option == QImageIOHandler::ImageFormat) {
        if (auto d = device()) {
            // transactions works on both random and sequential devices
            d->startTransaction();
            try {
                K_IStream istr(d, QByteArray());
                Imf::RgbaInputFile file(istr);
                v = QVariant::fromValue(imageFormat(file));
            } catch (const std::exception &) {
                // broken file or unsupported version
            }
            d->rollbackTransaction();
        }
    }

    if (option == QImageIOHandler::CompressionRatio) {
        v = QVariant(m_compressionRatio);
    }

    if (option == QImageIOHandler::Quality) {
        v = QVariant(m_quality);
    }

    return v;
}

bool EXRHandler::jumpToNextImage()
{
    return jumpToImage(m_imageNumber + 1);
}

bool EXRHandler::jumpToImage(int imageNumber)
{
    if (imageNumber < 0 || imageNumber >= imageCount()) {
        return false;
    }
    m_imageNumber = imageNumber;
    return true;
}

int EXRHandler::imageCount() const
{
    // NOTE: image count is cached for performance reason
    auto &&count = m_imageCount;
    if (count > 0) {
        return count;
    }

    count = QImageIOHandler::imageCount();

    auto d = device();
    d->startTransaction();

    try {
        K_IStream istr(d, QByteArray());
        Imf::RgbaInputFile file(istr);
        auto views = viewList(file.header());
        if (!views.isEmpty()) {
            count = views.size();
        }
    } catch (const std::exception &) {
        // do nothing
    }

    d->rollbackTransaction();

    return count;
}

int EXRHandler::currentImageNumber() const
{
    return m_imageNumber;
}

bool EXRHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("EXRHandler::canRead() called with no device");
        return false;
    }

    const QByteArray head = device->peek(4);

    return Imf::isImfMagic(head.data());
}

QImageIOPlugin::Capabilities EXRPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "exr") {
        return Capabilities(CanRead | CanWrite);
    }
    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && EXRHandler::canRead(device)) {
        cap |= CanRead;
    }
    if (device->isWritable()) {
        cap |= CanWrite;
    }
    return cap;
}

QImageIOHandler *EXRPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new EXRHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#include "moc_exr_p.cpp"
