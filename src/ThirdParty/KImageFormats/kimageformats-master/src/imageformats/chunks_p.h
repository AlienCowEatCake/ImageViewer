/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2025 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/*
 * Format specifications:
 * - https://wiki.amigaos.net/wiki/IFF_FORM_and_Chunk_Registry
 * - https://www.fileformat.info/format/iff/egff.htm
 * - https://download.autodesk.com/us/maya/2010help/index.html (Developer resources -> File formats -> Maya IFF)
 */

#ifndef KIMG_CHUNKS_P_H
#define KIMG_CHUNKS_P_H

#include <QByteArray>
#include <QDateTime>
#include <QHash>
#include <QImage>
#include <QIODevice>
#include <QLoggingCategory>
#include <QPoint>
#include <QSize>
#include <QSharedPointer>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QVector>
#endif

#include "microexif_p.h"

Q_DECLARE_LOGGING_CATEGORY(LOG_IFFPLUGIN)

// Main chunks (Standard)
#define CAT__CHUNK QByteArray("CAT ")
#define FILL_CHUNK QByteArray("    ")
#define FORM_CHUNK QByteArray("FORM")
#define LIST_CHUNK QByteArray("LIST")
#define PROP_CHUNK QByteArray("PROP")

// Main chuncks (Maya)
#define CAT4_CHUNK QByteArray("CAT4") // 4 byte alignment
#define FOR4_CHUNK QByteArray("FOR4")
#define LIS4_CHUNK QByteArray("LIS4")
#define PRO4_CHUNK QByteArray("PRO4")

#define CAT8_CHUNK QByteArray("CAT8") // 8 byte alignment (never seen)
#define FOR8_CHUNK QByteArray("FOR8")
#define LIS8_CHUNK QByteArray("LIS8")
#define PRO8_CHUNK QByteArray("PRO8")

// FORM ILBM IFF
#define ABIT_CHUNK QByteArray("ABIT")
#define BMHD_CHUNK QByteArray("BMHD")
#define BODY_CHUNK QByteArray("BODY")
#define CAMG_CHUNK QByteArray("CAMG")
#define CMAP_CHUNK QByteArray("CMAP")
#define CMYK_CHUNK QByteArray("CMYK") // https://wiki.amigaos.net/wiki/ILBM_IFF_Interleaved_Bitmap#ILBM.CMYK
#define DPI__CHUNK QByteArray("DPI ")
#define XBMI_CHUNK QByteArray("XBMI")

// Different palette for scanline
#define BEAM_CHUNK QByteArray("BEAM")
#define CTBL_CHUNK QByteArray("CTBL") // same as BEAM
#define PCHG_CHUNK QByteArray("PCHG") // encoded in a unknown way (to be investigated)
#define RAST_CHUNK QByteArray("RAST") // Atari ST(E)
#define SHAM_CHUNK QByteArray("SHAM")

// FOR4 CIMG IFF (Maya)
#define RGBA_CHUNK QByteArray("RGBA")
#define TBHD_CHUNK QByteArray("TBHD")

// FORx IFF (found on some IFF format specs)
#define ANNO_CHUNK QByteArray("ANNO")
#define AUTH_CHUNK QByteArray("AUTH")
#define COPY_CHUNK QByteArray("(c) ")
#define DATE_CHUNK QByteArray("DATE")
#define EXIF_CHUNK QByteArray("EXIF") // https://aminet.net/package/docs/misc/IFF-metadata
#define ICCN_CHUNK QByteArray("ICCN") // https://aminet.net/package/docs/misc/IFF-metadata
#define ICCP_CHUNK QByteArray("ICCP") // https://aminet.net/package/docs/misc/IFF-metadata
#define FVER_CHUNK QByteArray("FVER")
#define HIST_CHUNK QByteArray("HIST")
#define NAME_CHUNK QByteArray("NAME")
#define VERS_CHUNK QByteArray("VERS")
#define XMP0_CHUNK QByteArray("XMP0") // https://aminet.net/package/docs/misc/IFF-metadata

#define ACBM_FORM_TYPE QByteArray("ACBM")
#define ILBM_FORM_TYPE QByteArray("ILBM")
#define PBM__FORM_TYPE QByteArray("PBM ")
#define RGB8_FORM_TYPE QByteArray("RGB8")
#define RGBN_FORM_TYPE QByteArray("RGBN")

#define CIMG_FOR4_TYPE QByteArray("CIMG")
#define TBMP_FOR4_TYPE QByteArray("TBMP")

#define CHUNKID_DEFINE(a) static QByteArray defaultChunkId() { return a; }

// The 8-bit RGB format must be consistent. If you change it here, you have also to use the same
// when converting an image with BEAM/CTBL/SHAM chunks otherwise the option(QImageIOHandler::ImageFormat)
// could returns a wrong value.
// Warning: Changing it requires changing the algorithms. Se, don't touch! :)
#define FORMAT_RGB_8BIT QImage::Format_RGB888 // default one

#define FORMAT_RGBA_8BIT QImage::Format_RGBA8888 // used by PCHG chunk

/*!
 * \brief The IFFChunk class
 */
class IFFChunk
{
    friend class IFFHandlerPrivate;
public:
    using ChunkList = QList<QSharedPointer<IFFChunk>>;

    virtual ~IFFChunk();

