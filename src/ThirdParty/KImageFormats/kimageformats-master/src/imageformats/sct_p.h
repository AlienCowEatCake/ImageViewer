/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2024 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SIMG_SCITEX_P_H
#define SIMG_SCITEX_P_H

#include <QImageIOPlugin>
#include <QScopedPointer>

class ScitexHandlerPrivate;
class ScitexHandler : public QImageIOHandler
{
public:
    ScitexHandler();

    bool canRead() const override;
    bool read(QImage *image) override;

    bool supportsOption(QImageIOHandler::ImageOption option) const override;
    QVariant option(QImageIOHandler::ImageOption option) const override;

    static bool canRead(QIODevice *device);

private:
    const QScopedPointer<ScitexHandlerPrivate> d;
};

class ScitexPlugin : public QImageIOPlugin
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "sct.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

#endif // SIMG_SCITEX_P_H
