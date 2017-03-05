/*
   Copyright (C) 2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

class DecodersManager
{
private:
    DecodersManager();
    DecodersManager(const DecodersManager&);
    bool operator = (const DecodersManager&);

public:
    ~DecodersManager();
    static DecodersManager &getInstance();

    void registerDecoder(IDecoder *decoder);
    void registerDefaultDecoder(IDecoder *decoder, int priority);
    QStringList registeredDecoders() const;
    QStringList supportedFormats() const;
    QStringList supportedFormatsWithWildcards() const;

    QGraphicsItem *loadImage(const QString &filePath);

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif
