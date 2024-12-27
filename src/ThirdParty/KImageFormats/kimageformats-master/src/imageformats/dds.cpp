/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2015 The Qt Company Ltd
    SPDX-FileCopyrightText: 2013 Ivan Komissarov
    SPDX-FileCopyrightText: 2024 Mirco Miranda

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only
*/

// Forked from Qt 5.6 branch

#include "dds_p.h"
#include "util_p.h"
#include "scanlineconverter_p.h"

#include <QColorSpace>
#include <QDataStream>
#include <QDebug>
#include <QFloat16>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QVector>
#endif

#include <cmath>

#ifndef DDS_DISABLE_STRIDE_ALIGNMENT
// Disable the stride aligment based on DDS pitch: it is known that some writers do not set it correctly
// #define DDS_DISABLE_STRIDE_ALIGNMENT
#endif

enum Format {
    FormatUnknown              = 0,

    FormatR8G8B8               = 20,
    FormatA8R8G8B8             = 21,
    FormatX8R8G8B8             = 22,
    FormatR5G6B5               = 23,
    FormatX1R5G5B5             = 24,
    FormatA1R5G5B5             = 25,
    FormatA4R4G4B4             = 26,
    FormatR3G3B2               = 27,
    FormatA8                   = 28,
    FormatA8R3G3B2             = 29,
    FormatX4R4G4B4             = 30,
    FormatA2B10G10R10          = 31,
    FormatA8B8G8R8             = 32,
    FormatX8B8G8R8             = 33,
    FormatG16R16               = 34,
    FormatA2R10G10B10          = 35,
    FormatA16B16G16R16         = 36,

    FormatA8P8                 = 40,
    FormatP8                   = 41,

    FormatL8                   = 50,
    FormatA8L8                 = 51,
    FormatA4L4                 = 52,

    FormatV8U8                 = 60,
    FormatL6V5U5               = 61,
    FormatX8L8V8U8             = 62,
    FormatQ8W8V8U8             = 63,
    FormatV16U16               = 64,
    FormatA2W10V10U10          = 67,

    FormatUYVY                 = 0x59565955, // "UYVY"
    FormatR8G8B8G8             = 0x47424752, // "RGBG"
    FormatYUY2                 = 0x32595559, // "YUY2"
    FormatG8R8G8B8             = 0x42475247, // "GRGB"
    FormatDXT1                 = 0x31545844, // "DXT1"
    FormatDXT2                 = 0x32545844, // "DXT2"
    FormatDXT3                 = 0x33545844, // "DXT3"
    FormatDXT4                 = 0x34545844, // "DXT4"
    FormatDXT5                 = 0x35545844, // "DXT5"
    FormatRXGB                 = 0x42475852, // "RXGB"
    FormatATI2                 = 0x32495441, // "ATI2"

    FormatD16Lockable         = 70,
    FormatD32                  = 71,
    FormatD15S1                = 73,
    FormatD24S8                = 75,
    FormatD24X8                = 77,
    FormatD24X4S4              = 79,
    FormatD16                  = 80,

    FormatD32FLockable        = 82,
    FormatD24FS8               = 83,

    FormatD32Lockable         = 84,
    FormatS8Lockable          = 85,

    FormatL16                  = 81,

    FormatVertexData           =100,
    FormatIndex16              =101,
    FormatIndex32              =102,

    FormatQ16W16V16U16         = 110,

    FormatMulti2ARGB8         = 0x3154454d, // "MET1"

    FormatR16F                 = 111,
    FormatG16R16F              = 112,
    FormatA16B16G16R16F        = 113,

    FormatR32F                 = 114,
    FormatG32R32F              = 115,
    FormatA32B32G32R32F        = 116,

    FormatCxV8U8               = 117,

    FormatA1                   = 118,
    FormatA2B10G10R10_XR_BIAS  = 119,
    FormatBinaryBuffer         = 199,

    FormatP4,
    FormatA4P4,

    FormatLast                 = 0x7fffffff
};

enum DXGIFormat {
    DXGIFormatUNKNOWN = 0,
    DXGIFormatR32G32B32A32_TYPELESS = 1,
    DXGIFormatR32G32B32A32_FLOAT = 2,
    DXGIFormatR32G32B32A32_UINT = 3,
    DXGIFormatR32G32B32A32_SINT = 4,
    DXGIFormatR32G32B32_TYPELESS = 5,
    DXGIFormatR32G32B32_FLOAT = 6,
    DXGIFormatR32G32B32_UINT = 7,
    DXGIFormatR32G32B32_SINT = 8,
    DXGIFormatR16G16B16A16_TYPELESS = 9,
    DXGIFormatR16G16B16A16_FLOAT = 10,
    DXGIFormatR16G16B16A16_UNORM = 11,
    DXGIFormatR16G16B16A16_UINT = 12,
    DXGIFormatR16G16B16A16_SNORM = 13,
    DXGIFormatR16G16B16A16_SINT = 14,
    DXGIFormatR32G32_TYPELESS = 15,
    DXGIFormatR32G32_FLOAT = 16,
    DXGIFormatR32G32_UINT = 17,
    DXGIFormatR32G32_SINT = 18,
    DXGIFormatR32G8X24_TYPELESS = 19,
    DXGIFormatD32_FLOAT_S8X24_UINT = 20,
    DXGIFormatR32_FLOAT_X8X24_TYPELESS = 21,
    DXGIFormatX32_TYPELESS_G8X24_UINT = 22,
    DXGIFormatR10G10B10A2_TYPELESS = 23,
    DXGIFormatR10G10B10A2_UNORM = 24,
    DXGIFormatR10G10B10A2_UINT = 25,
    DXGIFormatR11G11B10_FLOAT = 26,
    DXGIFormatR8G8B8A8_TYPELESS = 27,
    DXGIFormatR8G8B8A8_UNORM = 28,
    DXGIFormatR8G8B8A8_UNORM_SRGB = 29,
    DXGIFormatR8G8B8A8_UINT = 30,
    DXGIFormatR8G8B8A8_SNORM = 31,
    DXGIFormatR8G8B8A8_SINT = 32,
    DXGIFormatR16G16_TYPELESS = 33,
    DXGIFormatR16G16_FLOAT = 34,
    DXGIFormatR16G16_UNORM = 35,
    DXGIFormatR16G16_UINT = 36,
    DXGIFormatR16G16_SNORM = 37,
    DXGIFormatR16G16_SINT = 38,
    DXGIFormatR32_TYPELESS = 39,
    DXGIFormatD32_FLOAT = 40,
    DXGIFormatR32_FLOAT = 41,
    DXGIFormatR32_UINT = 42,
    DXGIFormatR32_SINT = 43,
    DXGIFormatR24G8_TYPELESS = 44,
    DXGIFormatD24_UNORM_S8_UINT = 45,
    DXGIFormatR24_UNORM_X8_TYPELESS = 46,
    DXGIFormatX24_TYPELESS_G8_UINT = 47,
    DXGIFormatR8G8_TYPELESS = 48,
    DXGIFormatR8G8_UNORM = 49,
    DXGIFormatR8G8_UINT = 50,
    DXGIFormatR8G8_SNORM = 51,
    DXGIFormatR8G8_SINT = 52,
    DXGIFormatR16_TYPELESS = 53,
    DXGIFormatR16_FLOAT = 54,
    DXGIFormatD16_UNORM = 55,
    DXGIFormatR16_UNORM = 56,
    DXGIFormatR16_UINT = 57,
    DXGIFormatR16_SNORM = 58,
    DXGIFormatR16_SINT = 59,
    DXGIFormatR8_TYPELESS = 60,
    DXGIFormatR8_UNORM = 61,
    DXGIFormatR8_UINT = 62,
    DXGIFormatR8_SNORM = 63,
    DXGIFormatR8_SINT = 64,
    DXGIFormatA8_UNORM = 65,
    DXGIFormatR1_UNORM = 66,
    DXGIFormatR9G9B9E5_SHAREDEXP = 67,
    DXGIFormatR8G8_B8G8_UNORM = 68,
    DXGIFormatG8R8_G8B8_UNORM = 69,
    DXGIFormatBC1_TYPELESS = 70,
    DXGIFormatBC1_UNORM = 71,
    DXGIFormatBC1_UNORM_SRGB = 72,
    DXGIFormatBC2_TYPELESS = 73,
    DXGIFormatBC2_UNORM = 74,
    DXGIFormatBC2_UNORM_SRGB = 75,
    DXGIFormatBC3_TYPELESS = 76,
    DXGIFormatBC3_UNORM = 77,
    DXGIFormatBC3_UNORM_SRGB = 78,
    DXGIFormatBC4_TYPELESS = 79,
    DXGIFormatBC4_UNORM = 80,
    DXGIFormatBC4_SNORM = 81,
    DXGIFormatBC5_TYPELESS = 82,
    DXGIFormatBC5_UNORM = 83,
    DXGIFormatBC5_SNORM = 84,
    DXGIFormatB5G6R5_UNORM = 85,
    DXGIFormatB5G5R5A1_UNORM = 86,
    DXGIFormatB8G8R8A8_UNORM = 87,
    DXGIFormatB8G8R8X8_UNORM = 88,
    DXGIFormatR10G10B10_XR_BIAS_A2_UNORM = 89,
    DXGIFormatB8G8R8A8_TYPELESS = 90,
    DXGIFormatB8G8R8A8_UNORM_SRGB = 91,
    DXGIFormatB8G8R8X8_TYPELESS = 92,
    DXGIFormatB8G8R8X8_UNORM_SRGB = 93,
    DXGIFormatBC6H_TYPELESS = 94,
    DXGIFormatBC6H_UF16 = 95,
    DXGIFormatBC6H_SF16 = 96,
    DXGIFormatBC7_TYPELESS = 97,
    DXGIFormatBC7_UNORM = 98,
    DXGIFormatBC7_UNORM_SRGB = 99,
    DXGIFormatAYUV = 100,
    DXGIFormatY410 = 101,
    DXGIFormatY416 = 102,
    DXGIFormatNV12 = 103,
    DXGIFormatP010 = 104,
    DXGIFormatP016 = 105,
    DXGIFormat420_OPAQUE = 106,
    DXGIFormatYUY2 = 107,
    DXGIFormatY210 = 108,
    DXGIFormatY216 = 109,
    DXGIFormatNV11 = 110,
    DXGIFormatAI44 = 111,
    DXGIFormatIA44 = 112,
    DXGIFormatP8 = 113,
    DXGIFormatA8P8 = 114,
    DXGIFormatB4G4R4A4_UNORM = 115,
    DXGIFormatP208 = 130,
    DXGIFormatV208 = 131,
    DXGIFormatV408 = 132,
    DXGIFormatSAMPLER_FEEDBACK_MIN_MIP_OPAQUE,
    DXGIFormatSAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE,
    DXGIFormatFORCE_UINT = 0xffffffff
};

enum DXGIMiscFlags2
{
    // not really flags...
    DXGIAlphaModeUnknow = 0,
    DXGIAlphaModeStraight = 1,
    DXGIAlphaModePremultiplied = 2,
    DXGIAlphaModeOpaque = 3,
    DXGIAlphaModeCustom = 4
};

enum Colors {
    Red = 0,
    Green,
    Blue,
    Alpha,
    ColorCount
};

