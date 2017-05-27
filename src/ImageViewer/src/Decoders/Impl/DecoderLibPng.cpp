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

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>

#include <png.h>
#include <zlib.h>

#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QSysInfo>
#include <QDebug>

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/Animation/AbstractAnimationProvider.h"
#include "Internal/Animation/AnimationUtils.h"
#include "Internal/Animation/FramesCompositor.h"
#include "Internal/Utils/CmsUtils.h"

#ifndef png_jmpbuf
#define png_jmpbuf(png_ptr) ((png_ptr)->png_jmpbuf)
#endif
#define PNG_BYTES_TO_CHECK 4

/// @todo Посмотреть на реализацию APNG на чистом libpng, без патчей:
/// https://sourceforge.net/projects/apng/files/libpng/examples/code_examples.zip/download

namespace
{

// ====================================================================================================

class PngAnimationProvider : public AbstractAnimationProvider
{
public:
    PngAnimationProvider(const QString &filePath);

    bool checkIfPng();
    void applyICCProfile(png_const_structrp pngPtr, png_inforp infoPtr, QImage *image);
    bool readPng();

    QFile file;
};

// ====================================================================================================

void readPngData(png_structp pngPtr, png_bytep data, png_size_t length)
{
    PngAnimationProvider *provider = reinterpret_cast<PngAnimationProvider*>(png_get_io_ptr(pngPtr));
    provider->file.read(reinterpret_cast<char*>(data), static_cast<int>(length));
}

// ====================================================================================================

PngAnimationProvider::PngAnimationProvider(const QString &filePath)
    : file(filePath)
{
    m_error = !readPng();
}

bool PngAnimationProvider::checkIfPng()
{
    char buf[PNG_BYTES_TO_CHECK];

    // Open the prospective PNG file.
    if(!file.open(QIODevice::ReadOnly))
        return false;

    // Read in some of the signature bytes
    if(file.read(buf, PNG_BYTES_TO_CHECK) != PNG_BYTES_TO_CHECK)
    {
        file.close();
        return false;
    }

    // Compare the first PNG_BYTES_TO_CHECK bytes of the signature.
    // Return nonzero (true) if they match
    return(!png_sig_cmp(reinterpret_cast<png_const_bytep>(buf), static_cast<png_size_t>(0), PNG_BYTES_TO_CHECK));
}

void PngAnimationProvider::applyICCProfile(png_const_structrp pngPtr, png_inforp infoPtr, QImage *image)
{
    png_charp name;
    int compressionType;
    png_bytep profile;
    png_uint_32 proflen;
    if(png_get_iCCP(pngPtr, infoPtr, &name, &compressionType, &profile, &proflen) == PNG_INFO_iCCP)
    {
        qDebug() << "Found ICCP metadata";
        ICCProfile icc(QByteArray::fromRawData(reinterpret_cast<char*>(profile), static_cast<int>(proflen)));
        icc.applyToImage(image);
    }
}

bool PngAnimationProvider::readPng()
{
    if(!checkIfPng())
        return false;

    png_structp pngPtr = NULL;
    png_infop infoPtr = NULL;
    FramesCompositor compositor;

    // Create and initialize the png_struct with the desired error handler
    // functions.  If you want to use the default stderr and longjump method,
    // you can supply NULL for the last three parameters.  We also supply the
    // the compiler header file version, so that we know if the application
    // was compiled with a compatible version of the library.  REQUIRED
    pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!pngPtr)
    {
        qWarning() << "Can't initialize png_struct";
        file.close();
        return false;
    }

    // Allocate/initialize the memory for image information.  REQUIRED.
    infoPtr = png_create_info_struct(pngPtr);
    if(!infoPtr)
    {
        qWarning() << "Can't initialize png_info";
        png_destroy_read_struct(&pngPtr, NULL, NULL);
        file.close();
        return false;
    }

    // Set error handling if you are using the setjmp/longjmp method (this is
    // the normal method of doing things with libpng).  REQUIRED unless you
    // set up your own error handlers in the png_create_read_struct() earlier.
    if(setjmp(png_jmpbuf(pngPtr)))
    {
       // Free all of the memory associated with the png_ptr and info_ptr
       png_destroy_read_struct(&pngPtr, &infoPtr, NULL);
       file.close();
       // If we get here, we had a problem reading the file
       qWarning() << "Can't read PNG file";
       return false;
    }

    // If you are using replacement read functions, instead of calling
    // png_init_io() here you would call:
    png_set_read_fn(pngPtr, reinterpret_cast<void*>(this), &readPngData);

