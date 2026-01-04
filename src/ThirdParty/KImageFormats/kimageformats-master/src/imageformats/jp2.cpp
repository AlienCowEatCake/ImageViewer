/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2024 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "jp2_p.h"
#include "scanlineconverter_p.h"
#include "util_p.h"

#include <QColorSpace>
#include <QIODevice>
#include <QImage>
#include <QImageReader>
#include <QLoggingCategory>
#include <QThread>

#include <openjpeg.h>

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(LOG_JP2PLUGIN, "kf.imageformats.plugins.jp2", QtDebugMsg)
#else
Q_LOGGING_CATEGORY(LOG_JP2PLUGIN, "kf.imageformats.plugins.jp2", QtWarningMsg)
#endif

/* *** JP2_MAX_IMAGE_WIDTH and JP2_MAX_IMAGE_HEIGHT ***
 * The maximum size in pixel allowed by the plugin.
 */
#ifndef JP2_MAX_IMAGE_WIDTH
#define JP2_MAX_IMAGE_WIDTH KIF_LARGE_IMAGE_PIXEL_LIMIT
#endif
#ifndef JP2_MAX_IMAGE_HEIGHT
#define JP2_MAX_IMAGE_HEIGHT JP2_MAX_IMAGE_WIDTH
#endif

/* *** JP2_MAX_IMAGE_PIXELS ***
 * OpenJPEG seems limited to an image of 2 gigapixel size.
 */
#ifndef JP2_MAX_IMAGE_PIXELS
#define JP2_MAX_IMAGE_PIXELS std::numeric_limits<qint32>::max()
#endif

/* *** JP2_ENABLE_HDR ***
 * Enable float image formats. Disabled by default
 * due to lack of test images.
 */
#ifndef JP2_ENABLE_HDR
// #define JP2_ENABLE_HDR
#endif

#define JP2_SUBTYPE QByteArrayLiteral("JP2")
#define J2K_SUBTYPE QByteArrayLiteral("J2K")

static void error_callback(const char *msg, void *client_data)
{
    Q_UNUSED(client_data)
    qCCritical(LOG_JP2PLUGIN) << msg;
}

static void warning_callback(const char *msg, void *client_data)
{
    Q_UNUSED(client_data)
    qCWarning(LOG_JP2PLUGIN) << msg;
}

static void info_callback(const char *msg, void *client_data)
{
    Q_UNUSED(client_data)
    qCInfo(LOG_JP2PLUGIN) << msg;
}

static OPJ_SIZE_T jp2_read(void *p_buffer, OPJ_SIZE_T p_nb_bytes, void *p_user_data)
{
    auto dev = (QIODevice*)p_user_data;
    if (dev == nullptr || dev->atEnd()) {
        return OPJ_SIZE_T(-1);
    }
    return OPJ_SIZE_T(dev->read((char*)p_buffer, (qint64)p_nb_bytes));
}

static OPJ_SIZE_T jp2_write(void *p_buffer, OPJ_SIZE_T p_nb_bytes, void *p_user_data)
{
    auto dev = (QIODevice*)p_user_data;
    if (dev == nullptr) {
        return OPJ_SIZE_T(-1);
    }
    return OPJ_SIZE_T(dev->write((char*)p_buffer, (qint64)p_nb_bytes));
}

static OPJ_BOOL jp2_seek(OPJ_OFF_T p_nb_bytes, void *p_user_data)
{
    auto dev = (QIODevice*)p_user_data;
    if (dev == nullptr) {
        return OPJ_FALSE;
    }
    return dev->seek(p_nb_bytes) ? OPJ_TRUE : OPJ_FALSE;
}

static OPJ_OFF_T jp2_skip(OPJ_OFF_T p_nb_bytes, void *p_user_data)
{
    auto dev = (QIODevice*)p_user_data;
    if (dev == nullptr) {
        return OPJ_OFF_T();
    }
    if (dev->seek(dev->pos() + p_nb_bytes)) {
        return p_nb_bytes;
    }
    return OPJ_OFF_T();
}

