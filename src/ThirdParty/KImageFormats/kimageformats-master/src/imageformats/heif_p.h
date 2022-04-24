/*
    High Efficiency Image File Format (HEIF) support for QImage.

    SPDX-FileCopyrightText: 2020 Sirius Bakke <sirius@bakke.co>
    SPDX-FileCopyrightText: 2021 Daniel Novomesky <dnovomesky@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIMG_HEIF_P_H
#define KIMG_HEIF_P_H

#include <QByteArray>
#include <QImage>
#include <QImageIOPlugin>

class HEIFHandler : public QImageIOHandler
{
public:
    HEIFHandler();

    bool canRead() const override;
    bool read(QImage *image) override;
    bool write(const QImage &image) override;

    static bool canRead(QIODevice *device);

    QVariant option(ImageOption option) const override;
    void setOption(ImageOption option, const QVariant &value) override;
    bool supportsOption(ImageOption option) const override;

private:
    static bool isSupportedBMFFType(const QByteArray &header);
    bool ensureParsed() const;
    bool ensureDecoder();

    enum ParseHeicState {
        ParseHeicError = -1,
        ParseHeicNotParsed = 0,
        ParseHeicSuccess = 1,
    };

    ParseHeicState m_parseState;
    int m_quality;
    QImage m_current_image;
};

class HEIFPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "heif.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

#endif // KIMG_HEIF_P_H
