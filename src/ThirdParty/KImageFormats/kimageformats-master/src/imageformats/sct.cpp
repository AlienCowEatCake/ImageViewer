/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2024 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "sct_p.h"
#include "scanlineconverter_p.h"
#include "util_p.h"

#include <array>

#include <QtGlobal>
#include <QIODevice>
#include <QBuffer>
#include <QImage>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(LOG_IFFPLUGIN)
Q_LOGGING_CATEGORY(LOG_IFFPLUGIN, "kf.imageformats.plugins.scitex", QtWarningMsg)

#define CTRLBLOCK_SIZE 256
#define PRMSBLOCK_SIZE_CT 256

// For file stored on disk, each block is followed by 768 pads
#define HEADER_SIZE_CT (CTRLBLOCK_SIZE + PRMSBLOCK_SIZE_CT + 768 + 768)

#define FILETYPE_CT "CT"
#define FILETYPE_LW "LW"
#define FILETYPE_BM "BM"
#define FILETYPE_PG "PG"
#define FILETYPE_TX "TX"

class ScitexCtrlBlock
{
    using pchar_t = char *;
public:
    ScitexCtrlBlock() {}
    ScitexCtrlBlock(const ScitexCtrlBlock& other) = default;
    ScitexCtrlBlock& operator =(const ScitexCtrlBlock& other) = default;

    bool load(QIODevice *device)
    {
        auto ok = (device && device->isOpen());
        ok = ok && device->read(pchar_t(_name.data()), _name.size()) == qint64(_name.size());
        ok = ok && device->read(pchar_t(_fileType.data()), _fileType.size()) == qint64(_fileType.size());
        ok = ok && device->read(pchar_t(_blockSize.data()), _blockSize.size()) == qint64(_blockSize.size());
        ok = ok && device->read(pchar_t(_reserved.data()), _reserved.size()) == qint64(_reserved.size());
        ok = ok && device->read(pchar_t(&_count), sizeof(_count)) == qint64(sizeof(_count));
        ok = ok && device->read(pchar_t(_padding.data()), _padding.size()) == qint64(_padding.size());
        return ok;
    }

    QString name() const
    {
        return QString::fromLatin1(pchar_t(_name.data()), _name.size());
    }

    QString fileType() const
    {
        return QString::fromLatin1(pchar_t(_fileType.data()), _fileType.size());
    }

    quint8 sequenceCount() const
    {
        return _count;
    }

    std::array<quint8, 80> _name = {};
    std::array<quint8, 2> _fileType = {};
    std::array<quint8, 12> _blockSize = {};
    std::array<quint8, 12> _reserved = {};
    quint8 _count = 0;
    std::array<quint8, 149> _padding = {};
};

class ScitexParamsBlock
{
    using pchar_t = char *;
public:
    ScitexParamsBlock() {}
    ScitexParamsBlock(const ScitexParamsBlock& other) = default;
    ScitexParamsBlock& operator =(const ScitexParamsBlock& other) = default;

    bool load(QIODevice *device)
    {
        auto ok = (device && device->isOpen());
        ok = ok && device->read(pchar_t(&_unitsOfMeasurement), sizeof(_unitsOfMeasurement)) == qint64(sizeof(_unitsOfMeasurement));
        ok = ok && device->read(pchar_t(&_numOfColorSeparations), sizeof(_numOfColorSeparations)) == qint64(sizeof(_numOfColorSeparations));
        ok = ok && device->read(pchar_t(_separationBitMask.data()), _separationBitMask.size()) == qint64(_separationBitMask.size());
        ok = ok && device->read(pchar_t(_heightInUnits.data()), _heightInUnits.size()) == qint64(_heightInUnits.size());
        ok = ok && device->read(pchar_t(_widthInUnits.data()), _widthInUnits.size()) == qint64(_widthInUnits.size());
        ok = ok && device->read(pchar_t(_heightInPixels.data()), _heightInPixels.size()) == qint64(_heightInPixels.size());
        ok = ok && device->read(pchar_t(_widthInPixels.data()), _widthInPixels.size()) == qint64(_widthInPixels.size());
        ok = ok && device->read(pchar_t(&_scanDirection), sizeof(_scanDirection)) == qint64(sizeof(_scanDirection));
        ok = ok && device->read(pchar_t(_reserved.data()), _reserved.size()) == qint64(_reserved.size());
        return ok;
    }