class JP2HandlerPrivate
{
public:
    JP2HandlerPrivate()
        : m_jp2_stream(nullptr)
        , m_jp2_image(nullptr)
        , m_jp2_version(0)
        , m_jp2_codec(nullptr)
        , m_quality(-1)
        , m_subtype(JP2_SUBTYPE)
    {
        auto sver = QString::fromLatin1(QByteArray(opj_version())).split(QChar(u'.'));
        if (sver.size() == 3) {
            bool ok1, ok2, ok3;
            auto v1 = sver.at(0).toInt(&ok1);
            auto v2 = sver.at(1).toInt(&ok2);
            auto v3 = sver.at(2).toInt(&ok3);
            if (ok1 && ok2 && ok3)
                m_jp2_version = QT_VERSION_CHECK(v1, v2, v3);
        }
    }
    ~JP2HandlerPrivate()
    {
        if (m_jp2_image) {
            opj_image_destroy(m_jp2_image);
            m_jp2_image = nullptr;
        }
        if (m_jp2_stream) {
            opj_stream_destroy(m_jp2_stream);
            m_jp2_stream = nullptr;
        }
        if (m_jp2_codec) {
            opj_destroy_codec(m_jp2_codec);
            m_jp2_codec = nullptr;
        }
    }

    /*!
     * \brief detectDecoderFormat
     * \param device
     * \return The codec JP2 found.
     */
    OPJ_CODEC_FORMAT detectDecoderFormat(QIODevice *device) const
    {
        auto ba = device->peek(32);
        if (ba.left(12) == QByteArray::fromHex("0000000c6a5020200d0a870a")) {
            // if (ba.mid(20, 4) == QByteArray::fromHex("6a707820")) // 'jpx '
            //     return OPJ_CODEC_JPX; // JPEG 2000 Part 2 (not supported -> try reading as JP2)
            return OPJ_CODEC_JP2;
        }
        if (ba.left(5) == QByteArray::fromHex("ff4fff5100")) {
            return OPJ_CODEC_J2K;
        }
        return OPJ_CODEC_UNKNOWN;
    }

    bool createStream(QIODevice *device, bool read)
    {
        if (device == nullptr) {
            return false;
        }
        if (m_jp2_stream == nullptr) {
            m_jp2_stream = opj_stream_default_create(read ? OPJ_TRUE : OPJ_FALSE);
        }
        if (m_jp2_stream == nullptr) {
            return false;
        }
        opj_stream_set_user_data(m_jp2_stream, device, nullptr);
        opj_stream_set_user_data_length(m_jp2_stream, read ? device->size() : 0);
        opj_stream_set_read_function(m_jp2_stream, jp2_read);
        opj_stream_set_write_function(m_jp2_stream, jp2_write);
        opj_stream_set_skip_function(m_jp2_stream, jp2_skip);
        opj_stream_set_seek_function(m_jp2_stream, jp2_seek);
        return true;
    }

    bool isImageValid(const opj_image_t *i) const
    {
        return i && i->comps && i->numcomps > 0;
    }

    void enableThreads(opj_codec_t *codec) const
    {
        if (!opj_has_thread_support()) {
            qCInfo(LOG_JP2PLUGIN) << "OpenJPEG doesn't support multi-threading!";
        } else if (!opj_codec_set_threads(codec, std::max(1, QThread::idealThreadCount() / 2))) {
            qCWarning(LOG_JP2PLUGIN) << "Unable to enable multi-threading!";
        }
    }

