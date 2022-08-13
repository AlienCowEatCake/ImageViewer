// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2016 Ivan Komissarov.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QDDSHANDLER_H
#define QDDSHANDLER_H

#include <QtGui/qimageiohandler.h>
#include "ddsheader.h"

#ifndef QT_NO_DATASTREAM

QT_BEGIN_NAMESPACE

class QDDSHandler : public QImageIOHandler
{
public:
    QDDSHandler();

    bool canRead() const Q_DECL_OVERRIDE;
    bool read(QImage *image) Q_DECL_OVERRIDE;
    bool write(const QImage &image) Q_DECL_OVERRIDE;

    QVariant option(QImageIOHandler::ImageOption option) const Q_DECL_OVERRIDE;
    void setOption(ImageOption option, const QVariant &value) Q_DECL_OVERRIDE;
    bool supportsOption(QImageIOHandler::ImageOption option) const Q_DECL_OVERRIDE;

    int imageCount() const Q_DECL_OVERRIDE;
    bool jumpToImage(int imageNumber) Q_DECL_OVERRIDE;

    static bool canRead(QIODevice *device);

private:
    bool ensureScanned() const;
    bool verifyHeader(const DDSHeader &dds) const;

private:
    enum ScanState {
        ScanError = -1,
        ScanNotScanned = 0,
        ScanSuccess = 1,
    };

    DDSHeader m_header;
    int m_format;
    DDSHeaderDX10 m_header10;
    int m_currentImage;
    mutable ScanState m_scanState;
};

QT_END_NAMESPACE

#endif // QT_NO_DATASTREAM

#endif // QDDSHANDLER_H
