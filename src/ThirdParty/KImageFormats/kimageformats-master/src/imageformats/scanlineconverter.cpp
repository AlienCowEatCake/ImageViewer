/*
    SPDX-FileCopyrightText: 2023 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "scanlineconverter_p.h"
#include <cstring>

ScanLineConverter::ScanLineConverter(const QImage::Format &targetFormat)
    : _targetFormat(targetFormat)
{
}

ScanLineConverter::ScanLineConverter(const ScanLineConverter &other)
    : _targetFormat(other._targetFormat)
    , _colorSpace(other._colorSpace)
    , _defaultColorSpace(other._defaultColorSpace)
{
}

ScanLineConverter &ScanLineConverter::operator=(const ScanLineConverter &other)
{
    _targetFormat = other._targetFormat;
    _colorSpace = other._colorSpace;
    _defaultColorSpace = other._defaultColorSpace;
    return (*this);
}

QImage::Format ScanLineConverter::targetFormat() const
{
    return _targetFormat;
}

void ScanLineConverter::setTargetColorSpace(const QColorSpace &colorSpace)
{
    _colorSpace = colorSpace;
}

QColorSpace ScanLineConverter::targetColorSpace() const
{
    return _colorSpace;
}

void ScanLineConverter::setDefaultSourceColorSpace(const QColorSpace &colorSpace)
{
    _defaultColorSpace = colorSpace;
}

QColorSpace ScanLineConverter::defaultSourceColorSpace() const
{
    return _defaultColorSpace;
}

const uchar *ScanLineConverter::convertedScanLine(const QImage &image, qint32 y)
{
    auto colorSpaceConversion = isColorSpaceConversionNeeded(image);
    if (image.format() == _targetFormat && !colorSpaceConversion) {
        return image.constScanLine(y);
    }
    if (image.width() != _tmpBuffer.width() || image.format() != _tmpBuffer.format()) {
        _tmpBuffer = QImage(image.width(), 1, image.format());
    }
    if (_tmpBuffer.isNull()) {
        return nullptr;
    }
    std::memcpy(_tmpBuffer.bits(), image.constScanLine(y), std::min(_tmpBuffer.bytesPerLine(), image.bytesPerLine()));
    if (colorSpaceConversion) {
        auto cs = image.colorSpace();
        if (!cs.isValid()) {
            cs = _defaultColorSpace;
        }
        _tmpBuffer.setColorSpace(cs);
        _tmpBuffer.convertToColorSpace(_colorSpace);
    }
    _convBuffer = _tmpBuffer.convertToFormat(_targetFormat);
    if (_convBuffer.isNull()) {
        return nullptr;
    }
    return _convBuffer.constBits();
}

qsizetype ScanLineConverter::bytesPerLine() const
{
    if (_convBuffer.isNull()) {
        return 0;
    }
    return _convBuffer.bytesPerLine();
}

bool ScanLineConverter::isColorSpaceConversionNeeded(const QImage &image, const QColorSpace &targetColorSpace, const QColorSpace &defaultColorSpace)
{
    if (image.depth() < 24) { // RGB 8 bit or grater only
        return false;
    }
    auto sourceColorSpace = image.colorSpace();
    if (!sourceColorSpace.isValid()) {
        sourceColorSpace = defaultColorSpace;
    }
    if (!sourceColorSpace.isValid() || !targetColorSpace.isValid()) {
        return false;
    }

    auto stf = sourceColorSpace.transferFunction();
    auto spr = sourceColorSpace.primaries();
    auto ttf = targetColorSpace.transferFunction();
    auto tpr = targetColorSpace.primaries();
    // clang-format off
    if (stf == QColorSpace::TransferFunction::Custom ||
        ttf == QColorSpace::TransferFunction::Custom ||
        spr == QColorSpace::Primaries::Custom ||
        tpr == QColorSpace::Primaries::Custom) {
        return true;
    }
    // clang-format on
    if (stf == ttf && spr == tpr) {
        return false;
    }
    return true;
}
