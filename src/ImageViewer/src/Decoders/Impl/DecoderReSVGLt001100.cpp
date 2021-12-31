/*
   Copyright (C) 2018-2021 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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
#define RESVG_QT_BACKEND
extern "C" {
#include <resvg.h>
}
#endif

#include <QFileInfo>
#include <QImage>
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

#if !defined (LINKED_RESVG) || (LINKED_RESVG_VERSION < QT_VERSION_CHECK(0, 11, 0))
namespace
{

// ====================================================================================================

#if !defined (LINKED_RESVG)

const QStringList RESVG_LIBRARY_NAMES = QStringList()
        << QString::fromLatin1("resvg")
        << QString::fromLatin1("libresvg")
        << QString::fromLatin1("resvg_qt")      ///< 0.10.0
        << QString::fromLatin1("libresvg_qt")   ///< 0.10.0
           ;

typedef struct resvg_handle resvg_handle;
typedef struct resvg_render_tree resvg_render_tree;
typedef int resvg_error;
typedef int resvg_fit_to_type;
typedef int resvg_shape_rendering;
typedef int resvg_text_rendering;
typedef int resvg_image_rendering;

struct resvg_color
{
    quint8 r;
    quint8 g;
    quint8 b;
};

struct resvg_fit_to
{
    resvg_fit_to_type type;
    float value;
};

struct resvg_options_v020
{
    const char *path;
    double dpi;
    resvg_fit_to fit_to;
    bool draw_background;
    resvg_color background;
    bool keep_named_groups;
};
const size_t RESVG_OPTIONS_SIZE_V020 = offsetof(resvg_options_v020, keep_named_groups) + sizeof(((resvg_options_v020*)(Q_NULLPTR))->keep_named_groups);

struct resvg_options_v040
{
    const char *path;
    double dpi;
    const char *font_family;
    double font_size;
    const char *languages;
    resvg_fit_to fit_to;
    bool draw_background;
    resvg_color background;
    bool keep_named_groups;
};
const size_t RESVG_OPTIONS_SIZE_V040 = offsetof(resvg_options_v040, keep_named_groups) + sizeof(((resvg_options_v020*)(Q_NULLPTR))->keep_named_groups);

struct resvg_options_v070
{
    const char *path;
    double dpi;
    const char *font_family;
    double font_size;
    const char *languages;
    resvg_shape_rendering shape_rendering;
    resvg_text_rendering text_rendering;
    resvg_image_rendering image_rendering;
    resvg_fit_to fit_to;
    bool draw_background;
    resvg_color background;
    bool keep_named_groups;
};
const size_t RESVG_OPTIONS_SIZE_V070 = offsetof(resvg_options_v070, keep_named_groups) + sizeof(((resvg_options_v020*)(Q_NULLPTR))->keep_named_groups);

union resvg_options
{
    resvg_options_v020 opt_v020;
    resvg_options_v040 opt_v040;
    resvg_options_v070 opt_v070;
};

struct resvg_rect
{
    double x;
    double y;
    double width;
    double height;
};

struct resvg_size
{
    quint32 width;
    quint32 height;
};

class ReSVG
{
public:
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

    void resvg_init_options(resvg_options *opt)
    {
        typedef void (*func_t)(resvg_options*);
        func_t func = (func_t)m_resvg_init_options;
        if(func)
            return func(opt);
        memset(opt, 0, RESVG_OPTIONS_SIZE_V020);
        opt->opt_v020.dpi = 96;
    }

    int resvg_parse_tree_from_data(const char *data, const size_t len, const resvg_options *opt, resvg_render_tree **tree)
    {
        typedef int (*func_t)(const char*, const size_t, const resvg_options*, resvg_render_tree**);
        func_t func = (func_t)m_resvg_parse_tree_from_data;
        return func(data, len, opt, tree);
    }

//    bool resvg_is_image_empty(const resvg_render_tree *tree)
//    {
//        typedef bool (*func_t)(const resvg_render_tree*);
//        func_t func = (func_t)m_resvg_is_image_empty;
//        return func(tree);
//    }

    resvg_size resvg_get_image_size(const resvg_render_tree *tree)
    {
        typedef resvg_size (*func_t)(const resvg_render_tree*);
        func_t func = (func_t)m_resvg_get_image_size;
        return func(tree);
    }

    resvg_rect resvg_get_image_viewbox(const resvg_render_tree *tree)
    {
        typedef resvg_rect (*func_t)(const resvg_render_tree*);
        func_t func = (func_t)m_resvg_get_image_viewbox;
        return func(tree);
    }

    void resvg_tree_destroy(resvg_render_tree *tree)
    {
        typedef void (*func_t)(resvg_render_tree*);
        func_t func = (func_t)m_resvg_tree_destroy;
        return func(tree);
    }

    void resvg_qt_render_to_canvas(const resvg_render_tree *tree, const resvg_options *opt, resvg_size size, void *painter)
    {
        typedef void (*func_t)(const resvg_render_tree*, const resvg_options*, resvg_size size, void*);
        func_t func = (func_t)m_resvg_qt_render_to_canvas;
        return func(tree, opt, size, painter);
    }

private:
    ReSVG()
        : m_resvg_init_log(Q_NULLPTR)
        , m_resvg_init_options(Q_NULLPTR)
        , m_resvg_parse_tree_from_data(Q_NULLPTR)
//        , m_resvg_is_image_empty(Q_NULLPTR)
        , m_resvg_get_image_size(Q_NULLPTR)
        , m_resvg_get_image_viewbox(Q_NULLPTR)
        , m_resvg_tree_destroy(Q_NULLPTR)
        , m_resvg_qt_render_to_canvas(Q_NULLPTR)
        {
            if(!LibraryUtils::LoadQLibrary(m_library, RESVG_LIBRARY_NAMES))
                return;

            m_resvg_init_log = m_library.resolve("resvg_init_log");
            m_resvg_init_options = m_library.resolve("resvg_init_options");
            m_resvg_parse_tree_from_data = m_library.resolve("resvg_parse_tree_from_data");
//            m_resvg_is_image_empty = m_library.resolve("resvg_is_image_empty");
            m_resvg_get_image_size = m_library.resolve("resvg_get_image_size");
            m_resvg_get_image_viewbox = m_library.resolve("resvg_get_image_viewbox");
            m_resvg_tree_destroy = m_library.resolve("resvg_tree_destroy");
            m_resvg_qt_render_to_canvas = m_library.resolve("resvg_qt_render_to_canvas");

//            if(m_resvg_init_log)
//            {
//                typedef void (*func_t)();
//                func_t func = (func_t)m_resvg_init_log;
//                func();
//            }
    }

    ~ReSVG()
    {}

    bool isValid() const
    {
        return m_library.isLoaded()
                /*&& m_resvg_init_options*/
                && m_resvg_parse_tree_from_data && m_resvg_tree_destroy
                /*&& m_resvg_is_image_empty*/
                && m_resvg_get_image_size
                && m_resvg_get_image_viewbox
                && m_resvg_qt_render_to_canvas
                ;
    }

    QLibrary m_library;
    QFunctionPointer m_resvg_init_log;
    QFunctionPointer m_resvg_init_options;
    QFunctionPointer m_resvg_parse_tree_from_data;
