/*
    kimgio module for SGI images
    SPDX-FileCopyrightText: 2004 Melchior FRANZ <mfranz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/* this code supports:
 * reading:
 *     everything, except images with 1 dimension or images with
 *     mapmode != NORMAL (e.g. dithered); Images with 16 bit
 *     precision or more than 4 layers are stripped down.
 * writing:
 *     Run Length Encoded (RLE) or Verbatim (uncompressed)
 *     (whichever is smaller)
 *
 * Please report if you come across rgb/rgba/sgi/bw files that aren't
 * recognized. Also report applications that can't deal with images
 * saved by this filter.
 */

#include "rgb_p.h"
#include "util_p.h"

#include <cstring>

#include <QList>
#include <QMap>

#include <QDebug>
#include <QImage>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QVector>
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
class RLEData : public QList<uchar>
#else
class RLEData : public QVector<uchar>
#endif
{
public:
    RLEData()
    {
    }
    RLEData(const uchar *d, uint l, uint o)
        : _offset(o)
    {
        for (uint i = 0; i < l; i++) {
            append(d[i]);
        }
    }
    bool operator<(const RLEData &) const;
    void write(QDataStream &s);
    uint offset() const
    {
        return _offset;
    }

private:
    uint _offset;
};

class RLEMap : public QMap<RLEData, uint>
{
public:
    RLEMap()
        : _counter(0)
        , _offset(0)
    {
    }
    uint insert(const uchar *d, uint l);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QList<const RLEData *> vector();
#else
    QVector<const RLEData *> vector();
#endif
    void setBaseOffset(uint o)
    {
        _offset = o;
    }

private:
    uint _counter;
    uint _offset;
};

class SGIImagePrivate
{
public:
    SGIImagePrivate();
    ~SGIImagePrivate();

    bool readImage(QImage &);
    bool writeImage(const QImage &);

    bool isValid() const;
    bool isSupported() const;

    bool peekHeader(QIODevice *device);

    QSize size() const;
    QImage::Format format() const;

    void setDevice(QIODevice *device);

private:
    enum {
        NORMAL,
        DITHERED,
        SCREEN,
        COLORMAP,
    }; // colormap
    QIODevice *_dev;
    QDataStream _stream;

    quint16 _magic = 0;
    quint8 _rle = 0;
    quint8 _bpc = 0;
    quint16 _dim = 0;
    quint16 _xsize = 0;
    quint16 _ysize = 0;
    quint16 _zsize = 0;
    quint32 _pixmin = 0;
    quint32 _pixmax = 0;
    char _imagename[80];
    quint32 _colormap = 0;
    quint8 _unused[404];
    quint32 _unused32 = 0;

    quint32 *_starttab;
    quint32 *_lengthtab;
    QByteArray _data;
    QByteArray::Iterator _pos;
    RLEMap _rlemap;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QList<const RLEData *> _rlevector;
#else
    QVector<const RLEData *> _rlevector;
#endif
    uint _numrows;

    bool readData(QImage &);
    bool getRow(uchar *dest);
    bool readHeader();

    static bool readHeader(QDataStream &ds, SGIImagePrivate *sgi);

    bool writeHeader();
    bool writeRle();
    bool writeVerbatim(const QImage &);
    bool scanData(const QImage &);
    uint compact(uchar *, uchar *);
    uchar intensity(uchar);
};

SGIImagePrivate::SGIImagePrivate()
    : _dev(nullptr)
    , _starttab(nullptr)
    , _lengthtab(nullptr)
{
    std::memset(_imagename, 0, sizeof(_imagename));
    std::memset(_unused, 0, sizeof(_unused));
}

SGIImagePrivate::~SGIImagePrivate()
{
    delete[] _starttab;
    delete[] _lengthtab;
}

///////////////////////////////////////////////////////////////////////////////

void SGIImagePrivate::setDevice(QIODevice *device)
{
    _dev = device;
    _stream.setDevice(_dev);
}

bool SGIImagePrivate::getRow(uchar *dest)
{
    int n;
    int i;
    if (!_rle) {
        for (i = 0; i < _xsize; i++) {
            if (_pos >= _data.end()) {
                return false;
            }
            dest[i] = uchar(*_pos);
            _pos += _bpc;
        }
        return true;
    }

    for (i = 0; i < _xsize;) {
        if (_bpc == 2) {
            _pos++;
        }
        if (_pos >= _data.end()) {
            return false;
        }
        n = *_pos & 0x7f;
        if (!n) {
            break;
        }

        if (*_pos++ & 0x80) {
            for (; i < _xsize && _pos < _data.end() && n--; i++) {
                *dest++ = *_pos;
                _pos += _bpc;
            }
        } else {
            for (; i < _xsize && n--; i++) {
                *dest++ = *_pos;
            }

            _pos += _bpc;
        }
    }
    return i == _xsize;
}

