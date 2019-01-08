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

#include "Workarounds/BeginIgnoreShiftNegative.h"
#include <jasper/jasper.h>
#include "Workarounds/EndIgnoreShiftNegative.h"

#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QDebug>

#include "Utils/Global.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"

namespace
{

// ====================================================================================================

bool to_sRGB(jas_image_t *& jasImage)
{
    jas_cmprof_t *outProf = jas_cmprof_createfromclrspc(JAS_CLRSPC_SRGB);
    if(!outProf)
    {
        qWarning() << "Can't create sRGB profile";
        return false;
    }
    if(!jas_image_cmprof(jasImage))
    {
        qWarning() << "Image has NULL cmprof";
        jas_cmprof_destroy(outProf);
        return false;
    }
    jas_image_t *newimage = jas_image_chclrspc(jasImage, outProf, JAS_CMXFORM_INTENT_PER);
    if(!newimage)
    {
        qWarning() << "Can't convert to sRGB";
        jas_cmprof_destroy(outProf);
        return false;
    }
    jas_image_destroy(jasImage);
    jas_cmprof_destroy(outProf);
    jasImage = newimage;
    return true;
}

QImage renderRgbImage(jas_image_t *jasImage)
{
    const int cmptlut[3] =
    {
        jas_image_getcmptbytype(jasImage, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R)),
        jas_image_getcmptbytype(jasImage, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G)),
        jas_image_getcmptbytype(jasImage, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B))
    };
    if(cmptlut[0] < 0 || cmptlut[1] < 0 || cmptlut[2] < 0)
    {
        qWarning() << "Incorrect component type";
        return QImage();
    }

    jas_image_coord_t width[3];
    jas_image_coord_t height[3];
    jas_image_coord_t tlx[3];
    jas_image_coord_t tly[3];
    jas_image_coord_t vs[3];
    jas_image_coord_t hs[3];
    for(int i = 0; i < 3; i++)
    {
        width[i] = jas_image_cmptwidth(jasImage, cmptlut[i]);
        height[i] = jas_image_cmptheight(jasImage, cmptlut[i]);
        tlx[i] = jas_image_cmpttlx(jasImage, cmptlut[i]);
        tly[i] = jas_image_cmpttly(jasImage, cmptlut[i]);
        vs[i] = jas_image_cmptvstep(jasImage, cmptlut[i]);
        hs[i] = jas_image_cmpthstep(jasImage, cmptlut[i]);
        if(i != 0 && (width[i] != width[0] || height[i] != height[0]))
            qWarning() << "Component geometry differs from image geometry";
    }

    QImage result(static_cast<int>(jas_image_width(jasImage)), static_cast<int>(jas_image_height(jasImage)), QImage::Format_ARGB32);
    if(result.isNull())
    {
        qWarning() << "Image is too large";
        return QImage();
    }

    for(int i = 0; i < result.height(); ++i)
    {
        QRgb *scanline = reinterpret_cast<QRgb*>(result.scanLine(i));
        for(int j = 0; j < result.width(); ++j)
        {
            int v[3] = {0, 0, 0};
            for(int k = 0; k < 3; ++k)
            {
                int x = static_cast<int>((j - tlx[k]) / hs[k]);
                int y = static_cast<int>((i - tly[k]) / vs[k]);
                if(x >= 0 && x < width[k] && y >= 0 && y < height[k])
                {
                    v[k] = jas_image_readcmptsample(jasImage, cmptlut[k], x, y);
                    v[k] <<= 16 - jas_image_cmptprec(jasImage, cmptlut[k]);
                    v[k] = std::min(std::max(v[k], 0), 65535) / (65535 / 255);
                }
            }
            scanline[j] = qRgb(v[0], v[1], v[2]);
        }
    }
    return result;
}

