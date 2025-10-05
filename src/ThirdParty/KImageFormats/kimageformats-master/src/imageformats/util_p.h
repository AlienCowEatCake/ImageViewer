/*
    SPDX-FileCopyrightText: 2022 Albert Astals Cid <aacid@kde.org>
    SPDX-FileCopyrightText: 2022 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef UTIL_P_H
#define UTIL_P_H

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

#include <QImage>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QImageIOHandler>
#endif

// Default maximum width and height for the large image plugins.
#ifndef KIF_LARGE_IMAGE_PIXEL_LIMIT
#define KIF_LARGE_IMAGE_PIXEL_LIMIT 300000
#endif

// Image metadata keys to use in plugins (so they are consistent)
#define META_KEY_ALTITUDE "Altitude"
#define META_KEY_AUTHOR "Author"
#define META_KEY_COMMENT "Comment"
#define META_KEY_COPYRIGHT "Copyright"
#define META_KEY_CREATIONDATE "CreationDate"
#define META_KEY_DESCRIPTION "Description"
#define META_KEY_DIRECTION "Direction"
#define META_KEY_DOCUMENTNAME "DocumentName"
#define META_KEY_HOSTCOMPUTER "HostComputer"
#define META_KEY_LATITUDE "Latitude"
#define META_KEY_LONGITUDE "Longitude"
#define META_KEY_MODIFICATIONDATE "ModificationDate"
#define META_KEY_OWNER "Owner"
#define META_KEY_SOFTWARE "Software"
#define META_KEY_TITLE "Title"
#define META_KEY_XML_GIMP "XML:org.gimp.xml"
#define META_KEY_XMP_ADOBE "XML:com.adobe.xmp"

// Camera info metadata keys
#define META_KEY_MANUFACTURER "Manufacturer"
#define META_KEY_MODEL "Model"
#define META_KEY_SERIALNUMBER "SerialNumber"

// Lens info metadata keys
#define META_KEY_LENS_MANUFACTURER "LensManufacturer"
#define META_KEY_LENS_MODEL "LensModel"
#define META_KEY_LENS_SERIALNUMBER "LensSerialNumber"

// QList uses some extra space for stuff, hence the 32 here suggested by Thiago Macieira
static const int kMaxQVectorSize = std::numeric_limits<int>::max() - 32;

// On Qt 6 to make the plugins fail to allocate if the image size is greater than QImageReader::allocationLimit()
// it is necessary to allocate the image with QImageIOHandler::allocateImage().
inline QImage imageAlloc(const QSize &size, const QImage::Format &format)
{
    QImage img;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    img = QImage(size, format);
#else
    if (!QImageIOHandler::allocateImage(size, format, &img)) {
        img = QImage(); // paranoia
    }
#endif
    return img;
}

inline QImage imageAlloc(qint32 width, qint32 height, const QImage::Format &format)
{
    return imageAlloc(QSize(width, height), format);
}

template<class TI, class SF> // SF = source FP, TI = target INT
TI qRoundOrZero_T(SF d, bool *ok = nullptr)
{
    // checks for undefined behavior
    if (qIsNaN(d) || qIsInf(d) || d < SF() || d > SF(std::numeric_limits<TI>::max())) {
        if (ok) {
            *ok = false;
        }
        return 0;
    }
    if (ok) {
        *ok = true;
    }
    return qRound(d);
}

inline qint32 qRoundOrZero(double d, bool *ok = nullptr)
{
    return qRoundOrZero_T<qint32>(d, ok);
}
inline qint32 qRoundOrZero(float d, bool *ok = nullptr)
{
    return qRoundOrZero_T<qint32>(d, ok);
}

/*!
 * \brief dpi2ppm
 * Converts a value from DPI to PPM.
 * \return \a dpi converted to pixel per meter.
 */
inline qint32 dpi2ppm(double dpi, bool *ok = nullptr)
{
    return qRoundOrZero(dpi / double(25.4) * double(1000), ok);
}
inline qint32 dpi2ppm(float dpi, bool *ok = nullptr)
{
    return qRoundOrZero(dpi / float(25.4) * float(1000), ok);
}
inline qint32 dpi2ppm(quint16 dpi, bool *ok = nullptr)
{
    return qRoundOrZero(dpi / double(25.4) * double(1000), ok);
}

/*!
 * \brief ppm2dpi
 * Converts a value from PPM to DPI.
 * \return \a ppm converted to dot per inch.
 */
template<class TF, class SI> // SI = source INT, TF = target FP
TF ppm2dpi_T(SI ppm, bool *ok = nullptr)
{
    if (ok) {
        *ok = ppm > 0;
    }
    return ppm > 0 ? ppm * TF(25.4) / TF(1000) : TF();
}

inline double dppm2dpi(qint32 ppm, bool *ok = nullptr)
{
    return ppm2dpi_T<double>(ppm, ok);
}
inline float fppm2dpi(qint32 ppm, bool *ok = nullptr)
{
    return ppm2dpi_T<float>(ppm, ok);
}

#endif // UTIL_P_H