//    QFunctionPointer m_resvg_is_image_empty;
    QFunctionPointer m_resvg_get_image_size;
    QFunctionPointer m_resvg_get_image_viewbox;
    QFunctionPointer m_resvg_tree_destroy;
    QFunctionPointer m_resvg_qt_render_to_canvas;
};

void resvg_init_options(resvg_options *opt)
{
    if(ReSVG *resvg = ReSVG::instance())
        return resvg->resvg_init_options(opt);
    qWarning() << "Failed to load resvg";
}

int resvg_parse_tree_from_data(const char *data, const size_t len, const resvg_options *opt, resvg_render_tree **tree)
{
    if(ReSVG *resvg = ReSVG::instance())
        return resvg->resvg_parse_tree_from_data(data, len, opt, tree);
    qWarning() << "Failed to load resvg";
    return -1;
}

//bool resvg_is_image_empty(const resvg_render_tree *tree)
//{
//    if(ReSVG *resvg = ReSVG::instance())
//        return resvg->resvg_is_image_empty(tree);
//    qWarning() << "Failed to load resvg";
//    return true;
//}

resvg_size resvg_get_image_size(const resvg_render_tree *tree)
{
    if(ReSVG *resvg = ReSVG::instance())
        return resvg->resvg_get_image_size(tree);
    qWarning() << "Failed to load resvg";
    resvg_size result;
    memset(&result, 0, sizeof(resvg_size));
    return result;
}

resvg_rect resvg_get_image_viewbox(const resvg_render_tree *tree)
{
    if(ReSVG *resvg = ReSVG::instance())
        return resvg->resvg_get_image_viewbox(tree);
    qWarning() << "Failed to load resvg";
    resvg_rect result;
    memset(&result, 0, sizeof(resvg_rect));
    return result;
}

void resvg_tree_destroy(resvg_render_tree *tree)
{
    if(ReSVG *resvg = ReSVG::instance())
        return resvg->resvg_tree_destroy(tree);
    qWarning() << "Failed to load resvg";
}

