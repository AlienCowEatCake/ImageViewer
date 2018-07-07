/*
   Copyright (C) 2017-2018 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <cmath>
#include <cstring>
#include <cassert>
#include <algorithm>

#include <libwmf/api.h>
#include <libwmf/gd.h>

#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QPainter>
#include <QDebug>

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/Scaling/IScaledImageProvider.h"
#include "Internal/Utils/ZLibUtils.h"

namespace
{

// ====================================================================================================

const qreal MAX_IMAGE_DIMENSION = 16384;
const qreal MIN_IMAGE_DIMENSION = 1;

const char *wmfErrorToString(wmf_error_t error)
{
    switch(error)
    {
    case wmf_E_None:
        return "No error.";
    case wmf_E_InsMem:
        return "An attempt to allocate memory has failed.";
    case wmf_E_BadFile:
        return "Attempt to open an unreadable file, or to read from an unopened file.";
    case wmf_E_BadFormat:
        return "The metafile, if indeed it is a metafile, has been corrupted.";
    case wmf_E_EOF:
        return "An unexpected end-of-file has been reached.";
    case wmf_E_DeviceError:
        return "Device-layer error.";
    case wmf_E_Glitch:
        return "Programmer's error. Sorry.";
    case wmf_E_Assert:
        return "Internally forced error.";
    case wmf_E_UserExit:
        return "The status function has returned non-zero; exit is premature.";
    }
    assert(false);
    return "Unknown error.";
}

// ====================================================================================================

class WmfPixmapProvider : public IScaledImageProvider
{
    Q_DISABLE_COPY(WmfPixmapProvider)

public:
    WmfPixmapProvider(const QString &filePath)
        : m_isValid(false)
        , m_API(NULL)
        , m_ddata(NULL)
        , m_width(0)
        , m_height(0)
        , m_minScaleFactor(1)
        , m_maxScaleFactor(1)
    {
        memset(&m_bbox, 0, sizeof(wmfD_Rect));

        if(QFileInfo(filePath).suffix().toLower() == QString::fromLatin1("wmz"))
        {
            m_inBuffer = ZLibUtils::InflateFile(filePath);
        }
        else
        {
            QFile inFile(filePath);
            if(!inFile.open(QIODevice::ReadOnly))
            {
                qWarning() << "Can't open" << filePath;
                return;
            }
            m_inBuffer = inFile.readAll();
        }

        if(m_inBuffer.isEmpty())
        {
            qWarning() << "Can't read" << filePath;
            return;
        }

        unsigned char *bufferData = reinterpret_cast<unsigned char*>(m_inBuffer.data());
        const long bufferSize = static_cast<long>(m_inBuffer.size());

        wmfAPI_Options m_options;
        memset(&m_options, 0, sizeof(wmfAPI_Options));
        m_options.function = wmf_gd_function;

        wmf_error_t error;

        unsigned long flags = WMF_OPT_FUNCTION | WMF_OPT_IGNORE_NONFATAL;
        error = wmf_api_create(&m_API, flags, &m_options);
        if(error != wmf_E_None)
        {
            qWarning() << "Couldn't create WMF reader:" << wmfErrorToString(error);
            if(m_API)
                wmf_api_destroy(m_API);
            return;
        }

        m_ddata = WMF_GD_GetData(m_API);

        error = wmf_mem_open(m_API, bufferData, bufferSize);
        if(error != wmf_E_None)
        {
            qWarning() << "Couldn't create reader API:" << wmfErrorToString(error);
            wmf_api_destroy(m_API);
            return;
        }

        error = wmf_scan(m_API, 0, &m_bbox);
        if(error != wmf_E_None)
        {
            qWarning() << "Error scanning WMF file:" << wmfErrorToString(error);
            wmf_mem_close(m_API);
            wmf_api_destroy(m_API);
            return;
        }

        const double resolution = 72.0;
        error = wmf_display_size(m_API, &m_width, &m_height, resolution, resolution);
        if(error != wmf_E_None || m_width == 0 || m_height == 0)
        {
            qWarning() << "Couldn't determine image size:" << wmfErrorToString(error);
            wmf_mem_close(m_API);
            wmf_api_destroy(m_API);
            return;
        }

        m_isValid = true;
        m_minScaleFactor = std::max(MIN_IMAGE_DIMENSION / m_width, MIN_IMAGE_DIMENSION / m_height);
        m_maxScaleFactor = std::min(MAX_IMAGE_DIMENSION / m_width, MAX_IMAGE_DIMENSION / m_height);
    }

    ~WmfPixmapProvider()
    {
        if(!isValid())
            return;
        wmf_mem_close(m_API);
        wmf_api_destroy(m_API);
    }

    bool isValid() const
    {
        return m_isValid;
    }

    QRectF boundingRect() const
    {
        return QRectF(0, 0, m_width, m_height);
    }

    QImage image(const qreal scaleFactor)
    {
        m_ddata->type = wmf_gd_image;
        m_ddata->bbox.TL.x = m_bbox.TL.x;
        m_ddata->bbox.TL.y = m_bbox.TL.y;
        m_ddata->bbox.BR.x = m_bbox.BR.x;
        m_ddata->bbox.BR.y = m_bbox.BR.y;
        m_ddata->width  = static_cast<unsigned int>(std::ceil(m_width * scaleFactor));
        m_ddata->height = static_cast<unsigned int>(std::ceil(m_height * scaleFactor));

        const wmf_error_t error = wmf_play(m_API, 0, &m_ddata->bbox);
        if(error != wmf_E_None)
        {
            qWarning() << "Couldn't decode WMF file into pixbuf:" << wmfErrorToString(error);
            return QImage();
        }

        int *gdPixels = NULL;
        if(m_ddata->gd_image == NULL || (gdPixels = wmf_gd_image_pixels(m_ddata->gd_image)) == NULL)
        {
            qWarning() << "Couldn't decode WMF file - no output (huh?)" << wmfErrorToString(error);
            return QImage();
        }

        QImage image(static_cast<int>(m_ddata->width), static_cast<int>(m_ddata->height), QImage::Format_ARGB32);
        QRgb *imagePtr = reinterpret_cast<QRgb*>(image.bits());
        for(int i = 0; i < image.height(); i++)
        {
            for(int j = 0; j < image.width(); j++)
            {
                unsigned int pixel = static_cast<unsigned int>(*gdPixels++);
                unsigned char b = static_cast<unsigned char>(pixel & 0xff);
                pixel >>= 8;
                unsigned char g = static_cast<unsigned char>(pixel & 0xff);
                pixel >>= 8;
                unsigned char r = static_cast<unsigned char>(pixel & 0xff);
                pixel >>= 7;
                unsigned char a = static_cast<unsigned char>(pixel & 0xfe);
                a ^= 0xff;
                *(imagePtr++) = qRgba(r, g, b, a);
            }
        }
        return image;
    }

    qreal minScaleFactor() const
    {
        return m_minScaleFactor;
    }

    qreal maxScaleFactor() const
    {
        return m_maxScaleFactor;
    }

private:
    bool m_isValid;
    QByteArray m_inBuffer;
    wmfAPI *m_API;
    wmf_gd_t *m_ddata;
    wmfD_Rect m_bbox;
    unsigned int m_width;
    unsigned int m_height;
    qreal m_minScaleFactor;
    qreal m_maxScaleFactor;
};

// ====================================================================================================

class DecoderLibWmf : public IDecoder
{
public:
    QString name() const
    {
        return QString::fromLatin1("DecoderLibWmf");
    }

    QStringList supportedFormats() const
    {
        return QStringList()
                << QString::fromLatin1("wmf")
                << QString::fromLatin1("wmz");
    }

    QStringList advancedFormats() const
    {
        return QStringList();
    }

    bool isAvailable() const
    {
        return true;
    }

    QGraphicsItem *loadImage(const QString &filePath)
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return NULL;
        return GraphicsItemsFactory::instance().createScalableItem(new WmfPixmapProvider(filePath));
    }
};

DecoderAutoRegistrator registrator(new DecoderLibWmf);

// ====================================================================================================

} // namespace
