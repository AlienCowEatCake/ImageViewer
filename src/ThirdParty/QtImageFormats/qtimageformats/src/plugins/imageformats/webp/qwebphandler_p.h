// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBPHANDLER_P_H
#define QWEBPHANDLER_P_H

#include <QtGui/qcolor.h>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#include <QtGui/qcolorspace.h>
#endif
#include <QtGui/qimage.h>
#include <QtGui/qimageiohandler.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qsize.h>

#include "webp/decode.h"
#include "webp/demux.h"

class QWebpHandler : public QImageIOHandler
{
public:
    QWebpHandler();
    ~QWebpHandler();

public:
    bool canRead() const Q_DECL_OVERRIDE;
    bool read(QImage *image) Q_DECL_OVERRIDE;

    static bool canRead(QIODevice *device);

    bool write(const QImage &image) Q_DECL_OVERRIDE;
    QVariant option(ImageOption option) const Q_DECL_OVERRIDE;
    void setOption(ImageOption option, const QVariant &value) Q_DECL_OVERRIDE;
    bool supportsOption(ImageOption option) const Q_DECL_OVERRIDE;

    int imageCount() const Q_DECL_OVERRIDE;
    int currentImageNumber() const Q_DECL_OVERRIDE;
    QRect currentImageRect() const Q_DECL_OVERRIDE;
    int loopCount() const Q_DECL_OVERRIDE;
    int nextImageDelay() const Q_DECL_OVERRIDE;

private:
    bool ensureScanned() const;
    bool ensureDemuxer();

private:
    enum ScanState {
        ScanError = -1,
        ScanNotScanned = 0,
        ScanSuccess = 1,
    };

    int m_quality;
    mutable ScanState m_scanState;
    WebPBitstreamFeatures m_features;
    uint32_t m_formatFlags;
    int m_loop;
    int m_frameCount;
    QColor m_bgColor;
    QByteArray m_rawData;
    WebPData m_webpData;
    WebPDemuxer *m_demuxer;
    WebPIterator m_iter;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QColorSpace m_colorSpace;
#endif
    QImage *m_composited;   // For animation frames composition
};

#endif // WEBPHANDLER_H
