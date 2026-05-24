/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2026 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// Specs: https://tools.suckless.org/farbfeld/

#include "ff_p.h"
#include "util_p.h"

#include <QColorSpace>
#include <QIODevice>
#include <QImage>
#include <QLoggingCategory>
#include <QtEndian>

Q_DECLARE_LOGGING_CATEGORY(LOG_FFPLUGIN)

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(LOG_FFPLUGIN, "kf.imageformats.plugins.ff", QtDebugMsg)
#else
Q_LOGGING_CATEGORY(LOG_FFPLUGIN, "kf.imageformats.plugins.ff", QtWarningMsg)
#endif

/* *** FF_MAX_IMAGE_WIDTH and FF_MAX_IMAGE_HEIGHT ***
 * The maximum size in pixel allowed by the plugin.
 */
#ifndef FF_MAX_IMAGE_WIDTH
#define FF_MAX_IMAGE_WIDTH KIF_LARGE_IMAGE_PIXEL_LIMIT
#endif
#ifndef FF_MAX_IMAGE_HEIGHT
#define FF_MAX_IMAGE_HEIGHT FF_MAX_IMAGE_WIDTH
#endif

#define HEADER_SIZE 16

class FFHeader
{
private:
    QByteArray m_rawHeader;

public:
    FFHeader()
    {

    }

    bool isValid() const
    {
        if (m_rawHeader.size() < HEADER_SIZE) {
            return false;
        }
        return m_rawHeader.startsWith(QByteArray::fromRawData("farbfeld", 8));
    }

    bool isSupported() const
    {
        auto w = width();
        auto h = height();
        if (w < 1 || w > FF_MAX_IMAGE_WIDTH || h < 1 || h > FF_MAX_IMAGE_HEIGHT) {
            return false;
        }
        return format() != QImage::Format_Invalid;
    }

    qint32 width() const
    {
        if (!isValid()) {
            return 0;
        }
        return qFromBigEndian<qint32>(m_rawHeader.data() + 8);
    }

    qint32 height() const
    {
        if (!isValid()) {
            return 0;
        }
        return qFromBigEndian<qint32>(m_rawHeader.data() + 12);
    }

    QSize size() const
    {
        return QSize(width(), height());
    }

    QImage::Format format() const
    {
        if (!isValid()) {
            return QImage::Format_Invalid;
        }
        return QImage::Format_RGBA64;
    }

    bool read(QIODevice *d)
    {
        m_rawHeader = d->read(HEADER_SIZE);
        if (m_rawHeader.size() != HEADER_SIZE) {
            return false;
        }
        return isValid();
    }

    bool peek(QIODevice *d)
    {
        m_rawHeader = d->peek(HEADER_SIZE);
        if (m_rawHeader.size() != HEADER_SIZE) {
            return false;
        }
        return isValid();
    }
};

class FFHandlerPrivate
{
public:
    FFHandlerPrivate() {}
    ~FFHandlerPrivate() {}

    FFHeader m_header;
};

FFHandler::FFHandler()
    : QImageIOHandler()
    , d(new FFHandlerPrivate)
{
}

bool FFHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("ff");
        return true;
    }
    return false;
}

bool FFHandler::canRead(QIODevice *device)
{
    if (!device) {
        qCWarning(LOG_FFPLUGIN) << "FFHandler::canRead() called with no device";
        return false;
    }

    FFHeader h;
    if (!h.peek(device)) {
        return false;
    }

    return h.isSupported();
}

bool FFHandler::read(QImage *image)
{
    auto&& header = d->m_header;

    if (!header.read(device())) {
        qCWarning(LOG_FFPLUGIN) << "FFHandler::read() invalid header";
        return false;
    }

    auto img = imageAlloc(header.size(), header.format());
    if (img.isNull()) {
        qCWarning(LOG_FFPLUGIN) << "FFHandler::read() error while allocating the image";
        return false;
    }

    auto d = device();

    auto size = img.bytesPerLine();
    for (auto y = 0, h = img.height(); y < h; ++y) {
        auto line = reinterpret_cast<char*>(img.scanLine(y));
        if (d->read(line, size) != size) {
            qCWarning(LOG_FFPLUGIN) << "FFHandler::read() error while reading image scanline";
            return false;
        }
#if Q_LITTLE_ENDIAN
        for (auto i = 0; i < size; i += 2) {
            std::swap(line[i], line[i + 1]);
        }
#endif
    }

    img.setColorSpace(QColorSpace(QColorSpace::SRgb));

    *image = img;
    return true;
}

bool FFHandler::supportsOption(ImageOption option) const
{
    if (option == QImageIOHandler::Size) {
        return true;
    }
    if (option == QImageIOHandler::ImageFormat) {
        return true;
    }
    return false;
}

QVariant FFHandler::option(ImageOption option) const
{
    QVariant v;

    if (option == QImageIOHandler::Size) {
        auto&& h = d->m_header;
        if (h.isValid()) {
            v = QVariant::fromValue(h.size());
        } else if (auto d = device()) {
            if (h.peek(d)) {
                v = QVariant::fromValue(h.size());
            }
        }
    }

    if (option == QImageIOHandler::ImageFormat) {
        auto&& h = d->m_header;
        if (h.isValid()) {
            v = QVariant::fromValue(h.format());
        } else if (auto d = device()) {
            if (h.peek(d)) {
                v = QVariant::fromValue(h.format());
            }
        }
    }

    return v;
}

QImageIOPlugin::Capabilities FFPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "ff") {
        return Capabilities(CanRead);
    }
    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && FFHandler::canRead(device)) {
        cap |= CanRead;
    }
    return cap;
}

QImageIOHandler *FFPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new FFHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#include "moc_ff_p.cpp"
