/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2026 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIMG_FF_P_H
#define KIMG_FF_P_H

#include <QImageIOPlugin>
#include <QScopedPointer>

class FFHandlerPrivate;
class FFHandler : public QImageIOHandler
{
public:
    FFHandler();

    bool canRead() const override;
    bool read(QImage *image) override;

    bool supportsOption(QImageIOHandler::ImageOption option) const override;
    QVariant option(QImageIOHandler::ImageOption option) const override;

    static bool canRead(QIODevice *device);

private:
    const QScopedPointer<FFHandlerPrivate> d;
};

class FFPlugin : public QImageIOPlugin
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "ff.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

#endif // KIMG_FF_P_H