    quint8 colorCount() const
    {
        return _numOfColorSeparations;
    }

    quint16 bitMask() const
    {
        return ((_separationBitMask.at(0) << 8) | _separationBitMask.at(1));
    }

    quint8 _unitsOfMeasurement = 0;
    quint8 _numOfColorSeparations = 0;
    std::array<quint8, 2> _separationBitMask = {};
    std::array<quint8, 14> _heightInUnits = {};
    std::array<quint8, 14> _widthInUnits = {};
    std::array<quint8, 12> _heightInPixels = {};
    std::array<quint8, 12> _widthInPixels = {};
    quint8 _scanDirection = 0;
    std::array<quint8, 199> _reserved = {};
};

class ScitexHandlerPrivate
{
    using pchar_t = char *;
public:
    ScitexHandlerPrivate()
    {
    }
    ~ScitexHandlerPrivate()
    {
    }

    /*!
     * \brief isSupported
     * \return If the plugin can load it.
     */
    bool isSupported() const
    {
        if (!isValid()) {
            return false;
        }
        // Set a reasonable upper limit
        if (width() > 300000 || height() > 300000) {
            return false;
        }
        return m_cb.fileType() == QStringLiteral(FILETYPE_CT) && format() != QImage::Format_Invalid;
    }

    /*!
     * \brief isValid
     * \return True if is a valid Scitex image file.
     */
    bool isValid() const
    {
        if (width() == 0 || height() == 0) {
            return false;
        }
        QStringList ft = {
            QStringLiteral(FILETYPE_CT),
            QStringLiteral(FILETYPE_LW),
            QStringLiteral(FILETYPE_BM),
            QStringLiteral(FILETYPE_PG),
            QStringLiteral(FILETYPE_TX)
        };
        return ft.contains(m_cb.fileType());
    }

    QImage::Format format() const
    {
        auto format = QImage::Format_Invalid;
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
        if (m_pb.colorCount() == 4) {
            if (m_pb.bitMask() == 15)
                format = QImage::Format_CMYK8888;
        }
#endif
        if (m_pb.colorCount() == 3) {
            if (m_pb.bitMask() == 7)
                format = QImage::Format_RGB888;
        }
        if (m_pb.colorCount() == 1) {
            if (m_pb.bitMask() == 8)
                format = QImage::Format_Grayscale8;
        }
        return format;
    }

    quint32 width() const
    {
        auto ok = false;
        auto&& px = m_pb._widthInPixels;
        auto v = QString::fromLatin1(pchar_t(px.data()), px.size()).toUInt(&ok);
        return ok ? v : 0;
    }

    quint32 height() const
    {
        auto ok = false;
        auto&& px = m_pb._heightInPixels;
        auto v = QString::fromLatin1(pchar_t(px.data()), px.size()).toUInt(&ok);
        return ok ? v : 0;
    }

    qint32 dotsPerMeterX() const {
        auto ok = false;
        auto&& res = m_pb._widthInUnits;
        auto v = QString::fromLatin1(pchar_t(res.data()), res.size()).toDouble(&ok);
        if (ok && v > 0) {
            if (m_pb._unitsOfMeasurement) { // Inches
                return qRound(width() / v / 25.4 * 1000);
            }
            // Millimeters
            return qRound(width() / v * 1000);
        }
        return 0;
    }

    qint32 dotsPerMeterY() const {
        auto ok = false;
        auto&& res = m_pb._heightInUnits;
        auto v = QString::fromLatin1(pchar_t(res.data()), res.size()).toDouble(&ok);
        if (ok && v > 0) {
            if (m_pb._unitsOfMeasurement) { // Inches
                return qRound(width() / v / 25.4 * 1000);
            }
            // Millimeters
            return qRound(width() / v * 1000);
        }
        return 0;
    }