enum DXTVersions {
    One = 1,
    Two = 2,
    Three = 3,
    Four = 4,
    Five = 5,
    RXGB = 6
};

// All magic numbers are little-endian as long as dds format has little
// endian byte order
static const quint32 ddsMagic = 0x20534444; // "DDS "
static const quint32 dx10Magic = 0x30315844; // "DX10"

static const qint64 headerSize = 128;
static const quint32 ddsSize = 124; // headerSize without magic
static const quint32 pixelFormatSize = 32;

struct FaceOffset
{
    int x, y;
};

static const FaceOffset faceOffsets[6] = { {2, 1}, {0, 1}, {1, 0}, {1, 2}, {1, 1}, {3, 1} };

static int faceFlags[6] = {
    DDSHeader::Caps2CubeMapPositiveX,
    DDSHeader::Caps2CubeMapNegativeX,
    DDSHeader::Caps2CubeMapPositiveY,
    DDSHeader::Caps2CubeMapNegativeY,
    DDSHeader::Caps2CubeMapPositiveZ,
    DDSHeader::Caps2CubeMapNegativeZ
};

struct FormatInfo
{
    Format format;
    quint32 flags;
    quint32 bitCount;
    quint32 rBitMask;
    quint32 gBitMask;
    quint32 bBitMask;
    quint32 aBitMask;
};

static const FormatInfo formatInfos[] = {
    { FormatA8R8G8B8,    DDSPixelFormat::FlagRGBA, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 },
    { FormatX8R8G8B8,    DDSPixelFormat::FlagRGB,  32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 },
    { FormatA2B10G10R10, DDSPixelFormat::FlagRGBA, 32, 0x000003ff, 0x000ffc00, 0x3ff00000, 0xc0000000 },
    { FormatA8B8G8R8,    DDSPixelFormat::FlagRGBA, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 },
    { FormatX8B8G8R8,    DDSPixelFormat::FlagRGB,  32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000 },
    { FormatG16R16,      DDSPixelFormat::FlagRGBA, 32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000 },
    { FormatG16R16,      DDSPixelFormat::FlagRGB,  32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000 },
    { FormatA2R10G10B10, DDSPixelFormat::FlagRGBA, 32, 0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000 },

    { FormatR8G8B8,      DDSPixelFormat::FlagRGB,  24, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 },

    { FormatR5G6B5,      DDSPixelFormat::FlagRGB,  16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000 },
    { FormatX1R5G5B5,    DDSPixelFormat::FlagRGB,  16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00000000 },
    { FormatA1R5G5B5,    DDSPixelFormat::FlagRGBA, 16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00008000 },
    { FormatA4R4G4B4,    DDSPixelFormat::FlagRGBA, 16, 0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000 },
    { FormatA8R3G3B2,    DDSPixelFormat::FlagRGBA, 16, 0x000000e0, 0x0000001c, 0x00000003, 0x0000ff00 },
    { FormatX4R4G4B4,    DDSPixelFormat::FlagRGB,  16, 0x00000f00, 0x000000f0, 0x0000000f, 0x00000000 },
    { FormatA8L8,        DDSPixelFormat::FlagLA,   16, 0x000000ff, 0x00000000, 0x00000000, 0x0000ff00 },
    { FormatL16,   DDSPixelFormat::FlagLuminance,  16, 0x0000ffff, 0x00000000, 0x00000000, 0x00000000 },

    { FormatR3G3B2,      DDSPixelFormat::FlagRGB,  8,  0x000000e0, 0x0000001c, 0x00000003, 0x00000000 },
    { FormatA8,        DDSPixelFormat::FlagAlpha,  8,  0x00000000, 0x00000000, 0x00000000, 0x000000ff },
    { FormatL8,    DDSPixelFormat::FlagLuminance,  8,  0x000000ff, 0x00000000, 0x00000000, 0x00000000 },
    { FormatA4L4,        DDSPixelFormat::FlagLA,   8,  0x0000000f, 0x00000000, 0x00000000, 0x000000f0 },

    { FormatV8U8,        DDSPixelFormat::FlagNormal, 16, 0x000000ff, 0x0000ff00, 0x00000000, 0x00000000 },
    { FormatL6V5U5,                                0, 16, 0x0000001f, 0x000003e0, 0x0000fc00, 0x00000000 },
    { FormatX8L8V8U8,                              0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000 },
    { FormatQ8W8V8U8,    DDSPixelFormat::FlagNormal, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 },
    { FormatV16U16,      DDSPixelFormat::FlagNormal, 32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000 },
    { FormatA2W10V10U10, DDSPixelFormat::FlagNormal, 32, 0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000 }
};
static const size_t formatInfosSize = sizeof(formatInfos)/sizeof(FormatInfo);

static const Format knownFourCCs[] = {
    FormatA16B16G16R16,
    FormatV8U8,
    FormatUYVY,
    FormatR8G8B8G8,
    FormatYUY2,
    FormatG8R8G8B8,
    FormatDXT1,
    FormatDXT2,
    FormatDXT3,
    FormatDXT4,
    FormatDXT5,
    FormatRXGB,
    FormatATI2,
    FormatQ16W16V16U16,
    FormatR16F,
    FormatG16R16F,
    FormatA16B16G16R16F,
    FormatR32F,
    FormatG32R32F,
    FormatA32B32G32R32F,
    FormatCxV8U8
};
static const size_t knownFourCCsSize = sizeof(knownFourCCs)/sizeof(Format);

struct DXGIFormatToFormat
{
    DXGIFormat dxgiFormat;
    Format format;
};

static const DXGIFormatToFormat knownDXGIFormat[] = {
    { DXGIFormatR16G16B16A16_FLOAT, FormatA16B16G16R16F },
    { DXGIFormatR32G32B32A32_FLOAT, FormatA32B32G32R32F },
    { DXGIFormatR16G16_FLOAT, FormatG16R16F },
    { DXGIFormatR32G32_FLOAT, FormatG32R32F },
    { DXGIFormatR16_FLOAT, FormatR16F },
    { DXGIFormatR32_FLOAT, FormatR32F }
};
static const size_t knownDXGIFormatSize = sizeof(knownDXGIFormat)/sizeof(DXGIFormatToFormat);

struct FormatName
{
    Format format;
    const char *const name;
};
static const FormatName formatNames[] = {
    { FormatUnknown,  "unknown" },

    { FormatR8G8B8,   "R8G8B8"  },
    { FormatA8R8G8B8, "A8R8G8B8" },
    { FormatX8R8G8B8, "X8R8G8B8" },
    { FormatR5G6B5,   "R5G6B5" },
    { FormatX1R5G5B5, "X1R5G5B5" },
    { FormatA1R5G5B5, "A1R5G5B5" },
    { FormatA4R4G4B4, "A4R4G4B4" },
    { FormatR3G3B2, "R3G3B2" },
    { FormatA8, "A8" },
    { FormatA8R3G3B2, "A8R3G3B2" },
    { FormatX4R4G4B4, "X4R4G4B4" },
    { FormatA2B10G10R10, "A2B10G10R10" },
    { FormatA8B8G8R8, "A8B8G8R8" },
    { FormatX8B8G8R8, "X8B8G8R8" },
    { FormatG16R16, "G16R16" },
    { FormatA2R10G10B10, "A2R10G10B10" },
    { FormatA16B16G16R16, "A16B16G16R16" },

    { FormatA8P8, "A8P8" },
    { FormatP8, "P8" },

    { FormatL8, "L8" },
    { FormatA8L8, "A8L8" },
    { FormatA4L4, "A4L4" },

    { FormatV8U8, "V8U8" },
    { FormatL6V5U5, "L6V5U5" },
    { FormatX8L8V8U8, "X8L8V8U8" },
    { FormatQ8W8V8U8, "Q8W8V8U8" },
    { FormatV16U16, "V16U16" },
    { FormatA2W10V10U10, "A2W10V10U10" },

    { FormatUYVY, "UYVY" },
    { FormatR8G8B8G8, "R8G8_B8G8" },
    { FormatYUY2, "YUY2" },
    { FormatG8R8G8B8, "G8R8_G8B8" },
    { FormatDXT1, "DXT1" },
    { FormatDXT2, "DXT2" },
    { FormatDXT3, "DXT3" },
    { FormatDXT4, "DXT4" },
    { FormatDXT5, "DXT5" },
    { FormatRXGB, "RXGB" },
    { FormatATI2, "ATI2" },

    { FormatD16Lockable, "D16Lockable" },
    { FormatD32, "D32" },
    { FormatD15S1, "D15S1" },
    { FormatD24S8, "D24S8" },
    { FormatD24X8, "D24X8" },
    { FormatD24X4S4, "D24X4S4" },
    { FormatD16, "D16" },

    { FormatD32FLockable, "D32FLockable" },
    { FormatD24FS8, "D24FS8" },

    { FormatD32Lockable, "D32Lockable" },
    { FormatS8Lockable, "S8Lockable" },

    { FormatL16, "L16" },

    { FormatVertexData, "VertexData" },
    { FormatIndex32, "Index32" },
    { FormatIndex32, "Index32" },

    { FormatQ16W16V16U16, "Q16W16V16U16" },

    { FormatMulti2ARGB8, "Multi2ARGB8" },

    { FormatR16F, "R16F" },
    { FormatG16R16F, "G16R16F" },
    { FormatA16B16G16R16F, "A16B16G16R16F" },

    { FormatR32F, "R32F" },
    { FormatG32R32F, "G32R32F" },
    { FormatA32B32G32R32F, "A32B32G32R32F" },

    { FormatCxV8U8, "CxV8U8" },

    { FormatA1, "A1" },
    { FormatA2B10G10R10_XR_BIAS, "A2B10G10R10_XR_BIAS" },
    { FormatBinaryBuffer, "BinaryBuffer" },

    { FormatP4, "P4" },
    { FormatA4P4, "A4P4" }
};
static const size_t formatNamesSize = sizeof(formatNames)/sizeof(FormatName);

QDataStream &operator>>(QDataStream &s, DDSPixelFormat &pixelFormat)
{
    s >> pixelFormat.size;
    s >> pixelFormat.flags;
    s >> pixelFormat.fourCC;
    s >> pixelFormat.rgbBitCount;
    s >> pixelFormat.rBitMask;
    s >> pixelFormat.gBitMask;
    s >> pixelFormat.bBitMask;
    s >> pixelFormat.aBitMask;
    return s;
}

QDataStream &operator<<(QDataStream &s, const DDSPixelFormat &pixelFormat)
{
    s << pixelFormat.size;
    s << pixelFormat.flags;
    s << pixelFormat.fourCC;
    s << pixelFormat.rgbBitCount;
    s << pixelFormat.rBitMask;
    s << pixelFormat.gBitMask;
    s << pixelFormat.bBitMask;
    s << pixelFormat.aBitMask;
    return s;
}

QDataStream &operator>>(QDataStream &s, DDSHeaderDX10 &header)
{
    s >> header.dxgiFormat;
    s >> header.resourceDimension;
    s >> header.miscFlag;
    s >> header.arraySize;
    s >> header.miscFlags2;
    return s;
}

QDataStream &operator<<(QDataStream &s, const DDSHeaderDX10 &header)
{
    s << header.dxgiFormat;
    s << header.resourceDimension;
    s << header.miscFlag;
    s << header.arraySize;
    s << header.miscFlags2;
    return s;
}

