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

#include <cstdio>
#include <setjmp.h>
#include <jpeglib.h>

#include <QFileInfo>
#include <QImage>
#include <QPixmap>
#include <QFile>
#include <QByteArray>
#include <QGraphicsPixmapItem>
#include <QDebug>

#include "IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/ExifUtils.h"

#define DECODER_LIBJPEG_PRIORITY 1000

namespace
{

// Here's the extended error handler struct:
struct JpegError
{
    jpeg_error_mgr publicFields;    // "public" fields
    jmp_buf setjmpBuffer;           // for return to caller
};

// Here's the routine that will replace the standard error_exit method:
void jpegErrorExit(j_common_ptr cinfo)
{
    JpegError *error = reinterpret_cast<JpegError*>(cinfo->err);
    (*cinfo->err->output_message)(cinfo);
    longjmp(error->setjmpBuffer, 1);
}

// Sample routine for JPEG decompression.
QImage readJpegFile(const QString &filename)
{
    // This struct contains the JPEG decompression parameters and pointers to
    // working space (which is allocated as needed by the JPEG library).
    jpeg_decompress_struct cinfo;
    // We use our private extension JPEG error handler.
    // Note that this struct must live as long as the main JPEG parameter
    // struct, to avoid dangling-pointer problems.
    JpegError jerr;
    // More stuff
    JSAMPARRAY buffer;    // Output row buffer
    JDIMENSION rowStride; // physical row width in output buffer

    QFile inFile(filename);

    // In this example we want to open the input file before doing anything else,
    // so that the setjmp() error recovery below can assume the file is open.
    // VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
    // requires it in order to read binary files.
    if(!inFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "can't open" << filename;
        return QImage();
    }
    const QByteArray inBuffer = inFile.readAll();

    // Step 1: allocate and initialize JPEG decompression object

    // We set up the normal JPEG error routines, then override error_exit.
    cinfo.err = jpeg_std_error(&jerr.publicFields);
    jerr.publicFields.error_exit = jpegErrorExit;
    // Establish the setjmp return context for my_error_exit to use.
    if(setjmp(jerr.setjmpBuffer))
    {
        // If we get here, the JPEG code has signaled an error.
        // We need to clean up the JPEG object, close the input file, and return.
        jpeg_destroy_decompress(&cinfo);
        inFile.close();
        return QImage();
    }
    // Now we can initialize the JPEG decompression object.
    jpeg_create_decompress(&cinfo);

    // Step 2: specify data source (eg, a file)

    jpeg_mem_src(&cinfo, reinterpret_cast<const unsigned char*>(inBuffer.data()), static_cast<unsigned long>(inBuffer.size()));

    // Step 3: read file parameters with jpeg_read_header()

    jpeg_save_markers(&cinfo, JPEG_APP0 + 1, 0xFFFF);   // EXIF metadata
    jpeg_save_markers(&cinfo, JPEG_APP0 + 2, 0xFFFF);   // ICCP metadata

    (void)jpeg_read_header(&cinfo, TRUE);
    // We can ignore the return value from jpeg_read_header since
    //   (a) suspension is not possible with the stdio data source, and
    //   (b) we passed TRUE to reject a tables-only JPEG file as an error.
    // See libjpeg.txt for more info.

//    quint16 orientation = 1;
    for(jpeg_saved_marker_ptr marker = cinfo.marker_list; marker != NULL; marker = marker->next)
    {
        switch(marker->marker)
        {
        case JPEG_APP0 + 1:
            qDebug() << "Found EXIF metadata";
//            orientation = ExifUtils::GetExifOrientation()
            break;
        case JPEG_APP0 + 2:
            qDebug() << "Found ICCP metadata";
            break;
        default:
            break;
        }
    }

    // Step 4: set parameters for decompression

    // In this example, we don't need to change any of the defaults set by
    // jpeg_read_header(), so we do nothing here.
    cinfo.out_color_space = JCS_RGB;

    // Step 5: Start decompressor

    (void)jpeg_start_decompress(&cinfo);
    // We can ignore the return value since suspension is not possible
    // with the stdio data source.

    // We may need to do some setup of our own at this point before reading
    // the data.  After jpeg_start_decompress() we have the correct scaled
    // output image dimensions available, as well as the output colormap
    // if we asked for color quantization.
    // In this example, we need to make an output work buffer of the right size.
    QImage outImage = QImage(static_cast<int>(cinfo.output_width), static_cast<int>(cinfo.output_height), QImage::Format_RGB32);

    // JSAMPLEs per row in output buffer
    rowStride = cinfo.output_width * static_cast<JDIMENSION>(cinfo.output_components);
    // Make a one-row-high sample array that will go away when done with image
    buffer = (*cinfo.mem->alloc_sarray)(reinterpret_cast<j_common_ptr>(&cinfo), JPOOL_IMAGE, rowStride, 1);

    // Step 6: while (scan lines remain to be read)
    //           jpeg_read_scanlines(...);

    // Here we use the library's state variable cinfo.output_scanline as the
    // loop counter, so that we don't have to keep track ourselves.
    while(cinfo.output_scanline < cinfo.output_height)
    {
        // jpeg_read_scanlines expects an array of pointers to scanlines.
        // Here the array is only one element long, but you could ask for
        // more than one scanline at a time if that's more convenient.
        (void)jpeg_read_scanlines(&cinfo, buffer, 1);
        // Assume put_scanline_someplace wants a pointer and sample count.
        QRgb *outLine = reinterpret_cast<QRgb*>(outImage.scanLine(static_cast<int>(cinfo.output_scanline)));
        for(JDIMENSION j = 0; j < cinfo.output_width; j++)
        {
            unsigned char *inPixel = reinterpret_cast<unsigned char*>(buffer[0]) + j * static_cast<JDIMENSION>(cinfo.output_components);
            outLine[j] = qRgb(inPixel[0], inPixel[1], inPixel[2]);
        }
    }

    // Step 7: Finish decompression

    (void)jpeg_finish_decompress(&cinfo);
    // We can ignore the return value since suspension is not possible
    // with the stdio data source.

    // Step 8: Release JPEG decompression object

    // This is an important step since it will release a good deal of memory.
    jpeg_destroy_decompress(&cinfo);

    // After finish_decompress, we can close the input file.
    // Here we postpone it until after no more JPEG errors are possible,
    // so as to simplify the setjmp error logic above.  (Actually, I don't
    // think that jpeg_destroy can do an error exit, but why assume anything...)
    inFile.close();

    // At this point you may want to check to see whether any corrupt-data
    // warnings occurred (test whether jerr.pub.num_warnings is nonzero).

    // And we're done!
    return outImage;
}

class DecoderLibJpeg : public IDecoder
{
public:
    QString name() const
    {
        return QString::fromLatin1("DecoderLibJpeg");
    }

    QList<DecoderFormatInfo> supportedFormats() const
    {
        const QStringList jpegImageFormats = QStringList()
                << QString::fromLatin1("jpg")
                << QString::fromLatin1("jpeg")
                << QString::fromLatin1("jpe");
        QList<DecoderFormatInfo> result;
        for(QStringList::ConstIterator it = jpegImageFormats.constBegin(); it != jpegImageFormats.constEnd(); ++it)
        {
            DecoderFormatInfo info;
            info.decoderPriority = DECODER_LIBJPEG_PRIORITY;
            info.format = *it;
            result.append(info);
        }
        return result;
    }

    QGraphicsItem *loadImage(const QString &filePath)
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return NULL;

        const QImage image = readJpegFile(filePath);
        if(image.isNull())
            return NULL;

        return new QGraphicsPixmapItem(QPixmap::fromImage(image));
    }
};

DecoderAutoRegistrator registrator(new DecoderLibJpeg, DECODER_LIBJPEG_PRIORITY);

} // namespace
