/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2025 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "chunks_p.h"
#include "iff_p.h"
#include "util_p.h"

#include <QIODevice>
#include <QImage>
#include <QPainter>

class IFFHandlerPrivate
{
public:
    IFFHandlerPrivate()
        : m_imageNumber(0)
        , m_imageCount(0)
    {

    }
    ~IFFHandlerPrivate()
    {

    }

    /*!
     * \brief atariSTERast
     * On Atari STE images, the RAST chunk can be found outside
     * the FORM one so, I check if this is the case.
     * \param chunks The chunk list.
     */
    void atariSTERast(QIODevice *d, IFFChunk::ChunkList &chunks)
    {
        if (chunks.size() != 1 || d->isSequential()) {
            return;
        }
        auto &&c = chunks.first();
        if (c->chunkId() != FORMChunk::defaultChunkId()) {
            return;
        }

        // The RAST chunk is not aligned so I have to temporary change the
        // position and the alignment to read it successfully.
        auto pos = d->pos();
        auto align = c->alignBytes();
        c->setAlignBytes(1);
        d->seek(c->nextChunkPos());
        c->setAlignBytes(align);
        if (d->peek(4) == RAST_CHUNK) {
            auto rast = QSharedPointer<IFFChunk>(new RASTChunk());
            if (rast->readStructure(d) && rast->isValid())
                chunks.first()->_chunks.append(rast);
        }
        d->seek(pos);
    }

    bool readStructure(QIODevice *d)
    {
        if (d == nullptr) {
            return {};
        }

        if (!m_chunks.isEmpty()) {
            return true;
        }

        auto ok = false;
        auto chunks = IFFChunk::fromDevice(d, &ok);
        if (ok) {
            atariSTERast(d, chunks);
            m_chunks = chunks;
        }
        return ok;
    }

    template <class T>
    static QList<const T*> searchForms(const IFFChunk::ChunkList &chunks, bool supportedOnly = true)
    {
        QList<const T*> list;
        auto cid = T::defaultChunkId();
        auto forms = IFFChunk::search(cid, chunks);
        for (auto &&form : forms) {
            if (auto f = dynamic_cast<const T*>(form.data()))
                if (!supportedOnly || f->isSupported())
                    list << f;
        }
        return list;
    }

    template <class T>
    QList<const T*> searchForms(bool supportedOnly = true)
    {
        return searchForms<T>(m_chunks, supportedOnly);
    }

    IFFChunk::ChunkList m_chunks;

    /*!
     * \brief m_imageNumber
     * Value set by QImageReader::jumpToImage() or QImageReader::jumpToNextImage().
     * The number of view selected in a multiview image.
     */
    qint32 m_imageNumber;

    /*!
     * \brief m_imageCount
     * The total number of views (cache value)
     */
    mutable qint32 m_imageCount;
};


IFFHandler::IFFHandler()
    : QImageIOHandler()
    , d(new IFFHandlerPrivate)
{

}

bool IFFHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("iff");
        return true;
    }
    return false;
}

bool IFFHandler::canRead(QIODevice *device)
{
    if (!device) {
        qCWarning(LOG_IFFPLUGIN) << "IFFHandler::canRead(): called with no device";
        return false;
    }

    if (device->isSequential()) {
        return false;
    }

    // I avoid parsing obviously incorrect files
    auto cid = device->peek(4);
    if (cid != CAT__CHUNK &&
        cid != FORM_CHUNK &&
        cid != LIST_CHUNK &&
        cid != CAT4_CHUNK &&
        cid != FOR4_CHUNK &&
        cid != LIS4_CHUNK) {
        return false;
    }

    auto ok = false;
    auto pos = device->pos();
    auto chunks = IFFChunk::fromDevice(device, &ok);
    if (!device->seek(pos)) {
        qCWarning(LOG_IFFPLUGIN) << "IFFHandler::canRead(): unable to reset device position";
    }
    if (ok) {
        auto forms = IFFHandlerPrivate::searchForms<FORMChunk>(chunks, true);
        auto for4s = IFFHandlerPrivate::searchForms<FOR4Chunk>(chunks, true);
        ok = !forms.isEmpty() || !for4s.isEmpty();
    }
    return ok;
}

