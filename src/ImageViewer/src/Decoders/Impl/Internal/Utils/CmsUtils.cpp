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
#include <cstdlib>
#include <map>
#include <utility>

//#undef HAS_LCMS2
//#undef HAS_FALLBACK_ICCPROFILES

#if defined (Q_OS_WIN)
#include <Windows.h>
#endif

#if defined (HAS_LCMS2)
#include <lcms2.h>
#endif

#include <QDebug>
#include <QDir>
#include <QFile>
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
        for(std::map<std::pair<cmsUInt32Number, cmsUInt32Number>, cmsHTRANSFORM>::iterator it = transforms.begin(); it != transforms.end(); ++it)
        {
            cmsHTRANSFORM transform = it->second;
            if(transform)
                cmsDeleteTransform(transform);
        }
    }

    cmsHTRANSFORM getOrCreateTransform(cmsUInt32Number inFormat, cmsUInt32Number outFormat)
    {
        const std::pair<cmsUInt32Number, cmsUInt32Number> format = std::make_pair(inFormat, outFormat);
        std::map<std::pair<cmsUInt32Number, cmsUInt32Number>, cmsHTRANSFORM>::iterator it = transforms.find(format);
        if(it != transforms.end())
            return it->second;
        cmsHTRANSFORM transform = cmsCreateTransform(inProfile, inFormat, outProfile, outFormat, INTENT_PERCEPTUAL, 0);
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
    std::map<std::pair<cmsUInt32Number, cmsUInt32Number>, cmsHTRANSFORM> transforms;
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

bool ICCProfile::isValid() const
{
#if defined (HAS_LCMS2)
    if(!m_impl->inProfile || !m_impl->outProfile)
        return false;
    return true;
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    if(!m_impl->inColorSpace.isValid() || !m_impl->outColorSpace.isValid())
        return false;
    return true;
#else
    return false;
#endif
}

void ICCProfile::applyToImage(QImage *image)
{
    if(!image || !isValid())
        return;

#if defined (HAS_LCMS2)
    bool forceCmyk = false;
    cmsUInt32Number inFormat = 0;
    cmsUInt32Number outFormat = 0;
    QImage::Format outImageFormat = image->format();
    switch(image->format())
    {
    case QImage::Format_ARGB32_Premultiplied:
        inFormat = outFormat = (QSysInfo::ByteOrder == QSysInfo::LittleEndian) ? TYPE_BGRA_8_PREMUL : TYPE_ARGB_8_PREMUL;
        break;
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
        inFormat = outFormat = (QSysInfo::ByteOrder == QSysInfo::LittleEndian) ? TYPE_BGRA_8 : TYPE_ARGB_8;
        break;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    case QImage::Format_RGBA8888_Premultiplied:
        inFormat = outFormat = TYPE_RGBA_8_PREMUL;
        break;
    case QImage::Format_RGBA8888:
    case QImage::Format_RGBX8888:
        inFormat = outFormat = TYPE_RGBA_8;
        break;
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(6, 8, 0))
    case QImage::Format_CMYK8888:
        inFormat = TYPE_CMYK_8;
        outFormat = (QSysInfo::ByteOrder == QSysInfo::LittleEndian) ? TYPE_BGRA_8 : TYPE_ARGB_8;
        outImageFormat = QImage::Format_RGB32;
        break;
#endif
    default:
        *image = image->convertToFormat(QImage::Format_ARGB32);
        inFormat = outFormat = (QSysInfo::ByteOrder == QSysInfo::LittleEndian) ? TYPE_BGRA_8 : TYPE_ARGB_8;
        break;
    }
    cmsHTRANSFORM transform = m_impl->getOrCreateTransform(inFormat, outFormat);
    if(!transform && inFormat != TYPE_CMYK_8)
    {
        qWarning() << "[CmsUtils] Trying CMYK conversion transform";
        forceCmyk = true;
        inFormat = TYPE_CMYK_8;
        transform = m_impl->getOrCreateTransform(inFormat, outFormat);
    }
    if(!transform)
        return;

    if(inFormat == outFormat)
    {
        unsigned char *buffer = reinterpret_cast<unsigned char*>(image->bits());
        cmsDoTransform(transform, buffer, buffer, static_cast<cmsUInt32Number>(image->width() * image->height()));
    }
    else if(forceCmyk)
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 8, 0))
        QImage cmykBuffer = image->convertToFormat(QImage::Format_CMYK8888);
