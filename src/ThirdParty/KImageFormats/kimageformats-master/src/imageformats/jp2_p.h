/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2024 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KIMG_JP2_P_H
#define KIMG_JP2_P_H

#include <QImageIOPlugin>
#include <QScopedPointer>

class JP2HandlerPrivate;
class JP2Handler : public QImageIOHandler
{
public:
    JP2Handler();

    bool canRead() const override;
    bool read(QImage *image) override;
    bool write(const QImage &image) override;

    bool supportsOption(QImageIOHandler::ImageOption option) const override;
    void setOption(ImageOption option, const QVariant &value) override;
    QVariant option(QImageIOHandler::ImageOption option) const override;

    static bool canRead(QIODevice *device);

private:
    const QScopedPointer<JP2HandlerPrivate> d;
};

class JP2Plugin : public QImageIOPlugin
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "jp2.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

#endif // KIMG_JP2_P_H
