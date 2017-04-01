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

#include "CmsUtils.h"

#include <cassert>

#include <QImage>
#include <QTransform>
#include <QDebug>

#if defined (HAS_LCMS2)
#include <lcms2.h>
#endif

namespace CmsUtils {

void ApplyICCProfile(QImage *image, const QByteArray &profileData)
{
#if defined (HAS_LCMS2)
    cmsHPROFILE outProfile = cmsCreate_sRGBProfile();
    cmsHPROFILE inProfile = cmsOpenProfileFromMem(profileData.constData(), static_cast<cmsUInt32Number>(profileData.size()));
    cmsHTRANSFORM transform = cmsCreateTransform(inProfile, TYPE_RGBA_8, outProfile, TYPE_RGBA_8, INTENT_PERCEPTUAL, 0);
    cmsCloseProfile(inProfile);
    cmsCloseProfile(outProfile);
    for(int i = 0; i < image->height(); i++)
    {
        QRgb *line = reinterpret_cast<QRgb*>(image->scanLine(i));
        for(int j = 0; j < image->width(); j++)
        {
            QRgb &pixel = line[j];
            unsigned char rgba[4] =
            {
                static_cast<unsigned char>(qRed(pixel)),
                static_cast<unsigned char>(qGreen(pixel)),
                static_cast<unsigned char>(qBlue(pixel)),
                static_cast<unsigned char>(qAlpha(pixel))
            };
            cmsDoTransform(transform, rgba, rgba, 1);
            pixel = qRgba(rgba[0], rgba[1], rgba[2], rgba[3]);
        }
    }
    cmsDeleteTransform(transform);
#else
    Q_UNUSED(image);
    Q_UNUSED(iccProfileData);
    Q_UNUSED(iccProfileSize);
#endif
}

} // namespace CmsUtils
