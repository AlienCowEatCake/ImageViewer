/*
   Copyright (C) 2018-2020 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <cassert>
#include <cmath>
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
#include "Internal/Scaling/IScaledImageProvider.h"
#include "Internal/Utils/LibraryUtils.h"
#include "Internal/Utils/MappedBuffer.h"

namespace
{

// ====================================================================================================

#if !defined (LINKED_RESVG)

const QStringList RESVG_LIBRARY_NAMES = QStringList()
        << QString::fromLatin1("resvg")
        << QString::fromLatin1("libresvg")
        << QString::fromLatin1("resvg_qt")
        << QString::fromLatin1("libresvg_qt")
           ;

typedef struct resvg_handle resvg_handle;
typedef struct resvg_render_tree resvg_render_tree;
typedef struct resvg_image resvg_image;
typedef int resvg_error;
typedef int resvg_shape_rendering;
typedef int resvg_text_rendering;
typedef int resvg_image_rendering;

typedef enum resvg_fit_to_type
{
    RESVG_FIT_TO_ORIGINAL,
    RESVG_FIT_TO_WIDTH,
    RESVG_FIT_TO_HEIGHT,
    RESVG_FIT_TO_ZOOM,
} resvg_fit_to_type;

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

struct ReSVG
{
    QLibrary library;
    QFunctionPointer resvg_init_log;
    QFunctionPointer resvg_init_options;
    QFunctionPointer resvg_options_create;
    QFunctionPointer resvg_options_destroy;
    QFunctionPointer resvg_options_set_file_path;
    QFunctionPointer resvg_options_set_font_family;
    QFunctionPointer resvg_options_set_serif_family;
    QFunctionPointer resvg_options_set_sans_serif_family;
    QFunctionPointer resvg_options_set_cursive_family;
    QFunctionPointer resvg_options_set_fantasy_family;
    QFunctionPointer resvg_options_set_monospace_family;
    QFunctionPointer resvg_options_set_languages;
    QFunctionPointer resvg_parse_tree_from_data;
    QFunctionPointer resvg_is_image_empty;
    QFunctionPointer resvg_get_image_size;
    QFunctionPointer resvg_get_image_viewbox;
    QFunctionPointer resvg_tree_destroy;
    QFunctionPointer resvg_qt_render_to_canvas;
    QFunctionPointer resvg_render;
    QFunctionPointer resvg_image_get_width;
    QFunctionPointer resvg_image_get_height;
    QFunctionPointer resvg_image_get_data;
    QFunctionPointer resvg_image_destroy;

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
        , resvg_init_options(Q_NULLPTR)
        , resvg_options_create(Q_NULLPTR)
        , resvg_options_destroy(Q_NULLPTR)
        , resvg_options_set_file_path(Q_NULLPTR)
        , resvg_options_set_font_family(Q_NULLPTR)
        , resvg_options_set_serif_family(Q_NULLPTR)
        , resvg_options_set_sans_serif_family(Q_NULLPTR)
        , resvg_options_set_cursive_family(Q_NULLPTR)
        , resvg_options_set_fantasy_family(Q_NULLPTR)
        , resvg_options_set_monospace_family(Q_NULLPTR)
        , resvg_options_set_languages(Q_NULLPTR)
        , resvg_parse_tree_from_data(Q_NULLPTR)
        , resvg_is_image_empty(Q_NULLPTR)
        , resvg_get_image_size(Q_NULLPTR)
        , resvg_get_image_viewbox(Q_NULLPTR)
        , resvg_tree_destroy(Q_NULLPTR)
        , resvg_qt_render_to_canvas(Q_NULLPTR)
        , resvg_render(Q_NULLPTR)
        , resvg_image_get_width(Q_NULLPTR)
        , resvg_image_get_height(Q_NULLPTR)
        , resvg_image_get_data(Q_NULLPTR)
        , resvg_image_destroy(Q_NULLPTR)
        {
            if(!LibraryUtils::LoadQLibrary(library, RESVG_LIBRARY_NAMES))
                return;

            resvg_init_log = library.resolve("resvg_init_log");
            resvg_init_options = library.resolve("resvg_init_options");
            resvg_options_create = library.resolve("resvg_options_create");
            resvg_options_destroy = library.resolve("resvg_options_destroy");
            resvg_options_set_file_path = library.resolve("resvg_options_set_file_path");
            resvg_options_set_font_family = library.resolve("resvg_options_set_font_family");
            resvg_options_set_serif_family = library.resolve("resvg_options_set_serif_family");
            resvg_options_set_sans_serif_family = library.resolve("resvg_options_set_sans_serif_family");
            resvg_options_set_cursive_family = library.resolve("resvg_options_set_cursive_family");
            resvg_options_set_fantasy_family = library.resolve("resvg_options_set_fantasy_family");
            resvg_options_set_monospace_family = library.resolve("resvg_options_set_monospace_family");
            resvg_options_set_languages = library.resolve("resvg_options_set_languages");
            resvg_parse_tree_from_data = library.resolve("resvg_parse_tree_from_data");
            resvg_is_image_empty = library.resolve("resvg_is_image_empty");
            resvg_get_image_size = library.resolve("resvg_get_image_size");
            resvg_get_image_viewbox = library.resolve("resvg_get_image_viewbox");
            resvg_tree_destroy = library.resolve("resvg_tree_destroy");
            resvg_qt_render_to_canvas = library.resolve("resvg_qt_render_to_canvas");
            resvg_render = library.resolve("resvg_render");
            resvg_image_get_width = library.resolve("resvg_image_get_width");
            resvg_image_get_height = library.resolve("resvg_image_get_height");
            resvg_image_get_data = library.resolve("resvg_image_get_data");
            resvg_image_destroy = library.resolve("resvg_image_destroy");

            if(resvg_init_log)
            {
                typedef void (*func_t)();
                func_t func = (func_t)resvg_init_log;
                func();
            }
    }

    bool isValid() const
    {
        return library.isLoaded()
                /*&& resvg_init_options*/
                && resvg_parse_tree_from_data && resvg_tree_destroy
                /*&& resvg_is_image_empty*/
                && resvg_get_image_size && resvg_get_image_viewbox
                && (resvg_qt_render_to_canvas || (resvg_render && resvg_image_get_width && resvg_image_get_height && resvg_image_get_data));
    }
};

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

