/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2025 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "chunks_p.h"
#include "packbits_p.h"

#include <QBuffer>
#include <QColor>

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(LOG_IFFPLUGIN, "kf.imageformats.plugins.iff", QtDebugMsg)
#else
Q_LOGGING_CATEGORY(LOG_IFFPLUGIN, "kf.imageformats.plugins.iff", QtWarningMsg)
#endif

#define RECURSION_PROTECTION 10

#define BITPLANES_HAM_MAX 8
#define BITPLANES_HAM_MIN 5
#define BITPLANES_HALFBRIDE_MAX 8
#define BITPLANES_HALFBRIDE_MIN 1

static QString dataToString(const IFFChunk *chunk)
{
    if (chunk == nullptr || !chunk->isValid()) {
        return {};
    }
    auto dt = chunk->data();
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    for (; dt.endsWith(char()); dt = dt.removeLast());
#else
    for (; dt.endsWith(char()) && !dt.isEmpty(); dt.remove(dt.size() - 1, 1));
#endif
    return QString::fromUtf8(dt).trimmed();
}

IFFChunk::~IFFChunk()
{

}

IFFChunk::IFFChunk()
    : _chunkId{0}
    , _size{0}
    , _align{2}
    , _dataPos{0}
    , _recursionCnt{0}
{
}

bool IFFChunk::operator ==(const IFFChunk &other) const
{
    if (chunkId() != other.chunkId()) {
        return false;
    }
    return _size == other._size && _dataPos == other._dataPos;
}

bool IFFChunk::isValid() const
{
    auto cid = chunkId();
    if (cid.isEmpty()) {
        return false;
    }
    // A “type ID”, “property name”, “FORM type”, or any other IFF
    // identifier is a 32-bit value: the concatenation of four ASCII
    // characters in the range “ ” (SP, hex 20) through “~” (hex 7E).
    // Spaces (hex 20) should not precede printing characters;
    // trailing spaces are OK. Control characters are forbidden.
    if (cid.at(0) == ' ') {
        return false;
    }
    for (auto &&c : cid) {
        if (c < ' ' || c > '~')
            return false;
    }
    return true;
}

qint32 IFFChunk::alignBytes() const
{
    return _align;
}

bool IFFChunk::readStructure(QIODevice *d)
{
    auto ok = readInfo(d);
    if (recursionCounter() > RECURSION_PROTECTION - 1) {
        ok = ok && IFFChunk::innerReadStructure(d); // force default implementation (no more recursion)
    } else {
        ok = ok && innerReadStructure(d);
    }
    if (ok) {
        ok = d->seek(nextChunkPos());
    }
    return ok;
}

QByteArray IFFChunk::chunkId() const
{
    return QByteArray(_chunkId, 4);
}

quint32 IFFChunk::bytes() const
{
    return _size;
}

const QByteArray &IFFChunk::data() const
{
    return _data;
}

const IFFChunk::ChunkList &IFFChunk::chunks() const
{
    return _chunks;
}

quint8 IFFChunk::chunkVersion(const QByteArray &cid)
{
    if (cid.size() != 4) {
        return 0;
    }
    if (cid.at(3) >= char('2') && cid.at(3) <= char('9')) {
        return quint8(cid.at(3) - char('0'));
    }
    return 1;
}

bool IFFChunk::isChunkType(const QByteArray &cid) const
{
    if (chunkId() == cid) {
        return true;
    }
    if (chunkId().startsWith(cid.left(3)) && IFFChunk::chunkVersion(cid) > 1) {
        return true;
    }
    return false;
}

bool IFFChunk::readInfo(QIODevice *d)
{
    if (d == nullptr || d->read(_chunkId, 4) != 4) {
        return false;
    }
    if (!IFFChunk::isValid()) {
        return false;
    }
    auto sz = d->read(4);
    if (sz.size() != 4) {
        return false;
    }
    _size = ui32(sz.at(3), sz.at(2), sz.at(1), sz.at(0));
    _dataPos = d->pos();
    return true;
}

QByteArray IFFChunk::readRawData(QIODevice *d, qint64 relPos, qint64 size) const
{
    if (!seek(d, relPos)) {
        return{};
    }
    if (size == -1) {
        size = _size;
    }
    auto read = std::min(size, _size - relPos);
    return d->read(read);
}

bool IFFChunk::seek(QIODevice *d, qint64 relPos) const
{
    if (d == nullptr) {
        return false;
    }
    return d->seek(_dataPos + relPos);
}

bool IFFChunk::innerReadStructure(QIODevice *)
{
    return true;
}

void IFFChunk::setAlignBytes(qint32 bytes)
{
    _align = bytes;
}

qint64 IFFChunk::nextChunkPos() const
{
    auto pos = _dataPos + _size;
    if (auto align = pos % alignBytes())
        pos += alignBytes() - align;
    return pos;
}

IFFChunk::ChunkList IFFChunk::search(const QByteArray &cid, const QSharedPointer<IFFChunk> &chunk)
{
    return search(cid, ChunkList() << chunk);
}

IFFChunk::ChunkList IFFChunk::search(const QByteArray &cid, const ChunkList &chunks)
{
    IFFChunk::ChunkList list;
    for (auto &&chunk : chunks) {
        if (chunk->chunkId() == cid)
            list << chunk;
        list << IFFChunk::search(cid, chunk->_chunks);
    }
    return list;
}

bool IFFChunk::cacheData(QIODevice *d)
{
    if (bytes() > 8 * 1024 * 1024) {
        return false;
    }
    _data = readRawData(d);
    return _data.size() == _size;
}

void IFFChunk::setChunks(const ChunkList &chunks)
{
    _chunks = chunks;
}

qint32 IFFChunk::recursionCounter() const
{
    return _recursionCnt;
}

void IFFChunk::setRecursionCounter(qint32 cnt)
{
    _recursionCnt = cnt;
}

IFFChunk::ChunkList IFFChunk::innerFromDevice(QIODevice *d, bool *ok, IFFChunk *parent)
{
    auto tmp = false;
    if (ok == nullptr) {
        ok = &tmp;
    }
    *ok = false;

    if (d == nullptr) {
        return {};
    }

    auto alignBytes = qint32(2);
    auto recursionCnt = qint32();
    auto nextChunkPos = qint64();
    if (parent) {
        alignBytes = parent->alignBytes();
        recursionCnt = parent->recursionCounter();
        nextChunkPos = parent->nextChunkPos();
    }

    if (recursionCnt > RECURSION_PROTECTION) {
        return {};
    }

    IFFChunk::ChunkList list;
    for (; !d->atEnd() && (nextChunkPos == 0 || d->pos() < nextChunkPos);) {
        auto cid = d->peek(4);
        QSharedPointer<IFFChunk> chunk;
        if (cid == ABIT_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new ABITChunk());
        } else if (cid == ANNO_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new ANNOChunk());
        } else if (cid == AUTH_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new AUTHChunk());
        } else if (cid == BEAM_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new BEAMChunk());
        } else if (cid == BMHD_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new BMHDChunk());
        } else if (cid == BODY_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new BODYChunk());
        } else if (cid == CAMG_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new CAMGChunk());
        } else if (cid == CAT__CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new CATChunk());
        } else if (cid == CMAP_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new CMAPChunk());
        } else if (cid == CMYK_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new CMYKChunk());
        } else if (cid == COPY_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new COPYChunk());
        } else if (cid == CTBL_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new CTBLChunk());
        } else if (cid == DATE_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new DATEChunk());
        } else if (cid == DPI__CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new DPIChunk());
        } else if (cid == EXIF_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new EXIFChunk());
        } else if (cid == FOR4_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new FOR4Chunk());
        } else if (cid == FORM_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new FORMChunk());
        } else if (cid == FVER_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new FVERChunk());
        } else if (cid == HIST_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new HISTChunk());
        } else if (cid == ICCN_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new ICCNChunk());
        } else if (cid == ICCP_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new ICCPChunk());
        } else if (cid == NAME_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new NAMEChunk());
        } else if (cid == RAST_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new RASTChunk());
        } else if (cid == RGBA_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new RGBAChunk());
        } else if (cid == SHAM_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new SHAMChunk());
        } else if (cid == TBHD_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new TBHDChunk());
        } else if (cid == VERS_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new VERSChunk());
        } else if (cid == XMP0_CHUNK) {
            chunk = QSharedPointer<IFFChunk>(new XMP0Chunk());
        } else { // unknown chunk
            chunk = QSharedPointer<IFFChunk>(new IFFChunk());
            qCDebug(LOG_IFFPLUGIN) << "IFFChunk::innerFromDevice(): unknown chunk" << cid;
        }

        // change the alignment to the one of main chunk (required for unknown Maya IFF chunks)
        if (chunk->isChunkType(CAT__CHUNK)
            || chunk->isChunkType(FILL_CHUNK)
            || chunk->isChunkType(FORM_CHUNK)
            || chunk->isChunkType(LIST_CHUNK)
            || chunk->isChunkType(PROP_CHUNK)) {
            alignBytes = chunk->alignBytes();
        } else {
            chunk->setAlignBytes(alignBytes);
        }

        chunk->setRecursionCounter(recursionCnt + 1);
        if (!chunk->readStructure(d)) {
            *ok = false;
            return {};
        }

        // skip any non-IFF data at the end of the file.
        // NOTE: there should be no more chunks after the first (root)
        if (nextChunkPos == 0) {
            nextChunkPos = chunk->nextChunkPos();
        }

        list << chunk;
    }

    *ok = true;
    return list;
}

