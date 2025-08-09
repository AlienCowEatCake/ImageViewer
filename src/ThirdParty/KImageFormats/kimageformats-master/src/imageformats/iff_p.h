/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2025 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIMG_IFF_P_H
#define KIMG_IFF_P_H

#include <QImageIOPlugin>
#include <QScopedPointer>

class IFFHandlerPrivate;
class IFFHandler : public QImageIOHandler
{
public:
    IFFHandler();

    bool canRead() const override;
    bool read(QImage *image) override;

    bool supportsOption(QImageIOHandler::ImageOption option) const override;
    QVariant option(QImageIOHandler::ImageOption option) const override;

    bool jumpToNextImage() override;
    bool jumpToImage(int imageNumber) override;
    int imageCount() const override;
    int currentImageNumber() const override;

    static bool canRead(QIODevice *device);

private:
    bool readStandardImage(QImage *image);

    bool readMayaImage(QImage *image);

private:
    const QScopedPointer<IFFHandlerPrivate> d;
};

class IFFPlugin : public QImageIOPlugin
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "iff.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

#endif // KIMG_IFF_P_H