#if defined (RESVG_MAJOR_VERSION) && defined (RESVG_MINOR_VERSION) && defined (RESVG_PATCH_VERSION)
#define LINKED_RESVG_VERSION QT_VERSION_CHECK(RESVG_MAJOR_VERSION, RESVG_MINOR_VERSION, RESVG_PATCH_VERSION)
#else
#define LINKED_RESVG_VERSION QT_VERSION_CHECK(0, 0, 0)
#endif

class ReSVGWrapper
{
public:
#if defined (LINKED_RESVG) && (LINKED_RESVG_VERSION < QT_VERSION_CHECK(0, 11, 0))
    typedef void resvg_image;
#endif

    ReSVGWrapper()
        : m_opt(Q_NULLPTR)
        , m_optABI(ABI_UNKNOWN)
    {
        Q_UNUSED(&ReSVGWrapper::resvg_render_pre_0_11_0);
        Q_UNUSED(&ReSVGWrapper::resvg_image_get_width_pre_0_11_0);
        Q_UNUSED(&ReSVGWrapper::resvg_image_get_height_pre_0_11_0);
        Q_UNUSED(&ReSVGWrapper::resvg_image_get_data_pre_0_11_0);
        Q_UNUSED(&ReSVGWrapper::resvg_image_destroy_pre_0_11_0);
        Q_UNUSED(m_opt);
        Q_UNUSED(m_optABI);
    }

    ~ReSVGWrapper()
    {}

