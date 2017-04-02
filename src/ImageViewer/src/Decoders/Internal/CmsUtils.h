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

#if !defined(DECODER_CMSUTILS_H_INCLUDED)
#define DECODER_CMSUTILS_H_INCLUDED

#include <cstddef>
#include <QtGlobal>

class QImage;
class QByteArray;

class ICCProfile
{
public:
    ICCProfile(const QByteArray &profileData);
    ~ICCProfile();

    void applyToImage(QImage *image);
    void applyToRGBData(void *rgbData, std::size_t pixelsNum);
    void applyToRGBAData(void *rgbaData, std::size_t pixelsNum);

private:
    ICCProfile(const ICCProfile &);
    ICCProfile &operator = (const ICCProfile &);

    struct Impl;
    Impl *m_impl;
};

#endif
