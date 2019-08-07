/*
   Copyright (C) 2017-2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

   This file is part of the `ImageViewer' program.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#if !defined(DECODERS_MANAGER_H_INCLUDED)
#define DECODERS_MANAGER_H_INCLUDED

#include <QStringList>

#include "Utils/ScopedPointer.h"
#include "IDecoder.h"

class QSize;

class DecodersManager
{
    Q_DISABLE_COPY(DecodersManager)

public:
    ~DecodersManager();
    static DecodersManager &getInstance();

    void registerDecoder(IDecoder *decoder);
    void registerFallbackDecoder(IDecoder *decoder);
    QStringList registeredDecoders() const;
    QStringList supportedFormats() const;
    QStringList supportedFormatsWithWildcards() const;

    QStringList blackListedDecoders() const;
    void setBlackListedDecoders(const QStringList& blackListedDecoders) const;

    QSharedPointer<IImageData> loadImage(const QString &filePath);
    QSharedPointer<IImageData> loadImage(const QString &filePath, const QString &decoderName);

    QSharedPointer<IImageData> generateStub(const QSize &size, const QString &filePath);
    QSharedPointer<IImageData> generateStub(const QSharedPointer<IImageData> &base);

private:
    DecodersManager();

    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif
