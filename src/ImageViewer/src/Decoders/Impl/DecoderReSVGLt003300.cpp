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

#if defined (HAS_RESVG)
#define LINKED_RESVG
#endif

#include <cmath>
#include <cstring>
#include <algorithm>

#if defined (LINKED_RESVG)
extern "C" {
#include <resvg.h>
}
#endif

#include <QFileInfo>
#include <QImage>
#include <QDir>
#include <QFile>
#include <QByteArray>
#include <QDebug>
#include <QLibrary>
#include <QPainter>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QFunctionPointer>
#else
typedef void* QFunctionPointer;
#endif

#include "Utils/Global.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"
#include "Internal/Scaling/IScaledImageProvider.h"
#include "Internal/Utils/LibraryUtils.h"
#include "Internal/Utils/MappedBuffer.h"

#if defined (RESVG_MAJOR_VERSION) && defined (RESVG_MINOR_VERSION) && defined (RESVG_PATCH_VERSION)
#define LINKED_RESVG_VERSION QT_VERSION_CHECK(RESVG_MAJOR_VERSION, RESVG_MINOR_VERSION, RESVG_PATCH_VERSION)
#else
#define LINKED_RESVG_VERSION QT_VERSION_CHECK(0, 0, 0)
#endif

#if !defined (LINKED_RESVG) || (LINKED_RESVG_VERSION >= QT_VERSION_CHECK(0, 13, 0) && LINKED_RESVG_VERSION < QT_VERSION_CHECK(0, 33, 0))
namespace
{

#if defined (LINKED_RESVG) && (LINKED_RESVG_VERSION < QT_VERSION_CHECK(0, 20, 0))
#define RESVG_FIT_TO_TYPE_ORIGINAL  RESVG_FIT_TO_ORIGINAL
#define RESVG_FIT_TO_TYPE_WIDTH     RESVG_FIT_TO_WIDTH
#define RESVG_FIT_TO_TYPE_HEIGHT    RESVG_FIT_TO_HEIGHT
#define RESVG_FIT_TO_TYPE_ZOOM      RESVG_FIT_TO_ZOOM
#endif

// ====================================================================================================

#if !defined (LINKED_RESVG)

const QStringList RESVG_LIBRARY_NAMES = QStringList()
        << QString::fromLatin1("resvg")
        << QString::fromLatin1("libresvg")
           ;

typedef struct resvg_options resvg_options;
typedef struct resvg_render_tree resvg_render_tree;

typedef enum resvg_fit_to_type
{
    RESVG_FIT_TO_TYPE_ORIGINAL,
    RESVG_FIT_TO_TYPE_WIDTH,
    RESVG_FIT_TO_TYPE_HEIGHT,
    RESVG_FIT_TO_TYPE_ZOOM,
} resvg_fit_to_type;

struct resvg_fit_to
{
    resvg_fit_to_type type;
    float value;
};

struct resvg_size
{
    double width;
    double height;
};

struct resvg_transform
{
    double a;
    double b;
    double c;
    double d;
    double e;
    double f;
};

struct ReSVG
{
    QLibrary library;
    QFunctionPointer resvg_init_log;
    QFunctionPointer resvg_options_create;
    QFunctionPointer resvg_options_destroy;
    QFunctionPointer resvg_options_load_system_fonts;
    QFunctionPointer resvg_options_set_resources_dir;
    QFunctionPointer resvg_parse_tree_from_data;
    QFunctionPointer resvg_get_image_size;
    QFunctionPointer resvg_tree_destroy;
    QFunctionPointer resvg_render;
    QFunctionPointer resvg_transform_identity; ///< @note v0.20.0+
    QFunctionPointer resvg_options_set_keep_named_groups; ///< @note < v0.29.0

    static ReSVG *instance()
    {
        static ReSVG _;
        if(!_.isValid())
        {
            qWarning() << "Failed to load resvg";
            return Q_NULLPTR;
        }
        return &_;
    }

private:
    ReSVG()
        : resvg_init_log(Q_NULLPTR)
        , resvg_options_create(Q_NULLPTR)
        , resvg_options_destroy(Q_NULLPTR)
        , resvg_options_load_system_fonts(Q_NULLPTR)
        , resvg_options_set_resources_dir(Q_NULLPTR)
        , resvg_parse_tree_from_data(Q_NULLPTR)
        , resvg_get_image_size(Q_NULLPTR)
        , resvg_tree_destroy(Q_NULLPTR)
        , resvg_render(Q_NULLPTR)
        , resvg_transform_identity(Q_NULLPTR)
        , resvg_options_set_keep_named_groups(Q_NULLPTR)
        {
            if(!LibraryUtils::LoadQLibrary(library, RESVG_LIBRARY_NAMES))
                return;

            resvg_init_log = library.resolve("resvg_init_log");
            resvg_options_create = library.resolve("resvg_options_create");
            resvg_options_destroy = library.resolve("resvg_options_destroy");
            resvg_options_load_system_fonts = library.resolve("resvg_options_load_system_fonts");
            resvg_options_set_resources_dir = library.resolve("resvg_options_set_resources_dir");
            resvg_parse_tree_from_data = library.resolve("resvg_parse_tree_from_data");
            resvg_get_image_size = library.resolve("resvg_get_image_size");
            resvg_tree_destroy = library.resolve("resvg_tree_destroy");
            resvg_render = library.resolve("resvg_render");
            resvg_transform_identity = library.resolve("resvg_transform_identity");
            resvg_options_set_keep_named_groups = library.resolve("resvg_options_set_keep_named_groups");

//            if(resvg_init_log)
//            {
//                typedef void (*func_t)();
//                func_t func = (func_t)resvg_init_log;
//                func();
//            }
    }