bool SGIImagePrivate::readData(QImage &img)
{
    QRgb *c;
    quint32 *start = _starttab;
    QByteArray lguard(_xsize, 0);
    uchar *line = (uchar *)lguard.data();
    unsigned x;
    unsigned y;

    if (!_rle) {
        _pos = _data.begin();
    }

    for (y = 0; y < _ysize; y++) {
        if (_rle) {
            _pos = _data.begin() + *start++;
        }
        if (!getRow(line)) {
            return false;
        }
        c = (QRgb *)img.scanLine(_ysize - y - 1);
        for (x = 0; x < _xsize; x++, c++) {
            *c = qRgb(line[x], line[x], line[x]);
        }
    }

    if (_zsize == 1) {
        return true;
    }

    if (_zsize != 2) {
        for (y = 0; y < _ysize; y++) {
            if (_rle) {
                _pos = _data.begin() + *start++;
            }
            if (!getRow(line)) {
                return false;
            }
            c = (QRgb *)img.scanLine(_ysize - y - 1);
            for (x = 0; x < _xsize; x++, c++) {
                *c = qRgb(qRed(*c), line[x], line[x]);
            }
        }

        for (y = 0; y < _ysize; y++) {
            if (_rle) {
                _pos = _data.begin() + *start++;
            }
            if (!getRow(line)) {
                return false;
            }
            c = (QRgb *)img.scanLine(_ysize - y - 1);
            for (x = 0; x < _xsize; x++, c++) {
                *c = qRgb(qRed(*c), qGreen(*c), line[x]);
            }
        }

        if (_zsize == 3) {
            return true;
        }
    }

    for (y = 0; y < _ysize; y++) {
        if (_rle) {
            _pos = _data.begin() + *start++;
        }
        if (!getRow(line)) {
            return false;
        }
        c = (QRgb *)img.scanLine(_ysize - y - 1);
        for (x = 0; x < _xsize; x++, c++) {
            *c = qRgba(qRed(*c), qGreen(*c), qBlue(*c), line[x]);
        }
    }

    return true;
}