#else
        QImage cmykBuffer = image->convertToFormat(QImage::Format_ARGB32);
        for(int j = 0; j < image->height(); ++j)
        {
            const QRgb *rgbScanLine = reinterpret_cast<const QRgb*>(image->constScanLine(j));
            QRgb *cmykScanLine = reinterpret_cast<QRgb*>(cmykBuffer.scanLine(j));
            for(int i = 0; i < image->width(); ++i)
            {
                const QRgb rgb = rgbScanLine[i];
                int c = 255 - qRed(rgb);
                int m = 255 - qGreen(rgb);
                int y = 255 - qBlue(rgb);
                const int k = std::min(c, std::min(m, y));
                if(k != 255)
                {
                    c = (c - k) * 255 / (255 - k);
                    m = (m - k) * 255 / (255 - k);
                    y = (y - k) * 255 / (255 - k);
                }
                else
                {
                    c = m = y = 0;
                }
                quint8 *cmykPixel = reinterpret_cast<quint8*>(cmykScanLine + i);
                cmykPixel[0] = qBound(0, c, 255);
                cmykPixel[1] = qBound(0, m, 255);
                cmykPixel[2] = qBound(0, y, 255);
                cmykPixel[3] = qBound(0, k, 255);
            }
        }
#endif
        const unsigned char *inBuffer = reinterpret_cast<const unsigned char*>(cmykBuffer.constBits());
        unsigned char *outBuffer = reinterpret_cast<unsigned char*>(image->bits());
        cmsDoTransform(transform, inBuffer, outBuffer, static_cast<cmsUInt32Number>(image->width() * image->height()));
    }
    else
    {
        QImage outImage(image->width(), image->height(), outImageFormat);
        outImage.fill(Qt::black);
        unsigned char *outBuffer = reinterpret_cast<unsigned char*>(outImage.bits());
        const unsigned char *inBuffer = reinterpret_cast<const unsigned char*>(image->constBits());
        cmsDoTransform(transform, inBuffer, outBuffer, static_cast<cmsUInt32Number>(image->width() * image->height()));
        *image = outImage;
    }

#elif (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#if (QT_VERSION >= QT_VERSION_CHECK(6, 8, 0))
    if(m_impl->inColorSpace.colorModel() == QColorSpace::ColorModel::Cmyk && image->format() != QImage::Format_CMYK8888)
        *image = image->convertToFormat(QImage::Format_CMYK8888);
#endif
    image->setColorSpace(m_impl->inColorSpace);
    image->convertToColorSpace(m_impl->outColorSpace);

#endif
}

