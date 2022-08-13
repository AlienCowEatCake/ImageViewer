// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2016 Petroules Corporation.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QJP2HANDLER_H
#define QJP2HANDLER_H

#include <QtCore/qscopedpointer.h>
#include <QtGui/qimageiohandler.h>

QT_BEGIN_NAMESPACE

class QImage;
class QByteArray;
class QIODevice;
class QVariant;
class QJp2HandlerPrivate;

class QJp2Handler : public QImageIOHandler
{
public:
    QJp2Handler();
    ~QJp2Handler();
    static bool canRead(QIODevice *iod, QByteArray *subType);
    bool canRead() const Q_DECL_OVERRIDE;
    bool read(QImage *image) Q_DECL_OVERRIDE;
    bool write(const QImage &image) Q_DECL_OVERRIDE;
    QVariant option(ImageOption option) const Q_DECL_OVERRIDE;
    void setOption(ImageOption option, const QVariant &value) Q_DECL_OVERRIDE;
    bool supportsOption(ImageOption option) const Q_DECL_OVERRIDE;

private:
    Q_DECLARE_PRIVATE(QJp2Handler)
    QScopedPointer<QJp2HandlerPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QJP2HANDLER_P_H
