// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGAHANDLER_H
#define QTGAHANDLER_H

#include <QtGui/QImageIOHandler>

QT_BEGIN_NAMESPACE

class QTgaFile;

class QTgaHandler : public QImageIOHandler
{
public:
    QTgaHandler();
    ~QTgaHandler();

    bool canRead() const Q_DECL_OVERRIDE;
    bool read(QImage *image) Q_DECL_OVERRIDE;

    static bool canRead(QIODevice *device);

    QVariant option(ImageOption option) const Q_DECL_OVERRIDE;
    void setOption(ImageOption option, const QVariant &value) Q_DECL_OVERRIDE;
    bool supportsOption(ImageOption option) const Q_DECL_OVERRIDE;

private:
    mutable QTgaFile *tga;
};

QT_END_NAMESPACE

#endif // QTGAHANDLER_H