QDataStream &operator>>(QDataStream &s, DDSHeader &header)
{
    s >> header.magic;
    s >> header.size;
    s >> header.flags;
    s >> header.height;
    s >> header.width;
    s >> header.pitchOrLinearSize;
    s >> header.depth;
    s >> header.mipMapCount;
    for (int i = 0; i < DDSHeader::ReservedCount; i++)
        s >> header.reserved1[i];
    s >> header.pixelFormat;
    s >> header.caps;
    s >> header.caps2;
    s >> header.caps3;
    s >> header.caps4;
    s >> header.reserved2;
    if (header.pixelFormat.fourCC == dx10Magic)
        s >> header.header10;

    return s;
}

QDataStream &operator<<(QDataStream &s, const DDSHeader &header)
{
    s << header.magic;
    s << header.size;
    s << header.flags;
    s << header.height;
    s << header.width;
    s << header.pitchOrLinearSize;
    s << header.depth;
    s << header.mipMapCount;
    for (int i = 0; i < DDSHeader::ReservedCount; i++)
        s << header.reserved1[i];
    s << header.pixelFormat;
    s << header.caps;
    s << header.caps2;
    s << header.caps3;
    s << header.caps4;
    s << header.reserved2;
    if (header.pixelFormat.fourCC == dx10Magic)
        s << header.header10;

    return s;
}

inline qsizetype ptrDiff(const void *end, const void *start)
{
    return qsizetype(reinterpret_cast<const char*>(end) - reinterpret_cast<const char*>(start));
}

static inline int maskToShift(quint32 mask)
{
    if (mask == 0)
        return 0;

    int result = 0;
    while (!((mask >> result) & 1))
        result++;
    return result;
}

static inline int maskLength(quint32 mask)
{
    int result = 0;
    while (mask) {
       if (mask & 1)
           result++;
       mask >>= 1;
    }
    return result;
}

static inline quint32 readValue(QDataStream &s, quint32 size)
{
    quint32 value = 0;
    if (size != 8 && size != 16 && size != 24 && size != 32) {
        s.setStatus(QDataStream::ReadCorruptData);
        return value;
    }

    quint8 tmp;
    for (unsigned bit = 0; bit < size; bit += 8) {
        s >> tmp;
        value += (quint32(tmp) << bit);
    }
    return value;
}

static inline bool hasAlpha(const DDSHeader &dds)
{
    return (dds.pixelFormat.flags & (DDSPixelFormat::FlagAlphaPixels | DDSPixelFormat::FlagAlpha)) != 0;
}

static inline bool isCubeMap(const DDSHeader &dds)
{
    return (dds.caps2 & DDSHeader::Caps2CubeMap) != 0;
}

static inline QRgb yuv2rgb(quint8 Y, quint8 U, quint8 V)
{
    return qRgb(quint8(Y + 1.13983 * (V - 128)),
                quint8(Y - 0.39465 * (U - 128) - 0.58060 * (V - 128)),
                quint8(Y + 2.03211 * (U - 128)));
}

static void strideAlignment(QDataStream &s, const DDSHeader &dds, quint32 width)
{
#ifdef DDS_DISABLE_STRIDE_ALIGNMENT
    Q_UNUSED(s)
    Q_UNUSED(dds)
    Q_UNUSED(width)
#else
    if (dds.flags & DDSHeader::FlagPitch) {
        if (auto alignBytes = qint64(dds.pitchOrLinearSize) - (width * dds.pixelFormat.rgbBitCount + 7) / 8) {
            quint8 tmp;
            for (; alignBytes > 0 && alignBytes < 4; --alignBytes) {
                s >> tmp;
            }
        }
    }
#endif
}

static Format getFormat(const DDSHeader &dds)
{
    const DDSPixelFormat &format = dds.pixelFormat;
    if (format.flags & DDSPixelFormat::FlagPaletteIndexed4) {
        return FormatP4;
    } else if (format.flags & DDSPixelFormat::FlagPaletteIndexed8) {
        return FormatP8;
    } else if (format.flags & DDSPixelFormat::FlagFourCC) {
        if (dds.pixelFormat.fourCC == dx10Magic) {
            for (size_t i = 0; i < knownDXGIFormatSize; ++i) {
                if (dds.header10.dxgiFormat == knownDXGIFormat[i].dxgiFormat)
                    return knownDXGIFormat[i].format;
            }
        } else {
            for (size_t i = 0; i < knownFourCCsSize; ++i) {
                if (dds.pixelFormat.fourCC == knownFourCCs[i])
                    return knownFourCCs[i];
            }
        }
    } else {
        for (size_t i = 0; i < formatInfosSize; ++i) {
            const FormatInfo &info = formatInfos[i];
            if ((format.flags & info.flags) == info.flags &&
                 format.rgbBitCount == info.bitCount &&
                 format.rBitMask == info.rBitMask &&
                 format.gBitMask == info.gBitMask &&
                 format.bBitMask == info.bBitMask &&
                 format.aBitMask == info.aBitMask) {
                return info.format;
            }
        }
    }

    return FormatUnknown;
}

static inline quint8 getNormalZ(quint8 nx, quint8 ny)
{
    const double fx = nx / 127.5 - 1.0;
    const double fy = ny / 127.5 - 1.0;
    const double fxfy = 1.0 - fx * fx - fy * fy;
    return fxfy > 0 ? 255 * std::sqrt(fxfy) : 0;
}

static inline void decodeColor(quint16 color, quint8 &red, quint8 &green, quint8 &blue)
{
    red = ((color >> 11) & 0x1f) << 3;
    green = ((color >> 5) & 0x3f) << 2;
    blue = (color & 0x1f) << 3;
}

static inline quint8 calcC2(quint8 c0, quint8 c1)
{
    return 2.0 * c0 / 3.0 + c1 / 3.0;
}

static inline quint8 calcC2a(quint8 c0, quint8 c1)
{
    return c0 / 2.0 + c1 / 2.0;
}

static inline quint8 calcC3(quint8 c0, quint8 c1)
{
    return c0 / 3.0 + 2.0 * c1 / 3.0;
}

static void DXTFillColors(QRgb *result, quint16 c0, quint16 c1, quint32 table, bool dxt1a = false)
{
    quint8 r[4];
    quint8 g[4];
    quint8 b[4];
    quint8 a[4];

    a[0] = a[1] = a[2] = a[3] = 255;

    decodeColor(c0, r[0], g[0], b[0]);
    decodeColor(c1, r[1], g[1], b[1]);
    if (!dxt1a) {
        r[2] = calcC2(r[0], r[1]);
        g[2] = calcC2(g[0], g[1]);
        b[2] = calcC2(b[0], b[1]);
        r[3] = calcC3(r[0], r[1]);
        g[3] = calcC3(g[0], g[1]);
        b[3] = calcC3(b[0], b[1]);
    } else {
        r[2] = calcC2a(r[0], r[1]);
        g[2] = calcC2a(g[0], g[1]);
        b[2] = calcC2a(b[0], b[1]);
        r[3] = g[3] = b[3] = a[3] = 0;
    }

    for (int k = 0; k < 4; k++)
        for (int l = 0; l < 4; l++) {
            unsigned index = table & 0x0003;
            table >>= 2;

            result[k * 4 + l] = qRgba(r[index], g[index], b[index], a[index]);
        }
}

template <DXTVersions version>
inline void setAlphaDXT32Helper(QRgb *rgbArr, quint64 alphas)
{
    Q_STATIC_ASSERT(version == Two || version == Three);
    for (int i = 0; i < 16; i++) {
        quint8 alpha = 16 * (alphas & 0x0f);
        QRgb rgb = rgbArr[i];
        if (version == Two) // DXT2
            rgbArr[i] = qRgba(qRed(rgb) * alpha / 0xff, qGreen(rgb) * alpha / 0xff, qBlue(rgb) * alpha / 0xff, alpha);
        else if (version == Three) // DXT3
            rgbArr[i] = qRgba(qRed(rgb), qGreen(rgb), qBlue(rgb), alpha);
        alphas = alphas >> 4;
    }
}

template <DXTVersions version>
inline void setAlphaDXT45Helper(QRgb *rgbArr, quint64 alphas)
{
    Q_STATIC_ASSERT(version == Four || version == Five);
    quint8 a[8];
    a[0] = alphas & 0xff;
    a[1] = (alphas >> 8) & 0xff;
    if (a[0] > a[1]) {
        a[2] = (6*a[0] + 1*a[1]) / 7;
        a[3] = (5*a[0] + 2*a[1]) / 7;
        a[4] = (4*a[0] + 3*a[1]) / 7;
        a[5] = (3*a[0] + 4*a[1]) / 7;
        a[6] = (2*a[0] + 5*a[1]) / 7;
        a[7] = (1*a[0] + 6*a[1]) / 7;
    } else {
        a[2] = (4*a[0] + 1*a[1]) / 5;
        a[3] = (3*a[0] + 2*a[1]) / 5;
        a[4] = (2*a[0] + 3*a[1]) / 5;
        a[5] = (1*a[0] + 4*a[1]) / 5;
        a[6] = 0;
        a[7] = 255;
    }
    alphas >>= 16;
    for (int i = 0; i < 16; i++) {
        quint8 index = alphas & 0x07;
        quint8 alpha = a[index];
        QRgb rgb = rgbArr[i];
        if (version == Four) // DXT4
            rgbArr[i] = qRgba(qRed(rgb) * alpha / 0xff, qGreen(rgb) * alpha / 0xff, qBlue(rgb) * alpha / 0xff, alpha);
        else if (version == Five) // DXT5
            rgbArr[i] = qRgba(qRed(rgb), qGreen(rgb), qBlue(rgb), alpha);
        alphas = alphas >> 3;
    }
}

template <DXTVersions version>
inline void setAlphaDXT(QRgb *rgbArr, quint64 alphas)
{
    Q_UNUSED(rgbArr);
    Q_UNUSED(alphas);
}

template <>
inline void setAlphaDXT<Two>(QRgb *rgbArr, quint64 alphas)
{
    setAlphaDXT32Helper<Two>(rgbArr, alphas);
}

template <>
inline void setAlphaDXT<Three>(QRgb *rgbArr, quint64 alphas)
{
    setAlphaDXT32Helper<Three>(rgbArr, alphas);
}

template <>
inline void setAlphaDXT<Four>(QRgb *rgbArr, quint64 alphas)
{
    setAlphaDXT45Helper<Four>(rgbArr, alphas);
}

template <>
inline void setAlphaDXT<Five>(QRgb *rgbArr, quint64 alphas)
{
    setAlphaDXT45Helper<Five>(rgbArr, alphas);
}

template <>
inline void setAlphaDXT<RXGB>(QRgb *rgbArr, quint64 alphas)
{
    setAlphaDXT45Helper<Five>(rgbArr, alphas);
}

static inline QRgb invertRXGBColors(QRgb pixel)
{
    return qRgb(qAlpha(pixel), qGreen(pixel), qBlue(pixel));
}