    QImageIOHandler::Transformation transformation() const
    {
        auto t = QImageIOHandler::TransformationNone;
        switch (m_pb._scanDirection) {
        case 1:
            t = QImageIOHandler::TransformationFlip;
            break;
        case 2:
            t = QImageIOHandler::TransformationMirror;
            break;
        case 3:
            t = QImageIOHandler::TransformationRotate180;
            break;
        case 4:
            t = QImageIOHandler::TransformationFlipAndRotate90;
            break;
        case 5:
            t = QImageIOHandler::TransformationRotate270;
            break;
        case 6:
            t = QImageIOHandler::TransformationRotate90;
            break;
        case 7:
            t = QImageIOHandler::TransformationMirrorAndRotate90;
            break;
        default:
            t = QImageIOHandler::TransformationNone;
            break;
        }
        return t;
    }

    bool peekHeader(QIODevice *device)
    {
        if (device == nullptr) {
            return false;
        }
        auto ba = device->peek(HEADER_SIZE_CT);
        if (ba.size() != HEADER_SIZE_CT) {
            return false;
        }
        QBuffer b;
        b.setData(ba);
        if (!b.open(QIODevice::ReadOnly)) {
            return false;
        }
        return loadHeader(&b);
    }

    bool loadHeader(QIODevice *device) {
        if (device == nullptr) {
            return false;
        }
        if (!m_cb.load(device)) {
            return false;
        }
        auto pad1 = device->read(768);
        if (pad1.size() != 768) {
            return false;
        }
        if (!m_pb.load(device)) {
            return false;
        }
        auto pad2 = device->read(768);
        if (pad2.size() != 768) {
            return false;
        }
        return true;
    }

    ScitexCtrlBlock m_cb;
    ScitexParamsBlock m_pb;
};

ScitexHandler::ScitexHandler()
    : QImageIOHandler()
    , d(new ScitexHandlerPrivate)
{
}

bool ScitexHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("sct");
        return true;
    }
    return false;
}

bool ScitexHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("ScitexHandler::canRead() called with no device");
        return false;
    }
    ScitexHandlerPrivate hp;
    if (hp.peekHeader(device)) {
        return hp.isSupported();
    }
    return false;
}

bool ScitexHandler::read(QImage *image)
{
    auto dev = device();
    if (dev == nullptr) {
        qWarning("ScitexHandler::read() called with no device");
        return false;
    }
    if (!d->loadHeader(dev)) {
        return false;
    }
    if (!d->isSupported()) {
        return false;
    }

    auto img = imageAlloc(d->width(), d->height(), d->format());
    if (img.isNull()) {
        return false;
    }

    auto hres = d->dotsPerMeterX();
    if (hres > 0) {
        img.setDotsPerMeterX(hres);
    }
    auto vres = d->dotsPerMeterY();
    if (vres > 0) {
        img.setDotsPerMeterY(vres);
    }

    QByteArray line(img.width() * d->m_pb.colorCount(), char());
    if (img.bytesPerLine() < line.size()) {
        return false;
    }
    for (qint32 y = 0, h = img.height(); y < h; ++y) {
        if (dev->read(line.data(), line.size()) != line.size()) {
            return false;
        }
        auto scanLine = img.scanLine(y);
        for (qint32 c = 0, cc = d->m_pb.colorCount(); c < cc; ++c) {
            for (qint32 x = 0, w = img.width(); x < w; ++x) {
                scanLine[x * cc + c] = cc == 4 ? uchar(255) - uchar(line.at(c * w + x)) : uchar(line.at(c * w + x));
            }
        }
    }

    *image = img;
    return true;
}

bool ScitexHandler::supportsOption(ImageOption option) const
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

QVariant ScitexHandler::option(ImageOption option) const
{
    QVariant v;

    if (option == QImageIOHandler::Size) {
        if (!d->isValid()) {
            d->peekHeader(device());
        }
        if (d->isSupported()) {
            v = QSize(d->width(), d->height());
        }
    }

    if (option == QImageIOHandler::ImageFormat) {
        if (!d->isValid()) {
            d->peekHeader(device());
        }
        if (d->isSupported()) {
            v = d->format();
        }
    }

    if (option == QImageIOHandler::ImageTransformation) {
        if (!d->isValid()) {
            d->peekHeader(device());
        }
        if (d->isSupported()) {
            v = int(d->transformation());
        }
    }

    return v;
}

QImageIOPlugin::Capabilities ScitexPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "sct") {
        return Capabilities(CanRead);
    }
    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && ScitexHandler::canRead(device)) {
        cap |= CanRead;
    }
    return cap;
}

QImageIOHandler *ScitexPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new ScitexHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