    bool createDecoder(QIODevice *device)
    {
        if (m_jp2_codec) {
            return true;
        }
        auto jp2Format = detectDecoderFormat(device);
        if (jp2Format == OPJ_CODEC_UNKNOWN) {
            return false;
        }
        m_jp2_codec = opj_create_decompress(jp2Format);
        if (m_jp2_codec == nullptr) {
            return false;
        }
        enableThreads(m_jp2_codec);
#ifdef QT_DEBUG
        // opj_set_info_handler(m_jp2_codec, info_callback, nullptr);
        // opj_set_warning_handler(m_jp2_codec, warning_callback, nullptr);
#endif
        opj_set_error_handler(m_jp2_codec, error_callback, nullptr);
        return true;
    }

    bool readHeader(QIODevice *device)
    {
        if (!createStream(device, true)) {
            return false;
        }

        if (m_jp2_image) {
            return true;
        }

        if (!createDecoder(device)) {
            return false;
        }

        opj_set_default_decoder_parameters(&m_dparameters);
        if (!opj_setup_decoder(m_jp2_codec, &m_dparameters)) {
            qCCritical(LOG_JP2PLUGIN) << "Failed to setup JP2 decoder!";
            return false;
        }

        if (!opj_read_header(m_jp2_stream, m_jp2_codec, &m_jp2_image)) {
            qCCritical(LOG_JP2PLUGIN) << "Failed to read JP2 header!";
            return false;
        }

        return isImageValid(m_jp2_image);
    }

    template<class T>
    bool jp2ToImage(QImage *img) const
    {
        Q_ASSERT(img->depth() == 8 * sizeof(T) || img->depth() == 32 * sizeof(T));
        for (qint32 c = 0, cc = m_jp2_image->numcomps; c < cc; ++c) {
            auto cs = cc == 1 ? 1 : 4;
            auto &&jc = m_jp2_image->comps[c];
            if (jc.data == nullptr)
                return false;
            if (qint32(jc.w) != img->width() || qint32(jc.h) != img->height())
                return false;

            // discriminate between int and float (avoid complicating things by creating classes with template specializations)
            if (std::numeric_limits<T>::is_integer) {
                auto divisor = 1;
                if (jc.prec > sizeof(T) * 8) {
                    // convert to the wanted precision (e.g. 16-bit -> 8-bit: divisor = 65535 / 255 = 257)
                    divisor = std::max(1, int(((1ll << jc.prec) - 1) / ((1ll << (sizeof(T) * 8)) - 1)));
                }
                for (qint32 y = 0, h = img->height(); y < h; ++y) {
                    auto ptr = reinterpret_cast<T *>(img->scanLine(y));
                    for (qint32 x = 0, w = img->width(); x < w; ++x) {
                        qint32 v = jc.data[y * w + x] / divisor;
                        if (jc.sgnd) // never seen
                            v -= std::numeric_limits<typename std::make_signed<T>::type>::min();
                        *(ptr + x * cs + c) = qBound(qint32(std::numeric_limits<T>::lowest()), v, qint32(std::numeric_limits<T>::max()));
                    }
                }
            } else { // float
                for (qint32 y = 0, h = img->height(); y < h; ++y) {
                    auto ptr = reinterpret_cast<T *>(img->scanLine(y));
                    for (qint32 x = 0, w = img->width(); x < w; ++x) {
                        *(ptr + x * cs + c) = jc.data[y * w + x];
                    }
                }
            }
        }
        return true;
    }

    template<class T>
    void alphaFix(QImage *img) const
    {
        if (m_jp2_image->numcomps == 3) {
            Q_ASSERT(img->depth() == 32 * sizeof(T));
            for (qint32 y = 0, h = img->height(); y < h; ++y) {
                auto ptr = reinterpret_cast<T *>(img->scanLine(y));
                for (qint32 x = 0, w = img->width(); x < w; ++x) {
                    *(ptr + x * 4 + 3) = std::numeric_limits<T>::is_iec559 ? 1 : std::numeric_limits<T>::max();
                }
            }
        }
    }

