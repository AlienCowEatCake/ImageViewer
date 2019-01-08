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

#include <algorithm>

#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QRgb>
#include <QDebug>

#include <openjpeg.h>

#include "Utils/Global.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"
#include "Internal/Utils/CmsUtils.h"

namespace
{

// ====================================================================================================

bool hasAlphaChannel(opj_image_t *img)
{
    for(OPJ_UINT32 i = 0; i < img->numcomps; i++)
        if(img->comps[i].alpha)
            return true;
    return false;
}

bool is420(opj_image_t *img)
{
    return((img->numcomps == 3)
            && (img->comps[0].dx == 1)
            && (img->comps[1].dx == 2)
            && (img->comps[2].dx == 2)
            && (img->comps[0].dy == 1)
            && (img->comps[1].dy == 2)
            && (img->comps[2].dy == 2)); // horizontal and vertical sub-sample
}

bool is422(opj_image_t *img)
{
    return((img->numcomps == 3)
            && (img->comps[0].dx == 1)
            && (img->comps[1].dx == 2)
            && (img->comps[2].dx == 2)
            && (img->comps[0].dy == 1)
            && (img->comps[1].dy == 1)
            && (img->comps[2].dy == 1)); // horizontal and vertical sub-sample
}

bool is444(opj_image_t *img)
{
    return((img->numcomps == 3)
            && (img->comps[0].dx == 1)
            && (img->comps[1].dx == 1)
            && (img->comps[2].dx == 1)
            && (img->comps[0].dy == 1)
            && (img->comps[1].dy == 1)
            && (img->comps[2].dy == 1)); // no sub-sample
}

// ====================================================================================================

#define CONSTRAINT_COMPONENTS_NUMBER_EQUAL(IMG, NUM) \
    if((IMG)->numcomps != NUM) \
    { \
        qDebug() << "Failed" << __FUNCTION__; \
        qDebug() << " > Reason: numcomps" << (IMG)->numcomps << "!=" << NUM; \
        return QImage(); \
    }

#define CONSTRAINT_COMPONENTS_NUMBER_GREAT_OR_EQUAL(IMG, NUM) \
    if((IMG)->numcomps < NUM) \
    { \
        qDebug() << "Failed" << __FUNCTION__; \
        qDebug() << " > Reason: numcomps" << (IMG)->numcomps << "<" << NUM; \
        return QImage(); \
    }

#define CONSTRAINT_WITHOUT_ALPHA_CHANNEL(IMG) \
    for(OPJ_UINT32 i = 0; i < (IMG)->numcomps; i++) \
    { \
        if((IMG)->comps[i].alpha) \
        { \
            qDebug() << "Failed" << __FUNCTION__; \
            qDebug() << " > Reason: image contains alpha component" << i; \
            return QImage(); \
        } \
    }

#define CONSTRAINT_WITH_ALPHA_CHANNEL(IMG) \
    if(!hasAlphaChannel(IMG)) \
    { \
        qDebug() << "Failed" << __FUNCTION__; \
        qDebug() << " > Reason: image not contains alpha component"; \
        return QImage(); \
    }

#define CONSTRAINT_HAS_USUAL_YCC_COMPONENTS(IMG) \
    if(!(is420(IMG) || is422(IMG) || is444(IMG))) \
    { \
        qDebug() << "Failed" << __FUNCTION__; \
        qDebug() << " > Reason: image is not match usual YCC component configuration (444, 422 or 420)"; \
        return QImage(); \
    }

#define RETURN_CONVERTATION_RESULT(QIMG) \
    { \
        QImage result = (QIMG); \
        if(!result.isNull()) \
            qDebug() << "Completed" << __FUNCTION__; \
        return result; \
    }

// ====================================================================================================

QRgb syccToQRgb(opj_image_t *img, int componentsData[])
{
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

    const int y = componentsData[0];
    const int cb = componentsData[1] - (1 << (static_cast<int>(img->comps[1].prec) - 1));
    const int cr = componentsData[2] - (1 << (static_cast<int>(img->comps[2].prec) - 1));

    const float sy = 255.0f / ((1 << static_cast<int>(img->comps[0].prec)) - 1);
    const float scb = 255.0f / ((1 << static_cast<int>(img->comps[1].prec)) - 1);
    const float scr = 255.0f / ((1 << static_cast<int>(img->comps[2].prec)) - 1);

    const float yf = static_cast<float>(y) * sy;
    const float cbf = static_cast<float>(cb) * scb;
    const float crf = static_cast<float>(cr) * scr;

    return qRgb(qBound(0, static_cast<int>(yf + 1.402f * crf), 255),
                qBound(0, static_cast<int>(yf - (0.344f * cbf + 0.714f * crf)), 255),
                qBound(0, static_cast<int>(yf + 1.772f * cbf), 255));
}

QRgb cmykToQRgb(opj_image_t *img, int componentsData[])
{
    const float sC = 1.0f / static_cast<float>((1 << img->comps[0].prec) - 1);
    const float sM = 1.0f / static_cast<float>((1 << img->comps[1].prec) - 1);
    const float sY = 1.0f / static_cast<float>((1 << img->comps[2].prec) - 1);
    const float sK = 1.0f / static_cast<float>((1 << img->comps[3].prec) - 1);

    // CMYK values from 0 to 1
    float C = static_cast<float>(componentsData[0]) * sC;
    float M = static_cast<float>(componentsData[1]) * sM;
    float Y = static_cast<float>(componentsData[2]) * sY;
    float K = static_cast<float>(componentsData[3]) * sK;

    // Invert all CMYK values
    C = 1.0f - C;
    M = 1.0f - M;
    Y = 1.0f - Y;
    K = 1.0f - K;

    // CMYK -> RGB : RGB results from 0 to 255
    return qRgb(qBound(0, static_cast<int>(255.0f * C * K), 255),     // R
                qBound(0, static_cast<int>(255.0f * M * K), 255),     // G
                qBound(0, static_cast<int>(255.0f * Y * K), 255));    // B
}

QRgb esyccToQRgb(opj_image_t *img, int componentsData[])
{
    const int flip_value = (1 << (img->comps[0].prec - 1));
    const int max_value = (1 << img->comps[0].prec) - 1;
    const int sign1 = static_cast<int>(img->comps[1].sgnd);
    const int sign2 = static_cast<int>(img->comps[2].sgnd);

    const float correction = 255.0f / max_value;

    int y = componentsData[0];
    int cb = componentsData[1];
    int cr = componentsData[2];

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

    return qRgb(qBound(0, static_cast<int>(r * correction), 255),
                qBound(0, static_cast<int>(g * correction), 255),
                qBound(0, static_cast<int>(b * correction), 255));
}

QRgb rgbToQRgb(opj_image_t *img, int componentsData[])
{
    const float corrections[3] = {
        255.0f / ((1 << static_cast<int>(img->comps[0].prec)) - 1),
        255.0f / ((1 << static_cast<int>(img->comps[1].prec)) - 1),
        255.0f / ((1 << static_cast<int>(img->comps[2].prec)) - 1)
    };
    return qRgb(qBound(0, static_cast<int>(static_cast<float>(componentsData[0]) * corrections[0]), 255),
                qBound(0, static_cast<int>(static_cast<float>(componentsData[1]) * corrections[1]), 255),
                qBound(0, static_cast<int>(static_cast<float>(componentsData[2]) * corrections[2]), 255));
}

QRgb rgbaToQRgb(opj_image_t *img, int componentsData[])
{
    const float corrections[4] = {
        255.0f / ((1 << static_cast<int>(img->comps[0].prec)) - 1),
        255.0f / ((1 << static_cast<int>(img->comps[1].prec)) - 1),
        255.0f / ((1 << static_cast<int>(img->comps[2].prec)) - 1),
        255.0f / ((1 << static_cast<int>(img->comps[3].prec)) - 1)
    };
    const bool isARGB = img->comps[0].alpha;
    const size_t indexesARGB[4] = {1, 2, 3, 0};
    const size_t indexesRGBA[4] = {0, 1, 2, 3};
    const size_t *indexes = isARGB ? indexesARGB : indexesRGBA;
    return qRgba(qBound(0, static_cast<int>(static_cast<float>(componentsData[indexes[0]]) * corrections[indexes[0]]), 255),
                 qBound(0, static_cast<int>(static_cast<float>(componentsData[indexes[1]]) * corrections[indexes[1]]), 255),
                 qBound(0, static_cast<int>(static_cast<float>(componentsData[indexes[2]]) * corrections[indexes[2]]), 255),
                 qBound(0, static_cast<int>(static_cast<float>(componentsData[indexes[3]]) * corrections[indexes[3]]), 255));
}

QRgb grayToQRgb(opj_image_t *img, int componentsData[])
{
    const float correction = 255.0f / ((1 << static_cast<int>(img->comps[0].prec)) - 1);
    const int value = qBound(0, static_cast<int>(static_cast<float>(componentsData[0]) * correction), 255);
    return qRgb(value, value, value);
}

QRgb grayAlphaToQRgb(opj_image_t *img, int componentsData[])
{
    const float corrections[2] = {
        255.0f / ((1 << static_cast<int>(img->comps[0].prec)) - 1),
        255.0f / ((1 << static_cast<int>(img->comps[1].prec)) - 1)
    };
    const bool isAlphaFirst = img->comps[0].alpha;
    const size_t indexesAlphaFirst[4] = {1, 0};
    const size_t indexesAlphaLast[4] = {0, 1};
    const size_t *indexes = isAlphaFirst ? indexesAlphaFirst : indexesAlphaLast;
    const int value = qBound(0, static_cast<int>(static_cast<float>(componentsData[indexes[0]]) * corrections[indexes[0]]), 255);
    const int alphaValue = qBound(0, static_cast<int>(static_cast<float>(componentsData[indexes[1]]) * corrections[indexes[1]]), 255);
    return qRgba(value, value, value, alphaValue);
}

// ====================================================================================================

typedef QRgb(*componentsDataToQRgb)(opj_image_t *img, int componentsData[]);

template <const size_t numComps>
QImage convertToQImage(opj_image_t *img, componentsDataToQRgb func)
{
    CONSTRAINT_COMPONENTS_NUMBER_GREAT_OR_EQUAL(img, numComps);

    const OPJ_UINT32 width = img->x1 - img->x0;
    const OPJ_UINT32 height = img->y1 - img->y0;

    int upperBounds[numComps];
    int *componentsDataBounds[numComps];
    for(size_t i = 0; i < numComps; i++)
    {
        upperBounds[i] = (1 << static_cast<int>(img->comps[0].prec)) - 1;
        componentsDataBounds[i] = img->comps[i].data + img->comps[i].w * img->comps[i].h;
    }

    QImage result(static_cast<int>(width), static_cast<int>(height), QImage::Format_ARGB32);
    if(result.isNull())
    {
        qWarning() << "Image is too large";
        return QImage();
    }

    for(OPJ_UINT32 currentY = 0; currentY < height; currentY++)
    {
        int *componentsData[numComps];
        int *componentsDataLineBounds[numComps];
        for(size_t i = 0; i < numComps; i++)
        {
            componentsData[i] = img->comps[i].data + currentY / img->comps[i].dy * img->comps[i].w;
            componentsDataLineBounds[i] = componentsData[i] + img->comps[i].w;
        }

        QRgb *p = reinterpret_cast<QRgb*>(result.scanLine(static_cast<int>(currentY)));

        for(OPJ_UINT32 currentX = 0; currentX < width; currentX++)
        {
            int currentComponentsData[numComps];
            for(size_t i = 0; i < numComps; i++)
            {
                currentComponentsData[i] = img->comps[i].alpha ? upperBounds[i] : 0;
                if(img->comps[i].x0 > currentX + img->x0 || img->comps[i].y0 > currentY + img->y0)
                    continue;
                if(componentsData[i] >= componentsDataBounds[i])
                    continue;
                if(componentsData[i] >= componentsDataLineBounds[i])
                    continue;
                currentComponentsData[i] = *(componentsData[i]);
                if(!(currentX % img->comps[i].dx))
                    componentsData[i]++;
            }
            *(p++) = func(img, currentComponentsData);
        }
    }
    return result;
}

QImage syccToQImage(opj_image_t *img)
{
    CONSTRAINT_COMPONENTS_NUMBER_EQUAL(img, 3)
    CONSTRAINT_WITHOUT_ALPHA_CHANNEL(img)
    CONSTRAINT_HAS_USUAL_YCC_COMPONENTS(img)
    RETURN_CONVERTATION_RESULT(convertToQImage<3>(img, syccToQRgb));
}

QImage cmykToQImage(opj_image_t *img)
{
    CONSTRAINT_COMPONENTS_NUMBER_EQUAL(img, 4)
    CONSTRAINT_WITHOUT_ALPHA_CHANNEL(img)
    RETURN_CONVERTATION_RESULT(convertToQImage<4>(img, cmykToQRgb));
}

QImage esyccToQImage(opj_image_t *img)
{
    CONSTRAINT_COMPONENTS_NUMBER_EQUAL(img, 3)
    CONSTRAINT_WITHOUT_ALPHA_CHANNEL(img)
    CONSTRAINT_HAS_USUAL_YCC_COMPONENTS(img)
    RETURN_CONVERTATION_RESULT(convertToQImage<3>(img, esyccToQRgb));
}

QImage rgbToQImage(opj_image_t *img)
{
    CONSTRAINT_COMPONENTS_NUMBER_EQUAL(img, 3)
    CONSTRAINT_WITHOUT_ALPHA_CHANNEL(img)
    RETURN_CONVERTATION_RESULT(convertToQImage<3>(img, rgbToQRgb));
}

QImage rgbaToQImage(opj_image_t *img)
{
    CONSTRAINT_COMPONENTS_NUMBER_EQUAL(img, 4)
    CONSTRAINT_WITH_ALPHA_CHANNEL(img)
    RETURN_CONVERTATION_RESULT(convertToQImage<4>(img, rgbaToQRgb));
}

QImage grayToQImage(opj_image *img)
{
    CONSTRAINT_COMPONENTS_NUMBER_EQUAL(img, 1)
    CONSTRAINT_WITHOUT_ALPHA_CHANNEL(img)
    RETURN_CONVERTATION_RESULT(convertToQImage<1>(img, grayToQRgb));
}

QImage grayAlphaToQImage(opj_image *img)
{
    CONSTRAINT_COMPONENTS_NUMBER_EQUAL(img, 2)
    CONSTRAINT_WITH_ALPHA_CHANNEL(img)
    RETURN_CONVERTATION_RESULT(convertToQImage<2>(img, grayAlphaToQRgb));
}

QImage anyToQImage(opj_image *img)
{
    CONSTRAINT_COMPONENTS_NUMBER_GREAT_OR_EQUAL(img, 1)
    RETURN_CONVERTATION_RESULT(convertToQImage<1>(img, grayToQRgb));
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
    static const QByteArray j2kMarker("\xff\x4f\xff\x51", 4);
    static const QByteArray jp2Marker1("\x0d\x0a\x87\x0a", 4);
    static const QByteArray jp2Marker2("\x00\x00\x00\x0c\x6a\x50\x20\x20\x0d\x0a\x87\x0a", 12);
    QFile file(filePath);
    if(file.open(QIODevice::ReadOnly))
    {
        const QByteArray data = file.read(std::max(j2kMarker.size(), std::max(jp2Marker1.size(), jp2Marker2.size())));
        if(data.size() >= j2kMarker.size() && data.startsWith(j2kMarker))
            return OPJ_CODEC_J2K;
        if((data.size() >= jp2Marker1.size() && data.startsWith(jp2Marker1)) ||
           (data.size() >= jp2Marker2.size() && data.startsWith(jp2Marker2)))
            return OPJ_CODEC_JP2;
    }

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

    opj_image_t* image = Q_NULLPTR;
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
    if(image->comps[0].data == Q_NULLPTR)
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

    const bool hasAlpha = hasAlphaChannel(image);

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
        result = (hasAlpha ? rgbaToQImage(image) : rgbToQImage(image));
        break;
    case OPJ_CLRSPC_GRAY:           // grayscale
        qDebug() << "color_space = OPJ_CLRSPC_GRAY";
        result = (hasAlpha ? grayAlphaToQImage(image) : grayToQImage(image));
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
        result = rgbaToQImage(image);

    if(result.isNull())
        result = cmykToQImage(image);

    if(result.isNull())
        result = syccToQImage(image);

    if(result.isNull())
        result = grayToQImage(image);

    if(result.isNull())
        result = grayAlphaToQImage(image);

    if(result.isNull())
        result = anyToQImage(image);

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
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderOpenJPEG");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
                << QString::fromLatin1("jp2")
                << QString::fromLatin1("j2k")
                << QString::fromLatin1("jpc")
                << QString::fromLatin1("j2c")
                << QString::fromLatin1("jpt")
                   ;
    }

    QStringList advancedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList();
    }

    bool isAvailable() const Q_DECL_OVERRIDE
    {
        return true;
    }

    QSharedPointer<IImageData> loadImage(const QString &filePath) Q_DECL_OVERRIDE
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return QSharedPointer<IImageData>();
        QGraphicsItem *item = GraphicsItemsFactory::instance().createImageItem(readFile(filePath));
        IImageMetaData *metaData = ImageMetaData::createMetaData(filePath);
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), metaData));
    }
};

DecoderAutoRegistrator registrator(new DecoderOpenJPEG);

// ====================================================================================================

} // namespace