IFFChunk::ChunkList IFFChunk::fromDevice(QIODevice *d, bool *ok)
{
    return innerFromDevice(d, ok, nullptr);
}


/* ******************
 * *** BMHD Chunk ***
 * ****************** */

BMHDChunk::~BMHDChunk()
{

}

BMHDChunk::BMHDChunk() : IFFChunk()
{
}

bool BMHDChunk::isValid() const
{
    if (bytes() < 20) {
        return false;
    }
    return chunkId() == BMHDChunk::defaultChunkId();
}

bool BMHDChunk::innerReadStructure(QIODevice *d)
{
    return cacheData(d);
}

qint32 BMHDChunk::width() const
{
    if (!isValid()) {
        return 0;
    }
    return qint32(ui16(data().at(1), data().at(0)));
}

qint32 BMHDChunk::height() const
{
    if (!isValid()) {
        return 0;
    }
    return qint32(ui16(data().at(3), data().at(2)));
}

QSize BMHDChunk::size() const
{
    return QSize(width(), height());
}

qint32 BMHDChunk::left() const
{
    if (!isValid()) {
        return 0;
    }
    return qint32(ui16(data().at(5), data().at(4)));
}

qint32 BMHDChunk::top() const
{
    if (!isValid()) {
        return 0;
    }
    return qint32(ui16(data().at(7), data().at(6)));
}

quint8 BMHDChunk::bitplanes() const
{
    if (!isValid()) {
        return 0;
    }
    return quint8(data().at(8));
}

BMHDChunk::Masking BMHDChunk::masking() const
{
    if (!isValid()) {
        return BMHDChunk::Masking::None;
    }
    return BMHDChunk::Masking(quint8(data().at(9)));
}

BMHDChunk::Compression BMHDChunk::compression() const
{
    if (!isValid()) {
        return BMHDChunk::Compression::Uncompressed;
    }
    return BMHDChunk::Compression(data().at(10));

}

qint16 BMHDChunk::transparency() const
{
    if (!isValid()) {
        return 0;
    }
    return i16(data().at(13), data().at(12));
}

quint8 BMHDChunk::xAspectRatio() const
{
    if (!isValid()) {
        return 0;
    }
    return quint8(data().at(14));
}

quint8 BMHDChunk::yAspectRatio() const
{
    if (!isValid()) {
        return 0;
    }
    return quint8(data().at(15));
}

quint16 BMHDChunk::pageWidth() const
{
    if (!isValid()) {
        return 0;
    }
    return ui16(data().at(17), data().at(16));
}

quint16 BMHDChunk::pageHeight() const
{
    if (!isValid()) {
        return 0;
    }
    return ui16(data().at(19), data().at(18));
}

quint32 BMHDChunk::rowLen() const
{
    return ((quint32(width()) + 15) / 16) * 2;
}

/* ******************
 * *** CMAP Chunk ***
 * ****************** */

CMAPChunk::~CMAPChunk()
{

}

CMAPChunk::CMAPChunk() : IFFChunk()
{
}

bool CMAPChunk::isValid() const
{
    return chunkId() == CMAPChunk::defaultChunkId();
}