    QImage readImage(QIODevice *device)
    {
        if (!readHeader(device)) {
            return {};
        }

        auto img = imageAlloc(size(), format());
        if (img.isNull()) {
            return {};
        }

        if (!opj_decode(m_jp2_codec, m_jp2_stream, m_jp2_image)) {
            qCCritical(LOG_JP2PLUGIN) << "Failed to decoding JP2 image!";
            return {};
        }

        auto f = img.format();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
        if (f == QImage::Format_RGBA32FPx4 || f == QImage::Format_RGBX32FPx4) {
            if (!jp2ToImage<quint32>(&img))
                return {};
            alphaFix<float>(&img);
#else
        if (false) {
#endif
        } else if (f == QImage::Format_RGBA64 || f == QImage::Format_RGBX64 || f == QImage::Format_Grayscale16) {
            if (!jp2ToImage<quint16>(&img))
                return {};
            alphaFix<quint16>(&img);
        } else {
            if (!jp2ToImage<quint8>(&img))
                return {};
            alphaFix<quint8>(&img);
        }

        img.setColorSpace(colorSpace());

        return img;
    }

    bool checkSizeLimits(qint32 width, qint32 height, qint32 nchannels) const
    {
        if (width > JP2_MAX_IMAGE_WIDTH || height > JP2_MAX_IMAGE_HEIGHT || width < 1 || height < 1) {
            qCCritical(LOG_JP2PLUGIN) << "Maximum image size is limited to" << JP2_MAX_IMAGE_WIDTH << "x" << JP2_MAX_IMAGE_HEIGHT << "pixels";
            return false;
        }

        if (qint64(width) * qint64(height) > JP2_MAX_IMAGE_PIXELS) {
            qCCritical(LOG_JP2PLUGIN) << "Maximum image size is limited to" << JP2_MAX_IMAGE_PIXELS << "pixels";
            return false;
        }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        // OpenJPEG uses a shadow copy @32-bit/channel so we need to do a check
        const int allocationLimit = QImageReader::allocationLimit();
        if (allocationLimit > 0) {
            auto maxBytes = qint64(allocationLimit) * 1024 * 1024;
            auto neededBytes = qint64(width) * height * nchannels * 4;
            if (maxBytes > 0 && neededBytes > maxBytes) {
                qCCritical(LOG_JP2PLUGIN) << "Allocation limit set to" << (maxBytes / 1024 / 1024) << "MiB but" << (neededBytes / 1024 / 1024)
                                          << "MiB are needed!";
                return false;
            }
        }
#endif

        return true;
    }

    bool checkSizeLimits(const QSize &size, qint32 nchannels) const
    {
        return checkSizeLimits(size.width(), size.height(), nchannels);
    }

    QSize size() const
    {
        QSize sz;
        if (isImageValid(m_jp2_image)) {
            auto &&c0 = m_jp2_image->comps[0];
            auto tmp = QSize(c0.w, c0.h);
            if (checkSizeLimits(tmp, m_jp2_image->numcomps))
                sz = tmp;
        }
        return sz;
    }

    QImage::Format format() const
    {
        auto fmt = QImage::Format_Invalid;
        if (isImageValid(m_jp2_image)) {
            auto &&c0 = m_jp2_image->comps[0];
            auto prec = c0.prec;
            for (quint32 c = 1; c < m_jp2_image->numcomps; ++c) {
                auto &&cc = m_jp2_image->comps[c];
                if (cc.prec != prec)
                    prec = 0;
            }
            auto jp2cs = m_jp2_image->color_space;
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
            if (jp2cs == OPJ_CLRSPC_UNKNOWN || jp2cs == OPJ_CLRSPC_UNSPECIFIED) {
                auto cs = colorSpace();
                if (cs.colorModel() == QColorSpace::ColorModel::Cmyk)
                    jp2cs = OPJ_CLRSPC_CMYK;
                else if (cs.colorModel() == QColorSpace::ColorModel::Rgb)
                    jp2cs = OPJ_CLRSPC_SRGB;
                else if (cs.colorModel() == QColorSpace::ColorModel::Gray)
                    jp2cs = OPJ_CLRSPC_GRAY;
            }
#endif
            if (jp2cs == OPJ_CLRSPC_UNKNOWN || jp2cs == OPJ_CLRSPC_UNSPECIFIED) {
                if (m_jp2_image->numcomps == 1)
                    jp2cs = OPJ_CLRSPC_GRAY;
                else
                    jp2cs = OPJ_CLRSPC_SRGB;
            }

            // *** IMPORTANT: To keep the code simple, the returned formats must have 1 or 4 channels (8/16/32-bits)
            if (jp2cs == OPJ_CLRSPC_SRGB) {
                if (m_jp2_image->numcomps == 3 || m_jp2_image->numcomps == 4) {
                    auto hasAlpha = m_jp2_image->numcomps == 4;
                    if (prec == 8)
                        fmt = hasAlpha ? QImage::Format_RGBA8888 : QImage::Format_RGBX8888;
                    else if (prec == 16)
                        fmt = hasAlpha ? QImage::Format_RGBA64 : QImage::Format_RGBX64;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
#ifdef JP2_ENABLE_HDR
                    else if (prec == 32) // not sure about this
                        fmt = hasAlpha ? QImage::Format_RGBA32FPx4 : QImage::Format_RGBX32FPx4;
#endif
#endif
                }
            } else if (jp2cs == OPJ_CLRSPC_GRAY) {
                if (m_jp2_image->numcomps == 1) {
                    if (prec == 8)
                        fmt = QImage::Format_Grayscale8;
                    else if (prec == 16)
                        fmt = QImage::Format_Grayscale16;
                }
            } else if (jp2cs == OPJ_CLRSPC_CMYK) {
                if (m_jp2_image->numcomps == 4) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
                    if (prec == 8 || prec == 16)
                        fmt = QImage::Format_CMYK8888;
#endif
                }
            }
        }
        return fmt;
    }