    resvg_options *resvg_options_create()
    {
#if defined (LINKED_RESVG)
    #if (LINKED_RESVG_VERSION < QT_VERSION_CHECK(0, 11, 0))
        m_opt = new resvg_options;
        memset(m_opt, 0, sizeof(resvg_options));
        ::resvg_init_options(m_opt);
        return m_opt;
    #else
        return ::resvg_options_create();
    #endif
#else
        ReSVG *resvg = ReSVG::instance();
        if(!resvg)
            return Q_NULLPTR;
        if(resvg->resvg_options_create)
        {
            qDebug() << "Detected v0.11.0 ABI for resvg_options";
            m_optABI = ABI_0_11_0;
            typedef resvg_options *(*func_t)(void);
            func_t func = (func_t)resvg->resvg_options_create;
            m_opt = func();
        }
        else
        {
            const quint8 flagBit = 0xff;
            const size_t optionsSize = sizeof(resvg_options) * 5;
            quint8* optData = new quint8[optionsSize];
            memset(optData, *reinterpret_cast<const char*>(&flagBit), optionsSize);
            m_opt = reinterpret_cast<resvg_options*>(optData);
            if(resvg->resvg_init_options)
            {
                typedef void (*func_t)(resvg_options*);
                func_t func = (func_t)resvg->resvg_init_options;
                func(m_opt);
            }
            else
            {
                memset(m_opt, 0, RESVG_OPTIONS_SIZE_V020);
                m_opt->opt_v020.dpi = 96;
            }
            size_t effectiveOptSize = optionsSize;
            while(effectiveOptSize > 0)
            {
                if(optData[effectiveOptSize - 1] != flagBit)
                    break;
                else
                    effectiveOptSize--;
            }
            switch(effectiveOptSize)
            {
            case RESVG_OPTIONS_SIZE_V020:
                qDebug() << "Detected v0.2.0 ABI for resvg_options";
                m_optABI = ABI_0_2_0;
                break;
            case RESVG_OPTIONS_SIZE_V040:
                qDebug() << "Detected v0.4.0 ABI for resvg_options";
                m_optABI = ABI_0_4_0;
                break;
            case RESVG_OPTIONS_SIZE_V070:
                qDebug() << "Detected v0.7.0 ABI for resvg_options";
                m_optABI = ABI_0_7_0;
                break;
            default:
                qWarning() << "Can't detect ABI for resvg_options";
                qWarning() << "Got:" << effectiveOptSize;
                qWarning() << "Expected:" << RESVG_OPTIONS_SIZE_V020 << RESVG_OPTIONS_SIZE_V040 << RESVG_OPTIONS_SIZE_V070;
            }
        }
        return m_opt;
#endif
    }

    void resvg_options_destroy(resvg_options *opt)
    {
#if defined (LINKED_RESVG)
    #if (LINKED_RESVG_VERSION < QT_VERSION_CHECK(0, 11, 0))
        assert(opt == m_opt);
        delete opt;
        m_opt = Q_NULLPTR;
    #else
        ::resvg_options_destroy(opt);
    #endif
#else
        assert(opt == m_opt);
        ReSVG *resvg = ReSVG::instance();
        if(!resvg)
            return;
        if(resvg->resvg_options_destroy)
        {
            typedef void (*func_t)(resvg_options*);
            func_t func = (func_t)resvg->resvg_options_destroy;
            func(opt);
        }
        else
        {
            quint8* optData = reinterpret_cast<quint8*>(opt);
            delete [] optData;
        }
#endif
    }

    void resvg_options_set_file_path(resvg_options *opt, const char *path)
    {
#if defined (LINKED_RESVG)
    #if (LINKED_RESVG_VERSION < QT_VERSION_CHECK(0, 11, 0))
        opt->path = path;
    #else
        ::resvg_options_set_file_path(opt, path);
    #endif
#else
        assert(opt == m_opt);
        ReSVG *resvg = ReSVG::instance();
        if(!resvg)
            return;
        if(resvg->resvg_options_set_file_path)
        {
            typedef void (*func_t)(resvg_options*, const char*);
            func_t func = (func_t)resvg->resvg_options_set_file_path;
            func(opt, path);
        }
        else
        {
            switch(m_optABI)
            {
            case ABI_0_2_0:
                opt->opt_v020.path = path;
                break;
            case ABI_0_4_0:
                opt->opt_v040.path = path;
                break;
            case ABI_0_7_0:
                opt->opt_v070.path = path;
                break;
            default:
                break;
            }
        }
#endif
    }

    void resvg_options_set_font_family(resvg_options *opt, const char *family)
    {
#if defined (LINKED_RESVG)
    #if (LINKED_RESVG_VERSION < QT_VERSION_CHECK(0, 11, 0))
        opt->font_family = family;
    #else
        ::resvg_options_set_font_family(opt, family);
    #endif
#else
        assert(opt == m_opt);
        ReSVG *resvg = ReSVG::instance();
        if(!resvg)
            return;
        if(resvg->resvg_options_set_font_family)
        {
            typedef void (*func_t)(resvg_options*, const char*);
            func_t func = (func_t)resvg->resvg_options_set_font_family;
            func(opt, family);
        }
        else
        {
            switch(m_optABI)
            {
            case ABI_0_4_0:
                opt->opt_v040.font_family = family;
                break;
            case ABI_0_7_0:
                opt->opt_v070.font_family = family;
                break;
            default:
                break;
            }
        }
#endif
    }

