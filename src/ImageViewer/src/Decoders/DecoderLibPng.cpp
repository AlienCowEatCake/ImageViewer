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

#include <png.h>
#include <zlib.h>

#include <QFileInfo>
#include <QImage>
#include <QPixmap>
#include <QFile>
#include <QByteArray>
#include <QSysInfo>
#include <QDebug>

#include "IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/Animation/IAnimationProvider.h"
#include "Internal/Animation/AnimationUtils.h"
#include "Internal/CmsUtils.h"

#ifndef png_jmpbuf
#define png_jmpbuf(png_ptr) ((png_ptr)->png_jmpbuf)
#endif
#define PNG_BYTES_TO_CHECK 4

namespace
{

// ====================================================================================================

struct PngAnimationProvider : public IAnimationProvider
{
    PngAnimationProvider(const QString &filePath);
    ~PngAnimationProvider();

    bool isValid() const;
    bool isSingleFrame() const;
    int nextImageDelay() const;
    bool jumpToNextImage();
    QPixmap currentPixmap() const;

    bool check_if_png();
    void apply_icc_profile(png_const_structrp png_ptr, png_inforp info_ptr, QImage *image);
    bool read_png();

    QImage image;
    QFile file;
    bool error;
};

// ====================================================================================================

void readPngFile(png_structp png_ptr, png_bytep data, png_size_t length)
{
    PngAnimationProvider *provider = reinterpret_cast<PngAnimationProvider*>(png_get_io_ptr(png_ptr));
    provider->file.read(reinterpret_cast<char*>(data), static_cast<int>(length));
}

// ====================================================================================================

PngAnimationProvider::PngAnimationProvider(const QString &filePath)
    : file(filePath)
    , error(true)
{
    error = !read_png();
}

PngAnimationProvider::~PngAnimationProvider()
{}

bool PngAnimationProvider::isValid() const
{
    return !error;
}

bool PngAnimationProvider::isSingleFrame() const
{
    return true;
}

int PngAnimationProvider::nextImageDelay() const
{
    return 0;
}

bool PngAnimationProvider::jumpToNextImage()
{
    return true;
}

QPixmap PngAnimationProvider::currentPixmap() const
{
    return QPixmap::fromImage(image);
}

bool PngAnimationProvider::check_if_png()
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

void PngAnimationProvider::apply_icc_profile(png_const_structrp png_ptr, png_inforp info_ptr, QImage *image)
{
    png_charp name;
    int compression_type;
    png_bytep profile;
    png_uint_32 proflen;
    if(png_get_iCCP(png_ptr, info_ptr, &name, &compression_type, &profile, &proflen) == PNG_INFO_iCCP)
    {
        qDebug() << "Found ICCP metadata";
        ICCProfile icc(QByteArray::fromRawData(reinterpret_cast<char*>(profile), static_cast<int>(proflen)));
        icc.applyToImage(image);
    }
}

bool PngAnimationProvider::read_png()
{
    if(!check_if_png())
        return false;

    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;

    // Create and initialize the png_struct with the desired error handler
    // functions.  If you want to use the default stderr and longjump method,
    // you can supply NULL for the last three parameters.  We also supply the
    // the compiler header file version, so that we know if the application
    // was compiled with a compatible version of the library.  REQUIRED
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png_ptr)
    {
        qWarning() << "Can't initialize png_struct";
        file.close();
        return false;
    }