    QColorSpace colorSpace() const
    {
        QColorSpace cs;
        if (m_jp2_image) {
            if (m_jp2_image->icc_profile_buf && m_jp2_image->icc_profile_len > 0) {
                cs = QColorSpace::fromIccProfile(QByteArray((char *)m_jp2_image->icc_profile_buf, m_jp2_image->icc_profile_len));
            }
            if (!cs.isValid()) {
                if (m_jp2_image->color_space == OPJ_CLRSPC_SRGB)
                    cs = QColorSpace(QColorSpace::SRgb);
            }
        }
        return cs;
    }

    /*!
     * \brief isSupported
     * \return True if the current JP2 image i ssupported by the plugin. Otherwise false.
     */
    bool isSupported() const
    {
        return format() != QImage::Format_Invalid;
    }

    QByteArray subType() const
    {
        return m_subtype;
    }
    void setSubType(const QByteArray &type)
    {
        m_subtype = type;
    }

    qint32 quality() const
    {
        return m_quality;
    }
    void setQuality(qint32 quality)
    {
        m_quality = qBound(qint32(-1), quality, qint32(100));
    }

    /*!
     * \brief encoderFormat
     * \return The encoder format set by subType.
     */
    OPJ_CODEC_FORMAT encoderFormat() const
    {
        return subType() == J2K_SUBTYPE ? OPJ_CODEC_J2K : OPJ_CODEC_JP2;
    }

    /*!
     * \brief opjVersion
     * \return The runtime library version.
     */
    qint32 opjVersion() const
    {
        return m_jp2_version;
    }