    void resvg_options_set_serif_family(resvg_options *opt, const char *family)
    {
#if defined (LINKED_RESVG)
    #if (LINKED_RESVG_VERSION < QT_VERSION_CHECK(0, 11, 0))
        Q_UNUSED(opt);
        Q_UNUSED(family);
    #else
        ::resvg_options_set_serif_family(opt, family);
    #endif
#else
        assert(opt == m_opt);
        ReSVG *resvg = ReSVG::instance();
        if(!resvg)
            return;
        if(!resvg->resvg_options_set_serif_family)
            return;
        typedef void (*func_t)(resvg_options*, const char*);
        func_t func = (func_t)resvg->resvg_options_set_serif_family;
        func(opt, family);
#endif
    }

    void resvg_options_set_sans_serif_family(resvg_options *opt, const char *family)
    {
#if defined (LINKED_RESVG)
    #if (LINKED_RESVG_VERSION < QT_VERSION_CHECK(0, 11, 0))
        Q_UNUSED(opt);
        Q_UNUSED(family);
    #else
        ::resvg_options_set_sans_serif_family(opt, family);
    #endif
#else
        assert(opt == m_opt);
        ReSVG *resvg = ReSVG::instance();
        if(!resvg)
            return;
        if(!resvg->resvg_options_set_sans_serif_family)
            return;
        typedef void (*func_t)(resvg_options*, const char*);
        func_t func = (func_t)resvg->resvg_options_set_sans_serif_family;
        func(opt, family);
#endif
    }

    void resvg_options_set_cursive_family(resvg_options *opt, const char *family)
    {
#if defined (LINKED_RESVG)
    #if (LINKED_RESVG_VERSION < QT_VERSION_CHECK(0, 11, 0))
        Q_UNUSED(opt);
        Q_UNUSED(family);
    #else
        ::resvg_options_set_cursive_family(opt, family);
    #endif
#else
        assert(opt == m_opt);
        ReSVG *resvg = ReSVG::instance();
        if(!resvg)
            return;
        if(!resvg->resvg_options_set_cursive_family)
            return;
        typedef void (*func_t)(resvg_options*, const char*);
        func_t func = (func_t)resvg->resvg_options_set_cursive_family;
        func(opt, family);
#endif
    }

    void resvg_options_set_fantasy_family(resvg_options *opt, const char *family)
    {
#if defined (LINKED_RESVG)
    #if (LINKED_RESVG_VERSION < QT_VERSION_CHECK(0, 11, 0))
        Q_UNUSED(opt);
        Q_UNUSED(family);
    #else
        ::resvg_options_set_fantasy_family(opt, family);
    #endif
#else
        assert(opt == m_opt);
        ReSVG *resvg = ReSVG::instance();
        if(!resvg)
            return;
        if(!resvg->resvg_options_set_fantasy_family)
            return;
        typedef void (*func_t)(resvg_options*, const char*);
        func_t func = (func_t)resvg->resvg_options_set_fantasy_family;
        func(opt, family);
#endif
    }

    void resvg_options_set_monospace_family(resvg_options *opt, const char *family)
    {
#if defined (LINKED_RESVG)
    #if (LINKED_RESVG_VERSION < QT_VERSION_CHECK(0, 11, 0))
        Q_UNUSED(opt);
        Q_UNUSED(family);
    #else
        ::resvg_options_set_monospace_family(opt, family);
    #endif
#else
        assert(opt == m_opt);
        ReSVG *resvg = ReSVG::instance();
        if(!resvg)
            return;
        if(!resvg->resvg_options_set_monospace_family)
            return;
        typedef void (*func_t)(resvg_options*, const char*);
        func_t func = (func_t)resvg->resvg_options_set_monospace_family;
        func(opt, family);
#endif
    }