    /*!
     * \brief IFFChunk
     * Creates invalid chunk.
     * \sa isValid
     */
    IFFChunk();

    IFFChunk(const IFFChunk& other) = default;
    IFFChunk& operator =(const IFFChunk& other) = default;

    bool operator ==(const IFFChunk& other) const;

    /*!
     * \brief isValid
     * \return True if the chunk is valid, otherwise false.
     * \note The default implementation checks that chunkId() contains only valid characters.
     */
    virtual bool isValid() const;

    /*!
     * \brief alignBytes
     * \return The chunk alignment bytes. By default returns bytes set using setAlignBytes().
     */
    virtual qint32 alignBytes() const;

    /*!
     * \brief chunkId
     * \return The chunk Id of this chunk.
     */
    QByteArray chunkId() const;

    /*!
     * \brief bytes
     * \return The size (in bytes) of the chunck data.
     */

    quint32 bytes() const;

    /*!
     * \brief data
     * \return The data stored inside the class. If no data present, use readRawData().
     * \sa readRawData
     */
    const QByteArray& data() const;

    /*!
     * \brief chunks
     * \return The chunks inside this chunk.
     */
    const ChunkList& chunks() const;

    /*!
     * \brief chunkVersion
     * \param cid Chunk Id to extract the version from.
     * \return The version of the chunk. Zero means no valid chunk data.
     */
    static quint8 chunkVersion(const QByteArray& cid);

    /*!
     * \brief isChunkType
     * Check if the chunkId is of type of cid (any version).
     * \param cid Chunk Id to check.
     * \return True on success, otherwise false.
     */
    bool isChunkType(const QByteArray& cid) const;

    /*!
     * \brief readInfo
     * Reads chunkID, size and set the data position.
     * \param d The device.
     * \return True on success, otherwise false.
     */
    bool readInfo(QIODevice *d);

    /*!
     * \brief readStructure
     * Read the internal structure using innerReadStructure() of the Chunk and set device the position to the next chunks.
     * \param d The device.
     * \return True on success, otherwise false.
     */
    bool readStructure(QIODevice *d);

    /*!
     * \brief readRawData
     * \param d The device.
     * \param relPos The position to read relative to the chunk position.
     * \param size The size of the data to read (-1 means all chunk).
     * \return The data read or empty array on error.
     * \note Ignores any data already read and available with data().
     * \sa data
     */
    QByteArray readRawData(QIODevice *d, qint64 relPos = 0, qint64 size = -1) const;

    /*!
     * \brief seek
     * \param d The device.
     * \param relPos The position to read relative to the chunk position.
     * \return True on success, otherwise false.
     */
    bool seek(QIODevice *d, qint64 relPos = 0) const;

    /*!
     * \brief fromDevice
     * \param d The device.
     * \param ok Set to false if errors occurred.
     * \return The chunk list found.
     */
    static ChunkList fromDevice(QIODevice *d, bool *ok = nullptr);

    /*!
     * \brief search
     * Search for a chunk in the list of chunks.
     * \param cid The chunkId to search.
     * \param chunks The list of chunks to search for the requested chunk.
     * \return The list of chunks with the given chunkId.
     */
    static ChunkList search(const QByteArray &cid, const ChunkList& chunks);

    /*!
     * \brief search
     */
    static ChunkList search(const QByteArray &cid, const QSharedPointer<IFFChunk>& chunk);

    /*!
     * \brief searchT
     * Convenient search function to avoid casts.
     * \param chunk The chunk to search for the requested chunk type.
     * \return The list of chunks of T type.
     */
    template <class T>
    static QList<const T*> searchT(const IFFChunk *chunk) {
        QList<const T*> list;
        if (chunk == nullptr) {
            return list;
        }
        auto cid = T::defaultChunkId();
        if (chunk->chunkId() == cid) {
            if (auto c = dynamic_cast<const T*>(chunk))
                list << c;
        }
        auto tmp = chunk->chunks();
        for (auto &&c : tmp) {
            list << searchT<T>(c.data());
        }
        return list;
    }

    /*!
     * \brief searchT
     * Convenient search function to avoid casts.
     * \param chunks The list of chunks to search for the requested chunk.
     * \return The list of chunks of T type.
     */
    template <class T>
    static QList<const T*> searchT(const ChunkList& chunks) {
        QList<const T*> list;
        for (auto &&chunk : chunks) {
            list << searchT<T>(chunk.data());
        }
        return list;
    }

    CHUNKID_DEFINE(QByteArray())

protected:
    /*!
     * \brief innerReadStructure
     * Reads data structure. Default implementation does nothing.
     * \param d The device.
     * \return True on success, otherwise false.
     */
    virtual bool innerReadStructure(QIODevice *d);

    /*!
     * \brief setAlignBytes
     * \param bytes
     */
    void setAlignBytes(qint32 bytes);

    /*!
     * \brief nextChunkPos
     * Calculates the position of the next chunk. The position is already aligned.
     * \return The position of the next chunk from the beginning of the stream.
     */
    qint64 nextChunkPos() const;

    /*!
     * \brief cacheData
     * Read all chunk data and store it on _data.
     * \return True on success, otherwise false.
     * \warning This function does not load anything if the chunk size is larger than 8MiB. For larger chunks, use direct data access.
     */
    bool cacheData(QIODevice *d);

