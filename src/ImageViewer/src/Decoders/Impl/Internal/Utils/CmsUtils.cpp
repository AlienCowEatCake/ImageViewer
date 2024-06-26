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

#include "CmsUtils.h"

#include <cassert>
#include <map>

#if defined (HAS_LCMS2)
#include <lcms2.h>
#endif

#include <QDebug>
#include <QImage>
#include <QByteArray>
#include <QSysInfo>

#include "Utils/Global.h"

struct ICCProfile::Impl
{
#if defined (HAS_LCMS2)
    Impl()
        : inProfile(Q_NULLPTR)
        , outProfile(Q_NULLPTR)
    {
        cmsSetLogErrorHandler(&Impl::logErrorHandler);
    }

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

    static void logErrorHandler(cmsContext /*contextID*/, cmsUInt32Number errorCode, const char *text)
    {
        QString errorCodeString;
        switch(errorCode)
        {
#define ADD_CASE(VALUE) case (VALUE): errorCodeString = QString::fromLatin1(#VALUE); break
        ADD_CASE(cmsERROR_UNDEFINED);
        ADD_CASE(cmsERROR_FILE);
        ADD_CASE(cmsERROR_RANGE);
        ADD_CASE(cmsERROR_INTERNAL);
        ADD_CASE(cmsERROR_NULL);
        ADD_CASE(cmsERROR_READ);
        ADD_CASE(cmsERROR_SEEK);
        ADD_CASE(cmsERROR_WRITE);
        ADD_CASE(cmsERROR_UNKNOWN_EXTENSION);
        ADD_CASE(cmsERROR_COLORSPACE_CHECK);
        ADD_CASE(cmsERROR_ALREADY_DEFINED);
        ADD_CASE(cmsERROR_BAD_SIGNATURE);
        ADD_CASE(cmsERROR_CORRUPTION_DETECTED);
        ADD_CASE(cmsERROR_NOT_SUITABLE);
#undef ADD_CASE
        }
        qWarning() << "[CmsUtils] Error" << errorCodeString << ":" << text;
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

ICCProfile::ICCProfile(float *whitePoint,
                       float *primaryChromaticities,
                       unsigned short *transferFunctionRed,
                       unsigned short *transferFunctionGreen,
                       unsigned short *transferFunctionBlue)
    : m_impl(new Impl())
{
#if defined (HAS_LCMS2)
    if(!whitePoint)
        return;
    if(!primaryChromaticities)
        return;
    if(!transferFunctionRed)
        return;
    if(!transferFunctionGreen)
        return;
    if(!transferFunctionBlue)
        return;

    cmsCIExyY cmsWhitePoint;
    cmsWhitePoint.x = static_cast<cmsFloat64Number>(whitePoint[0]);
    cmsWhitePoint.y = static_cast<cmsFloat64Number>(whitePoint[1]);
    cmsWhitePoint.Y = static_cast<cmsFloat64Number>(1.0);

    cmsCIExyYTRIPLE cmsPrimaries;
    cmsPrimaries.Red.x = static_cast<cmsFloat64Number>(primaryChromaticities[0]);
    cmsPrimaries.Red.y = static_cast<cmsFloat64Number>(primaryChromaticities[1]);
    cmsPrimaries.Red.Y = static_cast<cmsFloat64Number>(1.0);
    cmsPrimaries.Green.x = static_cast<cmsFloat64Number>(primaryChromaticities[2]);
    cmsPrimaries.Green.y = static_cast<cmsFloat64Number>(primaryChromaticities[3]);
    cmsPrimaries.Green.Y = static_cast<cmsFloat64Number>(1.0);
    cmsPrimaries.Blue.x = static_cast<cmsFloat64Number>(primaryChromaticities[4]);
    cmsPrimaries.Blue.y = static_cast<cmsFloat64Number>(primaryChromaticities[5]);
    cmsPrimaries.Blue.Y = static_cast<cmsFloat64Number>(1.0);

    cmsToneCurve* cmsTransferFunction[3] =
    {
        cmsBuildTabulatedToneCurve16(Q_NULLPTR, 256, transferFunctionRed),
        cmsBuildTabulatedToneCurve16(Q_NULLPTR, 256, transferFunctionGreen),
        cmsBuildTabulatedToneCurve16(Q_NULLPTR, 256, transferFunctionBlue)
    };

    m_impl->outProfile = cmsCreate_sRGBProfile();
    m_impl->inProfile = cmsCreateRGBProfileTHR(Q_NULLPTR, &cmsWhitePoint, &cmsPrimaries, cmsTransferFunction);

    for(std::size_t i = 0; i < 3; i++)
        cmsFreeToneCurve(cmsTransferFunction[i]);

#else
    Q_UNUSED(whitePoint);
    Q_UNUSED(primaryChromaticities);
    Q_UNUSED(transferFunctionRed);
    Q_UNUSED(transferFunctionGreen);
    Q_UNUSED(transferFunctionBlue);
#endif
}

ICCProfile::~ICCProfile()
{}

void ICCProfile::applyToImage(QImage *image)
{
#if defined (HAS_LCMS2)
    if(!m_impl->inProfile || !m_impl->outProfile)
        return;

    cmsHTRANSFORM transform = Q_NULLPTR;
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

