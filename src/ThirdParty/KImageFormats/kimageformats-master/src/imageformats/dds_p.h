/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2015 The Qt Company Ltd
    SPDX-FileCopyrightText: 2013 Ivan Komissarov
    SPDX-FileCopyrightText: 2024 Mirco Miranda

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only
*/

#ifndef QDDSHANDLER_H
#define QDDSHANDLER_H

#include <QImageIOPlugin>

struct DDSPixelFormat
{
    enum DDSPixelFormatFlags {
        FlagAlphaPixels     = 0x00000001,
        FlagAlpha           = 0x00000002,
        FlagFourCC          = 0x00000004,
        FlagPaletteIndexed4 = 0x00000008,
        FlagPaletteIndexed8 = 0x00000020,
        FlagRGB             = 0x00000040,
        FlagYUV             = 0x00000200,
        FlagLuminance       = 0x00020000,
        FlagNormal          = 0x00080000,
        FlagRGBA = FlagAlphaPixels | FlagRGB,
        FlagLA = FlagAlphaPixels | FlagLuminance
    };

    quint32 size;
    quint32 flags;
    quint32 fourCC;
    quint32 rgbBitCount;
    quint32 rBitMask;
    quint32 gBitMask;
    quint32 bBitMask;
    quint32 aBitMask;
};

struct DDSHeaderDX10
{
    quint32 dxgiFormat = 0;
    quint32 resourceDimension = 0;
    quint32 miscFlag = 0;
    quint32 arraySize = 0;
    quint32 miscFlags2 = 0;
};

struct DDSHeader
{
    enum DDSFlags {
        FlagCaps        = 0x000001,
        FlagHeight      = 0x000002,
        FlagWidth       = 0x000004,
        FlagPitch       = 0x000008,
        FlagPixelFormat = 0x001000,
        FlagMipmapCount = 0x020000,
        FlagLinearSize  = 0x080000,
        FlagDepth       = 0x800000
    };

    enum DDSCapsFlags {
        CapsComplex = 0x000008,
        CapsTexture = 0x001000,
        CapsMipmap  = 0x400000
    };

    enum DDSCaps2Flags {
        Caps2CubeMap          = 0x0200,
        Caps2CubeMapPositiveX = 0x0400,
        Caps2CubeMapNegativeX = 0x0800,
        Caps2CubeMapPositiveY = 0x1000,
        Caps2CubeMapNegativeY = 0x2000,
        Caps2CubeMapPositiveZ = 0x4000,
        Caps2CubeMapNegativeZ = 0x8000,
        Caps2Volume           = 0x200000
    };

    enum { ReservedCount = 11 };

    quint32 magic;
    quint32 size;
    quint32 flags;
    quint32 height;
    quint32 width;
    quint32 pitchOrLinearSize;
    quint32 depth;
    quint32 mipMapCount;
    quint32 reserved1[ReservedCount];
    DDSPixelFormat pixelFormat;
    quint32 caps;
    quint32 caps2;
    quint32 caps3;
    quint32 caps4;
    quint32 reserved2;
    DDSHeaderDX10 header10;
};

class QDDSHandler : public QImageIOHandler
{
public:
    QDDSHandler();

    bool canRead() const override;
    bool read(QImage *image) override;
    bool write(const QImage &image) override;

    QVariant option(QImageIOHandler::ImageOption option) const override;
    void setOption(ImageOption option, const QVariant &value) override;
    bool supportsOption(QImageIOHandler::ImageOption option) const override;

    int imageCount() const override;
    bool jumpToImage(int imageNumber) override;

    static bool canRead(QIODevice *device);

private:
    bool ensureScanned() const;
    bool verifyHeader(const DDSHeader &dds) const;

private:
    enum ScanState {
        ScanError = -1,
        ScanNotScanned = 0,
        ScanSuccess = 1,
    };

    DDSHeader m_header;
    int m_format;
    int m_currentImage;
    mutable ScanState m_scanState;
};

class QDDSPlugin : public QImageIOPlugin
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "dds.json")
public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

#endif // QDDSHANDLER_H