bool SGIImagePrivate::readImage(QImage &img)
{
    if (!readHeader() || !isSupported()) {
        return false;
    }

    if (_stream.atEnd()) {
        return false;
    }

    img = imageAlloc(size(), format());
    if (img.isNull()) {
        qWarning() << "Failed to allocate image, invalid dimensions?" << QSize(_xsize, _ysize);
        return false;
    }

    if (_zsize > 4) {
        //         qDebug() << "using first 4 of " << _zsize << " channels";
        // Only let this continue if it won't cause a int overflow later
        // this is most likely a broken file anyway
        if (_ysize > std::numeric_limits<int>::max() / _zsize) {
            return false;
        }
    }

    _numrows = _ysize * _zsize;

    if (_rle) {
        uint l;
        _starttab = new quint32[_numrows];
        for (l = 0; !_stream.atEnd() && l < _numrows; l++) {
            _stream >> _starttab[l];
            _starttab[l] -= 512 + _numrows * 2 * sizeof(quint32);
        }
        for (; l < _numrows; l++) {
            _starttab[l] = 0;
        }

        _lengthtab = new quint32[_numrows];
        for (l = 0; !_stream.atEnd() && l < _numrows; l++) {
            _stream >> _lengthtab[l];
        }
    }

    if (_stream.status() != QDataStream::Ok) {
        return false;
    }

    _data = _dev->readAll();

    // sanity check
    if (_rle) {
        for (uint o = 0; o < _numrows; o++) {
            // don't change to greater-or-equal!
            if (_starttab[o] + _lengthtab[o] > (uint)_data.size()) {
                //                 qDebug() << "image corrupt (sanity check failed)";
                return false;
            }
        }
    }

    if (!readData(img)) {
        //         qDebug() << "image corrupt (incomplete scanline)";
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void RLEData::write(QDataStream &s)
{
    for (int i = 0; i < size(); i++) {
        s << at(i);
    }
}

bool RLEData::operator<(const RLEData &b) const
{
    uchar ac;
    uchar bc;
    for (int i = 0; i < qMin(size(), b.size()); i++) {
        ac = at(i);
        bc = b[i];
        if (ac != bc) {
            return ac < bc;
        }
    }
    return size() < b.size();
}

uint RLEMap::insert(const uchar *d, uint l)
{
    RLEData data = RLEData(d, l, _offset);
    Iterator it = find(data);
    if (it != end()) {
        return it.value();
    }

    _offset += l;
    return QMap<RLEData, uint>::insert(data, _counter++).value();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
QList<const RLEData *> RLEMap::vector()
#else
QVector<const RLEData *> RLEMap::vector()
#endif
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QList<const RLEData *> v(size());
#else
    QVector<const RLEData *> v(size());
#endif
    for (Iterator it = begin(); it != end(); ++it) {
        v.replace(it.value(), &it.key());
    }

    return v;
}

uchar SGIImagePrivate::intensity(uchar c)
{
    if (c < _pixmin) {
        _pixmin = c;
    }
    if (c > _pixmax) {
        _pixmax = c;
    }
    return c;
}

uint SGIImagePrivate::compact(uchar *d, uchar *s)
{
    uchar *dest = d;
    uchar *src = s;
    uchar patt;
    uchar *t;
    uchar *end = s + _xsize;
    int i;
    int n;
    while (src < end) {
        for (n = 0, t = src; t + 2 < end && !(*t == t[1] && *t == t[2]); t++) {
            n++;
        }

        while (n) {
            i = n > 126 ? 126 : n;
            n -= i;
            *dest++ = 0x80 | i;
            while (i--) {
                *dest++ = *src++;
            }
        }

        if (src == end) {
            break;
        }

        patt = *src++;
        for (n = 1; src < end && *src == patt; src++) {
            n++;
        }

        while (n) {
            i = n > 126 ? 126 : n;
            n -= i;
            *dest++ = i;
            *dest++ = patt;
        }
    }
    *dest++ = 0;
    return dest - d;
}

bool SGIImagePrivate::scanData(const QImage &img)
{
    quint32 *start = _starttab;
    QByteArray lineguard(_xsize * 2, 0);
    QByteArray bufguard(_xsize, 0);
    uchar *line = (uchar *)lineguard.data();
    uchar *buf = (uchar *)bufguard.data();
    const QRgb *c;
    unsigned x;
    unsigned y;
    uint len;

    for (y = 0; y < _ysize; y++) {
        const int yPos = _ysize - y - 1; // scanline doesn't do any sanity checking
        if (yPos >= img.height()) {
            qWarning() << "Failed to get scanline for" << yPos;
            return false;
        }

        c = reinterpret_cast<const QRgb *>(img.scanLine(yPos));

        for (x = 0; x < _xsize; x++) {
            buf[x] = intensity(qRed(*c++));
        }
        len = compact(line, buf);
        *start++ = _rlemap.insert(line, len);
    }

    if (_zsize == 1) {
        return true;
    }

    if (_zsize != 2) {
        for (y = 0; y < _ysize; y++) {
            const int yPos = _ysize - y - 1;
            if (yPos >= img.height()) {
                qWarning() << "Failed to get scanline for" << yPos;
                return false;
            }

            c = reinterpret_cast<const QRgb *>(img.scanLine(yPos));
            for (x = 0; x < _xsize; x++) {
                buf[x] = intensity(qGreen(*c++));
            }
            len = compact(line, buf);
            *start++ = _rlemap.insert(line, len);
        }

        for (y = 0; y < _ysize; y++) {
            const int yPos = _ysize - y - 1;
            if (yPos >= img.height()) {
                qWarning() << "Failed to get scanline for" << yPos;
                return false;
            }

            c = reinterpret_cast<const QRgb *>(img.scanLine(yPos));
            for (x = 0; x < _xsize; x++) {
                buf[x] = intensity(qBlue(*c++));
            }
            len = compact(line, buf);
            *start++ = _rlemap.insert(line, len);
        }

        if (_zsize == 3) {
            return true;
        }
    }

    for (y = 0; y < _ysize; y++) {
        const int yPos = _ysize - y - 1;
        if (yPos >= img.height()) {
            qWarning() << "Failed to get scanline for" << yPos;
            return false;
        }

        c = reinterpret_cast<const QRgb *>(img.scanLine(yPos));
        for (x = 0; x < _xsize; x++) {
            buf[x] = intensity(qAlpha(*c++));
        }
        len = compact(line, buf);
        *start++ = _rlemap.insert(line, len);
    }

    return true;
}

bool SGIImagePrivate::isValid() const
{
    // File signature/magic number
    if (_magic != 0x01da) {
        return false;
    }
    // Compression, 0 = Uncompressed, 1 = RLE compressed
    if (_rle > 1) {
        return false;
    }
    // Bytes per pixel, 1 = 8 bit, 2 = 16 bit
    if (_bpc != 1 && _bpc != 2) {
        return false;
    }
    // Image dimension, 3 for RGBA image
    if (_dim < 1 || _dim > 3) {
        return false;
    }
    // Number channels in the image file, 4 for RGBA image
    if (_zsize < 1) {
        return false;
    }
    return true;
}

bool SGIImagePrivate::isSupported() const
{
    if (!isValid()) {
        return false;
    }
    if (_colormap != NORMAL) {
        return false; // only NORMAL supported
    }
    if (_dim == 1) {
        return false;
    }
    return true;
}

bool SGIImagePrivate::peekHeader(QIODevice *device)
{
    QDataStream ds(device->peek(512));
    return SGIImagePrivate::readHeader(ds, this) && isValid();
}

QSize SGIImagePrivate::size() const
{
    return QSize(_xsize, _ysize);
}

QImage::Format SGIImagePrivate::format() const
{
    if (_zsize == 2 || _zsize == 4) {
        return QImage::Format_ARGB32;
    }
    return QImage::Format_RGB32;
}

bool SGIImagePrivate::readHeader()
{
    return readHeader(_stream, this);
}

bool SGIImagePrivate::readHeader(QDataStream &ds, SGIImagePrivate *sgi)
{
    // magic
    ds >> sgi->_magic;

    // verbatim/rle
    ds >> sgi->_rle;

    // bytes per channel
    ds >> sgi->_bpc;

    // number of dimensions
    ds >> sgi->_dim;

    ds >> sgi->_xsize >> sgi->_ysize >> sgi->_zsize >> sgi->_pixmin >> sgi->_pixmax >> sgi->_unused32;

    // name
    ds.readRawData(sgi->_imagename, 80);
    sgi->_imagename[79] = '\0';

    ds >> sgi->_colormap;

    for (size_t i = 0; i < sizeof(_unused); i++) {
        ds >> sgi->_unused[i];
    }

    return ds.status() == QDataStream::Ok;
}

bool SGIImagePrivate::writeHeader()
{
    _stream << _magic;
    _stream << _rle << _bpc << _dim;
    _stream << _xsize << _ysize << _zsize;
    _stream << _pixmin << _pixmax;
    _stream << _unused32;

    for (int i = 0; i < 80; i++) {
        _imagename[i] = '\0';
    }
    _stream.writeRawData(_imagename, 80);

    _stream << _colormap;
    for (size_t i = 0; i < sizeof(_unused); i++) {
        _stream << _unused[i];
    }
    return _stream.status() == QDataStream::Ok;
}

bool SGIImagePrivate::writeRle()
{
    _rle = 1;
    //     qDebug() << "writing RLE data";
    if (!writeHeader()) {
        return false;
    }

    uint i;

    // write start table
    for (i = 0; i < _numrows; i++) {
        _stream << quint32(_rlevector[_starttab[i]]->offset());
    }

    // write length table
    for (i = 0; i < _numrows; i++) {
        _stream << quint32(_rlevector[_starttab[i]]->size());
    }

    // write data
    for (i = 0; (int)i < _rlevector.size(); i++) {
        const_cast<RLEData *>(_rlevector[i])->write(_stream);
    }

    return _stream.status() == QDataStream::Ok;
}

bool SGIImagePrivate::writeVerbatim(const QImage &img)
{
    _rle = 0;
    if (!writeHeader()) {
        return false;
    }

    const QRgb *c;
    unsigned x;
    unsigned y;

    for (y = 0; y < _ysize; y++) {
        c = reinterpret_cast<const QRgb *>(img.scanLine(_ysize - y - 1));
        for (x = 0; x < _xsize; x++) {
            _stream << quint8(qRed(*c++));
        }
    }

    if (_zsize == 1) {
        return _stream.status() == QDataStream::Ok;
    }

    if (_zsize != 2) {
        for (y = 0; y < _ysize; y++) {
            c = reinterpret_cast<const QRgb *>(img.scanLine(_ysize - y - 1));
            for (x = 0; x < _xsize; x++) {
                _stream << quint8(qGreen(*c++));
            }
        }

        for (y = 0; y < _ysize; y++) {
            c = reinterpret_cast<const QRgb *>(img.scanLine(_ysize - y - 1));
            for (x = 0; x < _xsize; x++) {
                _stream << quint8(qBlue(*c++));
            }
        }

        if (_zsize == 3) {
            return _stream.status() == QDataStream::Ok;
        }
    }

    for (y = 0; y < _ysize; y++) {
        c = reinterpret_cast<const QRgb *>(img.scanLine(_ysize - y - 1));
        for (x = 0; x < _xsize; x++) {
            _stream << quint8(qAlpha(*c++));
        }
    }

    return _stream.status() == QDataStream::Ok;
}

bool SGIImagePrivate::writeImage(const QImage &image)
{
    //     qDebug() << "writing "; // TODO add filename
    QImage img = image;
    if (img.allGray()) {
        _dim = 2, _zsize = 1;
    } else {
        _dim = 3, _zsize = 3;
    }

    auto hasAlpha = img.hasAlphaChannel();
    if (hasAlpha) {
        _dim = 3, _zsize++;
    }

    if (hasAlpha && img.format() != QImage::Format_ARGB32) {
        img = img.convertToFormat(QImage::Format_ARGB32);
    } else if (!hasAlpha && img.format() != QImage::Format_RGB32) {
        img = img.convertToFormat(QImage::Format_RGB32);
    }
    if (img.isNull()) {
        //         qDebug() << "can't convert image to depth 32";
        return false;
    }

    const int w = img.width();
    const int h = img.height();

    if (w > 65535 || h > 65535) {
        return false;
    }

    _magic = 0x01da;
    _bpc = 1;
    _xsize = w;
    _ysize = h;
    _pixmin = ~0u;
    _pixmax = 0;
    _colormap = NORMAL;
    _numrows = _ysize * _zsize;
    _starttab = new quint32[_numrows];
    _rlemap.setBaseOffset(512 + _numrows * 2 * sizeof(quint32));

    if (!scanData(img)) {
        //         qDebug() << "this can't happen";
        return false;
    }

    _rlevector = _rlemap.vector();

    long verbatim_size = _numrows * _xsize;
    long rle_size = _numrows * 2 * sizeof(quint32);
    for (int i = 0; i < _rlevector.size(); i++) {
        rle_size += _rlevector[i]->size();
    }

    if (verbatim_size <= rle_size) {
        return writeVerbatim(img);
    }
    return writeRle();
}

///////////////////////////////////////////////////////////////////////////////

RGBHandler::RGBHandler()
    : QImageIOHandler()
    , d(new SGIImagePrivate)
{
}

bool RGBHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("rgb");
        return true;
    }
    return false;
}