static void addMetadata(QImage &img, const IFOR_Chunk *form)
{
    // standard IFF metadata
    auto annos = IFFChunk::searchT<ANNOChunk>(form);
    if (!annos.isEmpty()) {
        auto anno = annos.first()->value();
        if (!anno.isEmpty()) {
            img.setText(QStringLiteral(META_KEY_DESCRIPTION), anno);
        }
    }
    auto auths = IFFChunk::searchT<AUTHChunk>(form);
    if (!auths.isEmpty()) {
        auto auth = auths.first()->value();
        if (!auth.isEmpty()) {
            img.setText(QStringLiteral(META_KEY_AUTHOR), auth);
        }
    }
    auto dates = IFFChunk::searchT<DATEChunk>(form);
    if (!dates.isEmpty()) {
        auto dt = dates.first()->value();
        if (dt.isValid()) {
            img.setText(QStringLiteral(META_KEY_CREATIONDATE), dt.toString(Qt::ISODate));
        }
    }
    auto copys = IFFChunk::searchT<COPYChunk>(form);
    if (!copys.isEmpty()) {
        auto cp = copys.first()->value();
        if (!cp.isEmpty()) {
            img.setText(QStringLiteral(META_KEY_COPYRIGHT), cp);
        }
    }
    auto names = IFFChunk::searchT<NAMEChunk>(form);
    if (!names.isEmpty()) {
        auto name = names.first()->value();
        if (!name.isEmpty()) {
            img.setText(QStringLiteral(META_KEY_TITLE), name);
        }
    }

    // software info
    auto vers = IFFChunk::searchT<VERSChunk>(form);
    if (!vers.isEmpty()) {
        auto ver = vers.first()->value();
        if (!vers.isEmpty()) {
            img.setText(QStringLiteral(META_KEY_SOFTWARE), ver);
        }
    }

    // SView5 metadata
    auto resChanged = false;
    auto exifs = IFFChunk::searchT<EXIFChunk>(form);
    if (!exifs.isEmpty()) {
        auto exif = exifs.first()->value();
        exif.updateImageMetadata(img, false);
        resChanged = exif.updateImageResolution(img);
    }

    auto xmp0s = IFFChunk::searchT<XMP0Chunk>(form);
    if (!xmp0s.isEmpty()) {
        auto xmp = xmp0s.first()->value();
        if (!xmp.isEmpty()) {
            img.setText(QStringLiteral(META_KEY_XMP_ADOBE), xmp);
        }
    }

    auto iccps = IFFChunk::searchT<ICCPChunk>(form);
    if (!iccps.isEmpty()) {
        auto cs = iccps.first()->value();
        if (cs.isValid()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
            auto iccns = IFFChunk::searchT<ICCNChunk>(form);
            if (!iccns.isEmpty()) {
                auto desc = iccns.first()->value();
                if (!desc.isEmpty())
                    cs.setDescription(desc);
            }
#endif
            img.setColorSpace(cs);
        }
    }

    // resolution -> leave after set of EXIF chunk
    const DPIChunk *dpi = nullptr;
    auto dpis = IFFChunk::searchT<DPIChunk>(form);
    auto xbmis = IFFChunk::searchT<XBMIChunk>(form);
    if (!dpis.isEmpty()) {
        dpi = dpis.first();
    } else if (!xbmis.isEmpty()) {
        dpi = xbmis.first(); // never seen
    }
    if (dpi && dpi->isValid()) {
        img.setDotsPerMeterX(dpi->dotsPerMeterX());
        img.setDotsPerMeterY(dpi->dotsPerMeterY());
        resChanged = true;
    }

    // if no explicit resolution was found, apply the aspect ratio to the default one
    if (!resChanged) {
        auto headers = IFFChunk::searchT<BMHDChunk>(form);
        if (!headers.isEmpty()) {
            auto xr = headers.first()->xAspectRatio();
            auto yr = headers.first()->yAspectRatio();
            if (xr > 0 && yr > 0 && xr > yr) {
                img.setDotsPerMeterX(img.dotsPerMeterX() * yr / xr);
            } else if (xr > 0 && yr > 0 && xr < yr) {
                img.setDotsPerMeterY(img.dotsPerMeterY() * xr / yr);
            }
        }
    }
}

/*!
 * \brief convertIPAL
 * \param img The source image.
 * \param ipal The per line palette.
 * \return The new image converted or \a img if no conversion is needed or possible.
 */
static QImage convertIPAL(const QImage& img, const IPALChunk *ipal)
{
    if (img.format() != QImage::Format_Indexed8) {
        qCDebug(LOG_IFFPLUGIN) << "convertIPAL(): the image is not indexed!";
        return img;
    }

    auto tmp = img.convertToFormat(ipal->hasAlpha() ? FORMAT_RGBA_8BIT : FORMAT_RGB_8BIT);
    if (tmp.isNull()) {
        qCCritical(LOG_IFFPLUGIN) << "convertIPAL(): error while converting the image!";
        return img;
    }

    auto mul = tmp.hasAlphaChannel() ? 4 : 3;
    for (auto y = 0, h = img.height(); y < h; ++y) {
        auto src = reinterpret_cast<const quint8 *>(img.constScanLine(y));
        auto dst = tmp.scanLine(y);
        auto lpal = ipal->palette(y);
        for (auto x = 0, w = img.width(); x < w; ++x) {
            if (src[x] < lpal.size()) {
                auto xmul = x * mul;
                dst[xmul] = qRed(lpal.at(src[x]));
                dst[xmul + 1] = qGreen(lpal.at(src[x]));
                dst[xmul + 2] = qBlue(lpal.at(src[x]));
                if (mul == 4) {
                    dst[xmul + 3] = qAlpha(lpal.at(src[x]));
                }
            }
        }
    }

    return tmp;
}