template <DXTVersions version>
static QImage readDXT(QDataStream &s, quint32 width, quint32 height)
{
    QImage::Format format = (version == Two || version == Four) ?
                QImage::Format_ARGB32_Premultiplied : QImage::Format_ARGB32;

    QImage image = imageAlloc(width, height, format);
    if (image.isNull()) {
        return image;
    }

    for (quint32 i = 0; i < height; i += 4) {
        for (quint32 j = 0; j < width; j += 4) {
            quint64 alpha = 0;
            quint16 c0, c1;
            quint32 table;
            if (version != One)
                s >> alpha;
            s >> c0;
            s >> c1;
            s >> table;
            if (s.status() != QDataStream::Ok)
                return QImage();

            QRgb arr[16];

            DXTFillColors(arr, c0, c1, table, version == One && c0 <= c1);
            setAlphaDXT<version>(arr, alpha);

            const quint32 kMax = qMin<quint32>(4, height - i);
            const quint32 lMax = qMin<quint32>(4, width - j);
            for (quint32 k = 0; k < kMax; k++) {
                QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(i + k));
                for (quint32 l = 0; l < lMax; l++) {
                    QRgb pixel = arr[k * 4 + l];
                    if (version == RXGB)
                        pixel = invertRXGBColors(pixel);

                    line[j + l] = pixel;
                }
            }
        }
    }
    return image;
}

static inline QImage readDXT1(QDataStream &s, quint32 width, quint32 height)
{
    return readDXT<One>(s, width, height);
}

static inline QImage readDXT2(QDataStream &s, quint32 width, quint32 height)
{
    return readDXT<Two>(s, width, height);
}

static inline QImage readDXT3(QDataStream &s, quint32 width, quint32 height)
{
    return readDXT<Three>(s, width, height);
}

static inline QImage readDXT4(QDataStream &s, quint32 width, quint32 height)
{
    return readDXT<Four>(s, width, height);
}

static inline QImage readDXT5(QDataStream &s, quint32 width, quint32 height)
{
    return readDXT<Five>(s, width, height);
}

static inline QImage readRXGB(QDataStream &s, quint32 width, quint32 height)
{
    return readDXT<RXGB>(s, width, height);
}

static QImage readATI2(QDataStream &s, quint32 width, quint32 height)
{
    QImage image = imageAlloc(width, height, QImage::Format_RGB32);
    if (image.isNull()) {
        return image;
    }

    for (quint32 i = 0; i < height; i += 4) {
        for (quint32 j = 0; j < width; j += 4) {
            quint64 alpha1;
            quint64 alpha2;
            s >> alpha1;
            s >> alpha2;
            if (s.status() != QDataStream::Ok)
                return QImage();

            QRgb arr[16];
            memset(arr, 0, sizeof(QRgb) * 16);
            setAlphaDXT<Five>(arr, alpha1);
            for (int k = 0; k < 16; ++k) {
                quint8 a = qAlpha(arr[k]);
                arr[k] = qRgba(0, 0, a, 0);
            }
            setAlphaDXT<Five>(arr, alpha2);

            const quint32 kMax = qMin<quint32>(4, height - i);
            const quint32 lMax = qMin<quint32>(4, width - j);
            for (quint32 k = 0; k < kMax; k++) {
                QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(i + k));
                for (quint32 l = 0; l < lMax; l++) {
                    QRgb pixel = arr[k * 4 + l];
                    const quint8 nx = qAlpha(pixel);
                    const quint8 ny = qBlue(pixel);
                    const quint8 nz = getNormalZ(nx, ny);
                    line[j + l] = qRgb(nx, ny, nz);
                }
            }
        }
    }
    return image;
}

static QImage readUnsignedImage(QDataStream &s, const DDSHeader &dds, quint32 width, quint32 height, bool hasAlpha)
{
    quint32 flags = dds.pixelFormat.flags;

    quint32 masks[ColorCount];
    quint8 shifts[ColorCount];
    quint8 bits[ColorCount];
    masks[Red] = dds.pixelFormat.rBitMask;
    masks[Green] = dds.pixelFormat.gBitMask;
    masks[Blue] = dds.pixelFormat.bBitMask;
    masks[Alpha] = hasAlpha ? dds.pixelFormat.aBitMask : 0;
    for (int i = 0; i < ColorCount; ++i) {
        shifts[i] = maskToShift(masks[i]);
        bits[i] = maskLength(masks[i]);

        // move mask to the left
        if (bits[i] <= 8)
            masks[i] = (masks[i] >> shifts[i]) << (8 - bits[i]);
    }

    QImage::Format format = hasAlpha ? QImage::Format_ARGB32 : QImage::Format_RGB32;
    if (!hasAlpha && (flags & DDSPixelFormat::FlagLuminance))
        format = QImage::Format_Grayscale8;
    QImage image = imageAlloc(width, height, format);
    if (image.isNull()) {
        return image;
    }

    for (quint32 y = 0; y < height; y++) {
        for (quint32 x = 0; x < width; x++) {
            quint8 *byteLine = reinterpret_cast<quint8 *>(image.scanLine(y));
            QRgb *line = reinterpret_cast<QRgb *>(byteLine);

            quint32 value = readValue(s, dds.pixelFormat.rgbBitCount);
            quint8 colors[ColorCount];

            for (int c = 0; c < ColorCount; ++c) {
                if (bits[c] > 8) {
                    // truncate unneseccary bits
                    colors[c] = (value & masks[c]) >> shifts[c] >> (bits[c] - 8);
                } else {
                    // move color to the left
                    quint8 color = value >> shifts[c] << (8 - bits[c]) & masks[c];
                    if (masks[c])
                        colors[c] = color * 0xff / masks[c];
                    else
                        colors[c] = c == Alpha ? 0xff : 0;
                }
            }

            if (flags & DDSPixelFormat::FlagLuminance) {
                if (hasAlpha)
                    line[x] = qRgba(colors[Red], colors[Red], colors[Red], colors[Alpha]);
                else
                    byteLine[x] = colors[Red];
            }
            else if (flags & DDSPixelFormat::FlagYUV) {
                line[x] = yuv2rgb(colors[Red], colors[Green], colors[Blue]);
            }
            else {
                line[x] = qRgba(colors[Red], colors[Green], colors[Blue], colors[Alpha]);
            }

            if (s.status() != QDataStream::Ok)
                return QImage();
        }
        strideAlignment(s, dds, width); // some dds seems aligned to 32 bits
    }

    return image;
}

static qfloat16 readFloat16(QDataStream &s)
{
    qfloat16 f16;

    quint16 rawData;
    s >> rawData;
    memcpy(&f16, &rawData, sizeof(rawData));

    return f16;
}

static inline float readFloat32(QDataStream &s)
{
    Q_ASSERT(sizeof(float) == 4);
    float value;
    // TODO: find better way to avoid setting precision each time
    QDataStream::FloatingPointPrecision precision = s.floatingPointPrecision();
    s.setFloatingPointPrecision(QDataStream::SinglePrecision);
    s >> value;
    s.setFloatingPointPrecision(precision);
    return value;
}

static QImage readR16F(QDataStream &s, const quint32 width, const quint32 height)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
    QImage image = imageAlloc(width, height, QImage::Format_RGBX16FPx4);
    if (image.isNull()) {
        return image;
    }

    for (quint32 y = 0; y < height; y++) {
        qfloat16 *line = reinterpret_cast<qfloat16 *>(image.scanLine(y));
        for (quint32 x = 0; x < width; x++) {
            line[x * 4] = readFloat16(s);
            line[x * 4 + 1] = 0;
            line[x * 4 + 2] = 0;
            line[x * 4 + 3] = 1;
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }
#else
    QImage image = imageAlloc(width, height, QImage::Format_RGB32);
    if (image.isNull()) {
        return image;
    }

    for (quint32 y = 0; y < height; y++) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (quint32 x = 0; x < width; x++) {
            const int r = qBound<int>(0, static_cast<int>(readFloat16(s) * 255.0f), 255);
            line[x] = qRgb(r, 0, 0);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }
#endif

    image.setColorSpace(QColorSpace(QColorSpace::SRgbLinear));
    return image;
}

static QImage readRG16F(QDataStream &s, const quint32 width, const quint32 height)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
    QImage image = imageAlloc(width, height, QImage::Format_RGBX16FPx4);
    if (image.isNull()) {
        return image;
    }

    for (quint32 y = 0; y < height; y++) {
        qfloat16 *line = reinterpret_cast<qfloat16 *>(image.scanLine(y));
        for (quint32 x = 0; x < width; x++) {
            line[x * 4] = readFloat16(s);
            line[x * 4 + 1] = readFloat16(s);
            line[x * 4 + 2] = 0;
            line[x * 4 + 3] = 1;
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }
#else
    QImage image = imageAlloc(width, height, QImage::Format_RGB32);
    if (image.isNull()) {
        return image;
    }

    for (quint32 y = 0; y < height; y++) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (quint32 x = 0; x < width; x++) {
            const int r = qBound<int>(0, static_cast<int>(readFloat16(s) * 255.0f), 255);
            const int g = qBound<int>(0, static_cast<int>(readFloat16(s) * 255.0f), 255);
            line[x] = qRgb(r, g, 0);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }
#endif

    image.setColorSpace(QColorSpace(QColorSpace::SRgbLinear));
    return image;
}

static QImage readARGB16F(QDataStream &s, const quint32 width, const quint32 height, bool alphaPremul)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
    QImage image = imageAlloc(width, height, alphaPremul ? QImage::Format_RGBA16FPx4_Premultiplied : QImage::Format_RGBA16FPx4);
    if (image.isNull()) {
        return image;
    }

    for (quint32 y = 0; y < height; y++) {
        qfloat16 *line = reinterpret_cast<qfloat16 *>(image.scanLine(y));
        for (quint32 x = 0; x < width; x++) {
            line[x * 4] = readFloat16(s);
            line[x * 4 + 1] = readFloat16(s);
            line[x * 4 + 2] = readFloat16(s);
            line[x * 4 + 3] = readFloat16(s);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }
#else
    QImage image = imageAlloc(width, height, QImage::Format_ARGB32);
    if (image.isNull()) {
        return image;
    }

    for (quint32 y = 0; y < height; y++) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (quint32 x = 0; x < width; x++) {
            const float rf = readFloat16(s);
            const float gf = readFloat16(s);
            const float bf = readFloat16(s);
            const float af = readFloat16(s);
            const int r = qBound<int>(0, static_cast<int>((alphaPremul ? (rf / af) : rf) * 255.0f), 255);
            const int g = qBound<int>(0, static_cast<int>((alphaPremul ? (gf / af) : gf) * 255.0f), 255);
            const int b = qBound<int>(0, static_cast<int>((alphaPremul ? (bf / af) : bf) * 255.0f), 255);
            const int a = qBound<int>(0, static_cast<int>(af * 255.0f), 255);
            line[x] = qRgba(r, g, b, a);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }
#endif

    image.setColorSpace(QColorSpace(QColorSpace::SRgbLinear));
    return image;
}

static QImage readR32F(QDataStream &s, const quint32 width, const quint32 height)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
    QImage image = imageAlloc(width, height, QImage::Format_RGBX32FPx4);
    if (image.isNull()) {
        return image;
    }

    for (quint32 y = 0; y < height; y++) {
        float *line = reinterpret_cast<float *>(image.scanLine(y));
        for (quint32 x = 0; x < width; x++) {
            line[x * 4] = readFloat32(s);
            line[x * 4 + 1] = 0;
            line[x * 4 + 2] = 0;
            line[x * 4 + 3] = 1;
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }
#else
    QImage image = imageAlloc(width, height, QImage::Format_RGB32);
    if (image.isNull()) {
        return image;
    }

    for (quint32 y = 0; y < height; y++) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (quint32 x = 0; x < width; x++) {
            const int r = qBound<int>(0, static_cast<int>(readFloat32(s) * 255.0f), 255);
            line[x] = qRgb(r, 0, 0);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }
#endif

    image.setColorSpace(QColorSpace(QColorSpace::SRgbLinear));
    return image;
}