    bool imageToJp2(const QImage &image)
    {
        auto ncomp = image.hasAlphaChannel() ? 4 : 3;
        auto prec = 8;
        auto convFormat = image.format();
        auto isFloat = false;
        auto cs = OPJ_CLRSPC_SRGB;
        if (opjVersion() >= QT_VERSION_CHECK(2, 5, 4)) {
            auto ics = image.colorSpace();
            if (!(ics.isValid() && ics.primaries() == QColorSpace::Primaries::SRgb && ics.transferFunction() == QColorSpace::TransferFunction::SRgb)) {
                cs = OPJ_CLRSPC_UNKNOWN;
            }
        }

        switch (image.format()) {
        case QImage::Format_Mono:
        case QImage::Format_MonoLSB:
        case QImage::Format_Alpha8:
        case QImage::Format_Grayscale8:
            ncomp = 1;
            cs = OPJ_CLRSPC_GRAY;
            convFormat = QImage::Format_Grayscale8;
            break;
        case QImage::Format_Indexed8:
            if (image.isGrayscale()) {
                ncomp = 1;
                cs = OPJ_CLRSPC_GRAY;
                convFormat = QImage::Format_Grayscale8;
            } else {
                ncomp = 4;
                cs = OPJ_CLRSPC_SRGB;
                convFormat = QImage::Format_RGBA8888;
            }
            break;
        case QImage::Format_Grayscale16:
            ncomp = 1;
            prec = 16;
            cs = OPJ_CLRSPC_GRAY;
            convFormat = QImage::Format_Grayscale16;
            break;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
        case QImage::Format_RGBX16FPx4:
        case QImage::Format_RGBX32FPx4:
            isFloat = true;
#ifdef JP2_ENABLE_HDR
            prec = 32;
            convFormat = QImage::Format_RGBX32FPx4;
            cs = OPJ_CLRSPC_UNSPECIFIED;
            break;
#else
            Q_FALLTHROUGH();
#endif
#endif
        case QImage::Format_RGBX64:
        case QImage::Format_RGB30:
        case QImage::Format_BGR30:
            prec = 16;
            convFormat = QImage::Format_RGBX64;
            break;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
        case QImage::Format_RGBA16FPx4:
        case QImage::Format_RGBA16FPx4_Premultiplied:
        case QImage::Format_RGBA32FPx4:
        case QImage::Format_RGBA32FPx4_Premultiplied:
            isFloat = true;
#ifdef JP2_ENABLE_HDR
            prec = 32;
            convFormat = QImage::Format_RGBA32FPx4;
            cs = OPJ_CLRSPC_UNSPECIFIED;
            break;
#else
            Q_FALLTHROUGH();
#endif
#endif
        case QImage::Format_RGBA64:
        case QImage::Format_RGBA64_Premultiplied:
        case QImage::Format_A2RGB30_Premultiplied:
        case QImage::Format_A2BGR30_Premultiplied:
            prec = 16;
            convFormat = QImage::Format_RGBA64;
            break;
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
        case QImage::Format_CMYK8888: // requires OpenJPEG 2.5.3+
            if (opjVersion() >= QT_VERSION_CHECK(2, 5, 3)) {
                ncomp = 4;
                cs = OPJ_CLRSPC_CMYK;
                break;
            } else {
                Q_FALLTHROUGH();
            }
#endif
        default:
            if (image.depth() > 32) {
                qCWarning(LOG_JP2PLUGIN) << "The image is saved losing precision!";
            }
            convFormat = ncomp == 4 ? QImage::Format_RGBA8888 : QImage::Format_RGBX8888;
            break;
        }

        if (!checkSizeLimits(image.size(), ncomp)) {
            return false;
        }

        opj_set_default_encoder_parameters(&m_cparameters);
        m_cparameters.cod_format = encoderFormat();
        m_cparameters.tile_size_on = 1;
        m_cparameters.cp_tdx = 1024;
        m_cparameters.cp_tdy = 1024;

        if (m_quality > -1 && m_quality < 100) {
            m_cparameters.irreversible = 1;
            m_cparameters.tcp_numlayers = 1;
            m_cparameters.cp_disto_alloc = 1;
            m_cparameters.tcp_rates[0] = 100.0 - (m_quality < 10 ? m_quality : 10 + (std::log10(m_quality) - 1) * 90);
        }

        std::unique_ptr<opj_image_cmptparm_t> cmptparm(new opj_image_cmptparm_t[ncomp]);
        for (int i = 0; i < ncomp; ++i) {
            auto &&p = cmptparm.get() + i;
            memset(p, 0, sizeof(opj_image_cmptparm_t));
            p->dx = m_cparameters.subsampling_dx;
            p->dy = m_cparameters.subsampling_dy;
            p->w = image.width();
            p->h = image.height();
            p->x0 = 0;
            p->y0 = 0;
            p->prec = prec;
            p->sgnd = 0;
        }

        m_jp2_image = opj_image_create(ncomp, cmptparm.get(), cs);
        if (m_jp2_image == nullptr) {
            return false;
        }
        if (int(m_jp2_image->numcomps) != ncomp) {
            return false; // paranoia
        }
        m_jp2_image->x1 = image.width();
        m_jp2_image->y1 = image.height();

        ScanLineConverter scl(convFormat);
        if (prec < 32 && isFloat) {
            scl.setDefaultSourceColorSpace(QColorSpace(QColorSpace::SRgbLinear));
        }
        if (cs == OPJ_CLRSPC_SRGB) {
            scl.setTargetColorSpace(QColorSpace(QColorSpace::SRgb));
        } else {
            scl.setTargetColorSpace(image.colorSpace());
        }
        for (qint32 c = 0; c < ncomp; ++c) {
            auto &&comp = m_jp2_image->comps[c];
            auto mul = ncomp == 1 ? 1 : 4;
            for (qint32 y = 0, h = image.height(); y < h; ++y) {
                if (prec == 8) {
                    auto ptr = reinterpret_cast<const quint8 *>(scl.convertedScanLine(image, y));
                    for (qint32 x = 0, w = image.width(); x < w; ++x)
                        comp.data[y * w + x] = ptr[x * mul + c];
                } else if (prec == 16) {
                    auto ptr = reinterpret_cast<const quint16 *>(scl.convertedScanLine(image, y));
                    for (qint32 x = 0, w = image.width(); x < w; ++x)
                        comp.data[y * w + x] = ptr[x * mul + c];
                } else if (prec == 32) {
                    auto ptr = reinterpret_cast<const quint32 *>(scl.convertedScanLine(image, y));
                    for (qint32 x = 0, w = image.width(); x < w; ++x)
                        comp.data[y * w + x] = ptr[x * mul + c];
                }
            }
        }

        if (opjVersion() >= QT_VERSION_CHECK(2, 5, 4)) {
            auto colorSpace = scl.targetColorSpace().iccProfile();
            if (!colorSpace.isEmpty()) {
                m_jp2_image->icc_profile_buf = reinterpret_cast<OPJ_BYTE *>(malloc(colorSpace.size()));
                if (m_jp2_image->icc_profile_buf) {
                    memcpy(m_jp2_image->icc_profile_buf, colorSpace.data(), colorSpace.size());
                    m_jp2_image->icc_profile_len = colorSpace.size();
                }
            }
        }

        return true;
    }

