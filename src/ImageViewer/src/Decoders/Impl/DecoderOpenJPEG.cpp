/*
   Copyright (C) 2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <algorithm>

#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QRgb>
#include <QDebug>

#include <openjpeg.h>

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/Utils/CmsUtils.h"

namespace
{

// ====================================================================================================

/// @note Based on openjpeg-2.3.0/src/bin/common/color.c

// Matrix for sYCC, Amendment 1 to IEC 61966-2-1
//
// Y :   0.299   0.587    0.114   :R
// Cb:  -0.1687 -0.3312   0.5     :G
// Cr:   0.5    -0.4187  -0.0812  :B
//
// Inverse:
//
// R: 1        -3.68213e-05    1.40199      :Y
// G: 1.00003  -0.344125      -0.714128     :Cb - 2^(prec - 1)
// B: 0.999823  1.77204       -8.04142e-06  :Cr - 2^(prec - 1)

QRgb syccToQRgb(int offset, int upb, int y, int cb, int cr)
{
    cb -= offset;
    cr -= offset;
    const float correction = 255.0f / upb;
    const float yf = static_cast<float>(y);
    const float cbf = static_cast<float>(cb);
    const float crf = static_cast<float>(cr);
    return qRgb(qBound(0, static_cast<int>((yf + 1.402f * crf) * correction), 255),
                qBound(0, static_cast<int>((yf - (0.344f * cbf + 0.714f * crf)) * correction), 255),
                qBound(0, static_cast<int>((yf + 1.772f * cbf) * correction), 255));
}

QImage sycc444ToQImage(opj_image_t *img)
{
    int upb = static_cast<int>(img->comps[0].prec);
    int offset = 1 << (upb - 1);
    upb = (1 << upb) - 1;

    const size_t maxw = static_cast<size_t>(img->comps[0].w);
    const size_t maxh = static_cast<size_t>(img->comps[0].h);

    const int *y = img->comps[0].data;
    const int *cb = img->comps[1].data;
    const int *cr = img->comps[2].data;

    QImage result(static_cast<int>(maxw), static_cast<int>(maxh), QImage::Format_RGB32);
    for(int i = 0; i < result.height(); i++)
    {
        QRgb *p = reinterpret_cast<QRgb*>(result.scanLine(i));
        for(int j = 0; j < result.width(); j++)
            *(p++) = syccToQRgb(offset, upb, *(y++), *(cb++), *(cr++));
    }
    return result;
}

QImage sycc422ToQImage(opj_image_t *img)
{
    int upb = static_cast<int>(img->comps[0].prec);
    int offset = 1 << (upb - 1);
    upb = (1 << upb) - 1;

    const size_t maxw = static_cast<size_t>(img->comps[0].w);
    const size_t maxh = static_cast<size_t>(img->comps[0].h);

    const int *y = img->comps[0].data;
    const int *cb = img->comps[1].data;
    const int *cr = img->comps[2].data;

    // if img->x0 is odd, then first column shall use Cb/Cr = 0
    const size_t offx = img->x0 & 1U;
    const size_t loopmaxw = maxw - offx;

    QImage result(static_cast<int>(maxw), static_cast<int>(maxh), QImage::Format_RGB32);
    if(result.isNull())
    {
        qWarning() << "Image is too large";
        return QImage();
    }

    for(int i = 0; i < result.height(); i++)
    {
        QRgb *p = reinterpret_cast<QRgb*>(result.scanLine(i));

        if(offx > 0)
            *(p++) = syccToQRgb(offset, upb, *(y++), 0, 0);

        size_t j;
        for(j = 0; j < (loopmaxw & ~static_cast<size_t>(1)); j += 2)
        {
            *(p++) = syccToQRgb(offset, upb, *(y++), *cb, *cr);
            *(p++) = syccToQRgb(offset, upb, *(y++), *(cb++), *(cr++));
        }
        if(j < loopmaxw)
            *(p++) = syccToQRgb(offset, upb, *(y++), *(cb++), *(cr++));
    }
    return result;
}

QImage sycc420ToQImage(opj_image_t *img)
{
    int upb = static_cast<int>(img->comps[0].prec);
    int offset = 1 << (upb - 1);
    upb = (1 << upb) - 1;

    const size_t maxw = static_cast<size_t>(img->comps[0].w);
    const size_t maxh = static_cast<size_t>(img->comps[0].h);

    const int *y = img->comps[0].data;
    const int *cb = img->comps[1].data;
    const int *cr = img->comps[2].data;

    // if img->x0 is odd, then first column shall use Cb/Cr = 0
    const size_t offx = img->x0 & 1U;
    const size_t loopmaxw = maxw - offx;
    // if img->y0 is odd, then first line shall use Cb/Cr = 0
    const size_t offy = img->y0 & 1U;
    const size_t loopmaxh = maxh - offy;

    QImage result(static_cast<int>(maxw), static_cast<int>(maxh), QImage::Format_RGB32);
    if(result.isNull())
    {
        qWarning() << "Image is too large";
        return QImage();
    }

    QRgb *p = reinterpret_cast<QRgb*>(result.scanLine(0));

    if(offy > 0)
        for(size_t j = 0; j < maxw; ++j)
            *(p++) = syccToQRgb(offset, upb, *(y++), 0, 0);

    size_t i;
    for(i = 0; i < (loopmaxh & ~static_cast<size_t>(1)); i += 2)
    {
        size_t j;

        const int *ny = y + maxw;
        QRgb *np = p + maxw;

        if(offx > 0)
        {
            *(p++) = syccToQRgb(offset, upb, *(y++), 0, 0);
            *(np++) = syccToQRgb(offset, upb, *(ny++), *cb, *cr);
        }

        for(j = 0; j < (loopmaxw & ~static_cast<size_t>(1)); j += 2)
        {
            *(p++) = syccToQRgb(offset, upb, *(y++), *cb, *cr);
            *(p++) = syccToQRgb(offset, upb, *(y++), *cb, *cr);
            *(np++) = syccToQRgb(offset, upb, *(ny++), *cb, *cr);
            *(np++) = syccToQRgb(offset, upb, *(ny++), *(cb++), *(cr++));
        }
        if(j < loopmaxw)
        {
            *(p++) = syccToQRgb(offset, upb, *(y++), *cb, *cr);
            *(np++) = syccToQRgb(offset, upb, *(ny++), *(cb++), *(cr++));
        }
        y += maxw;
        p += maxw;
    }
    if(i < loopmaxh)
    {
        size_t j;
        for(j = 0; j < (maxw & ~static_cast<size_t>(1)); j += 2)
        {
            *(p++) = syccToQRgb(offset, upb, *(y++), *cb, *cr);
            *(p++) = syccToQRgb(offset, upb, *(y++), *(cb++), *(cr++));
        }
        if(j < maxw)
            *p = syccToQRgb(offset, upb, *y, *cb, *cr);
    }
    return result;
}

QImage syccToQImage(opj_image_t *img)
{
    if((img->comps[0].dx == 1)
            && (img->comps[1].dx == 2)
            && (img->comps[2].dx == 2)
            && (img->comps[0].dy == 1)
            && (img->comps[1].dy == 2)
            && (img->comps[2].dy == 2)) // horizontal and vertical sub-sample
        return sycc420ToQImage(img);
    if((img->comps[0].dx == 1)
            && (img->comps[1].dx == 2)
            && (img->comps[2].dx == 2)
            && (img->comps[0].dy == 1)
            && (img->comps[1].dy == 1)
            && (img->comps[2].dy == 1)) // horizontal sub-sample only
        return sycc422ToQImage(img);
    if((img->comps[0].dx == 1)
            && (img->comps[1].dx == 1)
            && (img->comps[2].dx == 1)
            && (img->comps[0].dy == 1)
            && (img->comps[1].dy == 1)
            && (img->comps[2].dy == 1)) // no sub-sample
        return sycc444ToQImage(img);

    qDebug() << "Failed" << __FUNCTION__;
    return QImage();
}

QImage cmykToQImage(opj_image_t *img)
{
    if((img->numcomps < 4) ||
            (img->comps[0].dx != img->comps[1].dx) ||
            (img->comps[0].dx != img->comps[2].dx) ||
            (img->comps[0].dx != img->comps[3].dx) ||
            (img->comps[0].dy != img->comps[1].dy) ||
            (img->comps[0].dy != img->comps[2].dy) ||
            (img->comps[0].dy != img->comps[3].dy))
    {
        qDebug() << "Failed" << __FUNCTION__;
        return QImage();
    }

    const float sC = 1.0f / static_cast<float>((1 << img->comps[0].prec) - 1);
    const float sM = 1.0f / static_cast<float>((1 << img->comps[1].prec) - 1);
    const float sY = 1.0f / static_cast<float>((1 << img->comps[2].prec) - 1);
    const float sK = 1.0f / static_cast<float>((1 << img->comps[3].prec) - 1);

    size_t index = 0;

    QImage result(static_cast<int>(img->comps[0].w), static_cast<int>(img->comps[0].h), QImage::Format_RGB32);
    if(result.isNull())
    {
        qWarning() << "Image is too large";
        return QImage();
    }

    for(int i = 0; i < result.height(); i++)
    {
        QRgb *p = reinterpret_cast<QRgb*>(result.scanLine(i));
        for(int j = 0; j < result.width(); j++)
        {
            // CMYK values from 0 to 1
            float C = static_cast<float>(img->comps[0].data[index]) * sC;
            float M = static_cast<float>(img->comps[1].data[index]) * sM;
            float Y = static_cast<float>(img->comps[2].data[index]) * sY;
            float K = static_cast<float>(img->comps[3].data[index]) * sK;

            // Invert all CMYK values
            C = 1.0f - C;
            M = 1.0f - M;
            Y = 1.0f - Y;
            K = 1.0f - K;

            // CMYK -> RGB : RGB results from 0 to 255
            *(p++) = qRgb(qBound(0, static_cast<int>(255.0f * C * K), 255),     // R
                          qBound(0, static_cast<int>(255.0f * M * K), 255),     // G
                          qBound(0, static_cast<int>(255.0f * Y * K), 255));    // B

            index++;
        }
    }
    return result;
}

// This code has been adopted from sjpx_openjpeg.c of ghostscript
QImage esyccToQImage(opj_image_t *img)
{
    if((img->numcomps < 3) ||
            (img->comps[0].dx != img->comps[1].dx) ||
            (img->comps[0].dx != img->comps[2].dx) ||
            (img->comps[0].dy != img->comps[1].dy) ||
            (img->comps[0].dy != img->comps[2].dy))
    {
        qDebug() << "Failed" << __FUNCTION__;
        return QImage();
    }

    const int flip_value = (1 << (img->comps[0].prec - 1));
    const int max_value = (1 << img->comps[0].prec) - 1;
    const int sign1 = static_cast<int>(img->comps[1].sgnd);
    const int sign2 = static_cast<int>(img->comps[2].sgnd);

    const float correction = 255.0f / max_value;
    size_t index = 0;

    QImage result(static_cast<int>(img->comps[0].w), static_cast<int>(img->comps[0].h), QImage::Format_RGB32);
    if(result.isNull())
    {
        qWarning() << "Image is too large";
        return QImage();
    }

    for(int i = 0; i < result.height(); i++)
    {
        QRgb *p = reinterpret_cast<QRgb*>(result.scanLine(i));
        for(int j = 0; j < result.width(); j++)
        {
            const int y = img->comps[0].data[index];
            int cb = img->comps[1].data[index];
            int cr = img->comps[2].data[index];

            if(!sign1)
                cb -= flip_value;
            if(!sign2)
                cr -= flip_value;

            const float yf = static_cast<float>(y);
            const float cbf = static_cast<float>(cb);
            const float crf = static_cast<float>(cr);

            const float r = (yf - 0.0000368f * cbf + 1.40199f * crf + 0.5f);
            const float g = (1.0003f * yf - 0.344125f * cbf - 0.7141128f * crf + 0.5f);
            const float b = (0.999823f * yf + 1.77204f * cbf - 0.000008f * crf + 0.5f);

            *(p++) = qRgb(qBound(0, static_cast<int>(r * correction), 255),
                          qBound(0, static_cast<int>(g * correction), 255),
                          qBound(0, static_cast<int>(b * correction), 255));

            index++;
        }
    }
    return result;
}

QImage rgbToQImage(opj_image_t *img)
{
    if((img->numcomps != 3) ||
            (img->comps[0].dx != img->comps[1].dx) ||
            (img->comps[0].dx != img->comps[2].dx) ||
            (img->comps[0].dy != img->comps[1].dy) ||
            (img->comps[0].dy != img->comps[2].dy))
    {
        qDebug() << "Failed" << __FUNCTION__;
        return QImage();
    }

    const int upb = (1 << static_cast<int>(img->comps[0].prec)) - 1;
    const float correction = 255.0f / upb;

    const int *r = img->comps[0].data;
    const int *g = img->comps[1].data;
    const int *b = img->comps[2].data;

    QImage result(static_cast<int>(img->comps[0].w), static_cast<int>(img->comps[0].h), QImage::Format_RGB32);
    if(result.isNull())
    {
        qWarning() << "Image is too large";
        return QImage();
    }

    for(int i = 0; i < result.height(); i++)
    {
        QRgb *p = reinterpret_cast<QRgb*>(result.scanLine(i));
        for(int j = 0; j < result.width(); j++)
        {
            *(p++) = qRgb(qBound(0, static_cast<int>(static_cast<float>(*(r++)) * correction), 255),
                          qBound(0, static_cast<int>(static_cast<float>(*(g++)) * correction), 255),
                          qBound(0, static_cast<int>(static_cast<float>(*(b++)) * correction), 255));
        }
    }
    return result;
}

QImage grayToQImage(opj_image *img)
{
    const int upb = (1 << static_cast<int>(img->comps[0].prec)) - 1;
    const float correction = 255.0f / upb;

    const int *v = img->comps[0].data;

    QImage result(static_cast<int>(img->comps[0].w), static_cast<int>(img->comps[0].h), QImage::Format_RGB32);
    if(result.isNull())
    {
        qWarning() << "Image is too large";
        return QImage();
    }

    for(int i = 0; i < result.height(); i++)
    {
        QRgb *p = reinterpret_cast<QRgb*>(result.scanLine(i));
        for(int j = 0; j < result.width(); j++)
        {
            int color = qBound(0, static_cast<int>(static_cast<float>(*(v++)) * correction), 255);
            *(p++) = qRgb(color, color, color);
        }
    }
    return result;
}

// ====================================================================================================

void errorCallback(const char *msg, void *client_data)
{
    Q_UNUSED(client_data);
    qWarning() << "[ERROR]" << QString::fromLatin1(msg).simplified().toLocal8Bit().constData();
}

void warningCallback(const char *msg, void *client_data)
{
    Q_UNUSED(client_data);
    qWarning() << "[WARNING]" << QString::fromLatin1(msg).simplified().toLocal8Bit().constData();
}

void infoCallback(const char *msg, void *client_data)
{
    Q_UNUSED(client_data);
    Q_UNUSED(msg);
}

OPJ_SIZE_T streamReadCallback(void *buffer, OPJ_SIZE_T nbBytes, void *userData)
{
    QFile * const file = reinterpret_cast<QFile*>(userData);
    const qint64 read = file->read(reinterpret_cast<char*>(buffer), static_cast<qint64>(nbBytes));
    return static_cast<OPJ_SIZE_T>(read <= 0 ? -1 : read);
}

OPJ_OFF_T streamSkipCallback(OPJ_OFF_T nbBytes, void *userData)
{
    QFile * const file = reinterpret_cast<QFile*>(userData);
    if(!file->seek(static_cast<qint64>(nbBytes)))
        return -1;
    return nbBytes;
}

OPJ_BOOL streamSeekCallback(OPJ_OFF_T nbBytes, void *userData)
{
    QFile * const file = reinterpret_cast<QFile*>(userData);
    return file->seek(static_cast<qint64>(nbBytes)) ? OPJ_TRUE : OPJ_FALSE;
}

void streamFreeCallback(void *userData)
{
    QFile * const file = reinterpret_cast<QFile*>(userData);
    file->close();
}

OPJ_CODEC_FORMAT getCodecFormat(const QString &filePath)
{
    const QByteArray suffix = QFileInfo(filePath).suffix().toLower().toLatin1();
    if(suffix == "j2k" || suffix == "j2c" || suffix == "jpc")
        return OPJ_CODEC_J2K; // JPEG-2000 codestream
    if(suffix == "jp2")
        return OPJ_CODEC_JP2; // JPEG 2000 compressed image data
    if(suffix == "jpt")
        return OPJ_CODEC_JPT; // JPEG 2000, JPIP
    return OPJ_CODEC_UNKNOWN;
}

QImage readFile(const QString &filePath)
{
    /// @note Based on openjpeg-2.3.0/src/bin/jp2/opj_decompress.c

    //
    // read the input file and put it in memory
    //

    QFile inFile(filePath);
    if(!inFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "Can't open" << filePath;
        return QImage();
    }

    opj_stream_t *stream = opj_stream_default_create(OPJ_TRUE);
    if(!stream)
    {
        qWarning() << "ERROR -> failed to create the stream from the file" << filePath;
        return QImage();
    }

    opj_stream_set_user_data(stream, reinterpret_cast<void*>(&inFile), streamFreeCallback);
    opj_stream_set_user_data_length(stream, static_cast<OPJ_UINT64>(inFile.size()));
    opj_stream_set_read_function(stream, streamReadCallback);
    opj_stream_set_skip_function(stream, streamSkipCallback);
    opj_stream_set_seek_function(stream, streamSeekCallback);

    //
    // decode the JPEG2000 stream
    //

    const OPJ_CODEC_FORMAT codecFormat = getCodecFormat(filePath);
    if(codecFormat == OPJ_CODEC_UNKNOWN)
    {
        qWarning() << "ERROR -> unknown codec for the file" << filePath;
        opj_stream_destroy(stream);
        return QImage();
    }

    // Get a decoder handle
    opj_codec_t* codec = opj_create_decompress(codecFormat);

    // catch events using our callbacks and give a local context
    opj_set_info_handler(codec, infoCallback, 00);
    opj_set_warning_handler(codec, warningCallback, 00);
    opj_set_error_handler(codec, errorCallback, 00);

    opj_dparameters_t parameters;
    memset(&parameters, 0, sizeof(opj_dparameters_t));
    // default decoding parameters (core)
    opj_set_default_decoder_parameters(&parameters);

    // Setup the decoder decoding parameters using user parameters
    if(!opj_setup_decoder(codec, &parameters))
    {
        qWarning() << "ERROR -> opj_decompress: failed to setup the decoder";
        opj_stream_destroy(stream);
        opj_destroy_codec(codec);
        return QImage();
    }

    opj_image_t* image = NULL;
    // Read the main header of the codestream and if necessary the JP2 boxes
    if(!opj_read_header(stream, codec, &image))
    {
        qWarning() << "ERROR -> opj_decompress: failed to read the header";
        opj_stream_destroy(stream);
        opj_destroy_codec(codec);
        opj_image_destroy(image);
        return QImage();
    }

    // Get the decoded image
    if(!(opj_decode(codec, stream, image) && opj_end_decompress(codec, stream)))
    {
        qWarning() << "ERROR -> opj_decompress: failed to decode image!";
        opj_destroy_codec(codec);
        opj_stream_destroy(stream);
        opj_image_destroy(image);
        return QImage();
    }

    // FIXME? Shouldn't that situation be considered as an error of opj_decode() / opj_get_decoded_tile() ?
    if(image->comps[0].data == NULL)
    {
        qWarning() << "ERROR -> opj_decompress: no image data!";
        opj_destroy_codec(codec);
        opj_stream_destroy(stream);
        opj_image_destroy(image);
        return QImage();
    }

    // Close the byte stream
    opj_stream_destroy(stream);

    //
    // Convert image to Qt
    //

    if(image->color_space != OPJ_CLRSPC_SYCC && image->numcomps == 3 && image->comps[0].dx == image->comps[0].dy && image->comps[1].dx != 1)
        image->color_space = OPJ_CLRSPC_SYCC;
    else if(image->numcomps <= 2)
        image->color_space = OPJ_CLRSPC_GRAY;

    QImage result;
    switch(image->color_space)
    {
    case OPJ_CLRSPC_UNKNOWN:        // not supported by the library
        qDebug() << "color_space = OPJ_CLRSPC_UNKNOWN";
        break;
    case OPJ_CLRSPC_UNSPECIFIED:    // not specified in the codestream
        qDebug() << "color_space = OPJ_CLRSPC_UNSPECIFIED";
        break;
    case OPJ_CLRSPC_SRGB:           // sRGB
        qDebug() << "color_space = OPJ_CLRSPC_SRGB";
        result = rgbToQImage(image);
        break;
    case OPJ_CLRSPC_GRAY:           // grayscale
        qDebug() << "color_space = OPJ_CLRSPC_GRAY";
        result = grayToQImage(image);
        break;
    case OPJ_CLRSPC_SYCC:           // YUV
        qDebug() << "color_space = OPJ_CLRSPC_SYCC";
        result = syccToQImage(image);
        break;
    case OPJ_CLRSPC_EYCC:           // e-YCC
        qDebug() << "color_space = OPJ_CLRSPC_EYCC";
        result = esyccToQImage(image);
        break;
    case OPJ_CLRSPC_CMYK:           // CMYK
        qDebug() << "color_space = OPJ_CLRSPC_CMYK";
        result = cmykToQImage(image);
        break;
    default:
        qWarning() << "color_space = <INVALID>" << image->color_space;
        break;
    }

    if(result.isNull())
        result = rgbToQImage(image);

    if(result.isNull())
        result = cmykToQImage(image);

    if(result.isNull())
        result = syccToQImage(image);

    if(result.isNull())
        result = esyccToQImage(image);

    if(result.isNull())
        result = grayToQImage(image);

    if(result.isNull())
        return result;

    if(image->icc_profile_buf && image->icc_profile_len)
    {
        qDebug() << "Found ICCP metadata";
        ICCProfile profile(QByteArray::fromRawData(reinterpret_cast<const char*>(image->icc_profile_buf), static_cast<int>(image->icc_profile_len)));
        profile.applyToImage(&result);
    }

    return result;
}

// ====================================================================================================

class DecoderOpenJPEG : public IDecoder
{
public:
    QString name() const
    {
        return QString::fromLatin1("DecoderOpenJPEG");
    }

    QStringList supportedFormats() const
    {
        return QStringList()
                << QString::fromLatin1("jp2")
                << QString::fromLatin1("j2k")
                << QString::fromLatin1("jpc")
                << QString::fromLatin1("j2c")
                << QString::fromLatin1("jpt")
                   ;
    }

    QStringList advancedFormats() const
    {
        return QStringList();
    }

    QGraphicsItem *loadImage(const QString &filePath)
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return NULL;
        return GraphicsItemsFactory::instance().createImageItem(readFile(filePath));
    }
};

DecoderAutoRegistrator registrator(new DecoderOpenJPEG);

// ====================================================================================================

} // namespace
