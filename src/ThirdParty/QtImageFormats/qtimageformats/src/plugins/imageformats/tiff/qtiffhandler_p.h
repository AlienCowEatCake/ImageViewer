// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QTIFFHANDLER_P_H
#define QTIFFHANDLER_P_H

#include <QtCore/QScopedPointer>
#include <QtGui/QImageIOHandler>

QT_BEGIN_NAMESPACE

class QTiffHandlerPrivate;
class QTiffHandler : public QImageIOHandler
{
public:
    QTiffHandler();

    bool canRead() const Q_DECL_OVERRIDE;
    bool read(QImage *image) Q_DECL_OVERRIDE;
    bool write(const QImage &image) Q_DECL_OVERRIDE;

    static bool canRead(QIODevice *device);

    QVariant option(ImageOption option) const Q_DECL_OVERRIDE;
    void setOption(ImageOption option, const QVariant &value) Q_DECL_OVERRIDE;
    bool supportsOption(ImageOption option) const Q_DECL_OVERRIDE;

    bool jumpToNextImage() Q_DECL_OVERRIDE;
    bool jumpToImage(int imageNumber) Q_DECL_OVERRIDE;
    int imageCount() const Q_DECL_OVERRIDE;
    int currentImageNumber() const Q_DECL_OVERRIDE;

    enum Compression {
        NoCompression = 0,
        LzwCompression = 1
    };
private:
    void convert32BitOrder(void *buffer, int width);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    void rgb48fixup(QImage *image, bool floatingPoint);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
    void rgb96fixup(QImage *image);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    void rgbFixup(QImage *image);
#endif
    const QScopedPointer<QTiffHandlerPrivate> d;
    bool ensureHaveDirectoryCount() const;
};

QT_END_NAMESPACE

#endif // QTIFFHANDLER_P_H