qint32 CMAPChunk::count() const
{
    if (!isValid()) {
        return 0;
    }
    return bytes() / 3;
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
QList<QRgb> CMAPChunk::palette(bool halfbride) const
#else
QVector<QRgb> CMAPChunk::palette(bool halfbride) const
#endif
{
    auto p = innerPalette();
    if (!halfbride) {
        return p;
    }
    auto tmp = p;
    for (auto &&v : tmp) {
        p << qRgb(qRed(v) / 2, qGreen(v) / 2, qBlue(v) / 2);
    }
    return p;
}

bool CMAPChunk::innerReadStructure(QIODevice *d)
{
    return cacheData(d);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
QList<QRgb> CMAPChunk::innerPalette() const
#else
QVector<QRgb> CMAPChunk::innerPalette() const
#endif
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QList<QRgb> l;
#else
    QVector<QRgb> l;
#endif
    auto &&d = data();
    for (qint32 i = 0, n = count(); i < n; ++i) {
        auto i3 = i * 3;
        l << qRgb(d.at(i3), d.at(i3 + 1), d.at(i3 + 2));
    }
    return l;
}


/* ******************
 * *** CMYK Chunk ***
 * ****************** */

CMYKChunk::~CMYKChunk()
{

}

CMYKChunk::CMYKChunk() : CMAPChunk()
{

}

bool CMYKChunk::isValid() const
{
    return chunkId() == CMYKChunk::defaultChunkId();
}

qint32 CMYKChunk::count() const
{
    if (!isValid()) {
        return 0;
    }
    return bytes() / 4;
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
QList<QRgb> CMYKChunk::innerPalette() const
#else
QVector<QRgb> CMYKChunk::innerPalette() const
#endif
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QList<QRgb> l;
#else
    QVector<QRgb> l;
#endif
    auto &&d = data();
    for (qint32 i = 0, n = count(); i < n; ++i) {
        auto i4 = i * 4;
        auto C = quint8(d.at(i4)) / 255.;
        auto M = quint8(d.at(i4 + 1)) / 255.;
        auto Y = quint8(d.at(i4 + 2)) / 255.;
        auto K = quint8(d.at(i4 + 3)) / 255.;
        l << QColor::fromCmykF(C, M, Y, K).toRgb().rgb();
    }
    return l;
}


/* ******************
 * *** CAMG Chunk ***
 * ****************** */

CAMGChunk::~CAMGChunk()
{

}

CAMGChunk::CAMGChunk() : IFFChunk()
{
}

bool CAMGChunk::isValid() const
{
    if (bytes() != 4) {
        return false;
    }
    return chunkId() == CAMGChunk::defaultChunkId();
}

CAMGChunk::ModeIds CAMGChunk::modeId() const
{
    if (!isValid()) {
        return CAMGChunk::ModeIds();
    }
    return CAMGChunk::ModeIds(ui32(data().at(3), data().at(2), data().at(1), data().at(0)));
}

bool CAMGChunk::innerReadStructure(QIODevice *d)
{
    return cacheData(d);
}

/* ******************
 * *** DPI  Chunk ***
 * ****************** */

DPIChunk::~DPIChunk()
{

}

DPIChunk::DPIChunk() : IFFChunk()
{
}

bool DPIChunk::isValid() const
{
    if (dpiX() == 0 || dpiY() == 0) {
        return false;
    }
    return chunkId() == DPIChunk::defaultChunkId();
}

quint16 DPIChunk::dpiX() const
{
    if (bytes() < 4) {
        return 0;
    }
    return i16(data().at(1), data().at(0));
}

quint16 DPIChunk::dpiY() const
{
    if (bytes() < 4) {
        return 0;
    }
    return i16(data().at(3), data().at(2));
}

qint32 DPIChunk::dotsPerMeterX() const
{
    return qRound(dpiX() / 25.4 * 1000);
}

qint32 DPIChunk::dotsPerMeterY() const
{
    return qRound(dpiY() / 25.4 * 1000);
}

bool DPIChunk::innerReadStructure(QIODevice *d)
{
    return cacheData(d);
}

/* ******************
 * *** BODY Chunk ***
 * ****************** */

BODYChunk::~BODYChunk()
{

}

BODYChunk::BODYChunk() : IFFChunk()
{
}

bool BODYChunk::isValid() const
{
    return chunkId() == BODYChunk::defaultChunkId();
}

// For each RGB value, a LONG-word (32 bits) is written:
// with the 24 RGB bits in the MSB positions; the "genlock"
// bit next, and then a 7 bit repeat count.
//
// See also: https://wiki.amigaos.net/wiki/RGBN_and_RGB8_IFF_Image_Data
inline qint64 rgb8Decompress(QIODevice *input, char *output, qint64 olen)
{
    qint64 j = 0;
    for (qint64 available = olen; j < olen; available = olen - j) {
        auto pos = input->pos();
        auto ba4 = input->read(4);
        if (ba4.size() != 4) {
            break;
        }
        auto cnt = qint32(ba4.at(3) & 0x7F);
        if (cnt * 3 > available) {
            if (!input->seek(pos))
                return -1;
            break;
        }
        for (qint32 i = 0; i < cnt; ++i) {
            output[j++] = ba4.at(0);
            output[j++] = ba4.at(1);
            output[j++] = ba4.at(2);
        }
    }
    return j;
}

// For each RGB value, a WORD (16-bits) is written: with the
// 12 RGB bits in the MSB (most significant bit) positions;
// the "genlock" bit next; and then a 3 bit repeat count.
// If the repeat count is greater than 7, the 3-bit count is
// zero, and a BYTE repeat count follows.  If the repeat count
// is greater than 255, the BYTE count is zero, and a WORD
// repeat count follows.  Repeat counts greater than 65536 are
// not supported.
//
// See also: https://wiki.amigaos.net/wiki/RGBN_and_RGB8_IFF_Image_Data
inline qint32 rgbnCount(QIODevice *input, quint8 &R, quint8& G, quint8 &B)
{
    auto ba2 = input->read(2);
    if (ba2.size() != 2)
        return 0;

    R = ba2.at(0) & 0xF0;
    R = R | (R >> 4);

    G = ba2.at(0) & 0x0F;
    G = G | (G << 4);

    B = ba2.at(1) & 0xF0;
    B = B | (B >> 4);

    auto cnt = ba2.at(1) & 7;
    if (cnt == 0) {
        auto ba1 = input->read(1);
        if (ba1.size() != 1)
            return 0;
        cnt = quint8(ba1.at(0));
    }
    if (cnt == 0) {
        auto baw = input->read(2);
        if (baw.size() != 2)
            return 0;
        cnt = qint32(quint8(baw.at(0))) << 8 | quint8(baw.at(1));
    }

    return cnt;
}

inline qint64 rgbNDecompress(QIODevice *input, char *output, qint64 olen)
{
    qint64 j = 0;
    for (qint64 available = olen; j < olen; available = olen - j) {
        quint8 R = 0, G = 0, B = 0;
        auto pos = input->pos();
        auto cnt = rgbnCount(input, R, G, B);
        if (cnt * 3 > available || cnt == 0) {
            if (!input->seek(pos))
                return -1;
            break;
        }
        for (qint32 i = 0; i < cnt; ++i) {
            output[j++] = R;
            output[j++] = G;
            output[j++] = B;
        }
    }
    return j;
}

QByteArray BODYChunk::strideRead(QIODevice *d, qint32 y, const BMHDChunk *header, const CAMGChunk *camg, const CMAPChunk *cmap, const IPALChunk *ipal, const QByteArray& formType) const
{
    if (!isValid() || header == nullptr || d == nullptr) {
        return {};
    }

    auto isRgbN = formType == RGBN_FORM_TYPE;
    auto isRgb8 = formType == RGB8_FORM_TYPE;
    auto isPbm = formType == PBM__FORM_TYPE;
    auto lineCompressed = isRgbN || isRgb8 ? false : true;
    auto readSize = strideSize(header, formType);
    auto bufSize = readSize;
    if (isRgbN) {
        bufSize = std::max(quint32(65536 * 3), readSize);
    }
    if (isRgb8) {
        bufSize = std::max(quint32(127 * 3), readSize);
    }
    for (auto nextPos = nextChunkPos(); !d->atEnd() && d->pos() < nextPos && _readBuffer.size() < readSize;) {
        QByteArray buf(bufSize, char());
        qint64 rr = -1;
        if (header->compression() == BMHDChunk::Compression::Rle) {
            // WARNING: The online spec says it's the same as TIFF but that's
            // not accurate: the RLE -128 code is not a noop.
            rr = packbitsDecompress(d, buf.data(), buf.size(), true);
        } else if (header->compression() == BMHDChunk::Compression::RgbN8) {
            if (isRgb8)
                rr = rgb8Decompress(d, buf.data(), buf.size());
            else if (isRgbN)
                rr = rgbNDecompress(d, buf.data(), buf.size());
        } else if (header->compression() == BMHDChunk::Compression::Uncompressed) {
            rr = d->read(buf.data(), buf.size()); // never seen
        } else {
            qCDebug(LOG_IFFPLUGIN) << "BODYChunk::strideRead(): unknown compression" << header->compression();
        }
        if ((rr != readSize && lineCompressed) || (rr < 1))
            return {};
        _readBuffer.append(buf.data(), rr);
    }

    auto planes = _readBuffer.left(readSize);
    _readBuffer.remove(0, readSize);
    if (isPbm) {
        return pbm(planes, y, header, camg, cmap, ipal);
    }
    if (isRgb8) {
        return rgb8(planes, y, header, camg, cmap, ipal);
    }
    if (isRgbN) {
        return rgbN(planes, y, header, camg, cmap, ipal);
    }
    return deinterleave(planes, y, header, camg, cmap, ipal);
}

bool BODYChunk::resetStrideRead(QIODevice *d) const
{
    _readBuffer.clear();
    return seek(d);
}

CAMGChunk::ModeIds BODYChunk::safeModeId(const BMHDChunk *header, const CAMGChunk *camg, const CMAPChunk *cmap)
{
    if (camg) {
        return camg->modeId();
    }
    if (header == nullptr) {
        return CAMGChunk::ModeIds();
    }
    auto cmapCount = cmap ? cmap->count() : 0;
    auto bitplanes = header->bitplanes();
    if (bitplanes >= BITPLANES_HALFBRIDE_MIN && bitplanes <= BITPLANES_HALFBRIDE_MAX) {
        if (cmapCount == (1 << (header->bitplanes() - 1)))
            return CAMGChunk::ModeIds(CAMGChunk::ModeId::HalfBrite);
    }
    if (bitplanes >= BITPLANES_HAM_MIN && bitplanes <= BITPLANES_HAM_MAX) {
        if (cmapCount == (1 << (header->bitplanes() - 2)))
            return CAMGChunk::ModeIds(CAMGChunk::ModeId::Ham);
    }
    return CAMGChunk::ModeIds();
}

quint32 BODYChunk::strideSize(const BMHDChunk *header, const QByteArray& formType) const
{
    // RGB8 / RGBN
    if (formType == RGB8_FORM_TYPE || formType == RGBN_FORM_TYPE) {
        return header->width() * 3;
    }

    // PBM
    if (formType == PBM__FORM_TYPE) {
        auto rs = header->width() * header->bitplanes() / 8;
        if (rs & 1)
            ++rs;
        return rs;
    }

    // ILBM
    return header->rowLen() * header->bitplanes();
}

QByteArray BODYChunk::pbm(const QByteArray &planes, qint32, const BMHDChunk *header, const CAMGChunk *, const CMAPChunk *, const IPALChunk *) const
{
    if (planes.size() != strideSize(header, PBM__FORM_TYPE)) {
        return {};
    }
    if (header->bitplanes() == 8) {
        // The data are contiguous.
        return planes;
    }
    return {};
}

QByteArray BODYChunk::rgb8(const QByteArray &planes, qint32, const BMHDChunk *header, const CAMGChunk *, const CMAPChunk *, const IPALChunk *) const
{
    if (planes.size() != strideSize(header, RGB8_FORM_TYPE)) {
        return {};
    }
    return planes;
}

QByteArray BODYChunk::rgbN(const QByteArray &planes, qint32, const BMHDChunk *header, const CAMGChunk *, const CMAPChunk *, const IPALChunk *) const
{
    if (planes.size() != strideSize(header, RGBN_FORM_TYPE)) {
        return {};
    }
    return planes;
}

QByteArray BODYChunk::deinterleave(const QByteArray &planes, qint32 y, const BMHDChunk *header, const CAMGChunk *camg, const CMAPChunk *cmap, const IPALChunk *ipal) const
{
    if (planes.size() != strideSize(header, ILBM_FORM_TYPE)) {
        return {};
    }

    auto rowLen = qint32(header->rowLen());
    auto bitplanes = header->bitplanes();
    auto modeId = BODYChunk::safeModeId(header, camg, cmap);

    QByteArray ba;
    switch (bitplanes) {
    case 1: // gray, indexed and rgb Ham mode
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        if ((modeId & CAMGChunk::ModeId::Ham) && (cmap) &&
            (bitplanes >= BITPLANES_HAM_MIN && bitplanes <= BITPLANES_HAM_MAX)) {
            // From A Quick Introduction to IFF.txt:
            //
            // Amiga HAM (Hold and Modify) mode lets the Amiga display all 4096 RGB values.
            // In HAM mode, the bits in the two last planes describe an R G or B
            // modification to the color of the previous pixel on the line to create the
            // color of the current pixel.  So a 6-plane HAM picture has 4 planes for
            // specifying absolute color pixels giving up to 16 absolute colors which would
            // be specified in the ILBM CMAP chunk.  The bits in the last two planes are
            // color modification bits which cause the Amiga, in HAM mode, to take the RGB
            // value of the previous pixel (Hold and), substitute the 4 bits in planes 0-3
            // for the previous color's R G or B component (Modify) and display the result
            // for the current pixel.  If the first pixel of a scan line is a modification
            // pixel, it modifies the RGB value of the border color (register 0).  The color
            // modification bits in the last two planes (planes 4 and 5) are interpreted as
            // follows:
            //    00 - no modification.  Use planes 0-3 as normal color register index
            //    10 - hold previous, replacing Blue component with bits from planes 0-3
            //    01 - hold previous, replacing Red component with bits from planes 0-3
            //    11 - hold previous. replacing Green component with bits from planes 0-3
            ba = QByteArray(rowLen * 8 * 3, char());
            auto pal = cmap->palette();
            if (ipal) {
                auto tmp = ipal->palette(y, header->height());
                if (tmp.size() == pal.size())
                    pal = tmp;
            }
            auto max = (1 << (bitplanes - 2)) - 1;
            quint8 prev[3] = {};
            for (qint32 i = 0, cnt = 0; i < rowLen; ++i) {
                for (qint32 j = 0; j < 8; ++j, ++cnt) {
                    quint8 idx = 0, ctl = 0;
                    for (qint32 k = 0, msk = (1 << (7 - j)); k < bitplanes; ++k) {
                        if ((planes.at(k * rowLen + i) & msk) == 0)
                            continue;
                        if (k < bitplanes - 2)
                            idx |= 1 << k;
                        else
                            ctl |= 1 << (bitplanes - k - 1);
                    }
                    switch (ctl) {
                    case 1: // red
                        prev[0] = idx * 255 / max;
                        break;
                    case 2: // blue
                        prev[2] = idx * 255 / max;
                        break;
                    case 3: // green
                        prev[1] = idx * 255 / max;
                        break;
                    default:
                        if (idx < pal.size()) {
                            prev[0] = qRed(pal.at(idx));
                            prev[1] = qGreen(pal.at(idx));
                            prev[2] = qBlue(pal.at(idx));
                        } else {
                            qCWarning(LOG_IFFPLUGIN) << "BODYChunk::deinterleave(): palette index" << idx << "is out of range";
                        }
                        break;
                    }
                    auto cnt3 = cnt * 3;
                    ba[cnt3] = char(prev[0]);
                    ba[cnt3 + 1] = char(prev[1]);
                    ba[cnt3 + 2] = char(prev[2]);
                }
            }
        } else if ((modeId & CAMGChunk::ModeId::HalfBrite) && (cmap) &&
                   (bitplanes >= BITPLANES_HALFBRIDE_MIN && bitplanes <= BITPLANES_HALFBRIDE_MAX)) {
            // From A Quick Introduction to IFF.txt:
            //
            // In HALFBRITE mode, the Amiga interprets the bit in the
            // last plane as HALFBRITE modification.  The bits in the other planes are
            // treated as normal color register numbers (RGB values for each color register
            // is specified in the CMAP chunk).  If the bit in the last plane is set (1),
            // then that pixel is displayed at half brightness.  This can provide up to 64
            // absolute colors.
            ba = QByteArray(rowLen * 8, char());
            auto palSize = cmap->count();
            for (qint32 i = 0, cnt = 0; i < rowLen; ++i) {
                for (qint32 j = 0; j < 8; ++j, ++cnt) {
                    quint8 idx = 0, ctl = 0;
                    for (qint32 k = 0, msk = (1 << (7 - j)); k < bitplanes; ++k) {
                        if ((planes.at(k * rowLen + i) & msk) == 0)
                            continue;
                        if (k < bitplanes - 1)
                            idx |= 1 << k;
                        else
                            ctl = 1;
                    }
                    if (idx < palSize) {
                        ba[cnt] = ctl ? idx + palSize : idx;
                    } else {
                        qCWarning(LOG_IFFPLUGIN) << "BODYChunk::deinterleave(): palette index" << idx << "is out of range";
                    }
                }
            }
        } else {
            // From A Quick Introduction to IFF.txt:
            //
            // If the ILBM is not HAM or HALFBRITE, then after parsing and uncompacting if
            // necessary, you will have N planes of pixel data.  Color register used for
            // each pixel is specified by looking at each pixel thru the planes.  I.e.,
            // if you have 5 planes, and the bit for a particular pixel is set in planes
            // 0 and 3:
            //
            //        PLANE     4 3 2 1 0
            //        PIXEL     0 1 0 0 1
            //
            // then that pixel uses color register binary 01001 = 9
            ba = QByteArray(rowLen * 8, char());
            for (qint32 i = 0; i < rowLen; ++i) {
                for (qint32 k = 0, i8 = i * 8; k < bitplanes; ++k) {
                    auto v = planes.at(k * rowLen + i);
                    if (v & (1 << 7))
                        ba[i8 + 0] = ba[i8 + 0] | (1 << k);
                    if (v & (1 << 6))
                        ba[i8 + 1] = ba[i8 + 1] | (1 << k);
                    if (v & (1 << 5))
                        ba[i8 + 2] = ba[i8 + 2] | (1 << k);
                    if (v & (1 << 4))
                        ba[i8 + 3] = ba[i8 + 3] | (1 << k);
                    if (v & (1 << 3))
                        ba[i8 + 4] = ba[i8 + 4] | (1 << k);
                    if (v & (1 << 2))
                        ba[i8 + 5] = ba[i8 + 5] | (1 << k);
                    if (v & (1 << 1))
                        ba[i8 + 6] = ba[i8 + 6] | (1 << k);
                    if (v & 1)
                        ba[i8 + 7] = ba[i8 + 7] | (1 << k);
                }
            }
        }
        break;

    case 24: // rgb
    case 32: // rgba (SView5 extension)
        // From A Quick Introduction to IFF.txt:
        //
        // If a deep ILBM (like 12 or 24 planes), there should be no CMAP
        // and instead the BODY planes are interpreted as the bits of RGB
        // in the order R0...Rn G0...Gn B0...Bn
        //
        // NOTE: This code does not support 12-planes images
        ba = QByteArray(rowLen * bitplanes, char());
        for (qint32 i = 0, cnt = 0, p = bitplanes / 8; i < rowLen; ++i) {
            for (qint32 j = 0; j < 8; ++j)
                for (qint32 k = 0; k < p; ++k, ++cnt) {
                    auto k8 = k * 8;
                    auto msk = (1 << (7 - j));
                    if (planes.at(k8 * rowLen + i) & msk)
                        ba[cnt] = ba[cnt] | 0x01;
                    if (planes.at((1 + k8) * rowLen + i) & msk)
                        ba[cnt] = ba[cnt] | 0x02;
                    if (planes.at((2 + k8) * rowLen + i) & msk)
                        ba[cnt] = ba[cnt] | 0x04;
                    if (planes.at((3 + k8) * rowLen + i) & msk)
                        ba[cnt] = ba[cnt] | 0x08;
                    if (planes.at((4 + k8) * rowLen + i) & msk)
                        ba[cnt] = ba[cnt] | 0x10;
                    if (planes.at((5 + k8) * rowLen + i) & msk)
                        ba[cnt] = ba[cnt] | 0x20;
                    if (planes.at((6 + k8) * rowLen + i) & msk)
                        ba[cnt] = ba[cnt] | 0x40;
                    if (planes.at((7 + k8) * rowLen + i) & msk)
                        ba[cnt] = ba[cnt] | 0x80;
                }
        }
        break;

    case 48: // rgb (SView5 extension)
    case 64: // rgba (SView5 extension)
        // From https://aminet.net/package/docs/misc/ILBM64:
        //
        // Previously, the IFF-ILBM fileformat has been
        // extended two times already, for 24 bit and 32 bit
        // image data:
        //
        //  24 bit -> 24 planes composing RGB 8:8:8 true color
        //  32 bit -> 32 planes composing RGBA 8:8:8:8 true color
        //                                             plus alpha
        //
        // The former extension quickly became a common one,
        // while the latter until recently mainly had been
        // used by some NewTek software.
        //
        // Now the following - as a consequent logical extension
        // of the previously mentioned definitions - is introduced
        // by SView5-Library:
        //
        // 48 bit -> 48 planes composing RGB 16:16:16 true color
        // 64 bit -> 64 planes composing RGBA 16:16:16:16 true color
        //                                                plus alpha
        //
        // The resulting data is intended to allow direct transformation
        // from the PNG format into the Amiga (ILBM) bitmap format.

        ba = QByteArray(rowLen * 64, char()); // the RGBX QT format is 64-bits
        const qint32 order[] = { 1, 0, 3, 2, 5, 4, 7, 6 };
        for (qint32 i = 0, cnt = 0, p = bitplanes / 8; i < rowLen; ++i) {
            for (qint32 j = 0; j < 8; ++j, cnt += 8) {
                for (qint32 k = 0; k < p; ++k) {
                    auto k8 = k * 8;
                    auto msk = (1 << (7 - j));
                    auto idx = cnt + order[k];
                    if (planes.at(k8 * rowLen + i) & msk)
                        ba[idx] = ba[idx] | 0x01;
                    if (planes.at((1 + k8) * rowLen + i) & msk)
                        ba[idx] = ba[idx] | 0x02;
                    if (planes.at((2 + k8) * rowLen + i) & msk)
                        ba[idx] = ba[idx] | 0x04;
                    if (planes.at((3 + k8) * rowLen + i) & msk)
                        ba[idx] = ba[idx] | 0x08;
                    if (planes.at((4 + k8) * rowLen + i) & msk)
                        ba[idx] = ba[idx] | 0x10;
                    if (planes.at((5 + k8) * rowLen + i) & msk)
                        ba[idx] = ba[idx] | 0x20;
                    if (planes.at((6 + k8) * rowLen + i) & msk)
                        ba[idx] = ba[idx] | 0x40;
                    if (planes.at((7 + k8) * rowLen + i) & msk)
                        ba[idx] = ba[idx] | 0x80;
                }
                if (p == 6) { // RGBX wants unused X data set to 0xFF
                    ba[cnt + 6] = char(0xFF);
                    ba[cnt + 7] = char(0xFF);
                }
            }
        }
        break;
    }
    return ba;
}

/* ******************
 * *** ABIT Chunk ***
 * ****************** */

ABITChunk::~ABITChunk()
{

}

ABITChunk::ABITChunk() : BODYChunk()
{

}

bool ABITChunk::isValid() const
{
    return chunkId() == ABITChunk::defaultChunkId();
}

QByteArray ABITChunk::strideRead(QIODevice *d, qint32 y, const BMHDChunk *header, const CAMGChunk *camg, const CMAPChunk *cmap, const IPALChunk *ipal, const QByteArray& formType) const
{
    if (!isValid() || header == nullptr || d == nullptr) {
        return {};
    }
    if (header->compression() != BMHDChunk::Compression::Uncompressed || formType != ACBM_FORM_TYPE) {
        return {};
    }

    // convert ABIT data to an ILBM line on the fly
    auto ilbmLine = QByteArray(strideSize(header, formType), char());
    auto rowSize = header->rowLen();
    auto height = header->height();
    if (y >= height) {
        return {};
    }
    for (qint32 plane = 0, planes = qint32(header->bitplanes()); plane < planes; ++plane) {
        if (!seek(d, qint64(plane) * rowSize * height + y * rowSize))
            return {};
        auto offset = qint64(plane) * rowSize;
        if (offset + rowSize > ilbmLine.size())
            return {};
        if (d->read(ilbmLine.data() + offset, rowSize) != rowSize)
            return {};
    }

    // decode the ILBM line
    QBuffer buf;
    buf.setData(ilbmLine);
    if (!buf.open(QBuffer::ReadOnly)) {
        return {};
    }
    return BODYChunk::strideRead(&buf, y, header, camg, cmap, ipal, ILBM_FORM_TYPE);
}

bool ABITChunk::resetStrideRead(QIODevice *d) const
{
    return BODYChunk::resetStrideRead(d);
}


/* **********************
 * *** FORM Interface ***
 * ********************** */

IFOR_Chunk::~IFOR_Chunk()
{

}

IFOR_Chunk::IFOR_Chunk() : IFFChunk()
{

}

QImageIOHandler::Transformation IFOR_Chunk::transformation() const
{
    auto exifs = IFFChunk::searchT<EXIFChunk>(chunks());
    if (!exifs.isEmpty()) {
        auto exif = exifs.first()->value();
        if (!exif.isEmpty())
            return exif.transformation();
    }
    return QImageIOHandler::Transformation::TransformationNone;
}

QImage::Format IFOR_Chunk::optionformat() const
{
    auto fmt = this->format();
    if (fmt == QImage::Format_Indexed8) {
        if (searchIPal())
            fmt = FORMAT_RGB_8BIT;
    }
    return fmt;
}

const IPALChunk *IFOR_Chunk::searchIPal() const
{
    const IPALChunk *ipal = nullptr;
    auto beam = IFFChunk::searchT<BEAMChunk>(this);
    if (!beam.isEmpty()) {
        ipal = beam.first();
    }
    auto ctbl = IFFChunk::searchT<CTBLChunk>(this);
    if (!ctbl.isEmpty()) {
        ipal = ctbl.first();
    }
    auto sham = IFFChunk::searchT<SHAMChunk>(this);
    if (!sham.isEmpty()) {
        ipal = sham.first();
    }
    auto rast = IFFChunk::searchT<RASTChunk>(this);
    if (!rast.isEmpty()) {
        ipal = rast.first();
    }
    if (ipal && ipal->isValid()) {
        return ipal;
    }
    return nullptr;
}


/* ******************
 * *** FORM Chunk ***
 * ****************** */

FORMChunk::~FORMChunk()
{

}

FORMChunk::FORMChunk() : IFOR_Chunk()
{
}

bool FORMChunk::isValid() const
{
    return chunkId() == FORMChunk::defaultChunkId();
}

bool FORMChunk::isSupported() const
{
    return format() != QImage::Format_Invalid;
}

bool FORMChunk::innerReadStructure(QIODevice *d)
{
    if (bytes() < 4) {
        return false;
    }
    _type = d->read(4);
    auto ok = true;

    // NOTE: add new supported type to CATChunk as well.
    if (_type == ILBM_FORM_TYPE) {
        setChunks(IFFChunk::innerFromDevice(d, &ok, this));
    } else if (_type == PBM__FORM_TYPE) {
        setChunks(IFFChunk::innerFromDevice(d, &ok, this));
    } else if (_type == ACBM_FORM_TYPE) {
        setChunks(IFFChunk::innerFromDevice(d, &ok, this));
    } else if (_type == RGB8_FORM_TYPE) {
        setChunks(IFFChunk::innerFromDevice(d, &ok, this));
    } else if (_type == RGBN_FORM_TYPE) {
        setChunks(IFFChunk::innerFromDevice(d, &ok, this));
    }
    return ok;
}

QByteArray FORMChunk::formType() const
{
    return _type;
}

QImage::Format FORMChunk::format() const
{
    auto headers = IFFChunk::searchT<BMHDChunk>(chunks());
    if (headers.isEmpty()) {
        return QImage::Format_Invalid;
    }

    if (auto &&h = headers.first()) {
        auto cmaps = IFFChunk::searchT<CMAPChunk>(chunks());
        if (cmaps.isEmpty()) {
            auto cmyks = IFFChunk::searchT<CMYKChunk>(chunks());
            for (auto &&cmyk : cmyks)
                cmaps.append(cmyk);
        }
        auto camgs = IFFChunk::searchT<CAMGChunk>(chunks());
        auto modeId = BODYChunk::safeModeId(h, camgs.isEmpty() ? nullptr : camgs.first(), cmaps.isEmpty() ? nullptr : cmaps.first());
        if (h->bitplanes() == 13) {
            return FORMAT_RGB_8BIT; // NOTE: with a little work you could use Format_RGB444
        }
        if (h->bitplanes() == 24 || h->bitplanes() == 25) {
            return FORMAT_RGB_8BIT;
        }
        if (h->bitplanes() == 48) {
            return QImage::Format_RGBX64;
        }
        if (h->bitplanes() == 32) {
            return QImage::Format_RGBA8888;
        }
        if (h->bitplanes() == 64) {
            return QImage::Format_RGBA64;
        }
        if (h->bitplanes() >= 1 && h->bitplanes() <= 8) {
            if (!IFFChunk::search(PCHG_CHUNK, chunks()).isEmpty()) {
                qCDebug(LOG_IFFPLUGIN) << "FORMChunk::format(): PCHG chunk is not supported";
                return QImage::Format_Invalid;
            }

            if (h->bitplanes() >= BITPLANES_HAM_MIN && h->bitplanes() <= BITPLANES_HAM_MAX) {
                if (modeId & CAMGChunk::ModeId::Ham)
                    return FORMAT_RGB_8BIT;
            }

            if (!cmaps.isEmpty()) {
                return QImage::Format_Indexed8;
            }

            return QImage::Format_Grayscale8;
        }
        qCDebug(LOG_IFFPLUGIN) << "FORMChunk::format(): Unsupported" << h->bitplanes() << "bitplanes";
    }

    return QImage::Format_Invalid;
}

QSize FORMChunk::size() const
{
    auto headers = IFFChunk::searchT<BMHDChunk>(chunks());
    if (headers.isEmpty()) {
        return {};
    }
    return headers.first()->size();
}

/* ******************
 * *** FOR4 Chunk ***
 * ****************** */

FOR4Chunk::~FOR4Chunk()
{

}

FOR4Chunk::FOR4Chunk() : IFOR_Chunk()
{

}

bool FOR4Chunk::isValid() const
{
    return chunkId() == FOR4Chunk::defaultChunkId();
}

qint32 FOR4Chunk::alignBytes() const
{
    return 4;
}

bool FOR4Chunk::isSupported() const
{
    return format() != QImage::Format_Invalid;
}

bool FOR4Chunk::innerReadStructure(QIODevice *d)
{
    if (bytes() < 4) {
        return false;
    }
    _type = d->read(4);
    auto ok = true;
    if (_type == CIMG_FOR4_TYPE) {
        setChunks(IFFChunk::innerFromDevice(d, &ok, this));
    } else if (_type == TBMP_FOR4_TYPE) {
        setChunks(IFFChunk::innerFromDevice(d, &ok, this));
    }
    return ok;
}

QByteArray FOR4Chunk::formType() const
{
    return _type;
}

QImage::Format FOR4Chunk::format() const
{
    auto headers = IFFChunk::searchT<TBHDChunk>(chunks());
    if (headers.isEmpty()) {
        return QImage::Format_Invalid;
    }
    return headers.first()->format();
}

QSize FOR4Chunk::size() const
{
    auto headers = IFFChunk::searchT<TBHDChunk>(chunks());
    if (headers.isEmpty()) {
        return {};
    }
    return headers.first()->size();
}

/* ******************
 * *** CAT Chunk ***
 * ****************** */

CATChunk::~CATChunk()
{

}

CATChunk::CATChunk() : IFFChunk()
{

}

bool CATChunk::isValid() const
{
    return chunkId() == CATChunk::defaultChunkId();
}

QByteArray CATChunk::catType() const
{
    return _type;
}

bool CATChunk::innerReadStructure(QIODevice *d)
{
    if (bytes() < 4) {
        return false;
    }
    _type = d->read(4);
    auto ok = true;

    // supports the image formats of FORMChunk.
    if (_type == ILBM_FORM_TYPE) {
        setChunks(IFFChunk::innerFromDevice(d, &ok, this));
    } else if (_type == PBM__FORM_TYPE) {
        setChunks(IFFChunk::innerFromDevice(d, &ok, this));
    } else if (_type == ACBM_FORM_TYPE) {
        setChunks(IFFChunk::innerFromDevice(d, &ok, this));
    } else if (_type == RGB8_FORM_TYPE) {
        setChunks(IFFChunk::innerFromDevice(d, &ok, this));
    } else if (_type == RGBN_FORM_TYPE) {
        setChunks(IFFChunk::innerFromDevice(d, &ok, this));
    }
    return ok;
}

/* ******************
 * *** TBHD Chunk ***
 * ****************** */

TBHDChunk::~TBHDChunk()
{

}

TBHDChunk::TBHDChunk()
{

}

bool TBHDChunk::isValid() const
{
    if (bytes() != 24 && bytes() != 32) {
        return false;
    }
    return chunkId() == TBHDChunk::defaultChunkId();
}

qint32 TBHDChunk::alignBytes() const
{
    return 4;
}

qint32 TBHDChunk::width() const
{
    if (!isValid()) {
        return 0;
    }
    return i32(data().at(3), data().at(2), data().at(1), data().at(0));
}

qint32 TBHDChunk::height() const
{
    if (!isValid()) {
        return 0;
    }
    return i32(data().at(7), data().at(6), data().at(5), data().at(4));
}

QSize TBHDChunk::size() const
{
    return QSize(width(), height());
}

qint32 TBHDChunk::left() const
{
    if (bytes() != 32) {
        return 0;
    }
    return i32(data().at(27), data().at(26), data().at(25), data().at(24));
}

qint32 TBHDChunk::top() const
{
    if (bytes() != 32) {
        return 0;
    }
    return i32(data().at(31), data().at(30), data().at(29), data().at(28));
}

TBHDChunk::Flags TBHDChunk::flags() const
{
    if (!isValid()) {
        return TBHDChunk::Flags();
    }
    return TBHDChunk::Flags(ui32(data().at(15), data().at(14), data().at(13), data().at(12)));
}

qint32 TBHDChunk::bpc() const
{
    if (!isValid()) {
        return 0;
    }
    return ui16(data().at(17), data().at(16)) ? 2 : 1;
}

qint32 TBHDChunk::channels() const
{
    if ((flags() & TBHDChunk::Flag::RgbA) == TBHDChunk::Flag::RgbA) {
        return 4;
    }
    if ((flags() & TBHDChunk::Flag::Rgb) == TBHDChunk::Flag::Rgb) {
        return 3;
    }
    return 0;
}

quint16 TBHDChunk::tiles() const
{
    if (!isValid()) {
        return 0;
    }
    return ui16(data().at(19), data().at(18));
}

QImage::Format TBHDChunk::format() const
{
    // Support for RGBA and RGB only for now.
    if ((flags() & TBHDChunk::Flag::RgbA) == TBHDChunk::Flag::RgbA) {
        if (bpc() == 2)
            return QImage::Format_RGBA64;
        else if (bpc() == 1)
            return QImage::Format_RGBA8888;
    } else if ((flags() & TBHDChunk::Flag::Rgb) == TBHDChunk::Flag::Rgb) {
        if (bpc() == 2)
            return QImage::Format_RGBX64;
        else if (bpc() == 1)
            return FORMAT_RGB_8BIT;
    }

    return QImage::Format_Invalid;
}

TBHDChunk::Compression TBHDChunk::compression() const
{
    if (!isValid()) {
        return TBHDChunk::Compression::Uncompressed;
    }
    return TBHDChunk::Compression(ui32(data().at(23), data().at(22), data().at(21), data().at(20)));
}

bool TBHDChunk::innerReadStructure(QIODevice *d)
{
    return cacheData(d);
}

/* ******************
 * *** RGBA Chunk ***
 * ****************** */

RGBAChunk::~RGBAChunk()
{
}

RGBAChunk::RGBAChunk()
{

}

bool RGBAChunk::isValid() const
{
    if (bytes() < 8) {
        return false;
    }
    return chunkId() == RGBAChunk::defaultChunkId();
}

qint32 RGBAChunk::alignBytes() const
{
    return 4;
}

bool RGBAChunk::isTileCompressed(const TBHDChunk *header) const
{
    if (!isValid() || header == nullptr) {
        return false;
    }
    return qint64(header->channels()) * size().width() * size().height() * header->bpc() > qint64(bytes() - 8);
}

QPoint RGBAChunk::pos() const
{
    return _posPx;
}

QSize RGBAChunk::size() const
{
    return _sizePx;
}

// Maya version of IFF uses a slightly different algorithm for RLE compression.
// To understand how it works I saved images with regular patterns from Photoshop
// and then checked the data. It is basically the same as packbits except for how
// the length is extracted: I don't know if it's a standard variant or not, so
// I'm keeping it private.
inline qint64 rleMayaDecompress(QIODevice *input, char *output, qint64 olen)
{
    qint64  j = 0;
    for (qint64 rr = 0, available = olen; j < olen; available = olen - j) {
        char n;

        // check the output buffer space for the next run
        if (available < 128) {
            if (input->peek(&n, 1) != 1) { // end of data (or error)
                break;
            }
            rr = qint64(n & 0x7F) + 1;
            if (rr > available)
                break;
        }

        // decompress
        if (input->read(&n, 1) != 1) { // end of data (or error)
            break;
        }

        rr = qint64(n & 0x7F) + 1;
        if ((n & 0x80) == 0) {
            auto read = input->read(output + j, rr);
            if (rr != read) {
                return -1;
            }
        } else {
            char b;
            if (input->read(&b, 1) != 1) {
                break;
            }
            std::memset(output + j, b, size_t(rr));
        }

        j += rr;
    }
    return j;
}

QByteArray RGBAChunk::readStride(QIODevice *d, const TBHDChunk *header) const
{
    auto readSize = size().width();
    if (readSize == 0) {
        return {};
    }

    // It seems that tiles are compressed independently only if there is space savings.
    // The compression method specified in the header is only to indicate the type of
    // compression if used.
    if (!isTileCompressed(header)) {
        // when not compressed, the line contains all channels
        readSize *= header->bpc() * header->channels();
        QByteArray buf(readSize, char());
        auto rr = d->read(buf.data(), buf.size());
        if (rr != buf.size()) {
            return {};
        }
        return buf;
    }

    // compressed
    for (auto nextPos = nextChunkPos(); !d->atEnd() && d->pos() < nextPos && _readBuffer.size() < readSize;) {
        QByteArray buf(readSize * size().height(), char());
        qint64 rr = -1;
        if (header->compression() == TBHDChunk::Compression::Rle) {
            rr = rleMayaDecompress(d, buf.data(), buf.size());
        }
        if (rr != buf.size()) {
            return {};
        }
        _readBuffer.append(buf.data(), rr);
    }

    auto buff = _readBuffer.left(readSize);
    _readBuffer.remove(0, readSize);

    return buff;
}

/*!
 * \brief compressedTile
 *
 * The compressed tile contains compressed data per channel.
 *
 * If 16 bit, high and low bytes are treated separately (so I have
 * channels * 2 compressed data blocks). First the high ones, then the low
 * ones (or vice versa): for the reconstruction I went by trial and error :)
 * \param d The device
 * \param header The header.
 * \return The tile as Qt image.
 */
QImage RGBAChunk::compressedTile(QIODevice *d, const TBHDChunk *header) const
{
    QImage img(size(), header->format());
    auto bpc = header->bpc();

    if (bpc == 1) {
        for (auto c = 0, cs = header->channels(); c < cs; ++c) {
            for (auto y = 0, h = img.height(); y < h; ++y) {
                auto ba = readStride(d, header);
                if (ba.isEmpty()) {
                    return {};
                }
                auto scl = reinterpret_cast<quint8*>(img.scanLine(y));
                for (auto x = 0, w = std::min(int(ba.size()), img.width()); x < w; ++x) {
                    scl[x * cs + cs - c - 1] = ba.at(x);
                }
            }
        }
    } else if (bpc == 2) {
        auto cs = header->channels();
        if (cs < 4) { // alpha on 64-bit images must be 0xFF
            std::memset(img.bits(), 0xFF, img.sizeInBytes());
        }
        for (auto c = 0, cc = header->channels() * header->bpc(); c < cc; ++c) {
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
            auto c_bcp = c / cs; // Not tried
#else
            auto c_bcp = 1 - c / cs;
#endif
            auto c_cs = (cs - 1 - c % cs) * bpc + c_bcp;
            for (auto y = 0, h = img.height(); y < h; ++y) {
                auto ba = readStride(d, header);
                if (ba.isEmpty()) {
                    return {};
                }
                auto scl = reinterpret_cast<quint8*>(img.scanLine(y));
                for (auto x = 0, w = std::min(int(ba.size()), img.width()); x < w; ++x) {
                    scl[x * 4 * bpc + c_cs] = ba.at(x); // * 4 -> Qt RGB 64-bit formats are always 4 channels
                }
            }
        }
    }

    return img;
}

/*!
 * \brief RGBAChunk::uncompressedTile
 *
 * The uncompressed tile scanline contains the data in
 * B0 G0 R0 A0 B1 G1 R1 A1... Bn Gn Rn An format.
 * \param d The device
 * \param header The header.
 * \return The tile as Qt image.
 */
QImage RGBAChunk::uncompressedTile(QIODevice *d, const TBHDChunk *header) const
{
    QImage img(size(), header->format());
    auto bpc = header->bpc();

    if (bpc == 1) {
        auto cs = header->channels();
        for (auto y = 0, h = img.height(); y < h; ++y) {
            auto ba = readStride(d, header);
            if (ba.isEmpty()) {
                return {};
            }
            auto scl = reinterpret_cast<quint8*>(img.scanLine(y));
            for (auto c = 0; c < cs; ++c) {
                for (auto x = 0, w = std::min(int(ba.size() / cs), img.width()); x < w; ++x) {
                    auto xcs = x * cs;
                    scl[xcs + cs - c - 1] = ba.at(xcs + c);
                }
            }
        }
    } else if (bpc == 2) {
        auto cs = header->channels();
        if (cs < 4) { // alpha on 64-bit images must be 0xFF
            std::memset(img.bits(), 0xFF, img.sizeInBytes());
        }

        for (auto y = 0, h = img.height(); y < h; ++y) {
            auto ba = readStride(d, header);
            if (ba.isEmpty()) {
                return {};
            }
            auto scl = reinterpret_cast<quint16*>(img.scanLine(y));
            auto src = reinterpret_cast<const quint16*>(ba.data());
            for (auto c = 0; c < cs; ++c) {
                for (auto x = 0, w = std::min(int(ba.size() / cs / bpc), img.width()); x < w; ++x) {
                    auto xcs = x * cs;
                    auto xcs4 = x * 4;
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
                    scl[xcs4 + cs - c - 1] = src[xcs + c]; // Not tried
#else
                    scl[xcs4 + cs - c - 1] = (src[xcs + c] >> 8) | (src[xcs + c] << 8);
#endif
                }
            }
        }
    }

    return img;
}

QImage RGBAChunk::tile(QIODevice *d, const TBHDChunk *header) const
{
    if (!isValid() || header == nullptr) {
        return {};
    }
    if (!seek(d, 8)) {
        return {};
    }

    if (isTileCompressed(header)) {
        return compressedTile(d, header);
    }

    return uncompressedTile(d, header);
}

bool RGBAChunk::innerReadStructure(QIODevice *d)
{
    auto ba = d->read(8);
    if (ba.size() != 8) {
        return false;
    }

    auto x0 = ui16(ba.at(1), ba.at(0));
    auto y0 = ui16(ba.at(3), ba.at(2));
    auto x1 = ui16(ba.at(5), ba.at(4));
    auto y1 = ui16(ba.at(7), ba.at(6));
    if (x0 > x1 || y0 > y1) {
        return false;
    }

    _posPx = QPoint(x0, y0);
    _sizePx = QSize(qint32(x1) - x0 + 1, qint32(y1) - y0 + 1);

    return true;
}


/* ******************
 * *** ANNO Chunk ***
 * ****************** */

ANNOChunk::~ANNOChunk()
{

}

ANNOChunk::ANNOChunk()
{

}

bool ANNOChunk::isValid() const
{
    return chunkId() == ANNOChunk::defaultChunkId();
}

QString ANNOChunk::value() const
{
    return dataToString(this);
}

bool ANNOChunk::innerReadStructure(QIODevice *d)
{
    return cacheData(d);
}

/* ******************
 * *** AUTH Chunk ***
 * ****************** */

AUTHChunk::~AUTHChunk()
{

}

AUTHChunk::AUTHChunk()
{

}

bool AUTHChunk::isValid() const
{
    return chunkId() == AUTHChunk::defaultChunkId();
}

QString AUTHChunk::value() const
{
    return dataToString(this);
}

bool AUTHChunk::innerReadStructure(QIODevice *d)
{
    return cacheData(d);
}


/* ******************
 * *** COPY Chunk ***
 * ****************** */

COPYChunk::~COPYChunk()
{

}

COPYChunk::COPYChunk()
{

}

bool COPYChunk::isValid() const
{
    return chunkId() == COPYChunk::defaultChunkId();
}

QString COPYChunk::value() const
{
    return dataToString(this);
}

bool COPYChunk::innerReadStructure(QIODevice *d)
{
    return cacheData(d);
}


/* ******************
 * *** DATE Chunk ***
 * ****************** */

DATEChunk::~DATEChunk()
{

}

DATEChunk::DATEChunk()
{

}

bool DATEChunk::isValid() const
{
    return chunkId() == DATEChunk::defaultChunkId();
}

QDateTime DATEChunk::value() const
{
    if (!isValid()) {
        return {};
    }
    return QDateTime::fromString(QString::fromLatin1(data()), Qt::TextDate);
}

bool DATEChunk::innerReadStructure(QIODevice *d)
{
    return cacheData(d);
}


/* ******************
 * *** EXIF Chunk ***
 * ****************** */

EXIFChunk::~EXIFChunk()
{

}

EXIFChunk::EXIFChunk()
{

}

bool EXIFChunk::isValid() const
{
    if (!data().startsWith(QByteArray("Exif\0\0"))) {
        return false;
    }
    return chunkId() == EXIFChunk::defaultChunkId();
}

MicroExif EXIFChunk::value() const
{
    if (!isValid()) {
        return {};
    }
    return MicroExif::fromByteArray(data().mid(6));
}

bool EXIFChunk::innerReadStructure(QIODevice *d)
{
    return cacheData(d);
}


/* ******************
 * *** ICCN Chunk ***
 * ****************** */

ICCNChunk::~ICCNChunk()
{

}

ICCNChunk::ICCNChunk()
{

}

bool ICCNChunk::isValid() const
{
    return chunkId() == ICCNChunk::defaultChunkId();
}

QString ICCNChunk::value() const
{
    return dataToString(this);
}

bool ICCNChunk::innerReadStructure(QIODevice *d)
{
    return cacheData(d);
}


/* ******************
 * *** ICCP Chunk ***
 * ****************** */

ICCPChunk::~ICCPChunk()
{

}

ICCPChunk::ICCPChunk()
{

}

bool ICCPChunk::isValid() const
{
    return chunkId() == ICCPChunk::defaultChunkId();
}

QColorSpace ICCPChunk::value() const
{
    if (!isValid()) {
        return {};
    }
    return QColorSpace::fromIccProfile(data());
}

bool ICCPChunk::innerReadStructure(QIODevice *d)
{
    return cacheData(d);
}

/* ******************
 * *** FVER Chunk ***
 * ****************** */

FVERChunk::~FVERChunk()
{

}

FVERChunk::FVERChunk()
{

}

bool FVERChunk::isValid() const
{
    return chunkId() == FVERChunk::defaultChunkId();
}

bool FVERChunk::innerReadStructure(QIODevice *d)
{
    return cacheData(d);
}

/* ******************
 * *** HIST Chunk ***
 * ****************** */

HISTChunk::~HISTChunk()
{

}

HISTChunk::HISTChunk()
{

}

bool HISTChunk::isValid() const
{
    return chunkId() == HISTChunk::defaultChunkId();
}

QString HISTChunk::value() const
{
    if (!isValid()) {
        return {};
    }
    return QString::fromLatin1(data());
}

bool HISTChunk::innerReadStructure(QIODevice *d)
{
    return cacheData(d);
}


/* ******************
 * *** NAME Chunk ***
 * ****************** */

NAMEChunk::~NAMEChunk()
{

}

NAMEChunk::NAMEChunk()
{

}

bool NAMEChunk::isValid() const
{
    return chunkId() == NAMEChunk::defaultChunkId();
}

QString NAMEChunk::value() const
{
    return dataToString(this);
}

bool NAMEChunk::innerReadStructure(QIODevice *d)
{
    return cacheData(d);
}


/* ******************
 * *** VERS Chunk ***
 * ****************** */

VERSChunk::~VERSChunk()
{

}

VERSChunk::VERSChunk()
{

}

bool VERSChunk::isValid() const
{
    return chunkId() == VERSChunk::defaultChunkId();
}

QString VERSChunk::value() const
{
    if (!isValid()) {
        return {};
    }
    return QString::fromLatin1(data());
}

bool VERSChunk::innerReadStructure(QIODevice *d)
{
    return cacheData(d);
}


/* ******************
 * *** XMP0 Chunk ***
 * ****************** */

XMP0Chunk::~XMP0Chunk()
{

}

XMP0Chunk::XMP0Chunk()
{

}

bool XMP0Chunk::isValid() const
{
    return chunkId() == XMP0Chunk::defaultChunkId();
}

QString XMP0Chunk::value() const
{
    return dataToString(this);
}

bool XMP0Chunk::innerReadStructure(QIODevice *d)
{
    return cacheData(d);
}


/* ******************
 * *** BEAM Chunk ***
 * ****************** */

BEAMChunk::~BEAMChunk()
{

}

BEAMChunk::BEAMChunk() : IPALChunk()
{

}

bool BEAMChunk::isValid() const
{
    return chunkId() == BEAMChunk::defaultChunkId();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
QList<QRgb> BEAMChunk::palette(qint32 y, qint32 height) const
#else
QVector<QRgb> BEAMChunk::palette(qint32 y, qint32 height) const
#endif
{
    if (height < 1) {
        return {};
    }
    auto bpp = bytes() / height;
    if (bytes() != height * bpp) {
        return {};
    }
    auto col = qint32(bpp / 2);
    auto &&dt = data();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QList<QRgb> pal;
#else
    QVector<QRgb> pal;
#endif
    for (auto c = 0; c < col; ++c) {
        // 2 bytes per color (0x0R 0xGB)
        auto idx = bpp * y + c * 2;
        auto r = quint8(dt[idx] & 0x0F);
        auto g = quint8(dt[idx + 1] & 0xF0);
        auto b = quint8(dt[idx + 1] & 0x0F);
        pal << qRgb(r | (r << 4), (g >> 4) | g, b | (b << 4));
    }
    return pal;
}

bool BEAMChunk::innerReadStructure(QIODevice *d)
{
    return cacheData(d);
}


/* ******************
 * *** CTBL Chunk ***
 * ****************** */

CTBLChunk::~CTBLChunk()
{

}

CTBLChunk::CTBLChunk() : BEAMChunk()
{

}

bool CTBLChunk::isValid() const
{
    return chunkId() == CTBLChunk::defaultChunkId();
}


/* ******************
 * *** SHAM Chunk ***
 * ****************** */

SHAMChunk::~SHAMChunk()
{

}

SHAMChunk::SHAMChunk() : IPALChunk()
{

}

bool SHAMChunk::isValid() const
{
    if (bytes() < 2) {
        return false;
    }
    auto &&dt = data();
    if (dt[0] != 0 && dt[1] != 0) {
        // In all the sham test cases I have them at zero...
        // if they are different from zero I suppose they should
        // be interpreted differently from what was done.
        return false;
    }
    return chunkId() == SHAMChunk::defaultChunkId();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
QList<QRgb> SHAMChunk::palette(qint32 y, qint32 height) const
#else
QVector<QRgb> SHAMChunk::palette(qint32 y, qint32 height) const
#endif
{
    if (height < 1) {
        return {};
    }
    auto bpp = 32; // always 32 bytes per palette (16 colors)
    auto div = 0;
    if (bytes() == quint32(height * bpp + 2)) {
        div = 1;
    } else if (bytes() == quint32(height / 2 * bpp + 2)) {
        div = 2;
    }
    if (div == 0) {
        return {};
    }
    auto &&dt = data();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QList<QRgb> pal;
#else
    QVector<QRgb> pal;
#endif
    for (auto c = 0, col = bpp / 2, idx0 = y / div * bpp + 2; c < col; ++c) {
        // 2 bytes per color (0x0R 0xGB)
        auto idx = idx0 + c * 2;
        auto r = quint8(dt[idx] & 0x0F);
        auto g = quint8(dt[idx + 1] & 0xF0);
        auto b = quint8(dt[idx + 1] & 0x0F);
        pal << qRgb(r | (r << 4), (g >> 4) | g, b | (b << 4));
    }
    return pal;
}

bool SHAMChunk::innerReadStructure(QIODevice *d)
{
    return cacheData(d);
}

/* ******************
 * *** RAST Chunk ***
 * ****************** */

RASTChunk::~RASTChunk()
{

}

RASTChunk::RASTChunk() : IPALChunk()
{

}

bool RASTChunk::isValid() const
{
    return chunkId() == RASTChunk::defaultChunkId();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
QList<QRgb> RASTChunk::palette(qint32 y, qint32 height) const
#else
QVector<QRgb> RASTChunk::palette(qint32 y, qint32 height) const
#endif
{
    if (height < 1) {
        return {};
    }
    auto bpp = bytes() / height;
    if (bytes() != height * bpp) {
        return {};
    }
    auto col = qint32(bpp / 2 - 1);
    auto &&dt = data();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QList<QRgb> pal;
#else
    QVector<QRgb> pal;
#endif
    for (auto c = 0; c < col; ++c) {
        auto idx = bpp * y + 2 + c * 2;
        // The Atari ST uses 3 bits per color (512 colors) while the Atari STE
        // uses 4 bits per color (4096 colors). This strange encoding with the
        // least significant bit set as MSB is, I believe, to ensure hardware
        // compatibility between the two machines.
        #define H1L(a) ((quint8(a) & 0x7) << 1) | ((quint8(a) >> 3) & 1)
        auto r = H1L(dt[idx]);
        auto g = H1L(dt[idx + 1] >> 4);
        auto b = H1L(dt[idx + 1]);
        #undef H1L
        pal << qRgb(r | (r << 4), (g << 4) | g, b | (b << 4));
    }
    return pal;
}

bool RASTChunk::innerReadStructure(QIODevice *d)
{
    return cacheData(d);
}
