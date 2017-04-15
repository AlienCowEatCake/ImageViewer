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
#include <map>

#include <QImage>
#include <QByteArray>
#include <QSysInfo>

#if defined (HAS_LCMS2)
#include <lcms2.h>
#endif

struct ICCProfile::Impl
{
#if defined (HAS_LCMS2)
    Impl()
        : inProfile(NULL)
        , outProfile(NULL)
    {}

    ~Impl()
    {
        if(inProfile)
            cmsCloseProfile(inProfile);
        if(outProfile)
            cmsCloseProfile(outProfile);
        for(std::map<cmsUInt32Number, cmsHTRANSFORM>::iterator it = transforms.begin(); it != transforms.end(); ++it)
        {
            cmsHTRANSFORM transform = it->second;
            if(transform)
                cmsDeleteTransform(transform);
        }
    }

    cmsHTRANSFORM getOrCreateTransform(cmsUInt32Number format)
    {
        std::map<cmsUInt32Number, cmsHTRANSFORM>::iterator it = transforms.find(format);
        if(it != transforms.end())
            return it->second;
        cmsHTRANSFORM transform = cmsCreateTransform(inProfile, format, outProfile, format, INTENT_PERCEPTUAL, 0);
        transforms[format] = transform;
        return transform;
    }

    void applyTransform(void *data, cmsUInt32Number pixelsNum, cmsUInt32Number format)
    {
        if(!inProfile || !outProfile)
            return;
        cmsHTRANSFORM transform = getOrCreateTransform(format);
        if(!transform)
            return;
        cmsDoTransform(getOrCreateTransform(format), data, data, pixelsNum);
    }

    cmsHPROFILE inProfile;
    cmsHPROFILE outProfile;
    std::map<cmsUInt32Number, cmsHTRANSFORM> transforms;
#endif
};

ICCProfile::ICCProfile(const QByteArray &profileData)
    : m_impl(new Impl())
{
#if defined (HAS_LCMS2)
    if(profileData.isEmpty())
        return;
    m_impl->outProfile = cmsCreate_sRGBProfile();
    m_impl->inProfile = cmsOpenProfileFromMem(profileData.constData(), static_cast<cmsUInt32Number>(profileData.size()));
#else
    Q_UNUSED(profileData);
#endif
}

ICCProfile::~ICCProfile()
{
    delete m_impl;
}

void ICCProfile::applyToImage(QImage *image)
{
#if defined (HAS_LCMS2)
    if(!m_impl->inProfile || !m_impl->outProfile)
        return;

    cmsHTRANSFORM transform = NULL;
    switch(image->format())
    {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
        transform = m_impl->getOrCreateTransform((QSysInfo::ByteOrder == QSysInfo::LittleEndian) ? TYPE_BGRA_8 : TYPE_ARGB_8);
        break;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    case QImage::Format_RGBA8888:
    case QImage::Format_RGBX8888:
        transform = m_impl->getOrCreateTransform(TYPE_RGBA_8);
        break;
#endif
    default:
        *image = image->convertToFormat(QImage::Format_ARGB32);
        transform = m_impl->getOrCreateTransform((QSysInfo::ByteOrder == QSysInfo::LittleEndian) ? TYPE_BGRA_8 : TYPE_ARGB_8);
        break;
    }
    if(!transform)
        return;

    unsigned char *rgba = reinterpret_cast<unsigned char*>(image->bits());
    cmsDoTransform(transform, rgba, rgba, static_cast<cmsUInt32Number>(image->width() * image->height()));

#else
    Q_UNUSED(image);
#endif
}

void ICCProfile::applyToRGBData(void *rgbData, std::size_t pixelsNum)
{
#if defined (HAS_LCMS2)
    m_impl->applyTransform(rgbData, static_cast<cmsUInt32Number>(pixelsNum), TYPE_RGB_8);
#else
    Q_UNUSED(rgbData);
    Q_UNUSED(pixelsNum);
#endif
}

void ICCProfile::applyToRGBAData(void *rgbaData, std::size_t pixelsNum)
{
#if defined (HAS_LCMS2)
    m_impl->applyTransform(rgbaData, static_cast<cmsUInt32Number>(pixelsNum), TYPE_RGBA_8);
#else
    Q_UNUSED(rgbaData);
    Q_UNUSED(pixelsNum);
#endif
}