static QImage readRG32F(QDataStream &s, const quint32 width, const quint32 height)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
    QImage image = imageAlloc(width, height, QImage::Format_RGBX32FPx4);
    if (image.isNull()) {
        return image;
    }

    for (quint32 y = 0; y < height; y++) {
        float *line = reinterpret_cast<float *>(image.scanLine(y));
        for (quint32 x = 0; x < width; x++) {
            line[x * 4] = readFloat32(s);
            line[x * 4 + 1] = readFloat32(s);
            line[x * 4 + 2] = 0;
            line[x * 4 + 3] = 1;
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }
#else
    QImage image = imageAlloc(width, height, QImage::Format_RGB32);
    if (image.isNull()) {
        return image;
    }

    for (quint32 y = 0; y < height; y++) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (quint32 x = 0; x < width; x++) {
            const int r = qBound<int>(0, static_cast<int>(readFloat32(s) * 255.0f), 255);
            const int g = qBound<int>(0, static_cast<int>(readFloat32(s) * 255.0f), 255);
            line[x] = qRgb(r, g, 0);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }
#endif

    image.setColorSpace(QColorSpace(QColorSpace::SRgbLinear));
    return image;
}

static QImage readARGB32F(QDataStream &s, const quint32 width, const quint32 height, bool alphaPremul)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
    QImage image = imageAlloc(width, height, alphaPremul ? QImage::Format_RGBA32FPx4_Premultiplied : QImage::Format_RGBA32FPx4);
    if (image.isNull()) {
        return image;
    }

    for (quint32 y = 0; y < height; y++) {
        float *line = reinterpret_cast<float *>(image.scanLine(y));
        for (quint32 x = 0; x < width; x++) {
            line[x * 4] = readFloat32(s);
            line[x * 4 + 1] = readFloat32(s);
            line[x * 4 + 2] = readFloat32(s);
            line[x * 4 + 3] = readFloat32(s);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }
#else
    QImage image = imageAlloc(width, height, QImage::Format_ARGB32);
    if (image.isNull()) {
        return image;
    }

    for (quint32 y = 0; y < height; y++) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (quint32 x = 0; x < width; x++) {
            const float rf = readFloat32(s);
            const float gf = readFloat32(s);
            const float bf = readFloat32(s);
            const float af = readFloat32(s);
            const int r = qBound<int>(0, static_cast<int>((alphaPremul ? (rf / af) : rf) * 255.0f), 255);
            const int g = qBound<int>(0, static_cast<int>((alphaPremul ? (gf / af) : gf) * 255.0f), 255);
            const int b = qBound<int>(0, static_cast<int>((alphaPremul ? (bf / af) : bf) * 255.0f), 255);
            const int a = qBound<int>(0, static_cast<int>(af * 255.0f), 255);
            line[x] = qRgba(r, g, b, a);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }
#endif

    image.setColorSpace(QColorSpace(QColorSpace::SRgbLinear));
    return image;
}

static QImage readQ16W16V16U16(QDataStream &s, const quint32 width, const quint32 height)
{
    QImage image = imageAlloc(width, height, QImage::Format_ARGB32);
    if (image.isNull()) {
        return image;
    }

    quint8 colors[ColorCount];
    qint16 tmp;
    for (quint32 y = 0; y < height; y++) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (quint32 x = 0; x < width; x++) {
            for (int i = 0; i < ColorCount; i++) {
                s >> tmp;
                colors[i] = (tmp + 0x7FFF) >> 8;
            }
            line[x] = qRgba(colors[Red], colors[Green], colors[Blue], colors[Alpha]);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }

    return image;
}

static QImage readCxV8U8(QDataStream &s, const quint32 width, const quint32 height)
{
    QImage image = imageAlloc(width, height, QImage::Format_RGB32);
    if (image.isNull()) {
        return image;
    }

    for (quint32 y = 0; y < height; y++) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (quint32 x = 0; x < width; x++) {
            qint8 v, u;
            s >> v >> u;

            const quint8 vn = v + 128;
            const quint8 un = u + 128;
            const quint8 c = getNormalZ(vn, un);

            line[x] = qRgb(vn, un, c);

            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }

    return image;
}

static QImage readPalette8Image(QDataStream &s, const DDSHeader &dds, quint32 width, quint32 height)
{
    QImage image = imageAlloc(width, height, QImage::Format_Indexed8);
    if (image.isNull()) {
        return image;
    }

    for (int i = 0; i < 256; ++i) {
        quint8 r, g, b, a;
        s >> r >> g >> b >> a;
        image.setColor(i, qRgba(r, g, b, a));
    }

    for (quint32 y = 0; y < height; y++) {
        quint8 *scanLine = reinterpret_cast<quint8 *>(image.scanLine(y));
        for (quint32 x = 0; x < width; x++) {
            quint32 value = readValue(s, dds.pixelFormat.rgbBitCount);
            if (s.status() != QDataStream::Ok)
                return QImage();
            scanLine[x] = (value & 0xff); // any alpha channel discarded
        }
    }

    return image;
}

