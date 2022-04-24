/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2005 Christoph Hormann <chris_hormann@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIMG_HDR_P_H
#define KIMG_HDR_P_H

#include <QImageIOPlugin>

class HDRHandler : public QImageIOHandler
{
public:
    HDRHandler();

    bool canRead() const override;
    bool read(QImage *outImage) override;

    static bool canRead(QIODevice *device);
};

class HDRPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "hdr.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

#endif // KIMG_HDR_P_H
