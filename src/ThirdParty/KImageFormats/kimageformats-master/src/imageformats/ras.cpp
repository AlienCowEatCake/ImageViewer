/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Dominik Seichter <domseichter@web.de>
    SPDX-FileCopyrightText: 2004 Ignacio Castaño <castano@ludicon.com>
    SPDX-FileCopyrightText: 2010 Troy Unrau <troy@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ras_p.h"
#include "util_p.h"

#include <QDataStream>
#include <QDebug>
#include <QImage>

namespace // Private.
{
// format info from http://www.fileformat.info/format/sunraster/egff.htm

// Header format of saved files.
quint32 rasMagicBigEndian = 0x59a66a95;
// quint32 rasMagicLittleEndian = 0x956aa659; # used to support wrong encoded files

enum RASType {
    RAS_TYPE_OLD = 0x0,
    RAS_TYPE_STANDARD = 0x1,
    RAS_TYPE_BYTE_ENCODED = 0x2,
    RAS_TYPE_RGB_FORMAT = 0x3,
    RAS_TYPE_TIFF_FORMAT = 0x4,
    RAS_TYPE_IFF_FORMAT = 0x5,
    RAS_TYPE_EXPERIMENTAL = 0xFFFF,
};

enum RASColorMapType {
    RAS_COLOR_MAP_TYPE_NONE = 0x0,
    RAS_COLOR_MAP_TYPE_RGB = 0x1,
    RAS_COLOR_MAP_TYPE_RAW = 0x2,
};

struct RasHeader {
    quint32 MagicNumber;
    quint32 Width;
    quint32 Height;
    quint32 Depth;
    quint32 Length;
    quint32 Type;
    quint32 ColorMapType;
    quint32 ColorMapLength;
    enum {
        SIZE = 32,
    }; // 8 fields of four bytes each
};

static QDataStream &operator>>(QDataStream &s, RasHeader &head)
{
    s >> head.MagicNumber;
    s >> head.Width;
    s >> head.Height;
    s >> head.Depth;
    s >> head.Length;
    s >> head.Type;
    s >> head.ColorMapType;
    s >> head.ColorMapLength;
    /*qDebug() << "MagicNumber: " << head.MagicNumber
             << "Width: " << head.Width
             << "Height: " << head.Height
             << "Depth: " << head.Depth
             << "Length: " << head.Length
             << "Type: " << head.Type
             << "ColorMapType: " << head.ColorMapType
             << "ColorMapLength: " << head.ColorMapLength;*/
    return s;
}

static bool IsSupported(const RasHeader &head)
{
    // check magic number
    if (head.MagicNumber != rasMagicBigEndian) {
        return false;
    }
    // check for an appropriate depth
    // we support 8bit+palette, 24bit and 32bit ONLY!
    // TODO: add support for 1bit
    if (!((head.Depth == 8 && head.ColorMapType == 1) || head.Depth == 24 || head.Depth == 32)) {
        return false;
    }
    // the Type field adds support for RLE(BGR), RGB and other encodings
    // we support Type 1: Normal(BGR) and Type 3: Normal(RGB) ONLY!
    // TODO: add support for Type 2: RLE(BGR) & Type 4,5: TIFF/IFF
    if (!(head.Type == 1 || head.Type == 3)) {
        return false;
    }
    // Old files didn't have Length set - reject them for now
    // TODO: add length recalculation to support old files
    if (!head.Length) {
        return false;
    }
    return true;
}

static bool LoadRAS(QDataStream &s, const RasHeader &ras, QImage &img)
{
    s.device()->seek(RasHeader::SIZE);

    if (ras.ColorMapLength > kMaxQVectorSize) {
        qWarning() << "LoadRAS() unsupported image color map length in file header" << ras.ColorMapLength;
        return false;
    }

    // Read palette if needed.
    QVector<quint8> palette(ras.ColorMapLength);
    if (ras.ColorMapType == 1) {
        for (quint32 i = 0; i < ras.ColorMapLength; ++i) {
            s >> palette[i];
        }
    }

    const int bpp = ras.Depth / 8;
    if (ras.Height == 0) {
        return false;
    }
    if (bpp == 0) {
        return false;
    }
    if (ras.Length / ras.Height / bpp < ras.Width) {
        qWarning() << "LoadRAS() mistmatch between height and width" << ras.Width << ras.Height << ras.Length << ras.Depth;
        return false;
    }
    if (ras.Length > kMaxQVectorSize) {
        qWarning() << "LoadRAS() unsupported image length in file header" << ras.Length;
        return false;
    }

    // each line must be a factor of 16 bits, so they may contain padding
    // this will be 1 if padding required, 0 otherwise
    const int paddingrequired = (ras.Width * bpp % 2);

    // qDebug() << "paddingrequired: " << paddingrequired;
    // don't trust ras.Length
    QVector<quint8> input(ras.Length);

    int i = 0;
    while (!s.atEnd() && i < input.size()) {
        s >> input[i];
        // I guess we need to find out if we're at the end of a line
        if (paddingrequired && i != 0 && !(i % (ras.Width * bpp))) {
            s >> input[i];
        }
        i++;
    }

    // Allocate image
    img = imageAlloc(ras.Width, ras.Height, QImage::Format_ARGB32);
    if (img.isNull()) {
        return false;
    }

    // Reconstruct image from RGB palette if we have a palette
    // TODO: make generic so it works with 24bit or 32bit palettes
    if (ras.ColorMapType == 1 && ras.Depth == 8) {
        quint8 red;
        quint8 green;
        quint8 blue;
        for (quint32 y = 0; y < ras.Height; y++) {
            for (quint32 x = 0; x < ras.Width; x++) {
                red = palette.value((int)input[y * ras.Width + x]);
                green = palette.value((int)input[y * ras.Width + x] + (ras.ColorMapLength / 3));
                blue = palette.value((int)input[y * ras.Width + x] + 2 * (ras.ColorMapLength / 3));
                img.setPixel(x, y, qRgb(red, green, blue));
            }
        }
    }

    if (ras.ColorMapType == 0 && ras.Depth == 24 && (ras.Type == 1 || ras.Type == 2)) {
        quint8 red;
        quint8 green;
        quint8 blue;
        for (quint32 y = 0; y < ras.Height; y++) {
            for (quint32 x = 0; x < ras.Width; x++) {
                red = input[y * 3 * ras.Width + x * 3 + 2];
                green = input[y * 3 * ras.Width + x * 3 + 1];
                blue = input[y * 3 * ras.Width + x * 3];
                img.setPixel(x, y, qRgb(red, green, blue));
            }
        }
    }

    if (ras.ColorMapType == 0 && ras.Depth == 24 && ras.Type == 3) {
        quint8 red;
        quint8 green;
        quint8 blue;
        for (quint32 y = 0; y < ras.Height; y++) {
            for (quint32 x = 0; x < ras.Width; x++) {
                red = input[y * 3 * ras.Width + x * 3];
                green = input[y * 3 * ras.Width + x * 3 + 1];
                blue = input[y * 3 * ras.Width + x * 3 + 2];
                img.setPixel(x, y, qRgb(red, green, blue));
            }
        }
    }

    if (ras.ColorMapType == 0 && ras.Depth == 32 && (ras.Type == 1 || ras.Type == 2)) {
        quint8 red;
        quint8 green;
        quint8 blue;
        for (quint32 y = 0; y < ras.Height; y++) {
            for (quint32 x = 0; x < ras.Width; x++) {
                red = input[y * 4 * ras.Width + x * 4 + 3];
                green = input[y * 4 * ras.Width + x * 4 + 2];
                blue = input[y * 4 * ras.Width + x * 4 + 1];
                img.setPixel(x, y, qRgb(red, green, blue));
            }
        }
    }

    if (ras.ColorMapType == 0 && ras.Depth == 32 && ras.Type == 3) {
        quint8 red;
        quint8 green;
        quint8 blue;
        for (quint32 y = 0; y < ras.Height; y++) {
            for (quint32 x = 0; x < ras.Width; x++) {
                red = input[y * 4 * ras.Width + x * 4 + 1];
                green = input[y * 4 * ras.Width + x * 4 + 2];
                blue = input[y * 4 * ras.Width + x * 4 + 3];
                img.setPixel(x, y, qRgb(red, green, blue));
            }
        }
    }

    return true;
}
} // namespace