void resvg_qt_render_to_canvas(const resvg_render_tree *tree, const resvg_options *opt, resvg_size size, void *painter)
{
    if(ReSVG *resvg = ReSVG::instance())
        return resvg->resvg_qt_render_to_canvas(tree, opt, size, painter);
    qWarning() << "Failed to load resvg";
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
        , m_filePath8bit(filePath.toLocal8Bit())
        , m_tree(Q_NULLPTR)
        , m_width(0)
        , m_height(0)
        , m_minScaleFactor(1)
        , m_maxScaleFactor(1)
    {
#if defined (LINKED_RESVG)
        m_optData.resize(sizeof(resvg_options));
        m_opt = reinterpret_cast<resvg_options*>(m_optData.data());
        resvg_init_options(m_opt);
#else
        const quint8 flagBit = 0xff;
        m_optData.resize(sizeof(resvg_options) * 3);
        m_optData.fill(*reinterpret_cast<const char*>(&flagBit), m_optData.size());
        m_opt = reinterpret_cast<resvg_options*>(m_optData.data());
        resvg_init_options(m_opt);
        int effectiveOptSize = m_optData.size();
        while(effectiveOptSize > 0)
        {
            if(m_optData[effectiveOptSize - 1] != *reinterpret_cast<const char*>(&flagBit))
                break;
            else
                effectiveOptSize--;
        }

        switch(effectiveOptSize)
        {
        case RESVG_OPTIONS_SIZE_V020:
            m_opt->opt_v020.path = m_filePath8bit.constData();
            qDebug() << "Detected v0.2.0 ABI for resvg_options";
            break;
        case RESVG_OPTIONS_SIZE_V040:
            m_opt->opt_v040.path = m_filePath8bit.constData();
            m_opt->opt_v040.font_family = "Times New Roman";
            m_opt->opt_v040.languages = "";
            qDebug() << "Detected v0.4.0 ABI for resvg_options";
            break;
        case RESVG_OPTIONS_SIZE_V070:
            m_opt->opt_v070.path = m_filePath8bit.constData();
            m_opt->opt_v070.font_family = "Times New Roman";
            m_opt->opt_v070.languages = "";
            qDebug() << "Detected v0.7.0 ABI for resvg_options";
            break;
        default:
            qWarning() << "Can't detect ABI for resvg_options";
            qWarning() << "Got:" << effectiveOptSize;
            qWarning() << "Expected:" << RESVG_OPTIONS_SIZE_V020 << RESVG_OPTIONS_SIZE_V040 << RESVG_OPTIONS_SIZE_V070;
            return;
        }
#endif

        MappedBuffer inBuffer(filePath);
        if(!inBuffer.isValid())
            return;

        const int err = resvg_parse_tree_from_data(inBuffer.dataAs<const char*>(), inBuffer.sizeAs<size_t>(), m_opt, &m_tree);
        if(err)
        {
            qWarning() << "Can't parse file, error =" << err;
            return;
        }

        /// @attention WTF?
//        if(!resvg_is_image_empty(m_tree))
//        {
//            qWarning() << "Couldn't determine image nodes";
//            return;
//        }

        m_viewBox = resvg_get_image_viewbox(m_tree);
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
        const int width  = static_cast<int>(std::ceil(m_width * scaleFactor));
        const int height = static_cast<int>(std::ceil(m_height * scaleFactor));
        QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
        if(image.isNull())
            return image;
        QPainter painter(&image);
        image.fill(Qt::transparent);
        painter.setRenderHint(QPainter::Antialiasing);
        const QRectF r = painter.viewport();
        const double sx = r.width() / m_viewBox.width;
        const double sy = r.height() / m_viewBox.height;
        painter.setTransform(QTransform(sx, 0, 0, sy, r.x(), r.y()), true);
        resvg_size imgSize;
        imgSize.width = static_cast<quint32>(m_viewBox.width);
        imgSize.height = static_cast<quint32>(m_viewBox.height);
        resvg_qt_render_to_canvas(m_tree, m_opt, imgSize, &painter);
        painter.end();
        return image;
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
    QByteArray m_filePath8bit;
    resvg_render_tree *m_tree;
    QByteArray m_optData;
    resvg_options *m_opt;
    resvg_rect m_viewBox;
    int m_width;
    int m_height;
    qreal m_minScaleFactor;
    qreal m_maxScaleFactor;
};

// ====================================================================================================

class DecoderReSVGLt001100 : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderReSVGLt001100");
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

DecoderAutoRegistrator registrator(new DecoderReSVGLt001100);

// ====================================================================================================

} // namespace
#endif
