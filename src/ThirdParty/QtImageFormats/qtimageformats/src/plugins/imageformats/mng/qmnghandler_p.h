// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QMNGHANDLER_P_H
#define QMNGHANDLER_P_H

#include <QtCore/qscopedpointer.h>
#include <QtGui/qimageiohandler.h>

QT_BEGIN_NAMESPACE

class QImage;
class QByteArray;
class QIODevice;
class QVariant;
class QMngHandlerPrivate;

class QMngHandler : public QImageIOHandler
{
    public:
    QMngHandler();
    ~QMngHandler();
    bool canRead() const Q_DECL_OVERRIDE;
    bool read(QImage *image) Q_DECL_OVERRIDE;
    bool write(const QImage &image) Q_DECL_OVERRIDE;
    int currentImageNumber() const Q_DECL_OVERRIDE;
    int imageCount() const Q_DECL_OVERRIDE;
    bool jumpToImage(int imageNumber) Q_DECL_OVERRIDE;
    bool jumpToNextImage() Q_DECL_OVERRIDE;
    int loopCount() const Q_DECL_OVERRIDE;
    int nextImageDelay() const Q_DECL_OVERRIDE;
    static bool canRead(QIODevice *device);
    QVariant option(ImageOption option) const Q_DECL_OVERRIDE;
    void setOption(ImageOption option, const QVariant & value) Q_DECL_OVERRIDE;
    bool supportsOption(ImageOption option) const Q_DECL_OVERRIDE;

    private:
    Q_DECLARE_PRIVATE(QMngHandler)
    QScopedPointer<QMngHandlerPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QMNGHANDLER_P_H