bool IFFHandler::readStandardImage(QImage *image)
{
    auto forms = d->searchForms<FORMChunk>();
    if (forms.isEmpty()) {
        return false;
    }
    auto cin = qBound(0, currentImageNumber(), int(forms.size() - 1));
    auto &&form = forms.at(cin);

    // show the first one (I don't have a sample with many images)
    auto headers = IFFChunk::searchT<BMHDChunk>(form);
    if (headers.isEmpty()) {
        qCWarning(LOG_IFFPLUGIN) << "IFFHandler::readStandardImage(): no supported image found";
        return false;
    }

    // create the image
    auto &&header = headers.first();
    auto img = imageAlloc(header->size(), form->format());
    if (img.isNull()) {
        qCWarning(LOG_IFFPLUGIN) << "IFFHandler::readStandardImage(): error while allocating the image";
        return false;
    }

    // set color table
    const CAMGChunk *camg = nullptr;
    auto camgs = IFFChunk::searchT<CAMGChunk>(form);
    if (!camgs.isEmpty()) {
        camg = camgs.first();
    }

    const CMAPChunk *cmap = nullptr;
    auto cmaps = IFFChunk::searchT<CMAPChunk>(form);
    if (cmaps.isEmpty()) {
        auto cmyks = IFFChunk::searchT<CMYKChunk>(form);
        for (auto &&cmyk : cmyks)
            cmaps.append(cmyk);
    }
    if (!cmaps.isEmpty()) {
        cmap = cmaps.first();
    }
    if (img.format() == QImage::Format_Indexed8) {
        if (cmap) {
            auto halfbride = BODYChunk::safeModeId(header, camg, cmap) & CAMGChunk::ModeId::HalfBrite ? true : false;
            img.setColorTable(cmap->palette(halfbride));
        }
    }

    // reading image data
    std::unique_ptr<IPALChunk> ipal;
    if (auto ptr = form->searchIPal()) {
        ipal = std::unique_ptr<IPALChunk>(ptr->clone());
    }
    if (ipal) {
        auto pal = img.colorTable();
        if (pal.isEmpty() && cmap)
            pal = cmap->palette();
        if (!ipal->initialize(pal, img.height())) {
            qCWarning(LOG_IFFPLUGIN) << "IFFHandler::readStandardImage(): unable to initialize palette changer";
            return false;
        }
    }
    auto bodies = IFFChunk::searchT<BODYChunk>(form);
    if (bodies.isEmpty()) {
        auto abits = IFFChunk::searchT<ABITChunk>(form);
        for (auto &&abit : abits)
            bodies.append(abit);
    }
    if (bodies.isEmpty()) {
        img.fill(0);
    } else {
        auto &&body = bodies.first();
        if (!body->resetStrideRead(device())) {
            qCWarning(LOG_IFFPLUGIN) << "IFFHandler::readStandardImage(): error while reading image data";
            return false;
        }
        for (auto y = 0, h = img.height(); y < h; ++y) {
            auto line = reinterpret_cast<char*>(img.scanLine(y));
            auto ba = body->strideRead(device(), y, header, camg, cmap, ipal.get(), form->formType());
            if (ba.isEmpty()) {
                qCWarning(LOG_IFFPLUGIN) << "IFFHandler::readStandardImage(): error while reading image scanline";
                return false;
            }
            memcpy(line, ba.constData(), std::min(img.bytesPerLine(), ba.size()));
        }
    }

    // BEAM / CTBL, SHAM, RAST, PCHG conversion (if not already done)
    if (ipal && img.format() == QImage::Format_Indexed8) {
        img = convertIPAL(img, ipal.get());
    }

    // set metadata (including image resolution)
    addMetadata(img, form);

    *image = img;
    return true;
}