    // Allocate/initialize the memory for image information.  REQUIRED.
    info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr)
    {
        qWarning() << "Can't initialize png_info";
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        file.close();
        return false;
    }

    // Set error handling if you are using the setjmp/longjmp method (this is
    // the normal method of doing things with libpng).  REQUIRED unless you
    // set up your own error handlers in the png_create_read_struct() earlier.
    if(setjmp(png_jmpbuf(png_ptr)))
    {
       // Free all of the memory associated with the png_ptr and info_ptr
       png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
       file.close();
       // If we get here, we had a problem reading the file
       qWarning() << "Can't read PNG file";
       return false;
    }

    // If you are using replacement read functions, instead of calling
    // png_init_io() here you would call:
    png_set_read_fn(png_ptr, reinterpret_cast<void*>(this), &readPngFile);

    // If we have already read some of the signature
    png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);

    // The call to png_read_info() gives us all of the information from the
    // PNG file before the first IDAT (image data chunk).  REQUIRED
    png_read_info(png_ptr, info_ptr);

    png_uint_32 width, height;
    int bit_depth, color_type, interlace_type;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);

    // Set up the data transformations you want.  Note that these are all
    // optional.  Only call them if you want/need them.  Many of the
    // transformations only work on specific types of images, and many
    // are mutually exclusive.

    // Tell libpng to strip 16 bits/color files down to 8 bits/color.
    // Use accurate scaling if it's available, otherwise just chop off the
    // low byte.
#ifdef PNG_READ_SCALE_16_TO_8_SUPPORTED
    png_set_scale_16(png_ptr);
#else
    png_set_strip_16(png_ptr);
#endif

    // Extract multiple pixels with bit depths of 1, 2, and 4 from a single
    // byte into separate bytes (useful for paletted and grayscale images).
//    png_set_packing(png_ptr); /// @todo Нужно ли это?

    // Expand paletted colors into true RGB triplets
//    if(color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);

    // Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel
//    if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png_ptr);

    // Конвертим черно-белое в RGB
//    if(color_type == PNG_COLOR_TYPE_GRAY)
        png_set_gray_to_rgb(png_ptr);

    // Expand paletted or RGB images with transparency to full alpha channels
    // so the data will be available as RGBA quartets.
//    if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) != 0)
        png_set_tRNS_to_alpha(png_ptr);

    if(QSysInfo::ByteOrder == QSysInfo::LittleEndian) // BGRA
    {
        // Flip the RGB pixels to BGR (or RGBA to BGRA)
//        if((color_type & PNG_COLOR_MASK_COLOR) != 0)
            png_set_bgr(png_ptr);

        // Add filler (or alpha) byte (before/after each RGB triplet)
        png_set_filler(png_ptr, 0xffff, PNG_FILLER_AFTER);
    }
    else // ARGB
    {
        /// @todo Check Me !!!

        // Swap the RGBA or GA data to ARGB or AG (or BGRA to ABGR)
        png_set_swap_alpha(png_ptr);

        // Add filler (or alpha) byte (before/after each RGB triplet)
        png_set_filler(png_ptr, 0xffff, PNG_FILLER_BEFORE);
    }

#ifdef PNG_READ_INTERLACING_SUPPORTED
    // Turn on interlace handling.  REQUIRED if you are not using
    // png_read_image().  To see how to handle interlacing passes,
    // see the png_read_row() method below:
    int number_passes = png_set_interlace_handling(png_ptr);
#else
    int number_passes = 1;
#endif

    // Optional call to gamma correct and add the background to the palette
    // and update info structure.  REQUIRED if you are expecting libpng to
    // update the palette for you (ie you selected such a transform above).
    png_read_update_info(png_ptr, info_ptr);

    // Allocate the memory to hold the image using the fields of info_ptr.
    image = QImage(static_cast<int>(width), static_cast<int>(height), QImage::Format_ARGB32);
    if(image.size().isEmpty())
    {
        qWarning() << "Invalid image size";
        longjmp(png_jmpbuf(png_ptr), 1);
    }
#if defined (QT_DEBUG)
    image.fill(Qt::red);
#endif

    // Now it's time to read the image.
    for(int pass = 0; pass < number_passes; pass++)
    {
        for(int y = 0; y < static_cast<int>(height); y++)
        {
            uchar *row_pointer = image.scanLine(y);
            png_read_rows(png_ptr, &row_pointer, NULL, 1);
        }
    }

    // Read rest of file, and get additional chunks in info_ptr - REQUIRED
    png_read_end(png_ptr, info_ptr);

    // At this point you have read the entire image

    apply_icc_profile(png_ptr, info_ptr, &image);

    // Clean up after the read, and free any memory allocated - REQUIRED
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

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