    void resvg_options_set_languages(resvg_options *opt, const char *languages)
    {
#if defined (LINKED_RESVG)
    #if (LINKED_RESVG_VERSION < QT_VERSION_CHECK(0, 11, 0))
        opt->languages = languages;
    #else
        ::resvg_options_set_languages(opt, languages);
    #endif
#else
        assert(opt == m_opt);
        ReSVG *resvg = ReSVG::instance();
        if(!resvg)
            return;
        if(resvg->resvg_options_set_languages)
        {
            typedef void (*func_t)(resvg_options*, const char*);
            func_t func = (func_t)resvg->resvg_options_set_languages;
            func(opt, languages);
        }
        else
        {
            switch(m_optABI)
            {
            case ABI_0_4_0:
                opt->opt_v040.languages = languages;
                break;
            case ABI_0_7_0:
                opt->opt_v070.languages = languages;
                break;
            default:
                break;
            }
        }
#endif
    }

    int resvg_parse_tree_from_data(const char *data, const size_t len, const resvg_options *opt, resvg_render_tree **tree)
    {
#if defined (LINKED_RESVG)
        return ::resvg_parse_tree_from_data(data, len, opt, tree);
#else
        assert(opt == m_opt);
        ReSVG *resvg = ReSVG::instance();
        if(!resvg)
            return -1;
        if(!resvg->resvg_parse_tree_from_data)
            return -1;
        typedef int (*func_t)(const char*, const size_t, const resvg_options*, resvg_render_tree**);
        func_t func = (func_t)resvg->resvg_parse_tree_from_data;
        return func(data, len, opt, tree);
#endif
    }

    void resvg_tree_destroy(resvg_render_tree *tree)
    {
#if defined (LINKED_RESVG)
        ::resvg_tree_destroy(tree);
#else
        ReSVG *resvg = ReSVG::instance();
        if(!resvg)
            return;
        if(!resvg->resvg_tree_destroy)
            return;
        typedef void (*func_t)(resvg_render_tree*);
        func_t func = (func_t)resvg->resvg_tree_destroy;
        return func(tree);
#endif
    }

    bool resvg_is_image_empty(const resvg_render_tree *tree)
    {
#if defined (LINKED_RESVG)
        return ::resvg_is_image_empty(tree);
#else
        ReSVG *resvg = ReSVG::instance();
        if(!resvg)
            return true;
        if(!resvg->resvg_is_image_empty)
            return false;
        typedef bool (*func_t)(const resvg_render_tree*);
        func_t func = (func_t)resvg->resvg_is_image_empty;
        return func(tree);
#endif
    }

    resvg_rect resvg_get_image_viewbox(const resvg_render_tree *tree)
    {
#if defined (LINKED_RESVG)
        return ::resvg_get_image_viewbox(tree);
#else
        ReSVG *resvg = ReSVG::instance();
        if(!resvg)
            return resvg_rect();
        if(!resvg->resvg_get_image_viewbox)
            return resvg_rect();
        typedef resvg_rect (*func_t)(const resvg_render_tree*);
        func_t func = (func_t)resvg->resvg_get_image_viewbox;
        return func(tree);
#endif
    }

    resvg_size resvg_get_image_size(const resvg_render_tree *tree)
    {
#if defined (LINKED_RESVG)
        return ::resvg_get_image_size(tree);
#else
        ReSVG *resvg = ReSVG::instance();
        if(!resvg)
            return resvg_size();
        if(!resvg->resvg_get_image_size)
            return resvg_size();
        typedef resvg_size (*func_t)(const resvg_render_tree*);
        func_t func = (func_t)resvg->resvg_get_image_size;
        return func(tree);
#endif
    }

    resvg_image *resvg_render(const resvg_render_tree *tree, resvg_fit_to fit_to, resvg_color *background)
    {
#if defined (LINKED_RESVG)
    #if (LINKED_RESVG_VERSION < QT_VERSION_CHECK(0, 11, 0))
        return resvg_render_pre_0_11_0(tree, fit_to, background);
    #else
        return ::resvg_render(tree, fit_to, background);
    #endif
#else
        ReSVG *resvg = ReSVG::instance();
        if(!resvg)
            return Q_NULLPTR;
        if(!resvg->resvg_render)
            return resvg_render_pre_0_11_0(tree, fit_to, background);
        typedef resvg_image *(*func_t)(const resvg_render_tree*, resvg_fit_to, resvg_color*);
        func_t func = (func_t)resvg->resvg_render;
        return func(tree, fit_to, background);
#endif
    }