    /*!
     * \brief setChunks
     * \param chunks
     */
    void setChunks(const ChunkList &chunks);

    /*!
     * \brief recursionCounter
     * Protection against stack overflow due to broken data.
     * \return The current recursion counter.
     */
    qint32 recursionCounter() const;
    void setRecursionCounter(qint32 cnt);

    inline quint16 ui16(quint8 c1, quint8 c2) const {
        return (quint16(c2) << 8) | quint16(c1);
    }
    inline quint16 ui16(const QByteArray &data, qint32 pos) const {
        return ui16(data.at(pos + 1), data.at(pos));
    }

    inline qint16 i16(quint8 c1, quint8 c2) const {
        return qint32(ui16(c1, c2));
    }
    inline qint16 i16(const QByteArray &data, qint32 pos) const {
        return i16(data.at(pos + 1), data.at(pos));
    }

    inline quint32 ui32(quint8 c1, quint8 c2, quint8 c3, quint8 c4) const {
        return (quint32(c4) << 24) | (quint32(c3) << 16) | (quint32(c2) << 8) | quint32(c1);
    }
    inline quint32 ui32(const QByteArray &data, qint32 pos) const {
        return ui32(data.at(pos + 3), data.at(pos + 2), data.at(pos + 1), data.at(pos));
    }

    inline qint32 i32(quint8 c1, quint8 c2, quint8 c3, quint8 c4) const {
        return qint32(ui32(c1, c2, c3, c4));
    }
    inline qint32 i32(const QByteArray &data, qint32 pos) const {
        return i32(data.at(pos + 3), data.at(pos + 2), data.at(pos + 1), data.at(pos));
    }

    static ChunkList innerFromDevice(QIODevice *d, bool *ok, IFFChunk *parent = nullptr);

private:
    char _chunkId[4];

    quint32 _size;

    qint32 _align;

    qint64 _dataPos;

    QByteArray _data;

    ChunkList _chunks;

    qint32 _recursionCnt;
};

/*!
 * \brief The IPALChunk class
 * Interface for additional per-line palette.
 */
class IPALChunk : public IFFChunk
{
public:
    virtual ~IPALChunk() override {}
    IPALChunk() : IFFChunk() {}
    IPALChunk(const IPALChunk& other) = default;
    IPALChunk& operator =(const IPALChunk& other) = default;

    /*!
     * \brief hasAlpha
     * \return True it the palette supports the alpha channel.
     */
    virtual bool hasAlpha() const { return false; }

    /*!
     * \brief clone
     * \return A new instance of the class with all data.
     */
    virtual IPALChunk *clone() const = 0;

    /*!
     * \brief palette
     * \param y The scanline.
     * \return The modified palette.
     */
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    virtual QList<QRgb> palette(qint32 y) const = 0;
#else
    virtual QVector<QRgb> palette(qint32 y) const = 0;
#endif

    /*!
     * \brief initialize
     * Initialize the palette changer.
     * \param cmapPalette The palette as stored in the CMAP chunk.
     * \param height The image height.
     * \return True on success, otherwise false.
     */
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    virtual bool initialize(const QList<QRgb>& cmapPalette, qint32 height) = 0;
#else
    virtual bool initialize(const QVector<QRgb>& cmapPalette, qint32 height) = 0;
#endif
};


/*!
 * \brief The BMHDChunk class
 * Bitmap Header
 */
class BMHDChunk: public IFFChunk
{
public:
    enum Compression {
        Uncompressed = 0, /**< Image data are uncompressed. */
        Rle = 1, /**< Image data are RLE compressed. */
        RgbN8 = 4 /**< RGB8/RGBN compresson. */
    };
    enum Masking {
        None = 0, /**< Designates an opaque rectangular image. */
        HasMask = 1, /**< A "mask" is an optional "plane" of data the same size (w, h) as a bitplane.
                          It tells how to "cut out" part of the image when painting it onto another
                          image.  "One" bits in the mask mean "copy the corresponding pixel to the
                          destination".  "Zero" mask bits mean "leave this destination pixel alone".  In
                          other words, "zero" bits designate transparent pixels.
                          The rows of the different bitplanes and mask are interleaved in the file.
                          This localizes all the information pertinent to each scan line.  It
                          makes it much easier to transform the data while reading it to adjust the
                          image size or depth.  It also makes it possible to scroll a big image by
                          swapping rows directly from the file without the need for random-access to
                          all the bitplanes. */
        HasTransparentColor = 2, /**< Pixels in the source planes matching transparentColor
                                      are to be considered “transparent”. (Actually, transparentColor
                                      isn’t a “color number” since it’s matched with numbers formed
                                      by the source bitmap rather than the possibly deeper destination
                                      bitmap. Note that having a transparent color implies ignoring
                                      one of the color registers. */
        Lasso = 3 /**< The reader may construct a mask by lassoing the image as in MacPaint.
                       To do this, put a 1 pixel border of transparentColor around the image rectangle.
                       Then do a seed fill from this border. Filled pixels are to be transparent. */
    };

    virtual ~BMHDChunk() override;

    BMHDChunk();
    BMHDChunk(const BMHDChunk& other) = default;
    BMHDChunk& operator =(const BMHDChunk& other) = default;

    virtual bool isValid() const override;

