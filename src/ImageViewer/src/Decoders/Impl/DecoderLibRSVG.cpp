/*
   Copyright (C) 2018 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

//#define LINKED_LIBRSVG

#include <cmath>
#include <algorithm>

#if defined (LINKED_LIBRSVG)
#include <librsvg/rsvg.h>
#endif

#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QDebug>
#include <QLibrary>
#include <QApplication>

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/Scaling/IScaledImageProvider.h"
#include "Internal/Utils/ZLibUtils.h"

namespace
{

// ====================================================================================================

#if !defined (LINKED_LIBRSVG)

const QString GLIB_LIBRARY_NAME = QString::fromLatin1("glib-2.0");
const QString GOBJECT_LIBRARY_NAME = QString::fromLatin1("gobject-2.0");
const QString CAIRO_LIBRARY_NAME = QString::fromLatin1("cairo");
const QString RSVG_LIBRARY_NAME = QString::fromLatin1("rsvg-2");

struct GError
{
    quint32     domain;
    int         code;
    char       *message;
};

struct RsvgDimensionData
{
    int width;
    int height;
    double em;
    double ex;
};

enum cairo_format_t
{
    CAIRO_FORMAT_ARGB32 = 0,
};

typedef int gboolean;
typedef void* gpointer;
typedef struct _cairo cairo_t;
typedef struct _cairo_surface cairo_surface_t;
typedef struct _RsvgHandle RsvgHandle;

class GLib
{
public:
    static GLib *instance()
    {
        static GLib _;
        if(!_.isValid())
        {
            qWarning() << "Failed to load" << GLIB_LIBRARY_NAME;
            return NULL;
        }
        return &_;
    }

    void g_error_free(GError *error)
    {
        typedef void (*g_error_free_t)(GError*);
        g_error_free_t g_error_free_f = (g_error_free_t)m_g_error_free;
        g_error_free_f(error);
    }

private:
    GLib()
        : m_library(GLIB_LIBRARY_NAME)
        , m_g_error_free(m_library.resolve("g_error_free"))
    {}

    ~GLib()
    {}

    bool isValid() const
    {
        return m_library.isLoaded() && m_g_error_free;
    }

    QLibrary m_library;
    QFunctionPointer m_g_error_free;
};

void g_error_free(GError *error)
{
    if(GLib *glib = GLib::instance())
        return glib->g_error_free(error);
    qWarning() << "Failed to load" << GLIB_LIBRARY_NAME;
}

class GObject
{
public:
    static GObject *instance()
    {
        static GObject _;
        if(!_.isValid())
        {
            qWarning() << "Failed to load" << GOBJECT_LIBRARY_NAME;
            return NULL;
        }
        return &_;
    }

    void g_object_unref(gpointer object)
    {
        typedef void (*g_object_unref_t)(gpointer);
        g_object_unref_t g_object_unref_f = (g_object_unref_t)m_g_object_unref;
        g_object_unref_f(object);
    }

private:
    GObject()
        : m_library(GOBJECT_LIBRARY_NAME)
        , m_g_object_unref(m_library.resolve("g_object_unref"))
    {}

    ~GObject()
    {}

    bool isValid() const
    {
        return m_library.isLoaded() && m_g_object_unref;
    }

    QLibrary m_library;
    QFunctionPointer m_g_object_unref;
};

void g_object_unref(gpointer object)
{
    if(GObject *gobject = GObject::instance())
        return gobject->g_object_unref(object);
    qWarning() << "Failed to load" << GOBJECT_LIBRARY_NAME;
}

class Cairo
{
public:
    static Cairo *instance()
    {
        static Cairo _;
        if(!_.isValid())
        {
            qWarning() << "Failed to load" << CAIRO_LIBRARY_NAME;
            return NULL;
        }
        return &_;
    }

    cairo_surface_t *cairo_image_surface_create_for_data(unsigned char *data, cairo_format_t format, int width, int height, int stride)
    {
        typedef cairo_surface_t *(*cairo_image_surface_create_for_data_t)(unsigned char*, cairo_format_t, int, int, int);
        cairo_image_surface_create_for_data_t cairo_image_surface_create_for_data_f = (cairo_image_surface_create_for_data_t)m_cairo_image_surface_create_for_data;
        return cairo_image_surface_create_for_data_f(data, format, width, height, stride);
    }

    cairo_t *cairo_create(cairo_surface_t *target)
    {
        typedef cairo_t *(*cairo_create_t)(cairo_surface_t*);
        cairo_create_t cairo_create_f = (cairo_create_t)m_cairo_create;
        return cairo_create_f(target);
    }

    void cairo_scale(cairo_t *cr, double sx, double sy)
    {
        typedef void (*cairo_scale_t)(cairo_t*, double, double);
        cairo_scale_t cairo_scale_f = (cairo_scale_t)m_cairo_scale;
        cairo_scale_f(cr, sx, sy);
    }

    void cairo_destroy(cairo_t *cr)
    {
        typedef void (*cairo_destroy_t)(cairo_t*);
        cairo_destroy_t cairo_destroy_f = (cairo_destroy_t)m_cairo_destroy;
        cairo_destroy_f(cr);
    }

    void cairo_surface_destroy(cairo_surface_t *surface)
    {
        typedef void (*cairo_surface_destroy_t)(cairo_surface_t*);
        cairo_surface_destroy_t cairo_surface_destroy_f = (cairo_surface_destroy_t)m_cairo_surface_destroy;
        cairo_surface_destroy_f(surface);
    }

private:
    Cairo()
        : m_library(CAIRO_LIBRARY_NAME)
        , m_cairo_image_surface_create_for_data(m_library.resolve("cairo_image_surface_create_for_data"))
        , m_cairo_create(m_library.resolve("cairo_create"))
        , m_cairo_scale(m_library.resolve("cairo_scale"))
        , m_cairo_destroy(m_library.resolve("cairo_destroy"))
        , m_cairo_surface_destroy(m_library.resolve("cairo_surface_destroy"))
    {}

    ~Cairo()
    {}

    bool isValid() const
    {
        return m_library.isLoaded() && m_cairo_image_surface_create_for_data && m_cairo_create
                && m_cairo_scale && m_cairo_destroy && m_cairo_surface_destroy;
    }

    QLibrary m_library;
    QFunctionPointer m_cairo_image_surface_create_for_data;
    QFunctionPointer m_cairo_create;
    QFunctionPointer m_cairo_scale;
    QFunctionPointer m_cairo_destroy;
    QFunctionPointer m_cairo_surface_destroy;
};

cairo_surface_t *cairo_image_surface_create_for_data(unsigned char *data, cairo_format_t format, int width, int height, int stride)
{
    if(Cairo *cairo = Cairo::instance())
        return cairo->cairo_image_surface_create_for_data(data, format, width, height, stride);
    qWarning() << "Failed to load" << CAIRO_LIBRARY_NAME;
    return NULL;
}

cairo_t *cairo_create(cairo_surface_t *target)
{
    if(Cairo *cairo = Cairo::instance())
        return cairo->cairo_create(target);
    qWarning() << "Failed to load" << CAIRO_LIBRARY_NAME;
    return NULL;
}

void cairo_scale(cairo_t *cr, double sx, double sy)
{
    if(Cairo *cairo = Cairo::instance())
        return cairo->cairo_scale(cr, sx, sy);
    qWarning() << "Failed to load" << CAIRO_LIBRARY_NAME;
}

void cairo_destroy(cairo_t *cr)
{
    if(Cairo *cairo = Cairo::instance())
        return cairo->cairo_destroy(cr);
    qWarning() << "Failed to load" << CAIRO_LIBRARY_NAME;
}

void cairo_surface_destroy(cairo_surface_t *surface)
{
    if(Cairo *cairo = Cairo::instance())
        return cairo->cairo_surface_destroy(surface);
    qWarning() << "Failed to load" << CAIRO_LIBRARY_NAME;
}

class RSVG
{
public:
    static RSVG *instance()
    {
        static RSVG _;
        if(!_.isValid())
        {
            qWarning() << "Failed to load" << RSVG_LIBRARY_NAME;
            return NULL;
        }
        return &_;
    }

    RsvgHandle *rsvg_handle_new_from_data(const quint8 *data, size_t data_len, GError **error)
    {
        typedef RsvgHandle *(*rsvg_handle_new_from_data_t)(const quint8*, size_t, GError**);
        rsvg_handle_new_from_data_t rsvg_handle_new_from_data_f = (rsvg_handle_new_from_data_t)m_rsvg_handle_new_from_data;
        return rsvg_handle_new_from_data_f(data, data_len, error);
    }

    void rsvg_handle_set_base_uri(RsvgHandle *handle, const char *base_uri)
    {
        typedef void (*rsvg_handle_set_base_uri_t)(RsvgHandle*, const char*);
        rsvg_handle_set_base_uri_t rsvg_handle_set_base_uri_f = (rsvg_handle_set_base_uri_t)m_rsvg_handle_set_base_uri;
        rsvg_handle_set_base_uri_f(handle, base_uri);
    }

    void rsvg_handle_get_dimensions(RsvgHandle *handle, RsvgDimensionData *dimension_data)
    {
        typedef void (*rsvg_handle_get_dimensions_t)(RsvgHandle*, RsvgDimensionData*);
        rsvg_handle_get_dimensions_t rsvg_handle_get_dimensions_f = (rsvg_handle_get_dimensions_t)m_rsvg_handle_get_dimensions;
        rsvg_handle_get_dimensions_f(handle, dimension_data);
    }

    gboolean rsvg_handle_render_cairo(RsvgHandle *handle, cairo_t *cr)
    {
        typedef gboolean (*rsvg_handle_render_cairo_t)(RsvgHandle*, cairo_t*);
        rsvg_handle_render_cairo_t rsvg_handle_render_cairo_f = (rsvg_handle_render_cairo_t)m_rsvg_handle_render_cairo;
        return rsvg_handle_render_cairo_f(handle, cr);
    }

private:
    RSVG()
        : m_library(RSVG_LIBRARY_NAME)
        , m_rsvg_handle_new_from_data(m_library.resolve("rsvg_handle_new_from_data"))
        , m_rsvg_handle_set_base_uri(m_library.resolve("rsvg_handle_set_base_uri"))
        , m_rsvg_handle_get_dimensions(m_library.resolve("rsvg_handle_get_dimensions"))
        , m_rsvg_handle_render_cairo(m_library.resolve("rsvg_handle_render_cairo"))
    {}

    ~RSVG()
    {}

    bool isValid() const
    {
        return m_library.isLoaded() && m_rsvg_handle_new_from_data && m_rsvg_handle_set_base_uri
                && m_rsvg_handle_get_dimensions && m_rsvg_handle_render_cairo;
    }

    QLibrary m_library;
    QFunctionPointer m_rsvg_handle_new_from_data;
    QFunctionPointer m_rsvg_handle_set_base_uri;
    QFunctionPointer m_rsvg_handle_get_dimensions;
    QFunctionPointer m_rsvg_handle_render_cairo;
};

RsvgHandle *rsvg_handle_new_from_data(const quint8 *data, size_t data_len, GError **error)
{
    if(RSVG *rsvg = RSVG::instance())
        return rsvg->rsvg_handle_new_from_data(data, data_len, error);
    qWarning() << "Failed to load" << RSVG_LIBRARY_NAME;
    return NULL;
}

void rsvg_handle_set_base_uri(RsvgHandle *handle, const char *base_uri)
{
    if(RSVG *rsvg = RSVG::instance())
        return rsvg->rsvg_handle_set_base_uri(handle, base_uri);
    qWarning() << "Failed to load" << RSVG_LIBRARY_NAME;
}

void rsvg_handle_get_dimensions(RsvgHandle *handle, RsvgDimensionData *dimension_data)
{
    if(RSVG *rsvg = RSVG::instance())
        return rsvg->rsvg_handle_get_dimensions(handle, dimension_data);
    qWarning() << "Failed to load" << RSVG_LIBRARY_NAME;
}

gboolean rsvg_handle_render_cairo(RsvgHandle *handle, cairo_t *cr)
{
    if(RSVG *rsvg = RSVG::instance())
        return rsvg->rsvg_handle_render_cairo(handle, cr);
    qWarning() << "Failed to load" << RSVG_LIBRARY_NAME;
    return 0;
}

bool isReady()
{
    return true
//            && !!GLib::instance()
//            && !!GObject::instance()
            && !!Cairo::instance()
            && !!RSVG::instance()
            ;
}

#else

bool isReady()
{
    return true;
}

#endif

// ====================================================================================================

const qreal MAX_IMAGE_DIMENSION = 16384;
const qreal MIN_IMAGE_DIMENSION = 1;

// ====================================================================================================

class RSVGPixmapProvider : public IScaledImageProvider
{
    Q_DISABLE_COPY(RSVGPixmapProvider)

public:
    RSVGPixmapProvider(const QString &filePath)
        : m_isValid(false)
        , m_rsvg(NULL)
        , m_width(0)
        , m_height(0)
        , m_minScaleFactor(1)
        , m_maxScaleFactor(1)
    {
        QByteArray inBuffer;
        if(QFileInfo(filePath).suffix().toLower() == QString::fromLatin1("svgz"))
        {
            inBuffer = ZLibUtils::InflateFile(filePath);
        }
        else
        {
            QFile inFile(filePath);
            if(!inFile.open(QIODevice::ReadOnly))
            {
                qWarning() << "Can't open" << filePath;
                return;
            }
            inBuffer = inFile.readAll();
        }

        if(inBuffer.isEmpty())
        {
            qWarning() << "Can't read" << filePath;
            return;
        }

        unsigned char *bufferData = reinterpret_cast<unsigned char*>(inBuffer.data());
        const size_t bufferSize = static_cast<size_t>(inBuffer.size());

        GError *error = NULL;
        m_rsvg = rsvg_handle_new_from_data(bufferData, bufferSize, &error);
        if(!m_rsvg)
        {
            qWarning() << "Error reading SVG:" << ((error && error->message) ? error->message : "Unknown error.");
            if(error)
                g_error_free(error);
            return;
        }

        const QByteArray baseUri = QFileInfo(filePath).absolutePath().toLocal8Bit();
        rsvg_handle_set_base_uri(m_rsvg, baseUri.data());

        RsvgDimensionData dimensions;
        memset(&dimensions, 0, sizeof(RsvgDimensionData));
        rsvg_handle_get_dimensions(m_rsvg, &dimensions);

        m_width = dimensions.width;
        m_height = dimensions.height;
        if(m_width < 1 || m_height < 1)
        {
            qWarning() << "Couldn't determine image size";
            return;
        }

        m_isValid = true;
        m_minScaleFactor = std::max(MIN_IMAGE_DIMENSION / m_width, MIN_IMAGE_DIMENSION / m_height);
        m_maxScaleFactor = std::min(MAX_IMAGE_DIMENSION / m_width, MAX_IMAGE_DIMENSION / m_height);
    }

    ~RSVGPixmapProvider()
    {
        if(m_rsvg)
            g_object_unref(m_rsvg);
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
        if(!isValid())
            return QImage();
        const int width  = static_cast<int>(std::ceil(m_width * scaleFactor));
        const int height = static_cast<int>(std::ceil(m_height * scaleFactor));
        QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
        if(image.isNull())
            return image;
        image.fill(Qt::transparent);
        cairo_surface_t *surface = cairo_image_surface_create_for_data(image.bits(), CAIRO_FORMAT_ARGB32, width, height, image.bytesPerLine());
        if(!surface)
            return image;
        cairo_t *cr = cairo_create(surface);
        if(cr)
        {
            cairo_scale(cr, scaleFactor, scaleFactor);
            rsvg_handle_render_cairo(m_rsvg, cr);
            cairo_destroy(cr);
        }
        cairo_surface_destroy(surface);
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
    RsvgHandle *m_rsvg;
    int m_width;
    int m_height;
    qreal m_minScaleFactor;
    qreal m_maxScaleFactor;
};

// ====================================================================================================

class DecoderLibRSVG : public IDecoder
{
public:
    QString name() const
    {
        return QString::fromLatin1("DecoderLibRSVG");
    }

    QStringList supportedFormats() const
    {
        return QStringList()
                << QString::fromLatin1("svg")
                << QString::fromLatin1("svgz")
                   ;
    }

    QStringList advancedFormats() const
    {
        return QStringList();
    }

    QGraphicsItem *loadImage(const QString &filePath)
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable() || !isReady())
            return NULL;
        return GraphicsItemsFactory::instance().createScalableItem(new RSVGPixmapProvider(filePath));
    }
};

DecoderAutoRegistrator registrator(new DecoderLibRSVG);

// ====================================================================================================

} // namespace