    quint32 resvg_image_get_width(resvg_image *image)
    {
#if defined (LINKED_RESVG)
    #if (LINKED_RESVG_VERSION < QT_VERSION_CHECK(0, 11, 0))
        return resvg_image_get_width_pre_0_11_0(image);
    #else
        return ::resvg_image_get_width(image);
    #endif
#else
        ReSVG *resvg = ReSVG::instance();
        if(!resvg)
            return 0;
        if(!resvg->resvg_image_get_width)
            return resvg_image_get_width_pre_0_11_0(image);
        typedef quint32 (*func_t)(resvg_image*);
        func_t func = (func_t)resvg->resvg_image_get_width;
        return func(image);
#endif
    }

    quint32 resvg_image_get_height(resvg_image *image)
    {
#if defined (LINKED_RESVG)
    #if (LINKED_RESVG_VERSION < QT_VERSION_CHECK(0, 11, 0))
        return resvg_image_get_height_pre_0_11_0(image);
    #else
        return ::resvg_image_get_height(image);
    #endif
#else
        ReSVG *resvg = ReSVG::instance();
        if(!resvg)
            return 0;
        if(!resvg->resvg_image_get_height)
            return resvg_image_get_height_pre_0_11_0(image);
        typedef quint32 (*func_t)(resvg_image*);
        func_t func = (func_t)resvg->resvg_image_get_height;
        return func(image);
#endif
    }

    const char *resvg_image_get_data(resvg_image *image, size_t *len)
    {
#if defined (LINKED_RESVG)
    #if (LINKED_RESVG_VERSION < QT_VERSION_CHECK(0, 11, 0))
        return resvg_image_get_data_pre_0_11_0(image, len);
    #else
        return ::resvg_image_get_data(image, len);
    #endif
#else
        ReSVG *resvg = ReSVG::instance();
        if(!resvg)
            return Q_NULLPTR;
        if(!resvg->resvg_image_get_data)
            return resvg_image_get_data_pre_0_11_0(image, len);
        typedef const char *(*func_t)(resvg_image*, size_t*);
        func_t func = (func_t)resvg->resvg_image_get_data;
        return func(image, len);
#endif
    }

    void resvg_image_destroy(resvg_image *image)
    {
#if defined (LINKED_RESVG)
    #if (LINKED_RESVG_VERSION < QT_VERSION_CHECK(0, 11, 0))
        resvg_image_destroy_pre_0_11_0(image);
    #else
        ::resvg_image_destroy(image);
    #endif
#else
        ReSVG *resvg = ReSVG::instance();
        if(!resvg)
            return;
        if(!resvg->resvg_image_destroy)
        {
            resvg_image_destroy_pre_0_11_0(image);
            return;
        }
        typedef void (*func_t)(resvg_image*);
        func_t func = (func_t)resvg->resvg_image_destroy;
        func(image);
#endif
    }

private:
    void resvg_qt_render_to_canvas(const resvg_render_tree *tree, const resvg_options *opt, resvg_size size, void *painter)
    {
#if defined (LINKED_RESVG)
    #if (LINKED_RESVG_VERSION < QT_VERSION_CHECK(0, 11, 0))
        ::resvg_qt_render_to_canvas(tree, opt, size, painter);
    #else
        Q_UNUSED(tree);
        Q_UNUSED(opt);
        Q_UNUSED(size);
        Q_UNUSED(painter);
    #endif
#else
        assert(opt == m_opt);
        ReSVG *resvg = ReSVG::instance();
        if(!resvg)
            return;
        if(!resvg->resvg_qt_render_to_canvas)
            return;
        typedef void (*func_t)(const resvg_render_tree*, const resvg_options*, resvg_size, void*);
        func_t func = (func_t)resvg->resvg_qt_render_to_canvas;
        return func(tree, opt, size, painter);
#endif
    }