    /*!
     * \brief width
     * \return Width of the bitmap in pixels.
     */
    qint32 width() const;

    /*!
     * \brief height
     * \return Height of the bitmap in pixels.
     */
    qint32 height() const;

    /*!
     * \brief size
     * \return Size in pixels.
     */
    QSize size() const;

    /*!
     * \brief left
     * \return The left position of the image.
     */
    qint32 left() const;

    /*!
     * \brief top
     * \return The top position of the image.
     */
    qint32 top() const;

    /*!
     * \brief bitplanes
     * \return The number of bit planes.
     */
    quint8 bitplanes() const;

    /*!
     * \brief masking
     * \return Kind of masking is to be used for this image.
     */
    Masking masking() const;

    /*!
     * \brief compression
     * \return The type of compression used.
     */
    Compression compression() const;

    /*!
     * \brief transparency
     * \return Transparent "color number".
     */
    qint16 transparency() const;

    /*!
     * \brief xAspectRatio
     * \return X pixel aspect.
     */
    quint8 xAspectRatio() const;

    /*!
     * \brief yAspectRatio
     * \return Y pixel aspect.
     */
    quint8 yAspectRatio() const;

    /*!
     * \brief pageWidth
     * \return Source "page" width in pixels.
     */
    quint16 pageWidth() const;

    /*!
     * \brief pageHeight
     * \return Source "page" height in pixels.
     */
    quint16 pageHeight() const;

    /*!
     * \brief rowLen
     * \return The row len of a plane.
     */
    quint32 rowLen() const;

    CHUNKID_DEFINE(BMHD_CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;
};

/*!
 * \brief The CMAPChunk class
 */
class CMAPChunk : public IFFChunk
{
public:
    virtual ~CMAPChunk() override;
    CMAPChunk();
    CMAPChunk(const CMAPChunk& other) = default;
    CMAPChunk& operator =(const CMAPChunk& other) = default;

    virtual bool isValid() const override;

    /*!
     * \brief count
     * \return The number of color in the palette.
     */
    virtual qint32 count() const;

    /*!
     * \brief palette
     * \param halfbride When True, the new palette values are appended using the halfbride method.
     * \return The color palette.
     * \note If \a halfbride is true, the returned palette size is count() * 2.
     */
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QList<QRgb> palette(bool halfbride = false) const;
#else
    QVector<QRgb> palette(bool halfbride = false) const;
#endif

    CHUNKID_DEFINE(CMAP_CHUNK)

protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    virtual QList<QRgb> innerPalette() const;
#else
    virtual QVector<QRgb> innerPalette() const;
#endif

    virtual bool innerReadStructure(QIODevice *d) override;
};

/*!
 * \brief The CMYKChunk class
 *
 * This chunk would allow color specification in terms of Cyan,
 * Magenta, Yellow, and Black as opposed to the current CMAP which uses RGB.
 * The format would be the same as the CMAP chunk with the exception that this
 * chunk uses four color components as opposed to three. The number of colors
 * contained within would be chunk length/4.  This chunk would be used in addition
 * to the CMAP chunk.
 */
class CMYKChunk : public CMAPChunk
{
public:
    virtual ~CMYKChunk() override;
    CMYKChunk();
    CMYKChunk(const CMYKChunk& other) = default;
    CMYKChunk& operator =(const CMYKChunk& other) = default;

    virtual bool isValid() const override;

    /*!
     * \brief count
     * \return The number of color in the palette.
     */
    virtual qint32 count() const override;

    CHUNKID_DEFINE(CMYK_CHUNK)

protected:
    /*!
     * \brief palette
     * \return The CMYK color palette converted to RGB one.
     */
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    virtual QList<QRgb> innerPalette() const override;
#else
    virtual QVector<QRgb> innerPalette() const override;
#endif
};

/*!
 * \brief The CAMGChunk class
 */
class CAMGChunk : public IFFChunk
{
public:
    enum ModeId {
        LoResLace = 0x0004,
        HalfBrite = 0x0080,
        LoResDpf = 0x0400,
        Ham = 0x0800,
        HiRes = 0x8000
    };

    Q_DECLARE_FLAGS(ModeIds, ModeId)

    virtual ~CAMGChunk() override;
    CAMGChunk();
    CAMGChunk(const CAMGChunk& other) = default;
    CAMGChunk& operator =(const CAMGChunk& other) = default;

    virtual bool isValid() const override;

    ModeIds modeId() const;

    CHUNKID_DEFINE(CAMG_CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;
};

/*!
 * \brief The DPIChunk class
 */
class DPIChunk : public IFFChunk
{
public:
    virtual ~DPIChunk() override;
    DPIChunk();
    DPIChunk(const DPIChunk& other) = default;
    DPIChunk& operator =(const DPIChunk& other) = default;

    virtual bool isValid() const override;

    /*!
     * \brief dpiX
     * \return The horizontal resolution in DPI.
     */
    virtual quint16 dpiX() const;

    /*!
     * \brief dpiY
     * \return The vertical resolution in DPI.
     */
    virtual quint16 dpiY() const;

    /*!
     * \brief dotsPerMeterX
     * \return X resolution as wanted by QImage.
     */
    qint32 dotsPerMeterX() const;

    /*!
     * \brief dotsPerMeterY
     * \return Y resolution as wanted by QImage.
     */
    qint32 dotsPerMeterY() const;

