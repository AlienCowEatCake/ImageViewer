/*
    High Efficiency Image File Format (HEIF) support for QImage.

    SPDX-FileCopyrightText: 2020 Sirius Bakke <sirius@bakke.co>
    SPDX-FileCopyrightText: 2021 Daniel Novomesky <dnovomesky@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "heif_p.h"
#include "libheif/heif_cxx.h"

#include <QColorSpace>
#include <QDebug>
#include <QPointF>
#include <QSysInfo>
#include <string.h>

namespace // Private.
{
struct HeifQIODeviceWriter : public heif::Context::Writer {
    HeifQIODeviceWriter(QIODevice *device)
        : m_ioDevice(device)
    {
    }

    heif_error write(const void *data, size_t size) override
    {
        heif_error error;
        error.code = heif_error_Ok;
        error.subcode = heif_suberror_Unspecified;
        error.message = errorOkMessage;

        qint64 bytesWritten = m_ioDevice->write(static_cast<const char *>(data), size);

        if (bytesWritten < static_cast<qint64>(size)) {
            error.code = heif_error_Encoding_error;
            error.message = QIODeviceWriteErrorMessage;
            error.subcode = heif_suberror_Cannot_write_output_data;
        }

        return error;
    }

    static constexpr const char *errorOkMessage = "Success";
    static constexpr const char *QIODeviceWriteErrorMessage = "Bytes written to QIODevice are smaller than input data size";

private:
    QIODevice *m_ioDevice;
};

} // namespace

HEIFHandler::HEIFHandler()
    : m_parseState(ParseHeicNotParsed)
    , m_quality(100)
{
}

bool HEIFHandler::canRead() const
{
    if (m_parseState == ParseHeicNotParsed && !canRead(device())) {
        return false;
    }

    if (m_parseState != ParseHeicError) {
        setFormat("heif");
        return true;
    }
    return false;
}

bool HEIFHandler::read(QImage *outImage)
{
    if (!ensureParsed()) {
        return false;
    }

    *outImage = m_current_image;
    return true;
}

bool HEIFHandler::write(const QImage &image)
{
    if (image.format() == QImage::Format_Invalid || image.isNull()) {
        qWarning("No image data to save");
        return false;
    }

    int save_depth; // 8 or 10bit per channel
    QImage::Format tmpformat; // format for temporary image
    const bool save_alpha = image.hasAlphaChannel();

    switch (image.format()) {
    case QImage::Format_BGR30:
    case QImage::Format_A2BGR30_Premultiplied:
    case QImage::Format_RGB30:
    case QImage::Format_A2RGB30_Premultiplied:
    case QImage::Format_Grayscale16:
    case QImage::Format_RGBX64:
    case QImage::Format_RGBA64:
    case QImage::Format_RGBA64_Premultiplied:
        save_depth = 10;
        break;
    default:
        if (image.depth() > 32) {
            save_depth = 10;
        } else {
            save_depth = 8;
        }
        break;
    }

    heif_chroma chroma;
    if (save_depth > 8) {
        if (save_alpha) {
            tmpformat = QImage::Format_RGBA64;
            chroma = (QSysInfo::ByteOrder == QSysInfo::LittleEndian) ? heif_chroma_interleaved_RRGGBBAA_LE : heif_chroma_interleaved_RRGGBBAA_BE;
        } else {
            tmpformat = QImage::Format_RGBX64;
            chroma = (QSysInfo::ByteOrder == QSysInfo::LittleEndian) ? heif_chroma_interleaved_RRGGBB_LE : heif_chroma_interleaved_RRGGBB_BE;
        }
    } else {
        if (save_alpha) {
            tmpformat = QImage::Format_RGBA8888;
            chroma = heif_chroma_interleaved_RGBA;
        } else {
            tmpformat = QImage::Format_RGB888;
            chroma = heif_chroma_interleaved_RGB;
        }
    }

    const QImage tmpimage = image.convertToFormat(tmpformat);

    try {
        heif::Context ctx;
        heif::Image heifImage;
        heifImage.create(tmpimage.width(), tmpimage.height(), heif_colorspace_RGB, chroma);

        QByteArray iccprofile = tmpimage.colorSpace().iccProfile();
        if (iccprofile.size() > 0) {
            std::vector<uint8_t> rawProfile(iccprofile.begin(), iccprofile.end());
            heifImage.set_raw_color_profile(heif_color_profile_type_prof, rawProfile);
        }

        heifImage.add_plane(heif_channel_interleaved, image.width(), image.height(), save_depth);
        int stride = 0;
        uint8_t *const dst = heifImage.get_plane(heif_channel_interleaved, &stride);
        size_t rowbytes;

        switch (save_depth) {
        case 10:
            if (save_alpha) {
                for (int y = 0; y < tmpimage.height(); y++) {
                    const uint16_t *src_word = reinterpret_cast<const uint16_t *>(tmpimage.constScanLine(y));
                    uint16_t *dest_word = reinterpret_cast<uint16_t *>(dst + (y * stride));
                    for (int x = 0; x < tmpimage.width(); x++) {
                        int tmp_pixelval;
                        // R
                        tmp_pixelval = (int)(((float)(*src_word) / 65535.0f) * 1023.0f + 0.5f);
                        *dest_word = qBound(0, tmp_pixelval, 1023);
                        src_word++;
                        dest_word++;
                        // G
                        tmp_pixelval = (int)(((float)(*src_word) / 65535.0f) * 1023.0f + 0.5f);
                        *dest_word = qBound(0, tmp_pixelval, 1023);
                        src_word++;
                        dest_word++;
                        // B
                        tmp_pixelval = (int)(((float)(*src_word) / 65535.0f) * 1023.0f + 0.5f);
                        *dest_word = qBound(0, tmp_pixelval, 1023);
                        src_word++;
                        dest_word++;
                        // A
                        tmp_pixelval = (int)(((float)(*src_word) / 65535.0f) * 1023.0f + 0.5f);
                        *dest_word = qBound(0, tmp_pixelval, 1023);
                        src_word++;
                        dest_word++;
                    }
                }
            } else { // no alpha channel
                for (int y = 0; y < tmpimage.height(); y++) {
                    const uint16_t *src_word = reinterpret_cast<const uint16_t *>(tmpimage.constScanLine(y));
                    uint16_t *dest_word = reinterpret_cast<uint16_t *>(dst + (y * stride));
                    for (int x = 0; x < tmpimage.width(); x++) {
                        int tmp_pixelval;
                        // R
                        tmp_pixelval = (int)(((float)(*src_word) / 65535.0f) * 1023.0f + 0.5f);
                        *dest_word = qBound(0, tmp_pixelval, 1023);
                        src_word++;
                        dest_word++;
                        // G
                        tmp_pixelval = (int)(((float)(*src_word) / 65535.0f) * 1023.0f + 0.5f);
                        *dest_word = qBound(0, tmp_pixelval, 1023);
                        src_word++;
                        dest_word++;
                        // B
                        tmp_pixelval = (int)(((float)(*src_word) / 65535.0f) * 1023.0f + 0.5f);
                        *dest_word = qBound(0, tmp_pixelval, 1023);
                        src_word++;
                        dest_word++;
                        // X
                        src_word++;
                    }
                }
            }
            break;
        case 8:
            rowbytes = save_alpha ? (tmpimage.width() * 4) : (tmpimage.width() * 3);
            for (int y = 0; y < tmpimage.height(); y++) {
                memcpy(dst + (y * stride), tmpimage.constScanLine(y), rowbytes);
            }
            break;
        default:
            qWarning() << "Unsupported depth:" << save_depth;
            return false;
            break;
        }

        heif::Encoder encoder(heif_compression_HEVC);

        encoder.set_lossy_quality(m_quality);
        if (m_quality > 90) {
            if (m_quality == 100) {
                encoder.set_lossless(true);
            }
            encoder.set_string_parameter("chroma", "444");
        }

        heif::Context::EncodingOptions encodingOptions;
        encodingOptions.save_alpha_channel = save_alpha;

        if ((tmpimage.width() % 2 == 1) || (tmpimage.height() % 2 == 1)) {
            qWarning() << "Image has odd dimension!\nUse even-numbered dimension(s) for better compatibility with other HEIF implementations.";
            if (save_alpha) {
                // This helps to save alpha channel when image has odd dimension
                encodingOptions.macOS_compatibility_workaround = 0;
            }
        }

        ctx.encode_image(heifImage, encoder, encodingOptions);

        HeifQIODeviceWriter writer(device());

        ctx.write(writer);

    } catch (const heif::Error &err) {
        qWarning() << "libheif error:" << err.get_message().c_str();
        return false;
    }

    return true;
}

bool HEIFHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("HEIFHandler::canRead() called with no device");
        return false;
    }

    const QByteArray header = device->peek(28);
    return HEIFHandler::isSupportedBMFFType(header);
}

bool HEIFHandler::isSupportedBMFFType(const QByteArray &header)
{
    if (header.size() < 28) {
        return false;
    }

    const char *buffer = header.constData();
    if (qstrncmp(buffer + 4, "ftyp", 4) == 0) {
        if (qstrncmp(buffer + 8, "heic", 4) == 0) {
            return true;
        }
        if (qstrncmp(buffer + 8, "heis", 4) == 0) {
            return true;
        }
        if (qstrncmp(buffer + 8, "heix", 4) == 0) {
            return true;
        }

        /* we want to avoid loading AVIF files via this plugin */
        if (qstrncmp(buffer + 8, "mif1", 4) == 0) {
            for (int offset = 16; offset <= 24; offset += 4) {
                if (qstrncmp(buffer + offset, "avif", 4) == 0) {
                    return false;
                }
            }
            return true;
        }

        if (qstrncmp(buffer + 8, "mif2", 4) == 0) {
            return true;
        }
        if (qstrncmp(buffer + 8, "msf1", 4) == 0) {
            return true;
        }
    }

    return false;
}