QImage renderGrayImage(jas_image_t *jasImage)
{
    const int cmptno = 0;
    if(cmptno >= jasImage->numcmpts_)
    {
        qWarning() << "Incorrect component number";
        return QImage();
    }

    jas_image_coord_t width = jas_image_cmptwidth(jasImage, cmptno);
    jas_image_coord_t height = jas_image_cmptheight(jasImage, cmptno);
    jas_image_coord_t tlx = jas_image_cmpttlx(jasImage, cmptno);
    jas_image_coord_t tly = jas_image_cmpttly(jasImage, cmptno);
    jas_image_coord_t vs = jas_image_cmptvstep(jasImage, cmptno);
    jas_image_coord_t hs = jas_image_cmpthstep(jasImage, cmptno);

    QImage result(static_cast<int>(jas_image_width(jasImage)), static_cast<int>(jas_image_height(jasImage)), QImage::Format_ARGB32);
    if(result.isNull())
    {
        qWarning() << "Image is too large";
        return QImage();
    }

    for(int i = 0; i < result.height(); ++i)
    {
        QRgb *scanline = reinterpret_cast<QRgb*>(result.scanLine(i));
        for(int j = 0; j < result.width(); ++j)
        {
            int v = 0;
            int x = static_cast<int>((j - tlx) / hs);
            int y = static_cast<int>((i - tly) / vs);
            if(x >= 0 && x < width && y >= 0 && y < height)
            {
                v = jas_image_readcmptsample(jasImage, cmptno, x, y);
                v <<= 16 - jas_image_cmptprec(jasImage, cmptno);
                v = std::min(std::max(v, 0), 65535) / (65535 / 255);
            }
            scanline[j] = qRgb(v, v, v);
        }
    }
    return result;
}

QImage readJp2File(const QString &filename)
{
    QFile inFile(filename);
    if(!inFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "Can't open" << filename;
        return QImage();
    }
    QByteArray inBuffer = inFile.readAll();

    if(jas_init())
    {
        qWarning() << "Can't init libjasper";
        return QImage();
    }

    jas_stream_t *stream = jas_stream_memopen(inBuffer.data(), inBuffer.size());
    int format = jas_image_getfmt(stream);
    if(format < 0)
    {
        qWarning() << "Image has unknown format";
//        jas_stream_close(stream);
//        jas_image_clearfmts();
//        return QImage();
    }
    jas_image_t *jasImage = jas_image_decode(stream, format, Q_NULLPTR);
    if(!jasImage)
    {
        qWarning() << "Can't load image data";
        jas_stream_close(stream);
        jas_image_clearfmts();
        return QImage();
    }
    to_sRGB(jasImage);

    QImage result;
    if(jas_image_numcmpts(jasImage) == 3)
        result = renderRgbImage(jasImage);
    if(result.isNull())
        result = renderGrayImage(jasImage);

    jas_stream_close(stream);
    jas_image_destroy(jasImage);
    jas_image_clearfmts();

    return result;
}

// ====================================================================================================

class DecoderLibJasPer : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderLibJasPer");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
                /// @note Нативные форматы JPEG 2000
#if !defined(EXCLUDE_JP2_SUPPORT)
                << QString::fromLatin1("jp2")
#endif
#if !defined(EXCLUDE_JPC_SUPPORT)
                << QString::fromLatin1("j2k")
                << QString::fromLatin1("jpc")
                << QString::fromLatin1("j2c")
#endif
#if !defined(EXCLUDE_PGX_SUPPORT)
                << QString::fromLatin1("pgx") // PGX (JPEG 2000)
#endif
                   ;
    }

    QStringList advancedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
                   /// @note Дополнительные форматы, открываемые libjasper
//#if !defined(EXCLUDE_JPG_SUPPORT)
//                << QString::fromLatin1("jpg")
//#endif
#if !defined(EXCLUDE_MIF_SUPPORT)
                << QString::fromLatin1("mif") // Magick Image File Format
#endif
#if !defined(EXCLUDE_PNM_SUPPORT)
                << QString::fromLatin1("pnm") // Portable anymap format
                << QString::fromLatin1("ppm") // Portable pixmap format
                << QString::fromLatin1("pgm") // Portable graymap format
                << QString::fromLatin1("pbm") // Portable bitmap format
#endif
#if !defined(EXCLUDE_RAS_SUPPORT)
                << QString::fromLatin1("ras") // Sun Raster
                << QString::fromLatin1("sun")
#endif
#if !defined(EXCLUDE_BMP_SUPPORT)
                << QString::fromLatin1("bmp")
                << QString::fromLatin1("dib")
#endif
                   ;
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
        QGraphicsItem *item = GraphicsItemsFactory::instance().createImageItem(readJp2File(filePath));
        IImageMetaData *metaData = ImageMetaData::createMetaData(filePath);
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), metaData));
    }
};

DecoderAutoRegistrator registrator(new DecoderLibJasPer);

// ====================================================================================================

} // namespace