    CHUNKID_DEFINE(DPI__CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;
};

/*!
 * \brief The XBMIChunk class
 */
class XBMIChunk : public DPIChunk
{
public:
    enum PictureType : quint16 {
        Indexed = 0,
        Grayscale = 1,
        Rgb = 2,
        RgbA = 3,
        Cmyk = 4,
        CmykA = 5,
        Bitmap = 6
    };

    virtual ~XBMIChunk() override;
    XBMIChunk();
    XBMIChunk(const XBMIChunk& other) = default;
    XBMIChunk& operator =(const XBMIChunk& other) = default;

    virtual bool isValid() const override;

    /*!
     * \brief dpiX
     * \return The horizontal resolution in DPI.
     */
    virtual quint16 dpiX() const override;

    /*!
     * \brief dpiY
     * \return The vertical resolution in DPI.
     */
    virtual quint16 dpiY() const override;

    /*!
     * \brief pictureType
     * \return The picture type
     */
    PictureType pictureType() const;

    CHUNKID_DEFINE(XBMI_CHUNK)
};


/*!
 * \brief The BODYChunk class
 */
class BODYChunk : public IFFChunk
{
public:
    virtual ~BODYChunk() override;
    BODYChunk();
    BODYChunk(const BODYChunk& other) = default;
    BODYChunk& operator =(const BODYChunk& other) = default;

    virtual bool isValid() const override;

    CHUNKID_DEFINE(BODY_CHUNK)

    /*!
     * \brief readStride
     * \param d The device.
     * \param header The bitmap header.
     * \param y The current scanline.
     * \param camg The CAMG chunk (optional)
     * \param cmap The CMAP chunk (optional)
     * \param formType The type of the current form chunk.
     * \return The scanline as requested for QImage.
     * \warning Call resetStrideRead() once before this one.
     */
    virtual QByteArray strideRead(QIODevice *d,
                                  qint32 y,
                                  const BMHDChunk *header,
                                  const CAMGChunk *camg = nullptr,
                                  const CMAPChunk *cmap = nullptr,
                                  const IPALChunk *ipal = nullptr,
                                  const QByteArray& formType = ILBM_FORM_TYPE) const;

    /*!
     * \brief resetStrideRead
     * Reset the stride read set the position at the beginning of the data and reset all buffers.
     * \param d The device.
     * \param header The BMHDChunk chunk (mandatory)
     * \param camg The CAMG chunk (optional)
     * \return True on success, otherwise false.
     * \sa strideRead
     * \note Must be called once before strideRead().
     */
    virtual bool resetStrideRead(QIODevice *d) const;

    /*!
     * \brief safeModeId
     * \param header The header.
     * \param camg The CAMG chunk.
     * \return The most likely ModeId if not explicitly specified.
     */
    static CAMGChunk::ModeIds safeModeId(const BMHDChunk *header, const CAMGChunk *camg, const CMAPChunk *cmap = nullptr);

protected:
    /*!
     * \brief strideSize
     * \param formType The type of the current form chunk.
     * \return The size of data to have to decode an image row.
     */
    quint32 strideSize(const BMHDChunk *header, const QByteArray& formType) const;

    QByteArray deinterleave(const QByteArray &planes, qint32 y, const BMHDChunk *header, const CAMGChunk *camg = nullptr, const CMAPChunk *cmap = nullptr, const IPALChunk *ipal = nullptr) const;

    QByteArray pbm(const QByteArray &planes, qint32 y, const BMHDChunk *header, const CAMGChunk *camg = nullptr, const CMAPChunk *cmap = nullptr, const IPALChunk *ipal = nullptr) const;

    QByteArray rgb8(const QByteArray &planes, qint32 y, const BMHDChunk *header, const CAMGChunk *camg = nullptr, const CMAPChunk *cmap = nullptr, const IPALChunk *ipal = nullptr) const;

    QByteArray rgbN(const QByteArray &planes, qint32 y, const BMHDChunk *header, const CAMGChunk *camg = nullptr, const CMAPChunk *cmap = nullptr, const IPALChunk *ipal = nullptr) const;

private:
    mutable QByteArray _readBuffer;
};


/*!
 * \brief The ABITChunk class
 */
class ABITChunk : public BODYChunk
{
public:
    virtual ~ABITChunk() override;
    ABITChunk();
    ABITChunk(const ABITChunk& other) = default;
    ABITChunk& operator =(const ABITChunk& other) = default;

    virtual bool isValid() const override;

    CHUNKID_DEFINE(ABIT_CHUNK)

    virtual QByteArray strideRead(QIODevice *d,
                                  qint32 y,
                                  const BMHDChunk *header,
                                  const CAMGChunk *camg = nullptr,
                                  const CMAPChunk *cmap = nullptr,
                                  const IPALChunk *ipal = nullptr,
                                  const QByteArray& formType = ACBM_FORM_TYPE) const override;

    virtual bool resetStrideRead(QIODevice *d) const override;
};

/*!
 * \brief The IFOR_Chunk class
 * Interface for FORM chunks.
 */
class IFOR_Chunk : public IFFChunk
{
public:
    virtual ~IFOR_Chunk() override;
    IFOR_Chunk();

