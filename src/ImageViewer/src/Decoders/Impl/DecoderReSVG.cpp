/*
   Copyright (C) 2018-2023 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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
#include <QGraphicsItem>
#include <QStyleOptionGraphicsItem>
#include <QVariant>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QFunctionPointer>
#else
typedef void* QFunctionPointer;
#endif

#include "Utils/Global.h"

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
#include "Internal/Utils/XmlStreamReader.h"

#if defined (RESVG_MAJOR_VERSION) && defined (RESVG_MINOR_VERSION) && defined (RESVG_PATCH_VERSION)
#define LINKED_RESVG_VERSION QT_VERSION_CHECK(RESVG_MAJOR_VERSION, RESVG_MINOR_VERSION, RESVG_PATCH_VERSION)
#else
#define LINKED_RESVG_VERSION QT_VERSION_CHECK(0, 0, 0)
#endif

#if !defined (LINKED_RESVG) || (LINKED_RESVG_VERSION >= QT_VERSION_CHECK(0, 33, 0))
namespace
{

// ====================================================================================================

#if !defined (LINKED_RESVG)

const QStringList RESVG_LIBRARY_NAMES = QStringList()
        << QString::fromLatin1("resvg")
        << QString::fromLatin1("libresvg")
           ;

typedef struct resvg_options resvg_options;
typedef struct resvg_render_tree resvg_render_tree;

struct resvg_size
{
    float width;
    float height;
};

struct resvg_rect {
    float x;
    float y;
    float width;
    float height;
};

struct resvg_transform
{
    float a;
    float b;
    float c;
    float d;
    float e;
    float f;
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
    QFunctionPointer resvg_transform_identity;
    QFunctionPointer resvg_options_set_keep_named_groups; ///< @note < v0.29.0
    QFunctionPointer resvg_get_image_bbox;
    QFunctionPointer resvg_get_node_stroke_bbox; ///< @note v0.38.0+

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
        , resvg_get_image_bbox(Q_NULLPTR)
        , resvg_get_node_stroke_bbox(Q_NULLPTR)
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
            resvg_get_image_bbox = library.resolve("resvg_get_image_bbox");
            resvg_get_node_stroke_bbox = library.resolve("resvg_get_node_stroke_bbox");

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
                && resvg_transform_identity
                && resvg_get_image_bbox
                && !resvg_options_set_keep_named_groups
                && (resvg_get_node_stroke_bbox || check_abi_003400())
                ;
    }

private:
    bool check_abi_003400() const
    {
        static const bool result = do_check_abi_003400();
        return result;
    }

    bool do_check_abi_003400() const
    {
        typedef resvg_options *(*resvg_options_create_t)(void);
        resvg_options_create_t resvg_options_create_f = (resvg_options_create_t)resvg_options_create;
        typedef void (*resvg_options_destroy_t)(resvg_options*);
        resvg_options_destroy_t resvg_options_destroy_f = (resvg_options_destroy_t)resvg_options_destroy;
        typedef int (*resvg_parse_tree_from_data_t)(const char*, const size_t, const resvg_options*, resvg_render_tree**);
        resvg_parse_tree_from_data_t resvg_parse_tree_from_data_f = (resvg_parse_tree_from_data_t)resvg_parse_tree_from_data;
        typedef void (*resvg_tree_destroy_t)(resvg_render_tree*);
        resvg_tree_destroy_t resvg_tree_destroy_f = (resvg_tree_destroy_t)resvg_tree_destroy;
        typedef bool (*resvg_get_image_bbox_t)(const resvg_render_tree*, resvg_rect*);
        resvg_get_image_bbox_t resvg_get_image_bbox_f = (resvg_get_image_bbox_t)resvg_get_image_bbox;

        bool result = false;
        const QByteArray dataBuf("<svg xmlns='http://www.w3.org/2000/svg' viewBox='1 1 9 9'><circle cx='4' cy='4' r='2' fill='red'/></svg>");
        const char *data = dataBuf.constData();
        const size_t len = dataBuf.size();

        resvg_options *opt = resvg_options_create_f();
        if(!opt)
            return result;

        resvg_render_tree *tree = Q_NULLPTR;
        const int err = resvg_parse_tree_from_data_f(data, len, opt, &tree);
        if(err)
        {
            resvg_options_destroy_f(opt);
            return result;
        }

        QByteArray bboxData;
        const quint8 flagBit = 0xff;
        bboxData.resize(sizeof(resvg_rect) * 3);
        bboxData.fill(*reinterpret_cast<const char*>(&flagBit), bboxData.size());
        resvg_rect *bbox = reinterpret_cast<resvg_rect*>(bboxData.data());
        if(!resvg_get_image_bbox_f(tree, bbox))
        {
            resvg_tree_destroy_f(tree);
            resvg_options_destroy_f(opt);
            return result;
        }

        for(int i = sizeof(resvg_rect); i < bboxData.size(); ++i)
        {
            if(bboxData[i] != *reinterpret_cast<const char*>(&flagBit))
            {
                resvg_tree_destroy_f(tree);
                resvg_options_destroy_f(opt);
                return result;
            }
        }

        result = true
            && qFuzzyCompare(bbox->width, 4.0f)
            && qFuzzyCompare(bbox->height, 4.0f)
            && qFuzzyCompare(bbox->x, 2.0f)
            && qFuzzyCompare(bbox->y, 2.0f)
            ;

        resvg_tree_destroy_f(tree);
        resvg_options_destroy_f(opt);
        return result;
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

void resvg_render(const resvg_render_tree *tree, resvg_transform transform, quint32 width, quint32 height, char *pixmap)
{
    ReSVG *resvg = ReSVG::instance();
    if(!resvg || !resvg->resvg_render)
        return;
    typedef void (*func_t)(const resvg_render_tree*, resvg_transform, quint32, quint32, char*);
    func_t func = (func_t)resvg->resvg_render;
    func(tree, transform, width, height, pixmap);
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
const qreal MAX_SCALE_FOR_PARTIAL_RENDER = 1000;

// ====================================================================================================

class ReSVGGraphicsItem :
        public QGraphicsItem,
        public IGrabImage,
        public IGrabScaledImage,
        public IScaledImageProvider
{
    Q_DISABLE_COPY(ReSVGGraphicsItem)

public:
    explicit ReSVGGraphicsItem(const QString &filePath, QGraphicsItem *parentItem = Q_NULLPTR)
        : QGraphicsItem(parentItem)
        , m_isValid(false)
        , m_exposedRectSupported(false)
        , m_tree(Q_NULLPTR)
        , m_opt(Q_NULLPTR)
        , m_width(0)
        , m_height(0)
        , m_minScaleFactor(1)
        , m_maxScaleFactor(1)
    {
#if (QT_VERSION >= QT_VERSION_CHECK(4, 6, 0))
        setFlag(QGraphicsItem::ItemUsesExtendedStyleOption, true);
#endif
        m_rasterizerCache.scaleFactor = 0;

        m_opt = resvg_options_create();
        resvg_options_load_system_fonts(m_opt);

        if(!filePath.isEmpty())
            load(filePath);
    }

    ~ReSVGGraphicsItem()
    {
        if(m_tree)
            resvg_tree_destroy(m_tree);
        if(m_opt)
            resvg_options_destroy(m_opt);
    }

    bool exposedRectSupported() const
    {
        return m_exposedRectSupported;
    }

    bool load(const QString &filePath)
    {
        if(m_tree)
            resvg_tree_destroy(m_tree);
        m_tree = Q_NULLPTR;
        m_isValid = false;
        m_exposedRectSupported = false;
        m_width = m_height = 0;
        m_minScaleFactor = m_maxScaleFactor = 1;
        m_rasterizerCache.scaleFactor = 0;

        MappedBuffer inBuffer(filePath, MappedBuffer::AutoInflate | MappedBuffer::AutoConvertXmlToUtf8);
        if(!inBuffer.isValid())
            return false;

        m_fileDirUtf8 = QFileInfo(filePath).dir().absolutePath().toUtf8();
        resvg_options_set_resources_dir(m_opt, m_fileDirUtf8.constData());

        const int err = resvg_parse_tree_from_data(inBuffer.dataAs<const char*>(), inBuffer.sizeAs<size_t>(), m_opt, &m_tree);
        if(err)
        {
            qWarning() << "Can't parse file, error =" << err;
            return false;
        }

        resvg_size size = resvg_get_image_size(m_tree);
        m_width = static_cast<qreal>(size.width);
        m_height = static_cast<qreal>(size.height);
        if(m_width < 1 || m_height < 1)
        {
            qWarning() << "Couldn't determine image size";
            return false;
        }

        // https://github.com/RazrFalcon/resvg/issues/642
        m_exposedRectSupported = !hasClipPath(inBuffer.dataAsByteArray());
        if(!m_exposedRectSupported)
            qWarning() << "Found clipPath, disable exposedRect";

        m_isValid = true;
        m_minScaleFactor = std::max(MIN_IMAGE_DIMENSION / m_width, MIN_IMAGE_DIMENSION / m_height);
        m_maxScaleFactor = std::min(MAX_IMAGE_DIMENSION / m_width, MAX_IMAGE_DIMENSION / m_height);
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

#define USE_RGBA_8888 (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0) && Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
        const int w = static_cast<int>(exposedRect.width() * scaleFactor);
        const int h = static_cast<int>(exposedRect.height() * scaleFactor);
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

        resvg_transform ts = resvg_transform_identity();
        ts.a = scaleFactor;
        ts.d = scaleFactor;
        ts.e = -exposedRect.x() * scaleFactor;
        ts.f = -exposedRect.y() * scaleFactor;

        img.fill(Qt::transparent);
        resvg_render(m_tree, ts,
                     static_cast<quint32>(img.width()),
                     static_cast<quint32>(img.height()),
                     reinterpret_cast<char*>(img.bits()));

#if (!USE_RGBA_8888)
        img = img.rgbSwapped();
#endif
        return img;
#undef USE_RGBA_8888
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
    static bool hasClipPath(const QByteArray &svgData)
    {
        for(XmlStreamReader reader(svgData); !reader.atEnd(); reader.readNext())
        {
            if(reader.tokenType() != QXmlStreamReader::StartElement)
                continue;
            if(!reader.name().toString().compare(QString::fromLatin1("clipPath"), Qt::CaseInsensitive))
                return true;
        }
        return false;
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
    QByteArray m_fileDirUtf8;
    resvg_render_tree *m_tree;
    resvg_options *m_opt;
    qreal m_width;
    qreal m_height;
    RasterizerCache m_rasterizerCache;
    qreal m_minScaleFactor;
    qreal m_maxScaleFactor;
};

// ====================================================================================================

class DecoderReSVG : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderReSVG");
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

        ReSVGGraphicsItem *resvgItem = new ReSVGGraphicsItem(filePath);
        QGraphicsItem *item = resvgItem;
        if(!resvgItem->isValid() || !resvgItem->exposedRectSupported())
        {
            IScaledImageProvider *provider = resvgItem;
            item = GraphicsItemsFactory::instance().createScalableItem(provider);
        }
        IImageMetaData *metaData = item ? ImageMetaData::createMetaData(filePath) : Q_NULLPTR;
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), metaData));
    }
};

DecoderAutoRegistrator registrator(new DecoderReSVG);

// ====================================================================================================

} // namespace
#endif
