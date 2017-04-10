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

#if !defined(DECODER_AUTOREGISTRATOR_H_INCLUDED)
#define DECODER_AUTOREGISTRATOR_H_INCLUDED

#include "Utils/ScopedPointer.h"
#include "../IDecoder.h"
#include "../DecodersManager.h"

class DecoderAutoRegistrator
{
public:
    DecoderAutoRegistrator(IDecoder *decoder, bool isFallback = false)
        : m_decoder(decoder)
    {
        DecodersManager::getInstance().registerDecoder(decoder);
        if(isFallback)
            DecodersManager::getInstance().registerFallbackDecoder(decoder);
    }
private:
    Q_DISABLE_COPY(DecoderAutoRegistrator)
    QScopedPointer<IDecoder> m_decoder;
};

#endif