    // If we have already read some of the signature
    png_set_sig_bytes(pngPtr, PNG_BYTES_TO_CHECK);

    // The call to png_read_info() gives us all of the information from the
    // PNG file before the first IDAT (image data chunk).  REQUIRED
    png_read_info(pngPtr, infoPtr);

    // Тут заполним базовую информацию о фреймах и выделим память под изображения
#if defined (PNG_APNG_SUPPORTED)
    int firstFrameNumber = png_get_first_frame_is_hidden(pngPtr, infoPtr) ? 1 : 0;
    if(png_get_valid(pngPtr, infoPtr, PNG_INFO_acTL))
    {
        m_numFrames = static_cast<int>(png_get_num_frames(pngPtr, infoPtr));
        m_numLoops = static_cast<int>(png_get_num_plays(pngPtr, infoPtr));
    }
#endif
    m_frames.resize(m_numFrames);

    const png_uint_32 width = png_get_image_width(pngPtr, infoPtr);
    const png_uint_32 height = png_get_image_height(pngPtr, infoPtr);
    for(int count = 0; count < m_numFrames; count++)
    {
//        int bitDepth = png_get_bit_depth(pngPtr, infoPtr);
//        int colorType = png_get_color_type(pngPtr, infoPtr);
        png_uint_32 nextFrameWidth = width, nextFrameHeight = height;
        png_uint_32 nextFrameOffsetX = 0, nextFrameOffsetY = 0;
#if defined (PNG_APNG_SUPPORTED)
        png_uint_16 nextFrameDelayNum = 0, nextFrameDelayDen = 1;
        png_byte nextFrameDisposeOp = PNG_DISPOSE_OP_NONE;
        png_byte nextFrameBlendOp = PNG_BLEND_OP_SOURCE;
        if(png_get_valid(pngPtr, infoPtr, PNG_INFO_acTL))
        {
            png_read_frame_head(pngPtr, infoPtr);
            if(png_get_valid(pngPtr, infoPtr, PNG_INFO_fcTL))
            {
                png_get_next_frame_fcTL(pngPtr, infoPtr,
                                        &nextFrameWidth, &nextFrameHeight,
                                        &nextFrameOffsetX, &nextFrameOffsetY,
                                        &nextFrameDelayNum, &nextFrameDelayDen,
                                        &nextFrameDisposeOp, &nextFrameBlendOp);
                if(nextFrameDelayDen == 0)
                    nextFrameDelayDen = 1;
            }
            if(count == firstFrameNumber)
            {
                nextFrameBlendOp = PNG_BLEND_OP_SOURCE;
                if(nextFrameDisposeOp == PNG_DISPOSE_OP_PREVIOUS)
                    nextFrameDisposeOp = PNG_DISPOSE_OP_BACKGROUND;
                compositor.startComposition(width, height);
            }
        }
#endif

        // Set up the data transformations you want.  Note that these are all
        // optional.  Only call them if you want/need them.  Many of the
        // transformations only work on specific types of images, and many
        // are mutually exclusive.

        // Tell libpng to strip 16 bits/color files down to 8 bits/color.
        // Use accurate scaling if it's available, otherwise just chop off the
        // low byte.
#if defined (PNG_READ_SCALE_16_TO_8_SUPPORTED)
        png_set_scale_16(pngPtr);
#else
        png_set_strip_16(pngPtr);
#endif

        // Extract multiple pixels with bit depths of 1, 2, and 4 from a single
        // byte into separate bytes (useful for paletted and grayscale images).
        png_set_packing(pngPtr); /// @todo Нужно ли это?

        // Expand paletted colors into true RGB triplets
//        if(colorType == PNG_COLOR_TYPE_PALETTE)
            png_set_palette_to_rgb(pngPtr);

        // Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel
//        if(colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
            png_set_expand_gray_1_2_4_to_8(pngPtr);

        // Конвертим черно-белое в RGB
//        if(colorType == PNG_COLOR_TYPE_GRAY)
            png_set_gray_to_rgb(pngPtr);

        // Expand paletted or RGB images with transparency to full alpha channels
        // so the data will be available as RGBA quartets.
//        if(png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS) != 0)
            png_set_tRNS_to_alpha(pngPtr);

        if(QSysInfo::ByteOrder == QSysInfo::LittleEndian) // BGRA
        {
            // Flip the RGB pixels to BGR (or RGBA to BGRA)
//            if((colorType & PNG_COLOR_MASK_COLOR) != 0)
                png_set_bgr(pngPtr);

            // Add filler (or alpha) byte (before/after each RGB triplet)
            png_set_filler(pngPtr, 0xffff, PNG_FILLER_AFTER);
        }
        else // ARGB
        {
            /// @todo Проверить работоспособность на Big Endian системах

            // Swap the RGBA or GA data to ARGB or AG (or BGRA to ABGR)
            png_set_swap_alpha(pngPtr);

            // Add filler (or alpha) byte (before/after each RGB triplet)
            png_set_filler(pngPtr, 0xffff, PNG_FILLER_BEFORE);
        }

#if defined (PNG_READ_INTERLACING_SUPPORTED)
        // Turn on interlace handling.  REQUIRED if you are not using
        // png_read_image().  To see how to handle interlacing passes,
        // see the png_read_row() method below:
        int number_passes = png_set_interlace_handling(pngPtr);
#else
        int number_passes = 1;
#endif

        // Optional call to gamma correct and add the background to the palette
        // and update info structure.  REQUIRED if you are expecting libpng to
        // update the palette for you (ie you selected such a transform above).
        png_read_update_info(pngPtr, infoPtr);

        // Allocate the memory to hold the image using the fields of info_ptr.
        m_frames[count].image = QImage(static_cast<int>(width), static_cast<int>(height), QImage::Format_ARGB32);
        QImage &image = m_frames[count].image;
        if(image.size().isEmpty())
        {
            qWarning() << "Invalid image size";
            longjmp(png_jmpbuf(pngPtr), 1);
        }
        image.fill(Qt::transparent);

        // Now it's time to read the image.
        for(int pass = 0; pass < number_passes; pass++)
        {
            for(png_uint_32 y = 0; y < nextFrameHeight; y++)
                png_read_row(pngPtr, image.scanLine(static_cast<int>(y + nextFrameOffsetY)) + nextFrameOffsetX * 4, NULL);
        }

        applyICCProfile(pngPtr, infoPtr, &image);

#if defined (PNG_APNG_SUPPORTED)
        if(compositor.isStarted())
        {
            const QRect frameRect = QRect(static_cast<int>(nextFrameOffsetX), static_cast<int>(nextFrameOffsetY),
                                          static_cast<int>(nextFrameWidth), static_cast<int>(nextFrameHeight));
            FramesCompositor::DisposeType compositorDisposeType = FramesCompositor::DISPOSE_NONE;
            switch(nextFrameDisposeOp)
            {
            case PNG_DISPOSE_OP_NONE:       compositorDisposeType = FramesCompositor::DISPOSE_NONE;         break;
            case PNG_DISPOSE_OP_BACKGROUND: compositorDisposeType = FramesCompositor::DISPOSE_BACKGROUND;   break;
            case PNG_DISPOSE_OP_PREVIOUS:   compositorDisposeType = FramesCompositor::DISPOSE_PREVIOUS;     break;
            }
            FramesCompositor::BlendType compositorBlendType = FramesCompositor::BLEND_NONE;
            switch(nextFrameBlendOp)
            {
            case PNG_BLEND_OP_SOURCE:   compositorBlendType = FramesCompositor::BLEND_NONE; break;
            case PNG_BLEND_OP_OVER:     compositorBlendType = FramesCompositor::BLEND_OVER; break;
            }
            image = compositor.compositeFrame(image, frameRect, compositorDisposeType, compositorBlendType);
        }
#endif

#if defined (PNG_APNG_SUPPORTED)
        m_frames[count].delay = nextFrameDelayNum * 1000 / nextFrameDelayDen;
#else
        m_frames[count].delay = -1;
#endif
    }

    // Read rest of file, and get additional chunks in info_ptr - REQUIRED
    png_read_end(pngPtr, infoPtr);

    // At this point you have read the entire image

    // Clean up after the read, and free any memory allocated - REQUIRED
    png_destroy_read_struct(&pngPtr, &infoPtr, NULL);

    // Close the file
    file.close();

    return true;
}

// ====================================================================================================

class DecoderLibPng : public IDecoder
{
public:
    QString name() const
    {
        return QString::fromLatin1("DecoderLibPng");
    }

    QStringList supportedFormats() const
    {
        return QStringList()
                << QString::fromLatin1("png");
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
        return AnimationUtils::CreateGraphicsItem(new PngAnimationProvider(filePath));
    }
};

DecoderAutoRegistrator registrator(new DecoderLibPng);

// ====================================================================================================

} // namespace
