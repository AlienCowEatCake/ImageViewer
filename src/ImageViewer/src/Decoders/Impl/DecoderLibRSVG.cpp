/*
   Copyright (C) 2018-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if defined (HAS_LIBRSVG)
#define LINKED_LIBRSVG
#endif

#include <cmath>
#include <cstring>
#include <algorithm>

#if defined (LINKED_LIBRSVG)
#include <librsvg/rsvg.h>
#endif

#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QLibrary>
#include <QPainter>
#include <QGraphicsItem>
#include <QStyleOptionGraphicsItem>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QFunctionPointer>
#else
typedef void* QFunctionPointer;
#endif

#include "Utils/Global.h"
#include "Utils/Logging.h"

#include "../IDecoder.h"
#include "../GraphicsItemFeatures/IGrabImage.h"
#include "../GraphicsItemFeatures/IGrabScaledImage.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/GraphicsItems/GraphicsItemUtils.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"
#include "Internal/Scaling/IScaledImageProvider.h"
#include "Internal/Utils/LibraryUtils.h"
#include "Internal/Utils/MappedBuffer.h"

namespace
{

// ====================================================================================================

#if !defined (LINKED_LIBRSVG)

const QStringList GLIB_LIBRARY_NAMES = QStringList()
        << QString::fromLatin1("glib-2.0")
        << QString::fromLatin1("glib-2.0.0")
        << QString::fromLatin1("libglib-2.0")
        << QString::fromLatin1("libglib-2.0.0")
        << QString::fromLatin1("libglib-2.0-0")
           ;
const QStringList GOBJECT_LIBRARY_NAMES = QStringList()
        << QString::fromLatin1("gobject-2.0")
        << QString::fromLatin1("gobject-2.0.0")
        << QString::fromLatin1("libgobject-2.0")
        << QString::fromLatin1("libgobject-2.0.0")
        << QString::fromLatin1("libgobject-2.0-0")
           ;
const QStringList CAIRO_LIBRARY_NAMES = QStringList()
        << QString::fromLatin1("cairo")
        << QString::fromLatin1("cairo.2")
        << QString::fromLatin1("libcairo")
        << QString::fromLatin1("libcairo.2")
        << QString::fromLatin1("libcairo-2")
           ;
const QStringList RSVG_LIBRARY_NAMES = QStringList()
        << QString::fromLatin1("rsvg-2")
        << QString::fromLatin1("rsvg-2.2")
        << QString::fromLatin1("librsvg-2")
        << QString::fromLatin1("librsvg-2.2")
        << QString::fromLatin1("librsvg-2-2")
           ;

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

struct RsvgRectangle
{
    double x;
    double y;
    double width;
    double height;
};

enum cairo_format_t
{
    CAIRO_FORMAT_ARGB32 = 0,
};

typedef int gboolean;
typedef double gdouble;
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
            LOG_WARNING() << LOGGING_CTX << "Failed to load libglib";
            return Q_NULLPTR;
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
        : m_g_error_free(Q_NULLPTR)
    {
        if(!LibraryUtils::LoadQLibrary(m_library, GLIB_LIBRARY_NAMES))
            return;

        m_g_error_free = m_library.resolve("g_error_free");
    }

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
    LOG_WARNING() << LOGGING_CTX << "Failed to load libglib";
}

class GObject
{
public:
    static GObject *instance()
    {
        static GObject _;
        if(!_.isValid())
        {
            LOG_WARNING() << LOGGING_CTX << "Failed to load libgobject";
            return Q_NULLPTR;
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
        : m_g_object_unref(Q_NULLPTR)
    {
        if(!LibraryUtils::LoadQLibrary(m_library, GOBJECT_LIBRARY_NAMES))
            return;

        m_g_object_unref = m_library.resolve("g_object_unref");
    }

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
    LOG_WARNING() << LOGGING_CTX << "Failed to load libgobject";
}

class Cairo
{
public:
    static Cairo *instance()
    {
        static Cairo _;
        if(!_.isValid())
        {
            LOG_WARNING() << LOGGING_CTX << "Failed to load libcairo";
            return Q_NULLPTR;
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

    void cairo_translate(cairo_t *cr, double tx, double ty)
    {
        typedef void (*cairo_translate_t)(cairo_t*, double, double);
        cairo_translate_t cairo_translate_f = (cairo_translate_t)m_cairo_translate;
        cairo_translate_f(cr, tx, ty);
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
        : m_cairo_image_surface_create_for_data(Q_NULLPTR)
        , m_cairo_create(Q_NULLPTR)
        , m_cairo_scale(Q_NULLPTR)
        , m_cairo_translate(Q_NULLPTR)
        , m_cairo_destroy(Q_NULLPTR)
        , m_cairo_surface_destroy(Q_NULLPTR)
    {
        if(!LibraryUtils::LoadQLibrary(m_library, CAIRO_LIBRARY_NAMES))
            return;

        m_cairo_image_surface_create_for_data = m_library.resolve("cairo_image_surface_create_for_data");
        m_cairo_create = m_library.resolve("cairo_create");
        m_cairo_scale = m_library.resolve("cairo_scale");
        m_cairo_translate = m_library.resolve("cairo_translate");
        m_cairo_destroy = m_library.resolve("cairo_destroy");
        m_cairo_surface_destroy = m_library.resolve("cairo_surface_destroy");
    }

    ~Cairo()
    {}

    bool isValid() const
    {
        return m_library.isLoaded() && m_cairo_image_surface_create_for_data && m_cairo_create
                && m_cairo_scale && m_cairo_translate && m_cairo_destroy && m_cairo_surface_destroy;
    }

    QLibrary m_library;
    QFunctionPointer m_cairo_image_surface_create_for_data;
    QFunctionPointer m_cairo_create;
    QFunctionPointer m_cairo_scale;
    QFunctionPointer m_cairo_translate;
    QFunctionPointer m_cairo_destroy;
    QFunctionPointer m_cairo_surface_destroy;
};

cairo_surface_t *cairo_image_surface_create_for_data(unsigned char *data, cairo_format_t format, int width, int height, int stride)
{
    if(Cairo *cairo = Cairo::instance())
        return cairo->cairo_image_surface_create_for_data(data, format, width, height, stride);
    LOG_WARNING() << LOGGING_CTX << "Failed to load libcairo";
    return Q_NULLPTR;
}

cairo_t *cairo_create(cairo_surface_t *target)
{
    if(Cairo *cairo = Cairo::instance())
        return cairo->cairo_create(target);
    LOG_WARNING() << LOGGING_CTX << "Failed to load libcairo";
    return Q_NULLPTR;
}

void cairo_scale(cairo_t *cr, double sx, double sy)
{
    if(Cairo *cairo = Cairo::instance())
        return cairo->cairo_scale(cr, sx, sy);
    LOG_WARNING() << LOGGING_CTX << "Failed to load libcairo";
}

void cairo_translate(cairo_t *cr, double tx, double ty)
{
    if(Cairo *cairo = Cairo::instance())
        return cairo->cairo_translate(cr, tx, ty);
    LOG_WARNING() << LOGGING_CTX << "Failed to load libcairo";
}

void cairo_destroy(cairo_t *cr)
{
    if(Cairo *cairo = Cairo::instance())
        return cairo->cairo_destroy(cr);
    LOG_WARNING() << LOGGING_CTX << "Failed to load libcairo";
}

void cairo_surface_destroy(cairo_surface_t *surface)
{
    if(Cairo *cairo = Cairo::instance())
        return cairo->cairo_surface_destroy(surface);
    LOG_WARNING() << LOGGING_CTX << "Failed to load libcairo";
}

class RSVG
{
public:
    static RSVG *instance()
    {
        static RSVG _;
        if(!_.isValid())
        {
            LOG_WARNING() << LOGGING_CTX << "Failed to load librsvg";
            return Q_NULLPTR;
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

    bool has_rsvg_handle_get_dimensions() const
    {
        return !!m_rsvg_handle_get_dimensions;
    }

    void rsvg_handle_get_dimensions(RsvgHandle *handle, RsvgDimensionData *dimension_data)
    {
        typedef void (*rsvg_handle_get_dimensions_t)(RsvgHandle*, RsvgDimensionData*);
        rsvg_handle_get_dimensions_t rsvg_handle_get_dimensions_f = (rsvg_handle_get_dimensions_t)m_rsvg_handle_get_dimensions;
        rsvg_handle_get_dimensions_f(handle, dimension_data);
    }

    bool has_rsvg_handle_get_intrinsic_size_in_pixels() const
    {
        return !!m_rsvg_handle_get_intrinsic_size_in_pixels;
    }

    gboolean rsvg_handle_get_intrinsic_size_in_pixels(RsvgHandle *handle, gdouble *out_width, gdouble *out_height)
    {
        typedef gboolean (*rsvg_handle_get_intrinsic_size_in_pixels_t)(RsvgHandle*, gdouble*, gdouble*);
        rsvg_handle_get_intrinsic_size_in_pixels_t rsvg_handle_get_intrinsic_size_in_pixels_f = (rsvg_handle_get_intrinsic_size_in_pixels_t)m_rsvg_handle_get_intrinsic_size_in_pixels;
        return rsvg_handle_get_intrinsic_size_in_pixels_f(handle, out_width, out_height);
    }

    bool has_rsvg_handle_render_cairo() const
    {
        return !!m_rsvg_handle_render_cairo;
    }

    gboolean rsvg_handle_render_cairo(RsvgHandle *handle, cairo_t *cr)
    {
        typedef gboolean (*rsvg_handle_render_cairo_t)(RsvgHandle*, cairo_t*);
        rsvg_handle_render_cairo_t rsvg_handle_render_cairo_f = (rsvg_handle_render_cairo_t)m_rsvg_handle_render_cairo;
        return rsvg_handle_render_cairo_f(handle, cr);
    }

    bool has_rsvg_handle_render_document() const
    {
        return !!m_rsvg_handle_render_document;
    }

    gboolean rsvg_handle_render_document(RsvgHandle *handle, cairo_t *cr, const RsvgRectangle *viewport, GError **error)
    {
        typedef gboolean (*rsvg_handle_render_document_t)(RsvgHandle*, cairo_t*, const RsvgRectangle*, GError**);
        rsvg_handle_render_document_t rsvg_handle_render_document_f = (rsvg_handle_render_document_t)m_rsvg_handle_render_document;
        return rsvg_handle_render_document_f(handle, cr, viewport, error);
    }

private:
    RSVG()
        : m_rsvg_handle_new_from_data(Q_NULLPTR)
        , m_rsvg_handle_set_base_uri(Q_NULLPTR)
        , m_rsvg_handle_get_dimensions(Q_NULLPTR)
        , m_rsvg_handle_get_intrinsic_size_in_pixels(Q_NULLPTR)
        , m_rsvg_handle_render_cairo(Q_NULLPTR)
        , m_rsvg_handle_render_document(Q_NULLPTR)
    {
        if(!LibraryUtils::LoadQLibrary(m_library, RSVG_LIBRARY_NAMES))
            return;

        m_rsvg_handle_new_from_data = m_library.resolve("rsvg_handle_new_from_data");
        m_rsvg_handle_set_base_uri = m_library.resolve("rsvg_handle_set_base_uri");
        m_rsvg_handle_get_dimensions = m_library.resolve("rsvg_handle_get_dimensions");
        m_rsvg_handle_get_intrinsic_size_in_pixels = m_library.resolve("rsvg_handle_get_intrinsic_size_in_pixels");
        m_rsvg_handle_render_cairo = m_library.resolve("rsvg_handle_render_cairo");
        m_rsvg_handle_render_document = m_library.resolve("rsvg_handle_render_document");
    }

    ~RSVG()
    {}

    bool isValid() const
    {
        return m_library.isLoaded() && m_rsvg_handle_new_from_data && m_rsvg_handle_set_base_uri
                && (m_rsvg_handle_get_dimensions || m_rsvg_handle_get_intrinsic_size_in_pixels)
                && (m_rsvg_handle_render_cairo || m_rsvg_handle_render_document);
    }

    QLibrary m_library;
    QFunctionPointer m_rsvg_handle_new_from_data;
    QFunctionPointer m_rsvg_handle_set_base_uri;
    QFunctionPointer m_rsvg_handle_get_dimensions;
    QFunctionPointer m_rsvg_handle_get_intrinsic_size_in_pixels;
    QFunctionPointer m_rsvg_handle_render_cairo;
    QFunctionPointer m_rsvg_handle_render_document;
};

RsvgHandle *rsvg_handle_new_from_data(const quint8 *data, size_t data_len, GError **error)
{
    if(RSVG *rsvg = RSVG::instance())
        return rsvg->rsvg_handle_new_from_data(data, data_len, error);
    LOG_WARNING() << LOGGING_CTX << "Failed to load librsvg";
    return Q_NULLPTR;
}

void rsvg_handle_set_base_uri(RsvgHandle *handle, const char *base_uri)
{
    if(RSVG *rsvg = RSVG::instance())
        return rsvg->rsvg_handle_set_base_uri(handle, base_uri);
    LOG_WARNING() << LOGGING_CTX << "Failed to load librsvg";
}

bool has_rsvg_handle_get_dimensions()
{
    if(const RSVG *rsvg = RSVG::instance())
        return rsvg->has_rsvg_handle_get_dimensions();
    return false;
}

void rsvg_handle_get_dimensions(RsvgHandle *handle, RsvgDimensionData *dimension_data)
{
    if(RSVG *rsvg = RSVG::instance())
        return rsvg->rsvg_handle_get_dimensions(handle, dimension_data);
    LOG_WARNING() << LOGGING_CTX << "Failed to load librsvg";
}

bool has_rsvg_handle_get_intrinsic_size_in_pixels()
{
    if(const RSVG *rsvg = RSVG::instance())
        return rsvg->has_rsvg_handle_get_intrinsic_size_in_pixels();
    return false;
}

gboolean rsvg_handle_get_intrinsic_size_in_pixels(RsvgHandle *handle, gdouble *out_width, gdouble *out_height)
{
    if(RSVG *rsvg = RSVG::instance())
        return rsvg->rsvg_handle_get_intrinsic_size_in_pixels(handle, out_width, out_height);
    LOG_WARNING() << LOGGING_CTX << "Failed to load librsvg";
    return 0;
}

bool has_rsvg_handle_render_cairo()
{
    if(const RSVG *rsvg = RSVG::instance())
        return rsvg->has_rsvg_handle_render_cairo();
    return false;
}

gboolean rsvg_handle_render_cairo(RsvgHandle *handle, cairo_t *cr)
{
    if(RSVG *rsvg = RSVG::instance())
        return rsvg->rsvg_handle_render_cairo(handle, cr);
    LOG_WARNING() << LOGGING_CTX << "Failed to load librsvg";
    return 0;
}

bool has_rsvg_handle_render_document()
{
    if(const RSVG *rsvg = RSVG::instance())
        return rsvg->has_rsvg_handle_render_document();
    return false;
}

gboolean rsvg_handle_render_document(RsvgHandle *handle, cairo_t *cr, const RsvgRectangle *viewport, GError **error)
{
    if(RSVG *rsvg = RSVG::instance())
        return rsvg->rsvg_handle_render_document(handle, cr, viewport, error);
    LOG_WARNING() << LOGGING_CTX << "Failed to load librsvg";
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

bool has_rsvg_handle_get_intrinsic_size_in_pixels()
{
#if QT_VERSION_CHECK(LIBRSVG_MAJOR_VERSION, LIBRSVG_MINOR_VERSION, LIBRSVG_MICRO_VERSION) >= QT_VERSION_CHECK(2, 52, 0)
    return true;
#else
    return false;
#endif
}

bool has_rsvg_handle_get_dimensions()
{
    return true;
}

bool has_rsvg_handle_render_document()
{
#if QT_VERSION_CHECK(LIBRSVG_MAJOR_VERSION, LIBRSVG_MINOR_VERSION, LIBRSVG_MICRO_VERSION) >= QT_VERSION_CHECK(2, 46, 0)
    return true;
#else
    return false;
#endif
}

bool has_rsvg_handle_render_cairo()
{
    return !has_rsvg_handle_render_document();
}

bool isReady()
{
    return true;
}

#endif

// ====================================================================================================

const qreal MAX_IMAGE_DIMENSION = 16384;
const qreal MIN_IMAGE_DIMENSION = 1;
const qreal MAX_SCALE_FOR_PARTIAL_RENDER = 1000;

// ====================================================================================================

class RSVGGraphicsItem :
        public QGraphicsItem,
        public IGrabImage,
        public IGrabScaledImage,
        public IScaledImageProvider
{
    Q_DISABLE_COPY(RSVGGraphicsItem)

public:
    explicit RSVGGraphicsItem(const QString &filePath, QGraphicsItem *parentItem = Q_NULLPTR)
        : QGraphicsItem(parentItem)
        , m_isValid(false)
        , m_exposedRectSupported(false)
        , m_rsvg(Q_NULLPTR)
        , m_width(0)
        , m_height(0)
        , m_minScaleFactor(1)
        , m_maxScaleFactor(1)
    {
#if (QT_VERSION >= QT_VERSION_CHECK(4, 6, 0))
        setFlag(QGraphicsItem::ItemUsesExtendedStyleOption, true);
#endif
        m_rasterizerCache.scaleFactor = 0;

        if(!filePath.isEmpty())
            load(filePath);
    }

    ~RSVGGraphicsItem()
    {
        if(m_rsvg)
            g_object_unref(m_rsvg);
    }

    bool exposedRectSupported() const
    {
        return m_exposedRectSupported;
    }

    bool load(const QString &filePath)
    {
        if(m_rsvg)
            g_object_unref(m_rsvg);
        m_rsvg = Q_NULLPTR;
        m_isValid = false;
        m_exposedRectSupported = false;
        m_width = m_height = 0;
        m_minScaleFactor = m_maxScaleFactor = 1;
        m_rasterizerCache.scaleFactor = 0;

        MappedBuffer inBuffer(filePath, MappedBuffer::AutoInflate);
        if(!inBuffer.isValid())
            return false;

        GError *error = Q_NULLPTR;
        m_rsvg = rsvg_handle_new_from_data(inBuffer.dataAs<unsigned char*>(), inBuffer.sizeAs<size_t>(), &error);
        if(!m_rsvg)
        {
            LOG_WARNING() << LOGGING_CTX << "Error reading SVG:" << ((error && error->message) ? error->message : "Unknown error.");
            if(error)
                g_error_free(error);
            return false;
        }

        const QByteArray baseUri = QFileInfo(filePath).absolutePath().toLocal8Bit();
        rsvg_handle_set_base_uri(m_rsvg, baseUri.data());

        bool sizeDetected = false;
        if(!sizeDetected && has_rsvg_handle_get_intrinsic_size_in_pixels())
        {
#if !defined(LINKED_LIBRSVG) || QT_VERSION_CHECK(LIBRSVG_MAJOR_VERSION, LIBRSVG_MINOR_VERSION, LIBRSVG_MICRO_VERSION) >= QT_VERSION_CHECK(2, 52, 0)
            gdouble w = 0.0, h = 0.0;
            if(rsvg_handle_get_intrinsic_size_in_pixels(m_rsvg, &w, &h))
            {
                m_width = w;
                m_height = h;
                sizeDetected = true;
            }
#endif
        }
        if(!sizeDetected && has_rsvg_handle_get_dimensions())
        {
            RsvgDimensionData dimensions;
            memset(&dimensions, 0, sizeof(RsvgDimensionData));
            rsvg_handle_get_dimensions(m_rsvg, &dimensions);
            m_width = dimensions.width;
            m_height = dimensions.height;
            sizeDetected = true;
        }
        if(!sizeDetected)
        {
            m_width = m_height = 512.0;
        }

        if(m_width < 1 || m_height < 1)
        {
            LOG_WARNING() << LOGGING_CTX << "Couldn't determine image size";
            return false;
        }

        m_isValid = true;
        m_minScaleFactor = std::max(MIN_IMAGE_DIMENSION / m_width, MIN_IMAGE_DIMENSION / m_height);
        m_maxScaleFactor = std::min(MAX_IMAGE_DIMENSION / m_width, MAX_IMAGE_DIMENSION / m_height);

        m_exposedRectSupported = true;

        return true;
    }

    QImage grabImage() Q_DECL_OVERRIDE
    {
        return grabImage(1.0);
    }

    QImage grabImage(qreal scaleFactor) Q_DECL_OVERRIDE
    {
        return grabImage(scaleFactor, boundingRect());
    }

    QImage grabImage(qreal scaleFactor, const QRectF &exposedRect)
    {
        if(!isValid())
            return QImage();
        const int width  = static_cast<int>(std::ceil(exposedRect.width() * scaleFactor));
        const int height = static_cast<int>(std::ceil(exposedRect.height() * scaleFactor));
        QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
        if(image.isNull())
        {
            LOG_WARNING() << LOGGING_CTX << "Invalid image size";
            return image;
        }
        image.fill(Qt::transparent);
        cairo_surface_t *surface = cairo_image_surface_create_for_data(image.bits(), CAIRO_FORMAT_ARGB32, width, height, image.bytesPerLine());
        if(!surface)
            return image;
        cairo_t *cr = cairo_create(surface);
        if(cr)
        {
            if(has_rsvg_handle_render_document())
            {
#if !defined(LINKED_LIBRSVG) || QT_VERSION_CHECK(LIBRSVG_MAJOR_VERSION, LIBRSVG_MINOR_VERSION, LIBRSVG_MICRO_VERSION) >= QT_VERSION_CHECK(2, 46, 0)
                GError *error = Q_NULLPTR;
                RsvgRectangle viewport;
    #if 0 ///< @todo It works, but slower... ¯\_(ツ)_/¯
                viewport.x = -exposedRect.x() * scaleFactor;
                viewport.y = -exposedRect.y() * scaleFactor;
                viewport.width = m_width * scaleFactor;
                viewport.height = m_height * scaleFactor;
    #else
                viewport.x = 0;
                viewport.y = 0;
                viewport.width = m_width;
                viewport.height = m_height;
                cairo_scale(cr, scaleFactor, scaleFactor);
                cairo_translate(cr, -exposedRect.x(), -exposedRect.y());
    #endif
                if(!rsvg_handle_render_document(m_rsvg, cr, &viewport, &error))
                {
                    LOG_WARNING() << LOGGING_CTX << "Error rendering SVG document:" << ((error && error->message) ? error->message : "Unknown error.");
                    if(error)
                        g_error_free(error);
                }
#endif
            }
            else if(has_rsvg_handle_render_cairo())
            {
#if !defined(LINKED_LIBRSVG) || QT_VERSION_CHECK(LIBRSVG_MAJOR_VERSION, LIBRSVG_MINOR_VERSION, LIBRSVG_MICRO_VERSION) < QT_VERSION_CHECK(2, 46, 0)
                cairo_scale(cr, scaleFactor, scaleFactor);
                cairo_translate(cr, -exposedRect.x(), -exposedRect.y());
                rsvg_handle_render_cairo(m_rsvg, cr);
#endif
            }
            cairo_destroy(cr);
        }
        cairo_surface_destroy(surface);
        return image;
    }

    QRectF boundingRect() const Q_DECL_OVERRIDE
    {
        return QRectF(0, 0, m_width, m_height);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) Q_DECL_OVERRIDE
    {
        Q_UNUSED(widget);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setRenderHint(QPainter::TextAntialiasing);
        painter->setRenderHint(QPainter::SmoothPixmapTransform);
        const qreal scaleFactor = std::min(GraphicsItemUtils::GetDeviceScaleFactor(painter), std::max(MAX_SCALE_FOR_PARTIAL_RENDER, maxScaleFactor()));
        const qreal offset = 2.0 / scaleFactor;
        const QRectF exposedRect = option->exposedRect.adjusted(-offset, -offset, offset, offset).intersected(boundingRect());
        if(!GraphicsItemUtils::IsFuzzyEqualScaleFactors(scaleFactor, m_rasterizerCache.scaleFactor) || !m_rasterizerCache.exposedRect.contains(exposedRect))
        {
            m_rasterizerCache.image = grabImage(scaleFactor, exposedRect);
            m_rasterizerCache.exposedRect = exposedRect;
            m_rasterizerCache.scaleFactor = scaleFactor;
        }
        painter->drawImage(m_rasterizerCache.exposedRect, m_rasterizerCache.image, m_rasterizerCache.image.rect());
    }

    bool isValid() const Q_DECL_OVERRIDE
    {
        return m_isValid;
    }

    bool requiresMainThread() const Q_DECL_OVERRIDE
    {
        return false;
    }

    QImage image(const qreal scaleFactor) Q_DECL_OVERRIDE
    {
        return grabImage(scaleFactor);
    }

    qreal minScaleFactor() const Q_DECL_OVERRIDE
    {
        return m_minScaleFactor;
    }

    qreal maxScaleFactor() const Q_DECL_OVERRIDE
    {
        return m_maxScaleFactor;
    }

private:
    struct RasterizerCache
    {
        QImage image;
        qreal scaleFactor;
        QRectF exposedRect;
    };

    bool m_isValid;
    bool m_exposedRectSupported;
    RsvgHandle *m_rsvg;
    qreal m_width;
    qreal m_height;
    RasterizerCache m_rasterizerCache;
    qreal m_minScaleFactor;
    qreal m_maxScaleFactor;
};

// ====================================================================================================

class DecoderLibRSVG : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderLibRSVG");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
                << QString::fromLatin1("svg")
#if defined (HAS_ZLIB)
                << QString::fromLatin1("svgz")
#endif
                   ;
    }

    QStringList advancedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList();
    }

    bool isAvailable() const Q_DECL_OVERRIDE
    {
        static const bool ready = isReady();
        return ready;
    }

    QSharedPointer<IImageData> loadImage(const QString &filePath) Q_DECL_OVERRIDE
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable() || !isAvailable())
            return QSharedPointer<IImageData>();

        RSVGGraphicsItem *rsvgItem = new RSVGGraphicsItem(filePath);
        QGraphicsItem *item = rsvgItem;
        if(!rsvgItem->isValid() || !rsvgItem->exposedRectSupported())
        {
            IScaledImageProvider *provider = rsvgItem;
            item = GraphicsItemsFactory::instance().createScalableItem(provider);
        }
        IImageMetaData *metaData = item ? ImageMetaData::createMetaData(filePath) : Q_NULLPTR;
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), metaData));
    }
};

DecoderAutoRegistrator registrator(new DecoderLibRSVG);

// ====================================================================================================

} // namespace