    /*!
     * \brief isSupported
     * \return True if the form is supported by the plugin.
     */
    virtual bool isSupported() const = 0;

    /*!
     * \brief formType
     * \return The type of image data contained in the form.
     */
    virtual QByteArray formType() const = 0;

    /*!
     * \brief format
     * \return The Qt image format the form is converted to.
     */
    virtual QImage::Format format() const = 0;

    /*!
     * \brief transformation
     * \return The image transformation.
     * \note The Default implentation returns the trasformation of EXIF chunk (if any).
     */
    virtual QImageIOHandler::Transformation transformation() const;

    /*!
     * \brief size
     * \return The image size in pixels.
     */
    virtual QSize size() const = 0;

    /*!
     * \brief optionformat
     * \return The format retuned by the plugin after all conversions.
     */
    QImage::Format optionformat() const;

    /*!
     * \brief searchIPal
     * Search the palett per line chunk.
     * \return The per line palette (BEAM, CTBL, SHAM, etc....).
     */
    const IPALChunk *searchIPal() const;
};

/*!
 * \brief The FORMChunk class
 */
class FORMChunk : public IFOR_Chunk
{
    QByteArray _type;

public:
    virtual ~FORMChunk() override;
    FORMChunk();
    FORMChunk(const FORMChunk& other) = default;
    FORMChunk& operator =(const FORMChunk& other) = default;

    virtual bool isValid() const override;

    virtual bool isSupported() const override;

    virtual QByteArray formType() const override;

    virtual QImage::Format format() const override;

    virtual QSize size() const override;

    CHUNKID_DEFINE(FORM_CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;
};


/*!
 * \brief The FOR4Chunk class
 */
class FOR4Chunk : public IFOR_Chunk
{
    QByteArray _type;

public:
    virtual ~FOR4Chunk() override;
    FOR4Chunk();
    FOR4Chunk(const FOR4Chunk& other) = default;
    FOR4Chunk& operator =(const FOR4Chunk& other) = default;

    virtual bool isValid() const override;

    virtual qint32 alignBytes() const override;

    virtual bool isSupported() const override;

    virtual QByteArray formType() const override;

    virtual QImage::Format format() const override;

    virtual QSize size() const override;

    CHUNKID_DEFINE(FOR4_CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;
};

/*!
 * \brief The CATChunk class
 */
class CATChunk : public IFFChunk
{
    QByteArray _type;

public:
    virtual ~CATChunk() override;
    CATChunk();
    CATChunk(const CATChunk& other) = default;
    CATChunk& operator =(const CATChunk& other) = default;

    virtual bool isValid() const override;

    QByteArray catType() const;

    CHUNKID_DEFINE(CAT__CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;
};

/*!
 * \brief The TBHDChunk class
 */
class TBHDChunk : public IFFChunk
{
public:
    enum Flag {
        Rgb = 0x01, /**< RGB image */
        Alpha = 0x02, /**< Image contains alpha channel */
        ZBuffer = 0x04, /**< If the image has a z-buffer, it is described by ZBUF blocks with the same structure as the RGBA blocks, RLE encoded. */

        RgbA = Rgb | Alpha /**< RGBA image */
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    enum Compression {
        Uncompressed = 0,
        Rle = 1
    };

    virtual ~TBHDChunk() override;

    TBHDChunk();
    TBHDChunk(const TBHDChunk& other) = default;
    TBHDChunk& operator =(const TBHDChunk& other) = default;

    virtual bool isValid() const override;

    virtual qint32 alignBytes() const override;

    /*!
     * \brief width
     * \return Image width in pixels.
     */
    qint32 width() const;

    /*!
     * \brief height
     * \return Image height in pixels.
     */
    qint32 height() const;

    /*!
     * \brief size
     * \return Image size in pixels.
     */
    QSize size() const;

    /*!
     * \brief left
     * \return
     */
    qint32 left() const;

    /*!
     * \brief top
     * \return
     */
    qint32 top() const;

    /*!
     * \brief flags
     * \return Image flags.
     */
    Flags flags() const;

    /*!
     * \brief bpc
     * \return Byte per channel (1 or 2)
     */
    qint32 bpc() const;

    /*!
     * \brief channels
     * \return
     */
    qint32 channels() const;

    /*!
     * \brief tiles
     * \return The number of tiles of the image.
     */
    quint16 tiles() const;

    /*!
     * \brief compression
     * \return The data compression.
     */
    Compression compression() const;

    /*!
     * \brief format
     * \return
     */
    QImage::Format format() const;

    CHUNKID_DEFINE(TBHD_CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;
};

/*!
 * \brief The RGBAChunk class
 */
class RGBAChunk : public IFFChunk
{
public:
    virtual ~RGBAChunk() override;
    RGBAChunk();
    RGBAChunk(const RGBAChunk& other) = default;
    RGBAChunk& operator =(const RGBAChunk& other) = default;

    virtual bool isValid() const override;

    virtual qint32 alignBytes() const override;

    /*!
     * \brief isTileCompressed
     * \param header The image header.
     * \return True if the tile is compressed, otherwise false.
     */
    bool isTileCompressed(const TBHDChunk *header) const;

    /*!
     * \brief pos
     * \return The tile position (top-left corner) in the final image.
     */
    QPoint pos() const;

    /*!
     * \brief size
     * \return The tile size in pixels.
     */
    QSize size() const;

