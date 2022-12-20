/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2005 Christoph Hormann <chris_hormann@gmx.de>
    SPDX-FileCopyrightText: 2005 Ignacio Castaño <castanyo@yahoo.es>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "hdr_p.h"
#include "util_p.h"

#include <QDataStream>
#include <QImage>
#include <QLoggingCategory>
#include <QRegularExpressionMatch>

#include <QDebug>

typedef unsigned char uchar;

Q_LOGGING_CATEGORY(HDRPLUGIN, "kf.imageformats.plugins.hdr", QtWarningMsg)

namespace // Private.
{
#define MAXLINE 1024
#define MINELEN 8 // minimum scanline length for encoding
#define MAXELEN 0x7fff // maximum scanline length for encoding

static inline uchar ClipToByte(float value)
{
    if (value > 255.0f) {
        return 255;
    }
    // else if (value < 0.0f) return 0;  // we know value is positive.
    return uchar(value);
}

// read an old style line from the hdr image file
// if 'first' is true the first byte is already read
static bool Read_Old_Line(uchar *image, int width, QDataStream &s)
{
    int rshift = 0;
    int i;

    while (width > 0) {
        s >> image[0];
        s >> image[1];
        s >> image[2];
        s >> image[3];

        if (s.atEnd()) {
            return false;
        }

        if ((image[0] == 1) && (image[1] == 1) && (image[2] == 1)) {
            for (i = image[3] << rshift; i > 0; i--) {
                // memcpy(image, image-4, 4);
                (uint &)image[0] = (uint &)image[0 - 4];
                image += 4;
                width--;
            }
            rshift += 8;
        } else {
            image += 4;
            width--;
            rshift = 0;
        }
    }
    return true;
}

static void RGBE_To_QRgbLine(uchar *image, QRgb *scanline, int width)
{
    for (int j = 0; j < width; j++) {
        // v = ldexp(1.0, int(image[3]) - 128);
        float v;
        int e = int(image[3]) - 128;
        if (e > 0) {
            v = float(1 << e);
        } else {
            v = 1.0f / float(1 << -e);
        }

        scanline[j] = qRgb(ClipToByte(float(image[0]) * v), ClipToByte(float(image[1]) * v), ClipToByte(float(image[2]) * v));

        image += 4;
    }
}

// Load the HDR image.
static bool LoadHDR(QDataStream &s, const int width, const int height, QImage &img)
{
    uchar val;
    uchar code;

    // Create dst image.
    img = imageAlloc(width, height, QImage::Format_RGB32);
    if (img.isNull()) {
        qCDebug(HDRPLUGIN) << "Couldn't create image with size" << width << height << "and format RGB32";
        return false;
    }

    QByteArray lineArray;
    lineArray.resize(4 * width);
    uchar *image = (uchar *)lineArray.data();

    for (int cline = 0; cline < height; cline++) {
        QRgb *scanline = (QRgb *)img.scanLine(cline);

        // determine scanline type
        if ((width < MINELEN) || (MAXELEN < width)) {
            Read_Old_Line(image, width, s);
            RGBE_To_QRgbLine(image, scanline, width);
            continue;
        }

        s >> val;

        if (s.atEnd()) {
            return true;
        }

        if (val != 2) {
            s.device()->ungetChar(val);
            Read_Old_Line(image, width, s);
            RGBE_To_QRgbLine(image, scanline, width);
            continue;
        }

        s >> image[1];
        s >> image[2];
        s >> image[3];

        if (s.atEnd()) {
            return true;
        }

        if ((image[1] != 2) || (image[2] & 128)) {
            image[0] = 2;
            Read_Old_Line(image + 4, width - 1, s);
            RGBE_To_QRgbLine(image, scanline, width);
            continue;
        }

        if ((image[2] << 8 | image[3]) != width) {
            qCDebug(HDRPLUGIN) << "Line of pixels had width" << (image[2] << 8 | image[3]) << "instead of" << width;
            return false;
        }

        // read each component
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < width;) {
                s >> code;
                if (s.atEnd()) {
                    qCDebug(HDRPLUGIN) << "Truncated HDR file";
                    return false;
                }
                if (code > 128) {
                    // run
                    code &= 127;
                    s >> val;
                    while (code != 0) {
                        image[i + j * 4] = val;
                        j++;
                        code--;
                    }
                } else {
                    // non-run
                    while (code != 0) {
                        s >> image[i + j * 4];
                        j++;
                        code--;
                    }
                }
            }
        }

        RGBE_To_QRgbLine(image, scanline, width);
    }

    return true;
}

} // namespace

bool HDRHandler::read(QImage *outImage)
{
    int len;
    QByteArray line(MAXLINE + 1, Qt::Uninitialized);
    QByteArray format;

    // Parse header
    do {
        len = device()->readLine(line.data(), MAXLINE);

        if (line.startsWith("FORMAT=")) {
            format = line.mid(7, len - 7 - 1 /*\n*/);
        }

    } while ((len > 0) && (line[0] != '\n'));

    if (format != "32-bit_rle_rgbe") {
        qCDebug(HDRPLUGIN) << "Unknown HDR format:" << format;
        return false;
    }

    len = device()->readLine(line.data(), MAXLINE);
    line.resize(len);

    /*
       TODO: handle flipping and rotation, as per the spec below
       The single resolution line consists of 4 values, a X and Y label each followed by a numerical
       integer value. The X and Y are immediately preceded by a sign which can be used to indicate
       flipping, the order of the X and Y indicate rotation. The standard coordinate system for
       Radiance images would have the following resolution string -Y N +X N. This indicates that the
       vertical axis runs down the file and the X axis is to the right (imagining the image as a
       rectangular block of data). A -X would indicate a horizontal flip of the image. A +Y would
       indicate a vertical flip. If the X value appears before the Y value then that indicates that
       the image is stored in column order rather than row order, that is, it is rotated by 90 degrees.
       The reader can convince themselves that the 8 combinations cover all the possible image orientations
       and rotations.
    */
    QRegularExpression resolutionRegExp(QStringLiteral("([+\\-][XY]) ([0-9]+) ([+\\-][XY]) ([0-9]+)\n"));
    QRegularExpressionMatch match = resolutionRegExp.match(QString::fromLatin1(line));
    if (!match.hasMatch()) {
        qCDebug(HDRPLUGIN) << "Invalid HDR file, the first line after the header didn't have the expected format:" << line;
        return false;
    }

    if ((match.captured(1).at(1) != u'Y') || (match.captured(3).at(1) != u'X')) {
        qCDebug(HDRPLUGIN) << "Unsupported image orientation in HDR file.";
        return false;
    }

    const int width = match.captured(4).toInt();
    const int height = match.captured(2).toInt();

    QDataStream s(device());

    QImage img;
    if (!LoadHDR(s, width, height, img)) {
        // qDebug() << "Error loading HDR file.";
        return false;
    }

    *outImage = img;
    return true;
}

HDRHandler::HDRHandler()
{
}

bool HDRHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("hdr");
        return true;
    }
    return false;
}

bool HDRHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("HDRHandler::canRead() called with no device");
        return false;
    }

    return device->peek(11) == "#?RADIANCE\n" || device->peek(7) == "#?RGBE\n";
}

QImageIOPlugin::Capabilities HDRPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "hdr") {
        return Capabilities(CanRead);
    }
    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && HDRHandler::canRead(device)) {
        cap |= CanRead;
    }
    return cap;
}

QImageIOHandler *HDRPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new HDRHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}