    ~ReSVG()
    {}

    bool isValid() const
    {
        return library.isLoaded()
                && resvg_options_create && resvg_options_destroy
                && resvg_options_load_system_fonts
                && resvg_options_set_resources_dir
                && resvg_parse_tree_from_data && resvg_tree_destroy
                && resvg_get_image_size
                && resvg_render
                && resvg_options_set_keep_named_groups ///< @note < v0.29.0
                ;
    }
};

resvg_options *resvg_options_create()
{
    ReSVG *resvg = ReSVG::instance();
    if(!resvg || !resvg->resvg_options_create)
        return Q_NULLPTR;
    typedef resvg_options *(*func_t)(void);
    func_t func = (func_t)resvg->resvg_options_create;
    return func();
}

void resvg_options_destroy(resvg_options *opt)
{
    ReSVG *resvg = ReSVG::instance();
    if(!resvg || !resvg->resvg_options_destroy)
        return;
    typedef void (*func_t)(resvg_options*);
    func_t func = (func_t)resvg->resvg_options_destroy;
    func(opt);
}

void resvg_options_load_system_fonts(resvg_options *opt)
{
    ReSVG *resvg = ReSVG::instance();
    if(!resvg || !resvg->resvg_options_load_system_fonts)
        return;
    typedef void (*func_t)(resvg_options*);
    func_t func = (func_t)resvg->resvg_options_load_system_fonts;
    func(opt);
}

void resvg_options_set_resources_dir(resvg_options *opt, const char *path)
{
    ReSVG *resvg = ReSVG::instance();
    if(!resvg || !resvg->resvg_options_set_resources_dir)
        return;
    typedef void (*func_t)(resvg_options*, const char*);
    func_t func = (func_t)resvg->resvg_options_set_resources_dir;
    func(opt, path);
}

int resvg_parse_tree_from_data(const char *data, const size_t len, const resvg_options *opt, resvg_render_tree **tree)
{
    ReSVG *resvg = ReSVG::instance();
    if(!resvg || !resvg->resvg_parse_tree_from_data)
        return -1;
    typedef int (*func_t)(const char*, const size_t, const resvg_options*, resvg_render_tree**);
    func_t func = (func_t)resvg->resvg_parse_tree_from_data;
    return func(data, len, opt, tree);
}

void resvg_tree_destroy(resvg_render_tree *tree)
{
    ReSVG *resvg = ReSVG::instance();
    if(!resvg || !resvg->resvg_tree_destroy)
        return;
    typedef void (*func_t)(resvg_render_tree*);
    func_t func = (func_t)resvg->resvg_tree_destroy;
    return func(tree);
}

resvg_size resvg_get_image_size(const resvg_render_tree *tree)
{
    ReSVG *resvg = ReSVG::instance();
    if(!resvg || !resvg->resvg_get_image_size)
        return resvg_size();
    typedef resvg_size (*func_t)(const resvg_render_tree*);
    func_t func = (func_t)resvg->resvg_get_image_size;
    return func(tree);
}

void resvg_render(const resvg_render_tree *tree, resvg_fit_to fit_to, resvg_transform transform, quint32 width, quint32 height, char *pixmap)
{
    ReSVG *resvg = ReSVG::instance();
    if(!resvg || !resvg->resvg_render)
        return;
    if(resvg->resvg_transform_identity) ///< @note v0.20.0+
    {
        typedef void (*func_t)(const resvg_render_tree*, resvg_fit_to, resvg_transform, quint32, quint32, char*);
        func_t func = (func_t)resvg->resvg_render;
        func(tree, fit_to, transform, width, height, pixmap);
    }
    else
    {
        typedef void (*func_t)(const resvg_render_tree*, resvg_fit_to, quint32, quint32, char*);
        func_t func = (func_t)resvg->resvg_render;
        func(tree, fit_to, width, height, pixmap);
    }
}

resvg_transform resvg_transform_identity()
{
    ReSVG *resvg = ReSVG::instance();
    if(!resvg || !resvg->resvg_transform_identity)
        return resvg_transform();
    typedef resvg_transform (*func_t)(void);
    func_t func = (func_t)resvg->resvg_transform_identity;
    return func();
}

bool isReady()
{
    return !!ReSVG::instance();
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

class ReSVGPixmapProvider : public IScaledImageProvider
{
    Q_DISABLE_COPY(ReSVGPixmapProvider)

public:
    explicit ReSVGPixmapProvider(const QString &filePath)
        : m_isValid(false)
        , m_fileDirUtf8(QFileInfo(filePath).dir().absolutePath().toUtf8())
        , m_tree(Q_NULLPTR)
        , m_opt(Q_NULLPTR)
        , m_width(0)
        , m_height(0)
        , m_minScaleFactor(1)
        , m_maxScaleFactor(1)
    {
        m_opt = resvg_options_create();
        resvg_options_load_system_fonts(m_opt);
        resvg_options_set_resources_dir(m_opt, m_fileDirUtf8.constData());

        MappedBuffer inBuffer(filePath, MappedBuffer::AutoInflate | MappedBuffer::AutoConvertXmlToUtf8);
        if(!inBuffer.isValid())
            return;

        const int err = resvg_parse_tree_from_data(inBuffer.dataAs<const char*>(), inBuffer.sizeAs<size_t>(), m_opt, &m_tree);
        if(err)
        {
            qWarning() << "Can't parse file, error =" << err;
            return;
        }

        resvg_size size = resvg_get_image_size(m_tree);
        m_width = static_cast<int>(size.width);
        m_height = static_cast<int>(size.height);
        if(m_width < 1 || m_height < 1)
        {
            qWarning() << "Couldn't determine image size";
            return;
        }

        m_isValid = true;
        m_minScaleFactor = std::max(MIN_IMAGE_DIMENSION / m_width, MIN_IMAGE_DIMENSION / m_height);
        m_maxScaleFactor = std::min(MAX_IMAGE_DIMENSION / m_width, MAX_IMAGE_DIMENSION / m_height);
    }

    ~ReSVGPixmapProvider()
    {
        if(m_tree)
            resvg_tree_destroy(m_tree);
        if(m_opt)
            resvg_options_destroy(m_opt);
    }

    bool isValid() const Q_DECL_OVERRIDE
    {
        return m_isValid;
    }

    bool requiresMainThread() const Q_DECL_OVERRIDE
    {
        return false;
    }

    QRectF boundingRect() const Q_DECL_OVERRIDE
    {
        return QRectF(0, 0, m_width, m_height);
    }

    QImage image(const qreal scaleFactor) Q_DECL_OVERRIDE
    {
        if(!isValid())
            return QImage();

#define USE_RGBA_8888 (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0) && Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
        const int w = static_cast<int>(m_width * scaleFactor);
        const int h = static_cast<int>(m_height * scaleFactor);
        QImage img(w, h,
#if (USE_RGBA_8888)
                   QImage::Format_RGBA8888_Premultiplied);
#else
                   QImage::Format_ARGB32_Premultiplied);
#endif

        if(img.isNull())
        {
            qWarning() << "Invalid image size";
            return img;
        }

        resvg_fit_to fitTo;
        fitTo.type = RESVG_FIT_TO_TYPE_ZOOM;
        fitTo.value = static_cast<float>(scaleFactor);

        img.fill(Qt::transparent);
        resvg_render(m_tree, fitTo,
#if !defined (LINKED_RESVG) || (LINKED_RESVG_VERSION >= QT_VERSION_CHECK(0, 20, 0))
                     resvg_transform_identity(),
#endif
                     static_cast<quint32>(img.width()),
                     static_cast<quint32>(img.height()),
                     reinterpret_cast<char*>(img.bits()));

#if (!USE_RGBA_8888)
        QImage_rgbSwap(img);
#endif
        return img;
#undef USE_RGBA_8888
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
    bool m_isValid;
    QByteArray m_fileDirUtf8;
    resvg_render_tree *m_tree;
    resvg_options *m_opt;
    int m_width;
    int m_height;
    qreal m_minScaleFactor;
    qreal m_maxScaleFactor;
};

// ====================================================================================================

class DecoderReSVGLt003300 : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderReSVGLt003300");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
                << QString::fromLatin1("svg")
                << QString::fromLatin1("svgz")
                   ;
    }

    QStringList advancedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList();
    }

    bool isAvailable() const Q_DECL_OVERRIDE
    {
        return isReady();
    }

    QSharedPointer<IImageData> loadImage(const QString &filePath) Q_DECL_OVERRIDE
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable() || !isAvailable())
            return QSharedPointer<IImageData>();
        IScaledImageProvider *provider = new ReSVGPixmapProvider(filePath);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createScalableItem(provider);
        IImageMetaData *metaData = item ? ImageMetaData::createMetaData(filePath) : Q_NULLPTR;
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), metaData));
    }
};

DecoderAutoRegistrator registrator(new DecoderReSVGLt003300);

// ====================================================================================================

} // namespace
#endif