static QString defaultCmykProfilePath()
{
    QString filePath;
#if defined (HAS_FALLBACK_ICCPROFILES)
    filePath = QString::fromLatin1(":/iccprofiles/ISOcoated_v2_bas.ICC");
#else
    QStringList cmykProfiles = QStringList()
            // Adobe Photoshop defaults
            << QString::fromLatin1("USWebCoatedSWOP.icc")
            // Recommended as the default Cmyk color space for untagged data:
            // https://lists.freedesktop.org/archives/openicc/2011q2/004130.html
            << QString::fromLatin1("ISOcoated_v2_bas.ICC")
            << QString::fromLatin1("coated_FOGRA39L_argl.icc")
            // Also try alternate versions
            << QString::fromLatin1("ISOcoated_v2_eci.icc")
            << QString::fromLatin1("ISOcoated_v2_300_bas.ICC")
            << QString::fromLatin1("ISOcoated_v2_300_eci.icc")
            // Adobe Photoshop alternate
            << QString::fromLatin1("CoatedFOGRA39.icc")
            ;
#if defined (Q_OS_WIN)
    cmykProfiles.append(QString::fromLatin1("RSWOP.icm"));
    QStringList profilePlaces;

    HMODULE hMscms = LoadLibraryA("mscms.dll");
    typedef BOOL(WINAPI *GetColorDirectoryA_t)(PCSTR, PSTR, PDWORD);
    GetColorDirectoryA_t GetColorDirectoryA_f = reinterpret_cast<GetColorDirectoryA_t>(GetProcAddress(hMscms, "GetColorDirectoryA"));
    if(GetColorDirectoryA_f)
    {
        DWORD size = 0;
        GetColorDirectoryA_f(Q_NULLPTR, Q_NULLPTR, &size);
        if(size > 0)
        {
            QByteArray buffer;
            buffer.resize(size + 1);
            if(GetColorDirectoryA_f(Q_NULLPTR, reinterpret_cast<PSTR>(buffer.data()), &size))
                profilePlaces.append(QDir::fromNativeSeparators(QString::fromLocal8Bit(buffer, size)));
        }
    }
    FreeLibrary(hMscms);

    const char *windir = getenv("windir");
    if(!windir)
        windir = getenv("SystemRoot");
    if(windir)
    {
        const QString place = QDir::fromNativeSeparators(QString::fromLocal8Bit(windir) + QString::fromLatin1("\\System32\\spool\\drivers\\color"));
        if(!profilePlaces.contains(place, Qt::CaseInsensitive))
            profilePlaces.append(place);
    }
#elif defined (Q_OS_MAC)
    // It looks like "Generic CMYK Profile.icc" is default CMYK profile for macOS
    cmykProfiles.insert(0, QString::fromLatin1("Generic CMYK Profile.icc"));
    const QStringList profilePlaces = QStringList()
            << QDir::homePath() + QString::fromLatin1("/Library/ColorSync/Profiles")
            << QString::fromLatin1("/Library/ColorSync/Profiles")
            << QString::fromLatin1("/System/Library/ColorSync/Profiles")
            ;
#else
    QString localShare;
    if(const char *xdgDataHome = getenv("XDG_DATA_HOME"))
        localShare = QString::fromLocal8Bit(xdgDataHome);
    else
        localShare = QDir::homePath() + QString::fromLatin1("/.local/share");
    const QStringList profilePlaces = QStringList()
            << localShare + QString::fromLatin1("/color/icc")
            << QDir::homePath() + QString::fromLatin1("/.color/icc")
            << QString::fromLatin1("/usr/local/share/color/icc")
            << QString::fromLatin1("/usr/share/color/icc")
            ;
#endif
    QList<std::pair<QString, QStringList> > entries;
    for(QStringList::ConstIterator it = profilePlaces.constBegin(); it != profilePlaces.constEnd() && filePath.isEmpty(); ++it)
    {
        const QDir &profilePlace(*it);
        if(profilePlace.exists())
        {
            QStringList entryList = profilePlace.entryList(QDir::Files | QDir::Readable);
            if(!entryList.isEmpty())
                entries.append(std::make_pair(profilePlace.absolutePath(), entryList));
        }
    }

    for(QStringList::ConstIterator it = cmykProfiles.constBegin(); it != cmykProfiles.constEnd() && filePath.isEmpty(); ++it)
    {
        for(QList<std::pair<QString, QStringList> >::ConstIterator jt = entries.constBegin(); jt != entries.constEnd() && filePath.isEmpty(); ++jt)
        {
            for(QStringList::ConstIterator kt = jt->second.constBegin(); kt != jt->second.constEnd() && filePath.isEmpty(); ++kt)
            {
                if(it->compare(*kt, Qt::CaseInsensitive) == 0)
                {
                    filePath = jt->first + QString::fromLatin1("/") + *kt;
                    qDebug() << "[CmsUtils] Default CMYK profile:" << filePath;
                }
            }
        }
    }
#endif
    return filePath;
}

QByteArray ICCProfile::defaultCmykProfileData()
{
    static const QString filePath = defaultCmykProfilePath();
    if(filePath.isEmpty())
    {
        qDebug() << "[CmsUtils] Can't find default CMYK profile";
        return QByteArray();
    }

    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "[CmsUtils] Can't open default CMYK profile" << filePath;
        return QByteArray();
    }

    return file.readAll();
}