static QImage readPalette4Image(QDataStream &s, quint32 width, quint32 height)
{
    QImage image = imageAlloc(width, height, QImage::Format_Indexed8);
    if (image.isNull()) {
        return image;
    }

    for (int i = 0; i < 16; ++i) {
        quint8 r, g, b, a;
        s >> r >> g >> b >> a;
        image.setColor(i, qRgba(r, g, b, a));
    }

    for (quint32 y = 0; y < height; y++) {
        quint8 index;
        for (quint32 x = 0; x < width - 1; ) {
            s >> index;
            image.setPixel(x++, y, (index & 0x0f) >> 0);
            image.setPixel(x++, y, (index & 0xf0) >> 4);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
        if (width % 2 == 1) {
            s >> index;
            image.setPixel(width - 1, y, (index & 0x0f) >> 0);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }

    return image;
}

static QImage readARGB16(QDataStream &s, quint32 width, quint32 height)
{
    QImage image = imageAlloc(width, height, QImage::Format_ARGB32);
    if (image.isNull()) {
        return image;
    }

    for (quint32 y = 0; y < height; y++) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (quint32 x = 0; x < width; x++) {
            quint8 colors[ColorCount];
            for (int i = 0; i < ColorCount; ++i) {
                quint16 color;
                s >> color;
                colors[i] = quint8(color >> 8);
            }
            line[x] = qRgba(colors[Red], colors[Green], colors[Blue], colors[Alpha]);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }

    return image;
}

static QImage readV8U8(QDataStream &s, quint32 width, quint32 height)
{
    QImage image = imageAlloc(width, height, QImage::Format_RGB32);
    if (image.isNull()) {
        return image;
    }

    for (quint32 y = 0; y < height; y++) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (quint32 x = 0; x < width; x++) {
            qint8 v, u;
            s >> v >> u;
            line[x] = qRgb(v + 128, u + 128, 255);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }

    return image;
}

static QImage readL6V5U5(QDataStream &s, quint32 width, quint32 height)
{
    QImage image = imageAlloc(width, height, QImage::Format_ARGB32);
    if (image.isNull()) {
        return image;
    }

    quint16 tmp;
    for (quint32 y = 0; y < height; y++) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (quint32 x = 0; x < width; x++) {
            s >> tmp;
            quint8 r = qint8((tmp & 0x001f) >> 0) * 0xff/0x1f + 128;
            quint8 g = qint8((tmp & 0x03e0) >> 5) * 0xff/0x1f + 128;
            quint8 b = quint8((tmp & 0xfc00) >> 10) * 0xff/0x3f;
            line[x] = qRgba(r, g, 0xff, b);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }
    return image;
}

static QImage readX8L8V8U8(QDataStream &s, quint32 width, quint32 height)
{
    QImage image = imageAlloc(width, height, QImage::Format_ARGB32);
    if (image.isNull()) {
        return image;
    }

    quint8 a, l;
    qint8 v, u;
    for (quint32 y = 0; y < height; y++) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (quint32 x = 0; x < width; x++) {
            s >> v >> u >> a >> l;
            line[x] = qRgba(v + 128, u + 128, 255, a);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }

    return image;
}

static QImage readQ8W8V8U8(QDataStream &s, quint32 width, quint32 height)
{
    QImage image = imageAlloc(width, height, QImage::Format_ARGB32);
    if (image.isNull()) {
        return image;
    }

    quint8 colors[ColorCount];
    qint8 tmp;
    for (quint32 y = 0; y < height; y++) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (quint32 x = 0; x < width; x++) {
            for (int i = 0; i < ColorCount; i++) {
                s >> tmp;
                colors[i] = tmp + 128;
            }
            line[x] = qRgba(colors[Red], colors[Green], colors[Blue], colors[Alpha]);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }

    return image;
}

static QImage readV16U16(QDataStream &s, quint32 width, quint32 height)
{
    QImage image = imageAlloc(width, height, QImage::Format_RGB32);
    if (image.isNull()) {
        return image;
    }

    for (quint32 y = 0; y < height; y++) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (quint32 x = 0; x < width; x++) {
            qint16 v, u;
            s >> v >> u;
            v = (v + 0x8000) >> 8;
            u = (u + 0x8000) >> 8;
            line[x] = qRgb(v, u, 255);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }

    return image;
}

static QImage readA2W10V10U10(QDataStream &s, quint32 width, quint32 height)
{
    QImage image = imageAlloc(width, height, QImage::Format_ARGB32);
    if (image.isNull()) {
        return image;
    }

    quint32 tmp;
    for (quint32 y = 0; y < height; y++) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (quint32 x = 0; x < width; x++) {
            s >> tmp;
            quint8 r = qint8((tmp & 0x3ff00000) >> 20 >> 2) + 128;
            quint8 g = qint8((tmp & 0x000ffc00) >> 10 >> 2) + 128;
            quint8 b = qint8((tmp & 0x000003ff) >> 0 >> 2) + 128;
            quint8 a = 0xff * ((tmp & 0xc0000000) >> 30) / 3;
            // dunno why we should swap b and r here
            std::swap(b, r);
            line[x] = qRgba(r, g, b, a);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }

    return image;
}

static QImage readUYVY(QDataStream &s, quint32 width, quint32 height)
{
    QImage image = imageAlloc(width, height, QImage::Format_RGB32);
    if (image.isNull()) {
        return image;
    }

    quint8 uyvy[4];
    for (quint32 y = 0; y < height; y++) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (quint32 x = 0; x < width - 1; ) {
            s >> uyvy[0] >> uyvy[1] >> uyvy[2] >> uyvy[3];
            line[x++] = yuv2rgb(uyvy[1], uyvy[0], uyvy[2]);
            line[x++] = yuv2rgb(uyvy[3], uyvy[0], uyvy[2]);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
        if (width % 2 == 1) {
            s >> uyvy[0] >> uyvy[1] >> uyvy[2] >> uyvy[3];
            line[width - 1] = yuv2rgb(uyvy[1], uyvy[0], uyvy[2]);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }

    return image;
}

static QImage readR8G8B8G8(QDataStream &s, quint32 width, quint32 height)
{
    QImage image = imageAlloc(width, height, QImage::Format_RGB32);
    if (image.isNull()) {
        return image;
    }

    quint8 rgbg[4];
    for (quint32 y = 0; y < height; y++) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (quint32 x = 0; x < width - 1; ) {
            s >> rgbg[1] >> rgbg[0] >> rgbg[3] >> rgbg[2];
            line[x++] = qRgb(rgbg[0], rgbg[1], rgbg[2]);
            line[x++] = qRgb(rgbg[0], rgbg[3], rgbg[2]);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
        if (width % 2 == 1) {
            s >> rgbg[1] >> rgbg[0] >> rgbg[3] >> rgbg[2];
            line[width - 1] = qRgb(rgbg[0], rgbg[1], rgbg[2]);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }

    return image;
}

static QImage readYUY2(QDataStream &s, quint32 width, quint32 height)
{
    QImage image = imageAlloc(width, height, QImage::Format_RGB32);
    if (image.isNull()) {
        return image;
    }

    quint8 yuyv[4];
    for (quint32 y = 0; y < height; y++) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (quint32 x = 0; x < width - 1; ) {
            s >> yuyv[0] >> yuyv[1] >> yuyv[2] >> yuyv[3];
            line[x++] = yuv2rgb(yuyv[0], yuyv[1], yuyv[3]);
            line[x++] = yuv2rgb(yuyv[2], yuyv[1], yuyv[3]);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
        if (width % 2 == 1) {
            s >> yuyv[0] >> yuyv[1] >> yuyv[2] >> yuyv[3];
            line[width - 1] = yuv2rgb(yuyv[2], yuyv[1], yuyv[3]);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }

    return image;
}

static QImage readG8R8G8B8(QDataStream &s, quint32 width, quint32 height)
{
    QImage image = imageAlloc(width, height, QImage::Format_RGB32);
    if (image.isNull()) {
        return image;
    }

    quint8 grgb[4];
    for (quint32 y = 0; y < height; y++) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (quint32 x = 0; x < width - 1; ) {
            s >> grgb[1] >> grgb[0] >> grgb[3] >> grgb[2];
            line[x++] = qRgb(grgb[1], grgb[0], grgb[3]);
            line[x++] = qRgb(grgb[1], grgb[2], grgb[3]);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
        if (width % 2 == 1) {
            s >> grgb[1] >> grgb[0] >> grgb[3] >> grgb[2];
            line[width - 1] = qRgb(grgb[1], grgb[0], grgb[3]);
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }

    return image;
}

static QImage readA2R10G10B10(QDataStream &s, const DDSHeader &dds, quint32 width, quint32 height)
{
    QImage image = readUnsignedImage(s, dds, width, height, true);
    if (image.isNull()) {
        return image;
    }

    for (quint32 y = 0; y < height; y++) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (quint32 x = 0; x < width; x++) {
            QRgb pixel = image.pixel(x, y);
            line[x] = qRgba(qBlue(pixel), qGreen(pixel), qRed(pixel), qAlpha(pixel));
            if (s.status() != QDataStream::Ok)
                return QImage();
        }
    }
    return image;
}

static QImage readLayer(QDataStream &s, const DDSHeader &dds, const int format, quint32 width, quint32 height)
{
    if (width * height == 0)
        return QImage();

    bool alphaPremul = dds.header10.miscFlags2 == DXGIAlphaModePremultiplied;

    switch (format) {
    case FormatR8G8B8:
    case FormatX8R8G8B8:
    case FormatR5G6B5:
    case FormatR3G3B2:
    case FormatX1R5G5B5:
    case FormatX4R4G4B4:
    case FormatX8B8G8R8:
    case FormatG16R16:
    case FormatL8:
    case FormatL16:
        return readUnsignedImage(s, dds, width, height, false);
    case FormatA8R8G8B8:
    case FormatA1R5G5B5:
    case FormatA4R4G4B4:
    case FormatA8:
    case FormatA8R3G3B2:
    case FormatA8B8G8R8:
    case FormatA8L8:
    case FormatA4L4:
        return readUnsignedImage(s, dds, width, height, true);
    case FormatA2R10G10B10:
    case FormatA2B10G10R10:
        return readA2R10G10B10(s, dds, width, height);
    case FormatP8:
    case FormatA8P8:
        return readPalette8Image(s, dds, width, height);
    case FormatP4:
    case FormatA4P4:
        return readPalette4Image(s, width, height);
    case FormatA16B16G16R16:
        return readARGB16(s, width, height);
    case FormatV8U8:
        return readV8U8(s, width, height);
    case FormatL6V5U5:
        return readL6V5U5(s, width, height);
    case FormatX8L8V8U8:
        return readX8L8V8U8(s, width, height);
    case FormatQ8W8V8U8:
        return readQ8W8V8U8(s, width, height);
    case FormatV16U16:
        return readV16U16(s, width, height);
    case FormatA2W10V10U10:
        return readA2W10V10U10(s, width, height);
    case FormatUYVY:
        return readUYVY(s, width, height);
    case FormatR8G8B8G8:
        return readR8G8B8G8(s, width, height);
    case FormatYUY2:
        return readYUY2(s, width, height);
    case FormatG8R8G8B8:
        return readG8R8G8B8(s, width, height);
    case FormatDXT1:
        return readDXT1(s, width, height);
    case FormatDXT2:
        return readDXT2(s, width, height);
    case FormatDXT3:
        return readDXT3(s, width, height);
    case FormatDXT4:
        return readDXT4(s, width, height);
    case FormatDXT5:
        return readDXT5(s, width, height);
    case FormatRXGB:
        return readRXGB(s, width, height);
    case FormatATI2:
        return readATI2(s, width, height);
    case FormatR16F:
        return readR16F(s, width, height);
    case FormatG16R16F:
        return readRG16F(s, width, height);
    case FormatA16B16G16R16F:
        return readARGB16F(s, width, height, alphaPremul);
    case FormatR32F:
        return readR32F(s, width, height);
    case FormatG32R32F:
        return readRG32F(s, width, height);
    case FormatA32B32G32R32F:
        return readARGB32F(s, width, height, alphaPremul);
    case FormatD16Lockable:
    case FormatD32:
    case FormatD15S1:
    case FormatD24S8:
    case FormatD24X8:
    case FormatD24X4S4:
    case FormatD16:
    case FormatD32FLockable:
    case FormatD24FS8:
    case FormatD32Lockable:
    case FormatS8Lockable:
    case FormatVertexData:
    case FormatIndex16:
    case FormatIndex32:
        break;
    case FormatQ16W16V16U16:
        return readQ16W16V16U16(s, width, height);
    case FormatMulti2ARGB8:
        break;
    case FormatCxV8U8:
        return readCxV8U8(s, width, height);
    case FormatA1:
    case FormatA2B10G10R10_XR_BIAS:
    case FormatBinaryBuffer:
    case FormatLast:
        break;
    }

    return QImage();
}

static inline QImage readTexture(QDataStream &s, const DDSHeader &dds, const int format, const int mipmapLevel)
{
    quint32 width = dds.width / (1 << mipmapLevel);
    quint32 height = dds.height / (1 << mipmapLevel);
    return readLayer(s, dds, format, width, height);
}

static qint64 mipmapSize(const DDSHeader &dds, const int format, const int level)
{
    quint32 w = dds.width/(1 << level);
    quint32 h = dds.height/(1 << level);

    switch (format) {
    case FormatR8G8B8:
    case FormatX8R8G8B8:
    case FormatR5G6B5:
    case FormatX1R5G5B5:
    case FormatX4R4G4B4:
    case FormatX8B8G8R8:
    case FormatG16R16:
    case FormatL8:
    case FormatL16:
        return w * h * dds.pixelFormat.rgbBitCount / 8;
    case FormatA8R8G8B8:
    case FormatA1R5G5B5:
    case FormatA4R4G4B4:
    case FormatA8:
    case FormatA8R3G3B2:
    case FormatA2B10G10R10:
    case FormatA8B8G8R8:
    case FormatA2R10G10B10:
    case FormatA8L8:
    case FormatA4L4:
        return w * h * dds.pixelFormat.rgbBitCount / 8;
    case FormatP8:
        return 256 + w * h * 8;
    case FormatA16B16G16R16:
        return w * h * 4 * 2;
    case FormatA8P8:
        break;
    case FormatV8U8:
    case FormatL6V5U5:
        return w * h * 2;
    case FormatX8L8V8U8:
    case FormatQ8W8V8U8:
    case FormatV16U16:
    case FormatA2W10V10U10:
        return w * h * 4;
    case FormatUYVY:
    case FormatR8G8B8G8:
    case FormatYUY2:
    case FormatG8R8G8B8:
        return w * h * 2;
    case FormatDXT1:
        return ((w + 3)/4) * ((h + 3)/4) * 8;
    case FormatDXT2:
    case FormatDXT3:
    case FormatDXT4:
    case FormatDXT5:
        return ((w + 3)/4) * ((h + 3)/4) * 16;
    case FormatD16Lockable:
    case FormatD32:
    case FormatD15S1:
    case FormatD24S8:
    case FormatD24X8:
    case FormatD24X4S4:
    case FormatD16:
    case FormatD32FLockable:
    case FormatD24FS8:
    case FormatD32Lockable:
    case FormatS8Lockable:
    case FormatVertexData:
    case FormatIndex16:
    case FormatIndex32:
        break;
    case FormatQ16W16V16U16:
        return w * h * 4 * 2;
    case FormatMulti2ARGB8:
        break;
    case FormatR16F:
        return w * h * 1 * 2;
    case FormatG16R16F:
        return w * h * 2 * 2;
    case FormatA16B16G16R16F:
        return w * h * 4 * 2;
    case FormatR32F:
        return w * h * 1 * 4;
    case FormatG32R32F:
        return w * h * 2 * 4;
    case FormatA32B32G32R32F:
        return w * h * 4 * 4;
    case FormatCxV8U8:
        return w * h * 2;
    case FormatA1:
    case FormatA2B10G10R10_XR_BIAS:
    case FormatBinaryBuffer:
    case FormatLast:
        break;
    }

    return 0;
}

static qint64 mipmapOffset(const DDSHeader &dds, const int format, const int level)
{
    qint64 result = 0;
    for (int i = 0; i < level; ++i)
        result += mipmapSize(dds, format, i);
    return result;
}

static QImage readCubeMap(QDataStream &s, const DDSHeader &dds, const int fmt)
{
    QImage::Format format = hasAlpha(dds) ? QImage::Format_ARGB32 : QImage::Format_RGB32;
    QImage image = imageAlloc(4 * dds.width, 3 * dds.height, format);
    if (image.isNull()) {
        return image;
    }

    image.fill(0);

    for (int i = 0; i < 6; i++) {
        if (!(dds.caps2 & faceFlags[i]))
            continue; // Skip face.

        QImage face = readLayer(s, dds, fmt, dds.width, dds.height);
        if (face.isNull()) {
            return {};
        }
        face.convertTo(format);
        if (face.isNull()) {
            return {};
        }
        if (face.colorSpace().isValid()) {
            image.setColorSpace(face.colorSpace());
        }

        // Compute face offsets.
        int offset_x = faceOffsets[i].x * dds.width;
        int offset_y = faceOffsets[i].y * dds.height;

        // Copy face on the image.
        for (quint32 y = 0; y < dds.height; y++) {
            if (y + offset_y >= quint32(image.height())) {
                return {};
            }
            const QRgb *src = reinterpret_cast<const QRgb *>(face.constScanLine(y));
            QRgb *dst = reinterpret_cast<QRgb *>(image.scanLine(y + offset_y)) + offset_x;
            qsizetype sz = sizeof(QRgb) * dds.width;
            if (ptrDiff(face.bits() + face.sizeInBytes(), src) < sz || ptrDiff(image.bits() + image.sizeInBytes(), dst) < sz) {
                return {};
            }
            memcpy(dst, src, sz);
        }
    }

    return image;
}

static QByteArray formatName(int format)
{
    for (size_t i = 0; i < formatNamesSize; ++i) {
        if (formatNames[i].format == format)
            return formatNames[i].name;
    }

    return formatNames[0].name;
}

static int formatByName(const QByteArray &name)
{
    const QByteArray loweredName = name.toLower();
    for (size_t i = 0; i < formatNamesSize; ++i) {
        if (QByteArray(formatNames[i].name).toLower() == loweredName)
            return formatNames[i].format;
    }

    return FormatUnknown;
}

QDDSHandler::QDDSHandler() :
    m_header(),
    m_format(FormatUnknown),
    m_currentImage(0),
    m_scanState(ScanNotScanned)
{
}

bool QDDSHandler::canRead() const
{
    if (m_scanState == ScanNotScanned && !canRead(device()))
        return false;

    if (m_scanState != ScanError) {
        setFormat(QByteArrayLiteral("dds"));
        return true;
    }

    return false;
}

bool QDDSHandler::read(QImage *outImage)
{
    if (!ensureScanned() || device()->isSequential())
        return false;

    qint64 pos = headerSize;
    if (m_header.pixelFormat.fourCC == dx10Magic)
        pos += 20;
    pos += mipmapOffset(m_header, m_format, m_currentImage);
    if (!device()->seek(pos))
        return false;
    QDataStream s(device());
    s.setByteOrder(QDataStream::LittleEndian);

    QImage image = isCubeMap(m_header) ?
                readCubeMap(s, m_header, m_format) :
                readTexture(s, m_header, m_format, m_currentImage);
    if (image.isNull()) {
        return false;
    }

    bool ok = s.status() == QDataStream::Ok && !image.isNull();
    if (ok)
        *outImage = image;
    return ok;
}

bool writeA8R8G8B8(const QImage &outImage, QDataStream &s)
{
    DDSHeader dds;
    // Filling header
    dds.magic = ddsMagic;
    dds.size = 124;
    dds.flags = DDSHeader::FlagCaps | DDSHeader::FlagHeight |
                DDSHeader::FlagWidth | DDSHeader::FlagPixelFormat |
                DDSHeader::FlagPitch;
    dds.height = outImage.height();
    dds.width = outImage.width();
    dds.pitchOrLinearSize = dds.width * 32 / 8;
    dds.depth = 0;
    dds.mipMapCount = 0;
    for (int i = 0; i < DDSHeader::ReservedCount; i++)
        dds.reserved1[i] = 0;
    dds.caps = DDSHeader::CapsTexture;
    dds.caps2 = 0;
    dds.caps3 = 0;
    dds.caps4 = 0;
    dds.reserved2 = 0;

    // Filling pixelformat
    dds.pixelFormat.size = 32;
    dds.pixelFormat.flags = DDSPixelFormat::FlagAlphaPixels | DDSPixelFormat::FlagRGB;
    dds.pixelFormat.fourCC = 0;
    dds.pixelFormat.rgbBitCount = 32;
    dds.pixelFormat.aBitMask = 0xff000000;
    dds.pixelFormat.rBitMask = 0x00ff0000;
    dds.pixelFormat.gBitMask = 0x0000ff00;
    dds.pixelFormat.bBitMask = 0x000000ff;

    s << dds;
    if (s.status() != QDataStream::Ok) {
        return false;
    }

    ScanLineConverter slc(QImage::Format_ARGB32);
    if(outImage.colorSpace().isValid())
        slc.setTargetColorSpace(QColorSpace(QColorSpace::SRgb));

    for (int y = 0, h = outImage.height(); y < h; ++y) {
        const QRgb *scanLine = reinterpret_cast<const QRgb*>(slc.convertedScanLine(outImage, y));
        if (scanLine == nullptr) {
            return false;
        }
        for (int x = 0, w = outImage.width(); x < w; ++x) {
            s << quint32(scanLine[x]);
        }
        if (s.status() != QDataStream::Ok) {
            return false;
        }
    }

    return true;
}

bool writeR8G8B8(const QImage &outImage, QDataStream &s)
{
    DDSHeader dds;
    // Filling header
    dds.magic = ddsMagic;
    dds.size = 124;
    dds.flags = DDSHeader::FlagCaps | DDSHeader::FlagHeight |
                DDSHeader::FlagWidth | DDSHeader::FlagPixelFormat |
                DDSHeader::FlagPitch;
    dds.height = outImage.height();
    dds.width = outImage.width();
    dds.pitchOrLinearSize = dds.width * 24 / 8;
    dds.depth = 1;
    dds.mipMapCount = 0;
    for (int i = 0; i < DDSHeader::ReservedCount; i++)
        dds.reserved1[i] = 0;
    dds.caps = DDSHeader::CapsTexture;
    dds.caps2 = 0;
    dds.caps3 = 0;
    dds.caps4 = 0;
    dds.reserved2 = 0;

    // Filling pixelformat
    dds.pixelFormat.size = 32;
    dds.pixelFormat.flags = DDSPixelFormat::FlagRGB;
    dds.pixelFormat.fourCC = 0;
    dds.pixelFormat.rgbBitCount = 24;
    dds.pixelFormat.aBitMask = 0x00000000;
    dds.pixelFormat.rBitMask = 0x00ff0000;
    dds.pixelFormat.gBitMask = 0x0000ff00;
    dds.pixelFormat.bBitMask = 0x000000ff;

    s << dds;
    if (s.status() != QDataStream::Ok) {
        return false;
    }

    ScanLineConverter slc(QImage::Format_RGB888);
    if(outImage.colorSpace().isValid())
        slc.setTargetColorSpace(QColorSpace(QColorSpace::SRgb));

    for (int y = 0, h = outImage.height(); y < h; ++y) {
        const quint8 *scanLine = reinterpret_cast<const quint8*>(slc.convertedScanLine(outImage, y));
        if (scanLine == nullptr) {
            return false;
        }
        for (int x = 0, w = outImage.width(); x < w; ++x) {
            size_t x3 = size_t(x) * 3;
            s << scanLine[x3 + 2];
            s << scanLine[x3 + 1];
            s << scanLine[x3];
        }
        if (s.status() != QDataStream::Ok) {
            return false;
        }
    }

    return true;
}

bool writeL8(const QImage &outImage, QDataStream &s)
{
    DDSHeader dds;
    // Filling header
    dds.magic = ddsMagic;
    dds.size = 124;
    dds.flags = DDSHeader::FlagCaps | DDSHeader::FlagHeight |
                DDSHeader::FlagWidth | DDSHeader::FlagPixelFormat |
                DDSHeader::FlagPitch;
    dds.height = outImage.height();
    dds.width = outImage.width();
    dds.pitchOrLinearSize = dds.width;
    dds.depth = 1;
    dds.mipMapCount = 0;
    for (int i = 0; i < DDSHeader::ReservedCount; i++)
        dds.reserved1[i] = 0;
    dds.caps = DDSHeader::CapsTexture;
    dds.caps2 = 0;
    dds.caps3 = 0;
    dds.caps4 = 0;
    dds.reserved2 = 0;

    // Filling pixelformat
    dds.pixelFormat.size = 32;
    dds.pixelFormat.flags = DDSPixelFormat::FlagLuminance | DDSPixelFormat::FlagRGB;
    dds.pixelFormat.fourCC = 0;
    dds.pixelFormat.rgbBitCount = 8;
    dds.pixelFormat.aBitMask = 0x00000000;
    dds.pixelFormat.rBitMask = 0x000000ff;
    dds.pixelFormat.gBitMask = 0x00000000;
    dds.pixelFormat.bBitMask = 0x00000000;

    s << dds;
    if (s.status() != QDataStream::Ok) {
        return false;
    }

    ScanLineConverter slc(QImage::Format_Grayscale8);
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    if(outImage.colorSpace().isValid())
        slc.setTargetColorSpace(QColorSpace(QPointF(0.3127, 0.3291), QColorSpace::TransferFunction::SRgb));
#endif

    for (int y = 0, h = outImage.height(); y < h; ++y) {
        const quint8 *scanLine = reinterpret_cast<const quint8*>(slc.convertedScanLine(outImage, y));
        if (scanLine == nullptr) {
            return false;
        }
        for (int x = 0, w = outImage.width(); x < w; ++x) {
            s << scanLine[x];
        }
        if (s.status() != QDataStream::Ok) {
            return false;
        }
    }

    return true;
}

bool writeP8(const QImage &image, QDataStream &s)
{
    QImage outImage = image;
    // indexed not supported by ScanlineConverter class
    if (image.format() != QImage::Format_Indexed8) {
        if (image.colorSpace().isValid())
            outImage.convertToColorSpace(QColorSpace(QColorSpace::SRgb));
        outImage = outImage.convertToFormat(QImage::Format_Indexed8);
    }

    DDSHeader dds;
    // Filling header
    dds.magic = ddsMagic;
    dds.size = 124;
    dds.flags = DDSHeader::FlagCaps | DDSHeader::FlagHeight |
                DDSHeader::FlagWidth | DDSHeader::FlagPixelFormat |
                DDSHeader::FlagPitch;
    dds.height = outImage.height();
    dds.width = outImage.width();
    dds.pitchOrLinearSize = dds.width;
    dds.depth = 1;
    dds.mipMapCount = 0;
    for (int i = 0; i < DDSHeader::ReservedCount; i++)
        dds.reserved1[i] = 0;
    dds.caps = DDSHeader::CapsTexture;
    dds.caps2 = 0;
    dds.caps3 = 0;
    dds.caps4 = 0;
    dds.reserved2 = 0;

    // Filling pixelformat
    dds.pixelFormat.size = 32;
    dds.pixelFormat.flags = DDSPixelFormat::FlagPaletteIndexed8;
    dds.pixelFormat.fourCC = 0;
    dds.pixelFormat.rgbBitCount = 8;
    dds.pixelFormat.aBitMask = 0x00000000;
    dds.pixelFormat.rBitMask = 0x00000000;
    dds.pixelFormat.gBitMask = 0x00000000;
    dds.pixelFormat.bBitMask = 0x00000000;

    s << dds;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QList<QRgb> palette = outImage.colorTable();
#else
    QVector<QRgb> palette = outImage.colorTable();
#endif
    for (int i = 0; i < 256; ++i) {
        quint8 r = 0, g = 0, b = 0, a = 0xff;
        if (i < palette.size()) {
            auto&& rgba = palette.at(i);
            r = qRed(rgba);
            g = qGreen(rgba);
            b = qBlue(rgba);
            a = qAlpha(rgba);
        }
        s << r;
        s << g;
        s << b;
        s << a;
    }

    if (s.status() != QDataStream::Ok) {
        return false;
    }

    for (int y = 0, h = outImage.height(); y < h; ++y) {
        const quint8 *scanLine = reinterpret_cast<const quint8*>(outImage.constScanLine(y));
        if (scanLine == nullptr) {
            return false;
        }
        for (int x = 0, w = outImage.width(); x < w; ++x) {
            s << scanLine[x];
        }
        if (s.status() != QDataStream::Ok) {
            return false;
        }
    }

    return true;
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
bool writeA16B16G16R16F(const QImage &outImage, QDataStream &s)
{
    DDSHeader dds;
    // Filling header
    dds.magic = ddsMagic;
    dds.size = 124;
    dds.flags = DDSHeader::FlagCaps | DDSHeader::FlagHeight |
                DDSHeader::FlagWidth | DDSHeader::FlagPitch |
                DDSHeader::FlagPixelFormat;
    dds.height = outImage.height();
    dds.width = outImage.width();
    dds.pitchOrLinearSize = dds.width * 64 / 8;
    dds.depth = 1;
    dds.mipMapCount = 0;
    for (int i = 0; i < DDSHeader::ReservedCount; i++)
        dds.reserved1[i] = 0;
    dds.caps = DDSHeader::CapsTexture;
    dds.caps2 = 0;
    dds.caps3 = 0;
    dds.caps4 = 0;
    dds.reserved2 = 0;

    // Filling pixelformat
    dds.pixelFormat.size = 32;
    dds.pixelFormat.flags = DDSPixelFormat::FlagFourCC;
    dds.pixelFormat.fourCC = 113;
    dds.pixelFormat.rgbBitCount = 0;
    dds.pixelFormat.aBitMask = 0;
    dds.pixelFormat.rBitMask = 0;
    dds.pixelFormat.gBitMask = 0;
    dds.pixelFormat.bBitMask = 0;

    s << dds;
    if (s.status() != QDataStream::Ok) {
        return false;
    }

    ScanLineConverter slc(QImage::Format_RGBA16FPx4);
    if(outImage.colorSpace().isValid())
        slc.setTargetColorSpace(QColorSpace(QColorSpace::SRgbLinear));

    for (int y = 0, h = outImage.height(); y < h; ++y) {
        const quint16 *scanLine = reinterpret_cast<const quint16*>(slc.convertedScanLine(outImage, y));
        if (scanLine == nullptr) {
            return false;
        }
        for (int x = 0, w = outImage.width(); x < w; ++x) {
            size_t x4 = size_t(x) * 4;
            s << scanLine[x4];
            s << scanLine[x4 + 1];
            s << scanLine[x4 + 2];
            s << scanLine[x4 + 3];
        }
        if (s.status() != QDataStream::Ok) {
            return false;
        }
    }

    return true;
}

bool writeA32B32G32R32F(const QImage &outImage, QDataStream &s)
{
    DDSHeader dds;
    // Filling header
    dds.magic = ddsMagic;
    dds.size = 124;
    dds.flags = DDSHeader::FlagCaps | DDSHeader::FlagHeight |
                DDSHeader::FlagWidth | DDSHeader::FlagPitch |
                DDSHeader::FlagPixelFormat;
    dds.height = outImage.height();
    dds.width = outImage.width();
    dds.pitchOrLinearSize = dds.width * 128 / 8;
    dds.depth = 1;
    dds.mipMapCount = 0;
    for (int i = 0; i < DDSHeader::ReservedCount; i++)
        dds.reserved1[i] = 0;
    dds.caps = DDSHeader::CapsTexture;
    dds.caps2 = 0;
    dds.caps3 = 0;
    dds.caps4 = 0;
    dds.reserved2 = 0;

    // Filling pixelformat
    dds.pixelFormat.size = 32;
    dds.pixelFormat.flags = DDSPixelFormat::FlagFourCC;
    dds.pixelFormat.fourCC = 116;
    dds.pixelFormat.rgbBitCount = 0;
    dds.pixelFormat.aBitMask = 0;
    dds.pixelFormat.rBitMask = 0;
    dds.pixelFormat.gBitMask = 0;
    dds.pixelFormat.bBitMask = 0;

    s << dds;
    if (s.status() != QDataStream::Ok) {
        return false;
    }

    ScanLineConverter slc(QImage::Format_RGBA32FPx4);
    if(outImage.colorSpace().isValid())
        slc.setTargetColorSpace(QColorSpace(QColorSpace::SRgbLinear));

    for (int y = 0, h = outImage.height(); y < h; ++y) {
        const quint32 *scanLine = reinterpret_cast<const quint32*>(slc.convertedScanLine(outImage, y));
        if (scanLine == nullptr) {
            return false;
        }
        for (int x = 0, w = outImage.width(); x < w; ++x) {
            size_t x4 = size_t(x) * 4;
            s << scanLine[x4];
            s << scanLine[x4 + 1];
            s << scanLine[x4 + 2];
            s << scanLine[x4 + 3];
        }
        if (s.status() != QDataStream::Ok) {
            return false;
        }
    }

    return true;
}
#endif

bool QDDSHandler::write(const QImage &outImage)
{
    if (outImage.isNull() || device() == nullptr) {
        return false;
    }

    QDataStream s(device());
    s.setByteOrder(QDataStream::LittleEndian);

    int format = m_format;
    if (format == FormatUnknown) {
        switch (outImage.format()) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
        case QImage::Format_RGBX16FPx4:
        case QImage::Format_RGBA16FPx4:
        case QImage::Format_RGBA16FPx4_Premultiplied:
            format = FormatA16B16G16R16F;
            break;

        case QImage::Format_RGBX32FPx4:
        case QImage::Format_RGBA32FPx4:
        case QImage::Format_RGBA32FPx4_Premultiplied:
            format = FormatA32B32G32R32F;
            break;
#endif

        case QImage::Format_Grayscale16:
        case QImage::Format_Grayscale8:
        case QImage::Format_Mono:
        case QImage::Format_MonoLSB:
            format = FormatL8;
            break;

        case QImage::Format_Indexed8:
            format = FormatP8;
            break;

        default:
            format = outImage.hasAlphaChannel() ? FormatA8R8G8B8 : FormatR8G8B8;
        }
    }

    if (format == FormatA8R8G8B8) {
        return writeA8R8G8B8(outImage, s);
    }

    if (format == FormatR8G8B8) {
        return writeR8G8B8(outImage, s);
    }

    if (format == FormatL8) {
        return writeL8(outImage, s);
    }

    if (format == FormatP8) {
        return writeP8(outImage, s);
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
    if (format == FormatA16B16G16R16F) {
        return writeA16B16G16R16F(outImage, s);
    }

    if (format == FormatA32B32G32R32F) {
        return writeA32B32G32R32F(outImage, s);
    }
#endif

    qWarning() << "Format" << formatName(format) << "is not supported";
    return false;
}

QVariant QDDSHandler::option(QImageIOHandler::ImageOption option) const
{
    if (!supportsOption(option)) {
        return QVariant();
    }

    // *** options that do not require a valid stream ***
    if (option == QImageIOHandler::SupportedSubTypes) {
        return QVariant::fromValue(QList<QByteArray>()
                                   << QByteArrayLiteral("Automatic")
                                   << formatName(FormatA8R8G8B8)
                                   << formatName(FormatR8G8B8)
                                   << formatName(FormatL8)
                                   << formatName(FormatP8)
                                   << formatName(FormatA16B16G16R16F)
                                   << formatName(FormatA32B32G32R32F));
    }

    // *** options that require a valid stream ***
    if (!ensureScanned()) {
        return QVariant();
    }

    if (option == QImageIOHandler::Size) {
        return isCubeMap(m_header) ? QSize(m_header.width * 4, m_header.height * 3) : QSize(m_header.width, m_header.height);
    }

    if (option == QImageIOHandler::SubType) {
        return m_format == FormatUnknown ? QByteArrayLiteral("Automatic") : formatName(m_format);
    }

    return QVariant();
}

void QDDSHandler::setOption(QImageIOHandler::ImageOption option, const QVariant &value)
{
    if (option == QImageIOHandler::SubType) {
        const QByteArray subType = value.toByteArray();
        m_format = formatByName(subType.toUpper());
    }
}

bool QDDSHandler::supportsOption(QImageIOHandler::ImageOption option) const
{
    return (option == QImageIOHandler::Size)
            || (option == QImageIOHandler::SubType)
            || (option == QImageIOHandler::SupportedSubTypes);
}

int QDDSHandler::imageCount() const
{
    if (!ensureScanned())
        return 0;

    return qMax<quint32>(1, m_header.mipMapCount);
}

bool QDDSHandler::jumpToImage(int imageNumber)
{
    if (imageNumber >= imageCount())
        return false;

    m_currentImage = imageNumber;
    return true;
}

bool QDDSHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning() << "DDSHandler::canRead() called with no device";
        return false;
    }

    if (device->isSequential())
        return false;

    return device->peek(4) == QByteArrayLiteral("DDS ");
}

bool QDDSHandler::ensureScanned() const
{
    if (device() == nullptr) {
        return false;
    }

    if (m_scanState != ScanNotScanned)
        return m_scanState == ScanSuccess;

    m_scanState = ScanError;

    QDDSHandler *that = const_cast<QDDSHandler *>(this);
    that->m_format = FormatUnknown;

    if (device()->isSequential()) {
        qWarning() << "Sequential devices are not supported";
        return false;
    }

    qint64 oldPos = device()->pos();
    device()->seek(0);

    QDataStream s(device());
    s.setByteOrder(QDataStream::LittleEndian);
    s >> that->m_header;

    device()->seek(oldPos);

    if (s.status() != QDataStream::Ok)
        return false;

    if (!verifyHeader(m_header))
        return false;

    that->m_format = getFormat(m_header);
    if (that->m_format == FormatUnknown)
        return false;

    m_scanState = ScanSuccess;
    return true;
}

bool QDDSHandler::verifyHeader(const DDSHeader &dds) const
{
    quint32 flags = dds.flags;
    quint32 requiredFlags = DDSHeader::FlagCaps | DDSHeader::FlagHeight
            | DDSHeader::FlagWidth | DDSHeader::FlagPixelFormat;
    if ((flags & requiredFlags) != requiredFlags) {
        qWarning() << "Wrong dds.flags - not all required flags present. "
                      "Actual flags :" << flags;
        return false;
    }

    if (dds.size != ddsSize) {
        qWarning() << "Wrong dds.size: actual =" << dds.size
                   << "expected =" << ddsSize;
        return false;
    }

    if (dds.pixelFormat.size != pixelFormatSize) {
        qWarning() << "Wrong dds.pixelFormat.size: actual =" << dds.pixelFormat.size
                   << "expected =" << pixelFormatSize;
        return false;
    }

    if (dds.width > INT_MAX || dds.height > INT_MAX) {
        qWarning() << "Can't read image with w/h bigger than INT_MAX";
        return false;
    }

    return true;
}

QImageIOPlugin::Capabilities QDDSPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == QByteArrayLiteral("dds"))
        return Capabilities(CanRead | CanWrite);
    if (!format.isEmpty())
        return {};
    if (!device || !device->isOpen())
        return {};

    Capabilities cap;
    if (device->isReadable() && QDDSHandler::canRead(device))
        cap |= CanRead;
    if (device->isWritable())
        cap |= CanWrite;
    return cap;
}

QImageIOHandler *QDDSPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new QDDSHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}