bool IFFHandler::readMayaImage(QImage *image)
{
    auto forms = d->searchForms<FOR4Chunk>();
    if (forms.isEmpty()) {
        return false;
    }
    auto cin = qBound(0, currentImageNumber(), int(forms.size() - 1));
    auto &&form = forms.at(cin);

    // show the first one (I don't have a sample with many images)
    auto headers = IFFChunk::searchT<TBHDChunk>(form);
    if (headers.isEmpty()) {
        qCWarning(LOG_IFFPLUGIN) << "IFFHandler::readMayaImage(): no supported image found";
        return false;
    }

    // create the image
    auto &&header = headers.first();
    auto img = imageAlloc(header->size(), form->format());
    if (img.isNull()) {
        qCWarning(LOG_IFFPLUGIN) << "IFFHandler::readMayaImage(): error while allocating the image";
        return false;
    }

    auto &&tiles = IFFChunk::searchT<RGBAChunk>(form);
    if ((tiles.size() & 0xFFFF) != header->tiles()) { // Photoshop, on large images saves more than 65535 tiles
        qCWarning(LOG_IFFPLUGIN) << "IFFHandler::readMayaImage(): tile number mismatch: found" << tiles.size() << "while expected" << header->tiles();
        return false;
    }
    for (auto &&tile : tiles) {
        auto tp = tile->pos();
        auto ts = tile->size();
        if (tp.x() < 0 || tp.x() + ts.width() > img.width()) {
            qCWarning(LOG_IFFPLUGIN) << "IFFHandler::readMayaImage(): wrong tile position or size";
            return false;
        }
        if (tp.y() < 0 || tp.y() + ts.height() > img.height()) {
            qCWarning(LOG_IFFPLUGIN) << "IFFHandler::readMayaImage(): wrong tile position or size";
            return false;
        }
        // For future releases: it might be a good idea not to use a QPainter
        auto ti = tile->tile(device(), header);
        if (ti.isNull()) {
            qCWarning(LOG_IFFPLUGIN) << "IFFHandler::readMayaImage(): error while decoding the tile";
            return false;
        }
        QPainter painter(&img);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.drawImage(tp, ti);
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    img.mirror(false, true);
#else
    img = img.mirrored(false, true);
#endif
#else
    img.flip(Qt::Orientation::Vertical);
#endif
    addMetadata(img, form);

    *image = img;
    return true;
}

bool IFFHandler::read(QImage *image)
{
    if (!d->readStructure(device())) {
        qCWarning(LOG_IFFPLUGIN) << "IFFHandler::read(): invalid IFF structure";
        return false;
    }

    if (readStandardImage(image)) {
        return true;
    }

    if (readMayaImage(image)) {
        return true;
    }

    qCWarning(LOG_IFFPLUGIN) << "IFFHandler::read(): no supported image found";
    return false;
}

bool IFFHandler::supportsOption(ImageOption option) const
{
    if (option == QImageIOHandler::Size) {
        return true;
    }
    if (option == QImageIOHandler::ImageFormat) {
        return true;
    }
    if (option == QImageIOHandler::ImageTransformation) {
        return true;
    }
    return false;
}

QVariant IFFHandler::option(ImageOption option) const
{
    if (!supportsOption(option)) {
        return {};
    }

    const IFOR_Chunk *form = nullptr;
    if (d->readStructure(device())) {
        auto forms = d->searchForms<FORMChunk>();
        auto for4s = d->searchForms<FOR4Chunk>();
        auto cin = currentImageNumber();
        if (!forms.isEmpty())
            form = cin < forms.size() ? forms.at(cin) : forms.first();
        else if (!for4s.isEmpty())
            form = cin < for4s.size() ? for4s.at(cin) : for4s.first();
    }
    if (form == nullptr) {
        return {};
    }

    if (option == QImageIOHandler::Size) {
        return QVariant::fromValue(form->size());
    }

    if (option == QImageIOHandler::ImageFormat) {
        return QVariant::fromValue(form->optionformat());
    }

    if (option == QImageIOHandler::ImageTransformation) {
        return QVariant::fromValue(int(form->transformation()));
    }

    return {};
}

bool IFFHandler::jumpToNextImage()
{
    return jumpToImage(d->m_imageNumber + 1);
}

bool IFFHandler::jumpToImage(int imageNumber)
{
    if (imageNumber < 0 || imageNumber >= imageCount()) {
        return false;
    }
    d->m_imageNumber = imageNumber;
    return true;
}

int IFFHandler::imageCount() const
{
    // NOTE: image count is cached for performance reason
    auto &&count = d->m_imageCount;
    if (count > 0) {
        return count;
    }

    count = QImageIOHandler::imageCount();
    if (!d->readStructure(device())) {
        qCWarning(LOG_IFFPLUGIN) << "IFFHandler::imageCount(): invalid IFF structure";
        return count;
    }

    auto forms = d->searchForms<FORMChunk>();
    auto for4s = d->searchForms<FOR4Chunk>();
    if (!forms.isEmpty())
        count = forms.size();
    else if (!for4s.isEmpty())
        count = for4s.size();

    return count;
}

int IFFHandler::currentImageNumber() const
{
    return d->m_imageNumber;
}

QImageIOPlugin::Capabilities IFFPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "iff" || format == "ilbm" || format == "lbm") {
        return Capabilities(CanRead);
    }
    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && IFFHandler::canRead(device)) {
        cap |= CanRead;
    }
    return cap;
}

QImageIOHandler *IFFPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new IFFHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#include "moc_iff_p.cpp"
