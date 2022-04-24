/*
    QImageIO Routines to read (and perhaps in the future, write) images
    in the high definition EXR format.

    SPDX-FileCopyrightText: 2003 Brad Hards <bradh@frogmouth.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIMG_EXR_P_H
#define KIMG_EXR_P_H

#include <QImageIOPlugin>

class EXRHandler : public QImageIOHandler
{
public:
    EXRHandler();

    bool canRead() const override;
    bool read(QImage *outImage) override;

    static bool canRead(QIODevice *device);
};

class EXRPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "exr.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

#endif // KIMG_EXR_P_H