QVariant HEIFHandler::option(ImageOption option) const
{
    if (option == Quality) {
        return m_quality;
    }

    if (!supportsOption(option) || !ensureParsed()) {
        return QVariant();
    }

    switch (option) {
    case Size:
        return m_current_image.size();
        break;
    default:
        return QVariant();
        break;
    }
}

void HEIFHandler::setOption(ImageOption option, const QVariant &value)
{
    switch (option) {
    case Quality:
        m_quality = value.toInt();
        if (m_quality > 100) {
            m_quality = 100;
        } else if (m_quality < 0) {
            m_quality = 100;
        }
        break;
    default:
        QImageIOHandler::setOption(option, value);
        break;
    }
}

bool HEIFHandler::supportsOption(ImageOption option) const
{
    return option == Quality || option == Size;
}

bool HEIFHandler::ensureParsed() const
{
    if (m_parseState == ParseHeicSuccess) {
        return true;
    }
    if (m_parseState == ParseHeicError) {
        return false;
    }

    HEIFHandler *that = const_cast<HEIFHandler *>(this);

    return that->ensureDecoder();
}
bool HEIFHandler::ensureDecoder()
{
    if (m_parseState != ParseHeicNotParsed) {
        if (m_parseState == ParseHeicSuccess) {
            return true;
        }
        return false;
    }

    const QByteArray buffer = device()->readAll();
    if (!HEIFHandler::isSupportedBMFFType(buffer)) {
        m_parseState = ParseHeicError;
        return false;
    }

    try {
        heif::Context ctx;
        ctx.read_from_memory_without_copy((const void *)(buffer.constData()), buffer.size());

        heif::ImageHandle handle = ctx.get_primary_image_handle();

        const bool hasAlphaChannel = handle.has_alpha_channel();
        const int bit_depth = handle.get_luma_bits_per_pixel();
        heif_chroma chroma;

        QImage::Format target_image_format;

        if (bit_depth == 10 || bit_depth == 12) {
            if (hasAlphaChannel) {
                chroma = (QSysInfo::ByteOrder == QSysInfo::LittleEndian) ? heif_chroma_interleaved_RRGGBBAA_LE : heif_chroma_interleaved_RRGGBBAA_BE;
                target_image_format = QImage::Format_RGBA64;
            } else {
                chroma = (QSysInfo::ByteOrder == QSysInfo::LittleEndian) ? heif_chroma_interleaved_RRGGBB_LE : heif_chroma_interleaved_RRGGBB_BE;
                target_image_format = QImage::Format_RGBX64;
            }
        } else if (bit_depth == 8) {
            if (hasAlphaChannel) {
                chroma = heif_chroma_interleaved_RGBA;
                target_image_format = QImage::Format_ARGB32;
            } else {
                chroma = heif_chroma_interleaved_RGB;
                target_image_format = QImage::Format_RGB32;
            }
        } else {
            m_parseState = ParseHeicError;
            if (bit_depth > 0) {
                qWarning() << "Unsupported bit depth:" << bit_depth;
            } else {
                qWarning() << "Undefined bit depth.";
            }
            return false;
        }

        heif::Image img = handle.decode_image(heif_colorspace_RGB, chroma);

        const int imageWidth = img.get_width(heif_channel_interleaved);
        const int imageHeight = img.get_height(heif_channel_interleaved);

        QSize imageSize(imageWidth, imageHeight);

        if (!imageSize.isValid()) {
            m_parseState = ParseHeicError;
            qWarning() << "HEIC image size invalid:" << imageSize;
            return false;
        }

        int stride = 0;
        const uint8_t *const src = img.get_plane(heif_channel_interleaved, &stride);

        if (!src || stride <= 0) {
            m_parseState = ParseHeicError;
            qWarning() << "HEIC data pixels information not valid!";
            return false;
        }

        m_current_image = QImage(imageSize, target_image_format);
        if (m_current_image.isNull()) {
            m_parseState = ParseHeicError;
            qWarning() << "Unable to allocate memory!";
            return false;
        }

        switch (bit_depth) {
        case 12:
            if (hasAlphaChannel) {
                for (int y = 0; y < imageHeight; y++) {
                    const uint16_t *src_word = reinterpret_cast<const uint16_t *>(src + (y * stride));
                    uint16_t *dest_data = reinterpret_cast<uint16_t *>(m_current_image.scanLine(y));
                    for (int x = 0; x < imageWidth; x++) {
                        int tmpvalue;
                        // R
                        tmpvalue = (int)(((float)(0x0fff & (*src_word)) / 4095.0f) * 65535.0f + 0.5f);
                        tmpvalue = qBound(0, tmpvalue, 65535);
                        *dest_data = (uint16_t)tmpvalue;
                        src_word++;
                        dest_data++;
                        // G
                        tmpvalue = (int)(((float)(0x0fff & (*src_word)) / 4095.0f) * 65535.0f + 0.5f);
                        tmpvalue = qBound(0, tmpvalue, 65535);
                        *dest_data = (uint16_t)tmpvalue;
                        src_word++;
                        dest_data++;
                        // B
                        tmpvalue = (int)(((float)(0x0fff & (*src_word)) / 4095.0f) * 65535.0f + 0.5f);
                        tmpvalue = qBound(0, tmpvalue, 65535);
                        *dest_data = (uint16_t)tmpvalue;
                        src_word++;
                        dest_data++;
                        // A
                        tmpvalue = (int)(((float)(0x0fff & (*src_word)) / 4095.0f) * 65535.0f + 0.5f);
                        tmpvalue = qBound(0, tmpvalue, 65535);
                        *dest_data = (uint16_t)tmpvalue;
                        src_word++;
                        dest_data++;
                    }
                }
            } else { // no alpha channel
                for (int y = 0; y < imageHeight; y++) {
                    const uint16_t *src_word = reinterpret_cast<const uint16_t *>(src + (y * stride));
                    uint16_t *dest_data = reinterpret_cast<uint16_t *>(m_current_image.scanLine(y));
                    for (int x = 0; x < imageWidth; x++) {
                        int tmpvalue;
                        // R
                        tmpvalue = (int)(((float)(0x0fff & (*src_word)) / 4095.0f) * 65535.0f + 0.5f);
                        tmpvalue = qBound(0, tmpvalue, 65535);
                        *dest_data = (uint16_t)tmpvalue;
                        src_word++;
                        dest_data++;
                        // G
                        tmpvalue = (int)(((float)(0x0fff & (*src_word)) / 4095.0f) * 65535.0f + 0.5f);
                        tmpvalue = qBound(0, tmpvalue, 65535);
                        *dest_data = (uint16_t)tmpvalue;
                        src_word++;
                        dest_data++;
                        // B
                        tmpvalue = (int)(((float)(0x0fff & (*src_word)) / 4095.0f) * 65535.0f + 0.5f);
                        tmpvalue = qBound(0, tmpvalue, 65535);
                        *dest_data = (uint16_t)tmpvalue;
                        src_word++;
                        dest_data++;
                        // X = 0xffff
                        *dest_data = 0xffff;
                        dest_data++;
                    }
                }
            }
            break;
        case 10:
            if (hasAlphaChannel) {
                for (int y = 0; y < imageHeight; y++) {
                    const uint16_t *src_word = reinterpret_cast<const uint16_t *>(src + (y * stride));
                    uint16_t *dest_data = reinterpret_cast<uint16_t *>(m_current_image.scanLine(y));
                    for (int x = 0; x < imageWidth; x++) {
                        int tmpvalue;
                        // R
                        tmpvalue = (int)(((float)(0x03ff & (*src_word)) / 1023.0f) * 65535.0f + 0.5f);
                        tmpvalue = qBound(0, tmpvalue, 65535);
                        *dest_data = (uint16_t)tmpvalue;
                        src_word++;
                        dest_data++;
                        // G
                        tmpvalue = (int)(((float)(0x03ff & (*src_word)) / 1023.0f) * 65535.0f + 0.5f);
                        tmpvalue = qBound(0, tmpvalue, 65535);
                        *dest_data = (uint16_t)tmpvalue;
                        src_word++;
                        dest_data++;
                        // B
                        tmpvalue = (int)(((float)(0x03ff & (*src_word)) / 1023.0f) * 65535.0f + 0.5f);
                        tmpvalue = qBound(0, tmpvalue, 65535);
                        *dest_data = (uint16_t)tmpvalue;
                        src_word++;
                        dest_data++;
                        // A
                        tmpvalue = (int)(((float)(0x03ff & (*src_word)) / 1023.0f) * 65535.0f + 0.5f);
                        tmpvalue = qBound(0, tmpvalue, 65535);
                        *dest_data = (uint16_t)tmpvalue;
                        src_word++;
                        dest_data++;
                    }
                }
            } else { // no alpha channel
                for (int y = 0; y < imageHeight; y++) {
                    const uint16_t *src_word = reinterpret_cast<const uint16_t *>(src + (y * stride));
                    uint16_t *dest_data = reinterpret_cast<uint16_t *>(m_current_image.scanLine(y));
                    for (int x = 0; x < imageWidth; x++) {
                        int tmpvalue;
                        // R
                        tmpvalue = (int)(((float)(0x03ff & (*src_word)) / 1023.0f) * 65535.0f + 0.5f);
                        tmpvalue = qBound(0, tmpvalue, 65535);
                        *dest_data = (uint16_t)tmpvalue;
                        src_word++;
                        dest_data++;
                        // G
                        tmpvalue = (int)(((float)(0x03ff & (*src_word)) / 1023.0f) * 65535.0f + 0.5f);
                        tmpvalue = qBound(0, tmpvalue, 65535);
                        *dest_data = (uint16_t)tmpvalue;
                        src_word++;
                        dest_data++;
                        // B
                        tmpvalue = (int)(((float)(0x03ff & (*src_word)) / 1023.0f) * 65535.0f + 0.5f);
                        tmpvalue = qBound(0, tmpvalue, 65535);
                        *dest_data = (uint16_t)tmpvalue;
                        src_word++;
                        dest_data++;
                        // X = 0xffff
                        *dest_data = 0xffff;
                        dest_data++;
                    }
                }
            }
            break;
        case 8:
            if (hasAlphaChannel) {
                for (int y = 0; y < imageHeight; y++) {
                    const uint8_t *src_byte = src + (y * stride);
                    uint32_t *dest_pixel = reinterpret_cast<uint32_t *>(m_current_image.scanLine(y));
                    for (int x = 0; x < imageWidth; x++) {
                        int red = *src_byte++;
                        int green = *src_byte++;
                        int blue = *src_byte++;
                        int alpha = *src_byte++;
                        *dest_pixel = qRgba(red, green, blue, alpha);
                        dest_pixel++;
                    }
                }
            } else { // no alpha channel
                for (int y = 0; y < imageHeight; y++) {
                    const uint8_t *src_byte = src + (y * stride);
                    uint32_t *dest_pixel = reinterpret_cast<uint32_t *>(m_current_image.scanLine(y));
                    for (int x = 0; x < imageWidth; x++) {
                        int red = *src_byte++;
                        int green = *src_byte++;
                        int blue = *src_byte++;
                        *dest_pixel = qRgb(red, green, blue);
                        dest_pixel++;
                    }
                }
            }
            break;
        default:
            m_parseState = ParseHeicError;
            qWarning() << "Unsupported bit depth:" << bit_depth;
            return false;
            break;
        }

        heif_color_profile_type profileType = heif_image_handle_get_color_profile_type(handle.get_raw_image_handle());
        struct heif_error err;
        if (profileType == heif_color_profile_type_prof || profileType == heif_color_profile_type_rICC) {
            int rawProfileSize = (int)heif_image_handle_get_raw_color_profile_size(handle.get_raw_image_handle());
            if (rawProfileSize > 0) {
                QByteArray ba(rawProfileSize, 0);
                err = heif_image_handle_get_raw_color_profile(handle.get_raw_image_handle(), ba.data());
                if (err.code) {
                    qWarning() << "icc profile loading failed";
                } else {
                    m_current_image.setColorSpace(QColorSpace::fromIccProfile(ba));
                    if (!m_current_image.colorSpace().isValid()) {
                        qWarning() << "HEIC image has Qt-unsupported or invalid ICC profile!";
                    }
                }
            } else {
                qWarning() << "icc profile is empty";
            }

        } else if (profileType == heif_color_profile_type_nclx) {
            struct heif_color_profile_nclx *nclx = nullptr;
            err = heif_image_handle_get_nclx_color_profile(handle.get_raw_image_handle(), &nclx);
            if (err.code || !nclx) {
                qWarning() << "nclx profile loading failed";
            } else {
                const QPointF redPoint(nclx->color_primary_red_x, nclx->color_primary_red_y);
                const QPointF greenPoint(nclx->color_primary_green_x, nclx->color_primary_green_y);
                const QPointF bluePoint(nclx->color_primary_blue_x, nclx->color_primary_blue_y);
                const QPointF whitePoint(nclx->color_primary_white_x, nclx->color_primary_white_y);

                QColorSpace::TransferFunction q_trc = QColorSpace::TransferFunction::Custom;
                float q_trc_gamma = 0.0f;

                switch (nclx->transfer_characteristics) {
                case 4:
                    q_trc = QColorSpace::TransferFunction::Gamma;
                    q_trc_gamma = 2.2f;
                    break;
                case 5:
                    q_trc = QColorSpace::TransferFunction::Gamma;
                    q_trc_gamma = 2.8f;
                    break;
                case 8:
                    q_trc = QColorSpace::TransferFunction::Linear;
                    break;
                case 2:
                case 13:
                    q_trc = QColorSpace::TransferFunction::SRgb;
                    break;
                default:
                    qWarning("CICP color_primaries: %d, transfer_characteristics: %d\nThe colorspace is unsupported by this plug-in yet.",
                             nclx->color_primaries,
                             nclx->transfer_characteristics);
                    q_trc = QColorSpace::TransferFunction::SRgb;
                    break;
                }

                if (q_trc != QColorSpace::TransferFunction::Custom) { // we create new colorspace using Qt
                    switch (nclx->color_primaries) {
                    case 1:
                    case 2:
                        m_current_image.setColorSpace(QColorSpace(QColorSpace::Primaries::SRgb, q_trc, q_trc_gamma));
                        break;
                    case 12:
                        m_current_image.setColorSpace(QColorSpace(QColorSpace::Primaries::DciP3D65, q_trc, q_trc_gamma));
                        break;
                    default:
                        m_current_image.setColorSpace(QColorSpace(whitePoint, redPoint, greenPoint, bluePoint, q_trc, q_trc_gamma));
                        break;
                    }
                }
                heif_nclx_color_profile_free(nclx);

                if (!m_current_image.colorSpace().isValid()) {
                    qWarning() << "HEIC plugin created invalid QColorSpace from NCLX!";
                }
            }

        } else {
            m_current_image.setColorSpace(QColorSpace(QColorSpace::SRgb));
        }

    } catch (const heif::Error &err) {
        m_parseState = ParseHeicError;
        qWarning() << "libheif error:" << err.get_message().c_str();
        return false;
    }

    m_parseState = ParseHeicSuccess;
    return true;
}

QImageIOPlugin::Capabilities HEIFPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "heif" || format == "heic") {
        Capabilities format_cap;
        if (heif_have_decoder_for_format(heif_compression_HEVC)) {
            format_cap |= CanRead;
        }
        if (heif_have_encoder_for_format(heif_compression_HEVC)) {
            format_cap |= CanWrite;
        }
        return format_cap;
    }
    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && HEIFHandler::canRead(device) && heif_have_decoder_for_format(heif_compression_HEVC)) {
        cap |= CanRead;
    }

    if (device->isWritable() && heif_have_encoder_for_format(heif_compression_HEVC)) {
        cap |= CanWrite;
    }
    return cap;
}

QImageIOHandler *HEIFPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new HEIFHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}