bool RGBHandler::read(QImage *outImage)
{
    d->setDevice(device());
    return d->readImage(*outImage);
}

bool RGBHandler::write(const QImage &image)
{
    d->setDevice(device());
    return d->writeImage(image);
}

bool RGBHandler::supportsOption(ImageOption option) const
{
    if (option == QImageIOHandler::Size) {
        return true;
    }
    if (option == QImageIOHandler::ImageFormat) {
        return true;
    }
    return false;
}

QVariant RGBHandler::option(ImageOption option) const
{
    QVariant v;

    if (option == QImageIOHandler::Size) {
        auto &&sgi = d;
        if (sgi->isSupported()) {
            v = QVariant::fromValue(sgi->size());
        } else if (auto dev = device()) {
            if (d->peekHeader(dev) && sgi->isSupported()) {
                v = QVariant::fromValue(sgi->size());
            }
        }
    }

    if (option == QImageIOHandler::ImageFormat) {
        auto &&sgi = d;
        if (sgi->isSupported()) {
            v = QVariant::fromValue(sgi->format());
        } else if (auto dev = device()) {
            if (d->peekHeader(dev) && sgi->isSupported()) {
                v = QVariant::fromValue(sgi->format());
            }
        }
    }

    return v;
}

bool RGBHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("RGBHandler::canRead() called with no device");
        return false;
    }

    SGIImagePrivate sgi;
    return sgi.peekHeader(device) && sgi.isSupported();
}

///////////////////////////////////////////////////////////////////////////////

QImageIOPlugin::Capabilities RGBPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "rgb" || format == "rgba" || format == "bw" || format == "sgi") {
        return Capabilities(CanRead | CanWrite);
    }
    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && RGBHandler::canRead(device)) {
        cap |= CanRead;
    }
    if (device->isWritable()) {
        cap |= CanWrite;
    }
    return cap;
}

QImageIOHandler *RGBPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new RGBHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#include "moc_rgb_p.cpp"