    bool writeImage(QIODevice *device, const QImage &image)
    {
        if (!imageToJp2(image)) {
            qCCritical(LOG_JP2PLUGIN) << "Error while creating JP2 image!";
            return false;
        }

        std::unique_ptr<opj_codec_t, std::function<void(opj_codec_t *)>> codec(opj_create_compress(encoderFormat()), opj_destroy_codec);
        if (codec == nullptr) {
            qCCritical(LOG_JP2PLUGIN) << "Error while creating encoder!";
            return false;
        }
        enableThreads(codec.get());
#ifdef QT_DEBUG
        // opj_set_info_handler(m_jp2_codec, info_callback, nullptr);
        // opj_set_warning_handler(m_jp2_codec, warning_callback, nullptr);
#endif
        opj_set_error_handler(m_jp2_codec, error_callback, nullptr);

        if (!opj_setup_encoder(codec.get(), &m_cparameters, m_jp2_image)) {
            return false;
        }

        if (!createStream(device, false)) {
            return false;
        }

        if (!opj_start_compress(codec.get(), m_jp2_image, m_jp2_stream)) {
            return false;
        }
        if (!opj_encode(codec.get(), m_jp2_stream)) {
            return false;
        }
        if (!opj_end_compress(codec.get(), m_jp2_stream)) {
            return false;
        }

        return true;
    }

private:
    // common
    opj_stream_t *m_jp2_stream;

