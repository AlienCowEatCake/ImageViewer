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

#include "Utils/ScopedPointer.h"

#include "IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/ExifUtils.h"
#include "Internal/CmsUtils.h"

#define DECODER_LIBJPEG_PRIORITY 1110

namespace
{

// Since an ICC profile can be larger than the maximum size of a JPEG marker
// (64K), we need provisions to split it into multiple markers.  The format
// defined by the ICC specifies one or more APP2 markers containing the
// following data:
//      Identifying string      ASCII "ICC_PROFILE\0"  (12 bytes)
//      Marker sequence number  1 for first APP2, 2 for next, etc (1 byte)
//      Number of markers       Total number of APP2's used (1 byte)
//      Profile data            (remainder of APP2 data)
// Decoders should use the marker sequence numbers to reassemble the profile,
// rather than assuming that the APP2 markers appear in the correct sequence.

#define EXIF_MARKER  (JPEG_APP0 + 1)    // JPEG marker code for EXIF
#define ICCP_MARKER  (JPEG_APP0 + 2)    // JPEG marker code for ICC
#define ICCP_OVERHEAD_LEN  14           // size of non-profile data in APP2
#define MAX_BYTES_IN_MARKER  65533      // maximum data len of a JPEG marker
#define MAX_DATA_BYTES_IN_MARKER  (MAX_BYTES_IN_MARKER - ICCP_OVERHEAD_LEN)

// Handy subroutine to test whether a saved marker is an ICC profile marker.
bool markerIsICCP(jpeg_saved_marker_ptr marker)
{
    return
            marker->marker == ICCP_MARKER &&
            marker->data_length >= ICCP_OVERHEAD_LEN &&
            // verify the identifying string
            GETJOCTET(marker->data[0]) == 0x49 &&
            GETJOCTET(marker->data[1]) == 0x43 &&
            GETJOCTET(marker->data[2]) == 0x43 &&
            GETJOCTET(marker->data[3]) == 0x5F &&
            GETJOCTET(marker->data[4]) == 0x50 &&
            GETJOCTET(marker->data[5]) == 0x52 &&
            GETJOCTET(marker->data[6]) == 0x4F &&
            GETJOCTET(marker->data[7]) == 0x46 &&
            GETJOCTET(marker->data[8]) == 0x49 &&
            GETJOCTET(marker->data[9]) == 0x4C &&
            GETJOCTET(marker->data[10]) == 0x45 &&
            GETJOCTET(marker->data[11]) == 0x0;
}

// See if there was an ICC profile in the JPEG file being read;
// if so, reassemble and return the profile data.
QByteArray readICCProfile(j_decompress_ptr cinfo)
{
    int num_markers = 0;
    int seq_no;
    const int MAX_SEQ_NO = 255;             // sufficient since marker numbers are bytes
    char marker_present[MAX_SEQ_NO+1];      // 1 if marker found
    unsigned int data_length[MAX_SEQ_NO+1]; // size of profile data in marker
    unsigned int data_offset[MAX_SEQ_NO+1]; // offset for data in marker

    // This first pass over the saved markers discovers whether there are
    // any ICC markers and verifies the consistency of the marker numbering.

    for(seq_no = 1; seq_no <= MAX_SEQ_NO; seq_no++)
        marker_present[seq_no] = 0;

    for(jpeg_saved_marker_ptr marker = cinfo->marker_list; marker != NULL; marker = marker->next)
    {
        if(markerIsICCP(marker))
        {
            if(num_markers == 0)
                num_markers = GETJOCTET(marker->data[13]);
            else if(num_markers != GETJOCTET(marker->data[13]))
                return QByteArray();    // inconsistent num_markers fields
            seq_no = GETJOCTET(marker->data[12]);
            if(seq_no <= 0 || seq_no > num_markers)
                return QByteArray();    // bogus sequence number
            if(marker_present[seq_no])
                return QByteArray();    // duplicate sequence numbers
            marker_present[seq_no] = 1;
            data_length[seq_no] = marker->data_length - ICCP_OVERHEAD_LEN;
        }
    }

    if(num_markers == 0)
        return QByteArray();

    // Check for missing markers, count total space needed,
    // compute offset of each marker's part of the data.

    unsigned int total_length = 0;
    for(seq_no = 1; seq_no <= num_markers; seq_no++)
    {
        if(marker_present[seq_no] == 0)
            return QByteArray();        // missing sequence number
        data_offset[seq_no] = total_length;
        total_length += data_length[seq_no];
    }

    if(total_length == 0)
        return QByteArray();    // found only empty markers?

    // Allocate space for assembled data
    QByteArray result(static_cast<int>(total_length * sizeof(JOCTET)), 0);
    JOCTET *icc_data = reinterpret_cast<JOCTET*>(result.data());

    // and fill it in
    for(jpeg_saved_marker_ptr marker = cinfo->marker_list; marker != NULL; marker = marker->next)
    {
        if(markerIsICCP(marker))
        {
            JOCTET *src_ptr;
            JOCTET *dst_ptr;
            unsigned int length;
            seq_no = GETJOCTET(marker->data[12]);
            dst_ptr = icc_data + data_offset[seq_no];
            src_ptr = marker->data + ICCP_OVERHEAD_LEN;
            length = data_length[seq_no];
            while(length--)
                *dst_ptr++ = *src_ptr++;
        }
    }

    return result;
}

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
        qWarning() << "Can't open" << filename;
        return QImage();
    }
    const QByteArray inBuffer = inFile.readAll();

    // Construct any C++ objects before setjmp!
    QScopedPointer<ICCProfile> iccProfile;
    QImage outImage;

    // Step 1: allocate and initialize JPEG decompression object

    // We set up the normal JPEG error routines, then override error_exit.
    cinfo.err = jpeg_std_error(&jerr.publicFields);
    jerr.publicFields.error_exit = jpegErrorExit;
    // Establish the setjmp return context for my_error_exit to use.
    if(setjmp(jerr.setjmpBuffer))
    {
        qDebug() << "Error lonjgmp completed";
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

    jpeg_save_markers(&cinfo, EXIF_MARKER, 0xFFFF); // EXIF metadata
    jpeg_save_markers(&cinfo, ICCP_MARKER, 0xFFFF); // ICCP metadata

    if(!jpeg_read_header(&cinfo, TRUE))
    {
        qWarning() << "Can't read JPEG header";
        longjmp(jerr.setjmpBuffer, 1);
    }
    // We can ignore the return value from jpeg_read_header since
    //   (a) suspension is not possible with the stdio data source, and
    //   (b) we passed TRUE to reject a tables-only JPEG file as an error.
    // See libjpeg.txt for more info.

    iccProfile.reset(new ICCProfile(readICCProfile(&cinfo)));
#if defined (QT_DEBUG)
    for(jpeg_saved_marker_ptr marker = cinfo.marker_list; marker != NULL; marker = marker->next)
    {
        switch(marker->marker)
        {
        case EXIF_MARKER:
            qDebug() << "Found EXIF metadata";
            break;
        case ICCP_MARKER:
            qDebug() << "Found ICCP metadata";
            break;
        default:
            break;
        }
    }
#endif

    // Step 4: set parameters for decompression

    // In this example, we don't need to change any of the defaults set by
    // jpeg_read_header(), so we do nothing here.
    cinfo.out_color_space = JCS_RGB;

    // Step 5: Start decompressor

    if(!jpeg_start_decompress(&cinfo))
    {
        qWarning() << "Can't start JPEG decompress";
        longjmp(jerr.setjmpBuffer, 1);
    }
    // We can ignore the return value since suspension is not possible
    // with the stdio data source.

    // We may need to do some setup of our own at this point before reading
    // the data.  After jpeg_start_decompress() we have the correct scaled
    // output image dimensions available, as well as the output colormap
    // if we asked for color quantization.
    // In this example, we need to make an output work buffer of the right size.
    outImage = QImage(static_cast<int>(cinfo.output_width), static_cast<int>(cinfo.output_height), QImage::Format_RGB32);
    if(outImage.size().isEmpty())
    {
        qWarning() << "Invalid image size";
        longjmp(jerr.setjmpBuffer, 1);
    }

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
        if(!jpeg_read_scanlines(&cinfo, buffer, 1))
        {
            qWarning() << "Can't read JPEG scanlines";
            longjmp(jerr.setjmpBuffer, 1);
        }
        // Assume put_scanline_someplace wants a pointer and sample count.
        QRgb *outLine = reinterpret_cast<QRgb*>(outImage.scanLine(static_cast<int>(cinfo.output_scanline) - 1));
        iccProfile->applyToRGBData(buffer[0], cinfo.output_width);
        for(JDIMENSION j = 0; j < cinfo.output_width; j++)
        {
            unsigned char *inPixel = reinterpret_cast<unsigned char*>(buffer[0]) + j * static_cast<JDIMENSION>(cinfo.output_components);
            outLine[j] = qRgb(inPixel[0], inPixel[1], inPixel[2]);
        }
    }

    // Step 7: Finish decompression

    if(!jpeg_finish_decompress(&cinfo))
    {
        qWarning() << "Can't finish JPEG decompress";
        longjmp(jerr.setjmpBuffer, 1);
    }
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

    quint16 orientation = ExifUtils::GetExifOrientation(filename);
    if(orientation > 1 && orientation <= 8)
        ExifUtils::ApplyExifOrientation(&outImage, orientation);

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

DecoderAutoRegistrator registrator(new DecoderLibJpeg);

} // namespace