    resvg_image *resvg_render_pre_0_11_0(const resvg_render_tree *tree, resvg_fit_to fit_to, resvg_color *background)
    {
        Q_UNUSED(background);
        assert(background == Q_NULLPTR);
        assert(fit_to.type == RESVG_FIT_TO_ZOOM);
        const qreal scaleFactor = static_cast<qreal>(fit_to.value);
        resvg_size size = resvg_get_image_size(tree);
        const int width  = static_cast<int>(std::ceil(size.width * scaleFactor));
        const int height = static_cast<int>(std::ceil(size.height * scaleFactor));
        QImage image(width, height, QImage::Format_ARGB32);
        if(image.isNull())
            return Q_NULLPTR;
        image.fill(Qt::transparent);
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing);
        const QRectF r = painter.viewport();
        const resvg_rect viewBox = resvg_get_image_viewbox(tree);
        const double sx = r.width() / viewBox.width;
        const double sy = r.height() / viewBox.height;
        painter.setTransform(QTransform(sx, 0, 0, sy, r.x(), r.y()), true);
        resvg_size imgSize;
        imgSize.width = static_cast<quint32>(viewBox.width);
        imgSize.height = static_cast<quint32>(viewBox.height);
        resvg_qt_render_to_canvas(tree, m_opt, imgSize, &painter);
        painter.end();
        return reinterpret_cast<resvg_image*>(new QImage(image.rgbSwapped()));
    }

    quint32 resvg_image_get_width_pre_0_11_0(resvg_image *image)
    {
        return image ? static_cast<quint32>(reinterpret_cast<const QImage*>(image)->width()) : 0;
    }

    quint32 resvg_image_get_height_pre_0_11_0(resvg_image *image)
    {
        return image ? static_cast<quint32>(reinterpret_cast<const QImage*>(image)->height()) : 0;
    }

    const char *resvg_image_get_data_pre_0_11_0(resvg_image *image, size_t *len)
    {
        if(len)
            *len = 0;
        if(!image)
            return Q_NULLPTR;
        return reinterpret_cast<const char*>(reinterpret_cast<const QImage*>(image)->bits());
    }

    void resvg_image_destroy_pre_0_11_0(resvg_image *image)
    {
       delete reinterpret_cast<QImage*>(image);
    }

private:
    resvg_options *m_opt;
    enum OptABI
    {
        ABI_UNKNOWN,
        ABI_0_2_0,
        ABI_0_4_0,
        ABI_0_7_0,
        ABI_0_11_0,
    };
    OptABI m_optABI;
};

// ====================================================================================================

const qreal MAX_IMAGE_DIMENSION = 16384;
const qreal MIN_IMAGE_DIMENSION = 1;

// ====================================================================================================

class ReSVGPixmapProvider : public IScaledImageProvider, public ReSVGWrapper
{
    Q_DISABLE_COPY(ReSVGPixmapProvider)

public:
    explicit ReSVGPixmapProvider(const QString &filePath)
        : m_isValid(false)
        , m_filePath8bit(filePath.toLocal8Bit())
        , m_tree(Q_NULLPTR)
        , m_opt(Q_NULLPTR)
        , m_width(0)
        , m_height(0)
        , m_minScaleFactor(1)
        , m_maxScaleFactor(1)
    {
        m_opt = resvg_options_create();
        resvg_options_set_file_path(m_opt, m_filePath8bit.constData());
        resvg_options_set_font_family(m_opt, "Times New Roman");
        resvg_options_set_languages(m_opt, "");

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
        resvg_fit_to fitTo;
        fitTo.type = RESVG_FIT_TO_ZOOM;
        fitTo.value = static_cast<float>(scaleFactor);
        resvg_image *resvgImg = resvg_render(m_tree, fitTo, Q_NULLPTR);
        if(!resvgImg)
            return QImage();
        size_t len;
        const char* imgData = resvg_image_get_data(resvgImg, &len);
        QImage image = QImage(reinterpret_cast<const uchar*>(imgData),
                              static_cast<int>(resvg_image_get_width(resvgImg)),
                              static_cast<int>(resvg_image_get_height(resvgImg)),
                              QImage::Format_ARGB32).rgbSwapped();
        resvg_image_destroy(resvgImg);
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
    resvg_options *m_opt;
    int m_width;
    int m_height;
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
        IScaledImageProvider *provider = new ReSVGPixmapProvider(filePath);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createScalableItem(provider);
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name()));
    }
};

DecoderAutoRegistrator registrator(new DecoderReSVG);

// ====================================================================================================

} // namespace