    opj_image_t *m_jp2_image;

    qint32 m_jp2_version;

    // read
    opj_codec_t *m_jp2_codec;

    opj_dparameters_t m_dparameters;

    // write
    opj_cparameters_t m_cparameters;

    qint32 m_quality;

    QByteArray m_subtype;
};


JP2Handler::JP2Handler()
    : QImageIOHandler()
    , d(new JP2HandlerPrivate)
{
}

bool JP2Handler::canRead() const
{
    if (canRead(device())) {
        setFormat("jp2");
        return true;
    }
    return false;
}

bool JP2Handler::canRead(QIODevice *device)
{
    if (!device) {
        qCWarning(LOG_JP2PLUGIN) << "JP2Handler::canRead() called with no device";
        return false;
    }

    if (device->isSequential()) {
        return false;
    }

    JP2HandlerPrivate handler;
    return handler.detectDecoderFormat(device) != OPJ_CODEC_UNKNOWN;
}

bool JP2Handler::read(QImage *image)
{
    auto dev = device();
    if (dev == nullptr) {
        return false;
    }
    auto img = d->readImage(dev);
    if (img.isNull()) {
        return false;
    }
    *image = img;
    return true;
}

bool JP2Handler::write(const QImage &image)
{
    if (image.isNull()) {
        return false;
    }
    auto dev = device();
    if (dev == nullptr) {
        return false;
    }
    return d->writeImage(dev, image);
}

bool JP2Handler::supportsOption(ImageOption option) const
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
    if (option == QImageIOHandler::Quality) {
        return true;
    }
    return false;
}

void JP2Handler::setOption(ImageOption option, const QVariant &value)
{
    if (option == QImageIOHandler::SubType) {
        auto st = value.toByteArray();
        if (this->option(QImageIOHandler::SupportedSubTypes).toList().contains(st))
            d->setSubType(st);
    }
    if (option == QImageIOHandler::Quality) {
        auto ok = false;
        auto q = value.toInt(&ok);
        if (ok) {
            d->setQuality(q);
        }
    }
}

QVariant JP2Handler::option(ImageOption option) const
{
    QVariant v;

    if (option == QImageIOHandler::Size) {
        if (d->readHeader(device())) {
            v = d->size();
        }
    }

    if (option == QImageIOHandler::ImageFormat) {
        if (d->readHeader(device())) {
            v = d->format();
        }
    }

    if (option == QImageIOHandler::SubType) {
        v = d->subType();
    }

    if (option == QImageIOHandler::SupportedSubTypes) {
        v = QVariant::fromValue(QList<QByteArray>() << JP2_SUBTYPE << J2K_SUBTYPE);
    }

    if (option == QImageIOHandler::Quality) {
        v = d->quality();
    }

    return v;
}

QImageIOPlugin::Capabilities JP2Plugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "jp2" || format == "j2k") {
        return Capabilities(CanRead | CanWrite);
    }
    // NOTE: JPF is the default extension of Photoshop for JP2 files.
    if (format == "jpf") {
        return Capabilities(CanRead);
    }
    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && JP2Handler::canRead(device)) {
        cap |= CanRead;
    }
    if (device->isWritable()) {
        cap |= CanWrite;
    }
    return cap;
}

QImageIOHandler *JP2Plugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new JP2Handler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#include "moc_jp2_p.cpp"