    /*!
     * \brief tile
     * Create the tile by reading the data from the device.
     * \param d The device.
     * \param header The image header.
     * \return The image tile.
     */
    QImage tile(QIODevice *d, const TBHDChunk *header) const;

    CHUNKID_DEFINE(RGBA_CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;

private:
    QImage compressedTile(QIODevice *d, const TBHDChunk *header) const;

    QImage uncompressedTile(QIODevice *d, const TBHDChunk *header) const;

    QByteArray readStride(QIODevice *d, const TBHDChunk *header) const;

private:
    QPoint _posPx;

    QSize _sizePx;

    mutable QByteArray _readBuffer;
};

/*!
 * \brief The ANNOChunk class
 */
class ANNOChunk : public IFFChunk
{
public:
    virtual ~ANNOChunk() override;
    ANNOChunk();
    ANNOChunk(const ANNOChunk& other) = default;
    ANNOChunk& operator =(const ANNOChunk& other) = default;

    virtual bool isValid() const override;

    QString value() const;

    CHUNKID_DEFINE(ANNO_CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;
};

/*!
 * \brief The AUTHChunk class
 */
class AUTHChunk : public IFFChunk
{
public:
    virtual ~AUTHChunk() override;
    AUTHChunk();
    AUTHChunk(const AUTHChunk& other) = default;
    AUTHChunk& operator =(const AUTHChunk& other) = default;

    virtual bool isValid() const override;

    QString value() const;

    CHUNKID_DEFINE(AUTH_CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;
};

/*!
 * \brief The COPYChunk class
 */
class COPYChunk : public IFFChunk
{
public:
    virtual ~COPYChunk() override;
    COPYChunk();
    COPYChunk(const COPYChunk& other) = default;
    COPYChunk& operator =(const COPYChunk& other) = default;

    virtual bool isValid() const override;

    QString value() const;

    CHUNKID_DEFINE(COPY_CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;
};

/*!
 * \brief The DATEChunk class
 */
class DATEChunk : public IFFChunk
{
public:
    virtual ~DATEChunk() override;
    DATEChunk();
    DATEChunk(const DATEChunk& other) = default;
    DATEChunk& operator =(const DATEChunk& other) = default;

    virtual bool isValid() const override;

    QDateTime value() const;

    CHUNKID_DEFINE(DATE_CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;
};

/*!
 * \brief The EXIFChunk class
 */
class EXIFChunk : public IFFChunk
{
public:
    virtual ~EXIFChunk() override;
    EXIFChunk();
    EXIFChunk(const EXIFChunk& other) = default;
    EXIFChunk& operator =(const EXIFChunk& other) = default;

    virtual bool isValid() const override;

    MicroExif value() const;

    CHUNKID_DEFINE(EXIF_CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;
};


/*!
 * \brief The NAMEChunk class
 */
class ICCNChunk : public IFFChunk
{
public:
    virtual ~ICCNChunk() override;
    ICCNChunk();
    ICCNChunk(const ICCNChunk& other) = default;
    ICCNChunk& operator =(const ICCNChunk& other) = default;

    virtual bool isValid() const override;

    QString value() const;

    CHUNKID_DEFINE(ICCN_CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;
};

/*!
 * \brief The ICCPChunk class
 */
class ICCPChunk : public IFFChunk
{
public:
    virtual ~ICCPChunk() override;
    ICCPChunk();
    ICCPChunk(const ICCPChunk& other) = default;
    ICCPChunk& operator =(const ICCPChunk& other) = default;

    virtual bool isValid() const override;

    QColorSpace value() const;

    CHUNKID_DEFINE(ICCP_CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;
};


/*!
 * \brief The FVERChunk class
 *
 * \warning The specifications on wiki.amigaos.net differ from what I see in a file saved in Maya format. I do not interpret the data for now.
 */
class FVERChunk : public IFFChunk
{
public:
    virtual ~FVERChunk() override;
    FVERChunk();
    FVERChunk(const FVERChunk& other) = default;
    FVERChunk& operator =(const FVERChunk& other) = default;

    virtual bool isValid() const override;

    CHUNKID_DEFINE(FVER_CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;
};


/*!
 * \brief The HISTChunk class
 */
class HISTChunk : public IFFChunk
{
public:
    virtual ~HISTChunk() override;
    HISTChunk();
    HISTChunk(const HISTChunk& other) = default;
    HISTChunk& operator =(const HISTChunk& other) = default;

    virtual bool isValid() const override;

    QString value() const;

    CHUNKID_DEFINE(HIST_CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;
};


/*!
 * \brief The NAMEChunk class
 */
class NAMEChunk : public IFFChunk
{
public:
    virtual ~NAMEChunk() override;
    NAMEChunk();
    NAMEChunk(const NAMEChunk& other) = default;
    NAMEChunk& operator =(const NAMEChunk& other) = default;

    virtual bool isValid() const override;

    QString value() const;

    CHUNKID_DEFINE(NAME_CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;
};

/*!
 * \brief The VERSChunk class
 */
class VERSChunk : public IFFChunk
{
public:
    virtual ~VERSChunk() override;
    VERSChunk();
    VERSChunk(const VERSChunk& other) = default;
    VERSChunk& operator =(const VERSChunk& other) = default;

    virtual bool isValid() const override;