RASHandler::RASHandler()
{
}

bool RASHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("ras");
        return true;
    }
    return false;
}

bool RASHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("RASHandler::canRead() called with no device");
        return false;
    }

    if (device->isSequential()) {
        // qWarning("Reading ras files from sequential devices not supported");
        return false;
    }

    qint64 oldPos = device->pos();
    QByteArray head = device->read(RasHeader::SIZE); // header is exactly 32 bytes, always FIXME
    int readBytes = head.size(); // this should always be 32 bytes

    device->seek(oldPos);

    if (readBytes < RasHeader::SIZE) {
        return false;
    }

    QDataStream stream(head);
    stream.setByteOrder(QDataStream::BigEndian);
    RasHeader ras;
    stream >> ras;
    return IsSupported(ras);
}

bool RASHandler::read(QImage *outImage)
{
    QDataStream s(device());
    s.setByteOrder(QDataStream::BigEndian);

    // Read image header.
    RasHeader ras;
    s >> ras;

    if (ras.ColorMapLength > std::numeric_limits<int>::max()) {
        return false;
    }

    // TODO: add support for old versions of RAS where Length may be zero in header
    s.device()->seek(RasHeader::SIZE + ras.Length + ras.ColorMapLength);

    // Check image file format. Type 2 is RLE, which causing seeking to be silly.
    if (!s.atEnd() && ras.Type != 2) {
        //         qDebug() << "This RAS file is not valid, or an older version of the format.";
        return false;
    }

    // Check supported file types.
    if (!IsSupported(ras)) {
        //         qDebug() << "This RAS file is not supported.";
        return false;
    }

    QImage img;
    bool result = LoadRAS(s, ras, img);

    if (result == false) {
        //         qDebug() << "Error loading RAS file.";
        return false;
    }

    *outImage = img;
    return true;
}

QImageIOPlugin::Capabilities RASPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "ras") {
        return Capabilities(CanRead);
    }
    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && RASHandler::canRead(device)) {
        cap |= CanRead;
    }
    return cap;
}

QImageIOHandler *RASPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new RASHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#include "moc_ras_p.cpp"
