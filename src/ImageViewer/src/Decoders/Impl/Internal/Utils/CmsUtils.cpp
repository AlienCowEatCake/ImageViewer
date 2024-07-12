/*
   Copyright (C) 2017-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <cmath>
#include <map>

//#undef HAS_LCMS2

#if defined (HAS_LCMS2)
#include <lcms2.h>
#endif

#include <QDebug>
#include <QImage>
#include <QByteArray>
#include <QSysInfo>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#include <QColorSpace>
#endif

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
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QColorSpace inColorSpace;
    QColorSpace outColorSpace;
#endif
};

ICCProfile::ICCProfile(const QByteArray &profileData)
    : m_impl(new Impl())
{
    if(profileData.isEmpty())
        return;

#if defined (HAS_LCMS2)
    m_impl->outProfile = cmsCreate_sRGBProfile();
    m_impl->inProfile = cmsOpenProfileFromMem(profileData.constData(), static_cast<cmsUInt32Number>(profileData.size()));
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    m_impl->outColorSpace = QColorSpace(QColorSpace::SRgb);
    m_impl->inColorSpace = QColorSpace::fromIccProfile(profileData);
    if(!m_impl->inColorSpace.isValid())
        qWarning() << "[CmsUtils] Invalid colorspace";
#endif
}

ICCProfile::ICCProfile(float *whitePoint,
                       float *primaryChromaticities,
                       unsigned short *transferFunctionRed,
                       unsigned short *transferFunctionGreen,
                       unsigned short *transferFunctionBlue,
                       std::size_t transferFunctionSize)
    : m_impl(new Impl())
{
#if defined (HAS_LCMS2)
    /// @note LittleCMS may not work with some incomplete profile and fails with error
    /// "cmsERROR_NOT_SUITABLE" : Couldn't link the profiles. So fill default values

    cmsCIExyY cmsWhitePoint;
    if(whitePoint)
    {
        cmsWhitePoint.x = static_cast<cmsFloat64Number>(whitePoint[0]);
        cmsWhitePoint.y = static_cast<cmsFloat64Number>(whitePoint[1]);
        cmsWhitePoint.Y = static_cast<cmsFloat64Number>(1.0);
    }
    else
    {
        /// @note CIE D65
        cmsWhitePoint.x = static_cast<cmsFloat64Number>(0.31271);
        cmsWhitePoint.y = static_cast<cmsFloat64Number>(0.32902);
        cmsWhitePoint.Y = static_cast<cmsFloat64Number>(1.0);
    }

    cmsCIExyYTRIPLE cmsPrimaries;
    if(primaryChromaticities)
    {
        cmsPrimaries.Red.x = static_cast<cmsFloat64Number>(primaryChromaticities[0]);
        cmsPrimaries.Red.y = static_cast<cmsFloat64Number>(primaryChromaticities[1]);
        cmsPrimaries.Red.Y = static_cast<cmsFloat64Number>(1.0);
        cmsPrimaries.Green.x = static_cast<cmsFloat64Number>(primaryChromaticities[2]);
        cmsPrimaries.Green.y = static_cast<cmsFloat64Number>(primaryChromaticities[3]);
        cmsPrimaries.Green.Y = static_cast<cmsFloat64Number>(1.0);
        cmsPrimaries.Blue.x = static_cast<cmsFloat64Number>(primaryChromaticities[4]);
        cmsPrimaries.Blue.y = static_cast<cmsFloat64Number>(primaryChromaticities[5]);
        cmsPrimaries.Blue.Y = static_cast<cmsFloat64Number>(1.0);
    }
    else
    {
        /// @note sRGB
        cmsPrimaries.Red.x = static_cast<cmsFloat64Number>(0.640);
        cmsPrimaries.Red.y = static_cast<cmsFloat64Number>(0.330);
        cmsPrimaries.Red.Y = static_cast<cmsFloat64Number>(1.0);
        cmsPrimaries.Green.x = static_cast<cmsFloat64Number>(0.300);
        cmsPrimaries.Green.y = static_cast<cmsFloat64Number>(0.600);
        cmsPrimaries.Green.Y = static_cast<cmsFloat64Number>(1.0);
        cmsPrimaries.Blue.x = static_cast<cmsFloat64Number>(0.150);
        cmsPrimaries.Blue.y = static_cast<cmsFloat64Number>(0.060);
        cmsPrimaries.Blue.Y = static_cast<cmsFloat64Number>(1.0);
    }

    cmsToneCurve* cmsTransferFunction[3];
    if(transferFunctionRed && transferFunctionGreen && transferFunctionBlue && transferFunctionSize > 0)
    {
        /// @note LittleCMS not work with too smooth transfer functions and fails with error
        /// "cmsERROR_RANGE" : Couldn't create tone curve of more than 65530 entries
        if(transferFunctionSize > 65530)
        {
            QVector<unsigned short> reducedTransferFunctions[3];
            const unsigned short *transferFunctions[3] = {
                transferFunctionRed,
                transferFunctionGreen,
                transferFunctionBlue
            };
            for(int i = 0; i < 3; ++i)
            {
                reducedTransferFunctions[i].resize(256);
                reducedTransferFunctions[i][0] = transferFunctions[i][0];
                reducedTransferFunctions[i][reducedTransferFunctions[i].size() - 1] = transferFunctions[i][transferFunctionSize - 1];
                for(int j = 1; j < reducedTransferFunctions[i].size() - 1; ++j)
                {
                    const double jd = static_cast<double>(j) / static_cast<double>(reducedTransferFunctions[i].size());
                    const qint64 js = qBound(static_cast<qint64>(0), static_cast<qint64>(std::floor(jd * static_cast<double>(transferFunctionSize) + 0.5)), static_cast<qint64>(transferFunctionSize - 1));
                    reducedTransferFunctions[i][j] = transferFunctions[i][static_cast<std::size_t>(js)];
                }
            }
            for(std::size_t i = 0; i < 3; i++)
                cmsTransferFunction[i] = cmsBuildTabulatedToneCurve16(Q_NULLPTR, static_cast<cmsUInt32Number>(reducedTransferFunctions[i].size()), reducedTransferFunctions[i].data());
        }
        else
        {
            cmsTransferFunction[0] = cmsBuildTabulatedToneCurve16(Q_NULLPTR, static_cast<cmsUInt32Number>(transferFunctionSize), transferFunctionRed);
            cmsTransferFunction[1] = cmsBuildTabulatedToneCurve16(Q_NULLPTR, static_cast<cmsUInt32Number>(transferFunctionSize), transferFunctionGreen);
            cmsTransferFunction[2] = cmsBuildTabulatedToneCurve16(Q_NULLPTR, static_cast<cmsUInt32Number>(transferFunctionSize), transferFunctionBlue);
        }
    }
    else
    {
        QVector<unsigned short> linearTransferFunction;
        linearTransferFunction.resize(256);
        linearTransferFunction[0] = 0;
        for(int i = 1; i < linearTransferFunction.size(); ++i)
        {
            const double t = static_cast<double>(i) / (static_cast<double>(linearTransferFunction.size()) - 1.0);
            linearTransferFunction[i] = static_cast<unsigned short>(std::floor(65535.0 * std::pow(t, 2.2) + 0.5));
        }
        for(std::size_t i = 0; i < 3; i++)
            cmsTransferFunction[i] = cmsBuildTabulatedToneCurve16(Q_NULLPTR, static_cast<cmsUInt32Number>(linearTransferFunction.size()), linearTransferFunction.data());
    }

    m_impl->outProfile = cmsCreate_sRGBProfile();
    m_impl->inProfile = cmsCreateRGBProfileTHR(Q_NULLPTR, &cmsWhitePoint, &cmsPrimaries, cmsTransferFunction);

    for(std::size_t i = 0; i < 3; i++)
        cmsFreeToneCurve(cmsTransferFunction[i]);

#elif (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    m_impl->outColorSpace = QColorSpace(QColorSpace::SRgb);
    m_impl->inColorSpace = QColorSpace(QColorSpace::SRgb);
    if(whitePoint && primaryChromaticities)
    {
        m_impl->inColorSpace.setPrimaries(
            QPointF(whitePoint[0], whitePoint[1]),
            QPointF(primaryChromaticities[0], primaryChromaticities[1]),
            QPointF(primaryChromaticities[2], primaryChromaticities[3]),
            QPointF(primaryChromaticities[4], primaryChromaticities[5]));
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 1, 0))
    if(transferFunctionRed && transferFunctionGreen && transferFunctionBlue && transferFunctionSize > 0)
    {
        m_impl->inColorSpace.setTransferFunctions(
            QList<uint16_t>(transferFunctionRed, transferFunctionRed + transferFunctionSize),
            QList<uint16_t>(transferFunctionGreen, transferFunctionGreen + transferFunctionSize),
            QList<uint16_t>(transferFunctionBlue, transferFunctionBlue + transferFunctionSize));
    }
#else
    Q_UNUSED(transferFunctionRed);
    Q_UNUSED(transferFunctionGreen);
    Q_UNUSED(transferFunctionBlue);
    Q_UNUSED(transferFunctionSize);
#endif

    if(!m_impl->inColorSpace.isValid())
        qWarning() << "[CmsUtils] Invalid colorspace";

#else
    Q_UNUSED(whitePoint);
    Q_UNUSED(primaryChromaticities);
    Q_UNUSED(transferFunctionRed);
    Q_UNUSED(transferFunctionGreen);
    Q_UNUSED(transferFunctionBlue);
    Q_UNUSED(transferFunctionSize);
#endif
}

ICCProfile::~ICCProfile()
{}

void ICCProfile::applyToImage(QImage *image)
{
    if(!image)
        return;

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

#elif (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    if(!m_impl->inColorSpace.isValid() || !m_impl->outColorSpace.isValid())
        return;

    image->setColorSpace(m_impl->inColorSpace);
    image->convertToColorSpace(m_impl->outColorSpace);

#endif
}