    QString value() const;

    CHUNKID_DEFINE(VERS_CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;
};


/*!
 * \brief The XMP0Chunk class
 */
class XMP0Chunk : public IFFChunk
{
public:
    virtual ~XMP0Chunk() override;
    XMP0Chunk();
    XMP0Chunk(const XMP0Chunk& other) = default;
    XMP0Chunk& operator =(const XMP0Chunk& other) = default;

    virtual bool isValid() const override;

    QString value() const;

    CHUNKID_DEFINE(XMP0_CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;
};


/*!
 * *** UNDOCUMENTED CHUNKS ***
 */

/*!
 * \brief The BEAMChunk class
 */
class BEAMChunk : public IPALChunk
{
public:
    virtual ~BEAMChunk() override;
    BEAMChunk();
    BEAMChunk(const BEAMChunk& other) = default;
    BEAMChunk& operator =(const BEAMChunk& other) = default;

    virtual bool isValid() const override;

    virtual IPALChunk *clone() const override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    virtual QList<QRgb> palette(qint32 y) const override;

    virtual bool initialize(const QList<QRgb>& cmapPalette, qint32 height) override;
#else
    virtual QVector<QRgb> palette(qint32 y) const override;

    virtual bool initialize(const QVector<QRgb>& cmapPalette, qint32 height) override;
#endif

    CHUNKID_DEFINE(BEAM_CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;

private:
    qint32 _height;
};

/*!
 * \brief The CTBLChunk class
 */
class CTBLChunk : public BEAMChunk
{
public:
    virtual ~CTBLChunk() override;
    CTBLChunk();
    CTBLChunk(const CTBLChunk& other) = default;
    CTBLChunk& operator =(const CTBLChunk& other) = default;

    virtual bool isValid() const override;

    CHUNKID_DEFINE(CTBL_CHUNK)
};

/*!
 * \brief The SHAMChunk class
 */
class SHAMChunk : public IPALChunk
{
public:
    virtual ~SHAMChunk() override;
    SHAMChunk();
    SHAMChunk(const SHAMChunk& other) = default;
    SHAMChunk& operator =(const SHAMChunk& other) = default;

    virtual bool isValid() const override;

    virtual IPALChunk *clone() const override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    virtual QList<QRgb> palette(qint32 y) const override;

    virtual bool initialize(const QList<QRgb>& cmapPalette, qint32 height) override;
#else
    virtual QVector<QRgb> palette(qint32 y) const override;

    virtual bool initialize(const QVector<QRgb>& cmapPalette, qint32 height) override;
#endif

    CHUNKID_DEFINE(SHAM_CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;

private:
    qint32 _height;
};

/*!
 * \brief The RASTChunk class
 * \note I found an Atari STE image with the RAST chunk outside
 *       the form chunk (Fish.neo.iff). To support it the IFF parser
 *       should be changed so, this kind of IFFs are shown wrong.
 */
class RASTChunk : public IPALChunk
{
public:
    virtual ~RASTChunk() override;
    RASTChunk();
    RASTChunk(const RASTChunk& other) = default;
    RASTChunk& operator =(const RASTChunk& other) = default;

    virtual bool isValid() const override;

    virtual IPALChunk *clone() const override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    virtual QList<QRgb> palette(qint32 y) const override;

    virtual bool initialize(const QList<QRgb>& cmapPalette, qint32 height) override;
#else
    virtual QVector<QRgb> palette(qint32 y) const override;

    virtual bool initialize(const QVector<QRgb>& cmapPalette, qint32 height) override;
#endif

    CHUNKID_DEFINE(RAST_CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;

private:
    qint32 _height;
};

/*!
 * \brief The PCHGChunk class
 */
class PCHGChunk : public IPALChunk
{
public:
    enum Compression {
        Uncompressed,
        Huffman
    };

    enum Flag {
        None = 0x00,
        F12Bit = 0x01,
        F32Bit = 0x02,
        UseAlpha = 0x04
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    virtual ~PCHGChunk() override;
    PCHGChunk();
    PCHGChunk(const PCHGChunk& other) = default;
    PCHGChunk& operator =(const PCHGChunk& other) = default;

    Compression compression() const;

    Flags flags() const;

    qint16 startLine() const;

    quint16 lineCount() const;

    quint16 changedLines() const;

    quint16 minReg() const;

    quint16 maxReg() const;

    quint16 maxChanges() const;

    quint32 totalChanges() const;

    virtual bool hasAlpha() const override;

    virtual bool isValid() const override;

    virtual IPALChunk *clone() const override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    virtual QList<QRgb> palette(qint32 y) const override;

    virtual bool initialize(const QList<QRgb>& cmapPalette, qint32 height) override;
#else
    virtual QVector<QRgb> palette(qint32 y) const override;

    virtual bool initialize(const QVector<QRgb>& cmapPalette, qint32 height) override;
#endif

    CHUNKID_DEFINE(PCHG_CHUNK)

protected:
    virtual bool innerReadStructure(QIODevice *d) override;

private:
    QHash<qint32, QHash<quint16, QRgb>> _paletteChanges;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QHash<qint32, QList<QRgb>> _palettes;
#else
    QHash<qint32, QVector<QRgb>> _palettes;
#endif
};

#endif // KIMG_CHUNKS_P_H
