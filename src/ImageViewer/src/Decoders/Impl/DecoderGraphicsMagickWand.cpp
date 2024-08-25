/*
   Copyright (C) 2020-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if defined (HAS_GRAPHICSMAGICKWAND)
//#define LINKED_GRAPHICSMAGICKWAND
#endif

#include "Utils/Global.h"

#include <cstdlib>

#if defined (LINKED_GRAPHICSMAGICKWAND)
#include <wand/wand_api.h>
#endif

#include <QFileInfo>
#include <QImage>
#include <QByteArray>
#include <QLibrary>
#include <QSysInfo>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QFunctionPointer>
#else
typedef void* QFunctionPointer;
#endif

#include "Utils/Logging.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"
#include "Internal/Animation/AbstractAnimationProvider.h"
#include "Internal/Animation/DelayCalculator.h"
#include "Internal/Animation/FramesCompositor.h"
#include "Internal/Utils/CmsUtils.h"
#include "Internal/Utils/LibraryUtils.h"
#include "Internal/Utils/MappedBuffer.h"

namespace
{

// ====================================================================================================

#if !defined (LINKED_GRAPHICSMAGICKWAND)

const QStringList GRAPHICSMAGICK_LIBRARY_NAMES = QStringList()
        << QString::fromLatin1("libGraphicsMagick")
           ;
const QStringList GRAPHICSMAGICKWAND_LIBRARY_NAMES = QStringList()
        << QString::fromLatin1("libGraphicsMagickWand")
           ;

enum StorageType
{
    CharPixel = 0,
};

enum DisposeType
{
    UndefinedDispose = 0,
    NoneDispose = 1,
    BackgroundDispose = 2,
    PreviousDispose = 3
};

enum CompositeOperator
{
    OverCompositeOp = 1
};

typedef int OrientationType;

struct ExceptionInfo
{
    quint8 _[256];
};

struct MagickWand;

struct MagickInfo
{
    void *_[2];
    char *name;
};

class GraphicsMagickLib
{
public:
    static GraphicsMagickLib *instance()
    {
        static GraphicsMagickLib _;
        if(!_.isValid())
        {
            LOG_WARNING() << LOGGING_CTX << "Failed to load GraphicsMagick";
            return Q_NULLPTR;
        }
        return &_;
    }

    void InitializeMagick_(const char *path)
    {
        typedef void (*InitializeMagick_t)(const char *);
        InitializeMagick_t InitializeMagick_f = (InitializeMagick_t)m_InitializeMagick;
        InitializeMagick_f(path);
    }

    void DestroyMagick_()
    {
        typedef void (*DestroyMagick_t)();
        DestroyMagick_t DestroyMagick_f = (DestroyMagick_t)m_DestroyMagick;
        DestroyMagick_f();
    }

    void GetExceptionInfo_(ExceptionInfo *exception)
    {
        typedef void(*GetExceptionInfo_t)(ExceptionInfo *);
        GetExceptionInfo_t GetExceptionInfo_f = (GetExceptionInfo_t)m_GetExceptionInfo;
        GetExceptionInfo_f(exception);
    }

    void DestroyExceptionInfo_(ExceptionInfo *exception)
    {
        typedef ExceptionInfo *(*DestroyExceptionInfo_t)(ExceptionInfo *);
        DestroyExceptionInfo_t DestroyExceptionInfo_f = (DestroyExceptionInfo_t)m_DestroyExceptionInfo;
        DestroyExceptionInfo_f(exception);
    }

    MagickInfo **GetMagickInfoArray_(ExceptionInfo *exception)
    {
        typedef MagickInfo **(*GetMagickInfoArray_t)(ExceptionInfo *);
        GetMagickInfoArray_t GetMagickInfoArray_f = (GetMagickInfoArray_t)m_GetMagickInfoArray;
        return GetMagickInfoArray_f(exception);
    }

    void MagickFree_(void *memory)
    {
        typedef void(*MagickFree_t)(void *);
        MagickFree_t MagickFree_f = (MagickFree_t)m_MagickFree;
        MagickFree_f(memory);
    }

private:
    GraphicsMagickLib()
        : m_InitializeMagick(Q_NULLPTR)
        , m_DestroyMagick(Q_NULLPTR)
        , m_GetExceptionInfo(Q_NULLPTR)
        , m_DestroyExceptionInfo(Q_NULLPTR)
        , m_GetMagickInfoArray(Q_NULLPTR)
        , m_MagickFree(Q_NULLPTR)
    {
        if(!LibraryUtils::LoadQLibrary(m_library, GRAPHICSMAGICK_LIBRARY_NAMES))
            return;

        m_InitializeMagick = m_library.resolve("InitializeMagick");
        m_DestroyMagick = m_library.resolve("DestroyMagick");
        m_GetExceptionInfo = m_library.resolve("GetExceptionInfo");
        m_DestroyExceptionInfo = m_library.resolve("DestroyExceptionInfo");
        m_GetMagickInfoArray = m_library.resolve("GetMagickInfoArray");
        m_MagickFree = m_library.resolve("MagickFree");
    }

    ~GraphicsMagickLib()
    {}

    bool isValid() const
    {
        return m_library.isLoaded()
                && m_InitializeMagick
                && m_DestroyMagick
                && m_GetExceptionInfo
                && m_DestroyExceptionInfo
                && m_GetMagickInfoArray
                && m_MagickFree
                ;
    }

    QLibrary m_library;
    QFunctionPointer m_InitializeMagick;
    QFunctionPointer m_DestroyMagick;
    QFunctionPointer m_GetExceptionInfo;
    QFunctionPointer m_DestroyExceptionInfo;
    QFunctionPointer m_GetMagickInfoArray;
    QFunctionPointer m_MagickFree;
};

class GraphicsMagickWandLib
{
public:
    static GraphicsMagickWandLib *instance()
    {
        static GraphicsMagickWandLib _;
        if(!_.isValid())
        {
            LOG_WARNING() << LOGGING_CTX << "Failed to load MagickWand";
            return Q_NULLPTR;
        }
        return &_;
    }

    MagickWand *NewMagickWand_()
    {
        typedef MagickWand *(*NewMagickWand_t)();
        NewMagickWand_t NewMagickWand_f = (NewMagickWand_t)m_NewMagickWand;
        return NewMagickWand_f();
    }

    unsigned int DestroyMagickWand_(MagickWand *wand)
    {
        typedef unsigned int (*DestroyMagickWand_t)(MagickWand *);
        DestroyMagickWand_t DestroyMagickWand_f = (DestroyMagickWand_t)m_DestroyMagickWand;
        return DestroyMagickWand_f(wand);
    }

    unsigned int MagickReadImageBlob_(MagickWand *wand, const unsigned char *blob, const size_t length)
    {
        typedef unsigned int (*MagickReadImageBlob_t)(MagickWand *, const unsigned char *, const size_t);
        MagickReadImageBlob_t MagickReadImageBlob_f = (MagickReadImageBlob_t)m_MagickReadImageBlob;
        return MagickReadImageBlob_f(wand, blob, length);
    }

    unsigned long MagickGetNumberImages_(MagickWand *wand)
    {
        typedef unsigned long (*MagickGetNumberImages_t)(MagickWand *);
        MagickGetNumberImages_t MagickGetNumberImages_f = (MagickGetNumberImages_t)m_MagickGetNumberImages;
        return MagickGetNumberImages_f(wand);
    }

    unsigned int MagickGetImagePage_(MagickWand *wand, unsigned long *width, unsigned long *height, long *x, long *y)
    {
        typedef unsigned int (*MagickGetImagePage_t)(MagickWand *, unsigned long *, unsigned long *, long *, long *);
        MagickGetImagePage_t MagickGetImagePage_f = (MagickGetImagePage_t)m_MagickGetImagePage;
        return MagickGetImagePage_f(wand, width, height, x, y);
    }

    unsigned int MagickNextImage_(MagickWand *wand)
    {
        typedef unsigned int (*MagickNextImage_t)(MagickWand *);
        MagickNextImage_t MagickNextImage_f = (MagickNextImage_t)m_MagickNextImage;
        return MagickNextImage_f(wand);
    }

    unsigned long MagickGetImageDelay_(MagickWand *wand)
    {
        typedef unsigned long (*MagickGetImageDelay_t)(MagickWand *);
        MagickGetImageDelay_t MagickGetImageDelay_f = (MagickGetImageDelay_t)m_MagickGetImageDelay;
        return MagickGetImageDelay_f(wand);
    }

    unsigned long MagickGetImageIterations_(MagickWand *wand)
    {
        typedef unsigned long (*MagickGetImageIterations_t)(MagickWand *);
        MagickGetImageIterations_t MagickGetImageIterations_f = (MagickGetImageIterations_t)m_MagickGetImageIterations;
        return MagickGetImageIterations_f(wand);
    }

    unsigned long MagickGetImageWidth_(MagickWand *wand)
    {
        typedef unsigned long (*MagickGetImageWidth_t)(MagickWand *);
        MagickGetImageWidth_t MagickGetImageWidth_f = (MagickGetImageWidth_t)m_MagickGetImageWidth;
        return MagickGetImageWidth_f(wand);
    }

    unsigned long MagickGetImageHeight_(MagickWand *wand)
    {
        typedef unsigned long (*MagickGetImageHeight_t)(MagickWand *);
        MagickGetImageHeight_t MagickGetImageHeight_f = (MagickGetImageHeight_t)m_MagickGetImageHeight;
        return MagickGetImageHeight_f(wand);
    }

    OrientationType MagickGetImageOrientation_(MagickWand *wand)
    {
        typedef OrientationType (*MagickGetImageOrientation_t)(MagickWand *);
        MagickGetImageOrientation_t MagickGetImageOrientation_f = (MagickGetImageOrientation_t)m_MagickGetImageOrientation;
        return MagickGetImageOrientation_f(wand);
    }

    unsigned int MagickGetImagePixels_(MagickWand *wand, const long x, const long y, const unsigned long columns, const unsigned long rows, const char *map, const StorageType storage, unsigned char *pixels)
    {
        typedef unsigned int (*MagickGetImagePixels_t)(MagickWand *, const long, const long, const unsigned long, const unsigned long, const char *, const StorageType, unsigned char *);
        MagickGetImagePixels_t MagickGetImagePixels_f = (MagickGetImagePixels_t)m_MagickGetImagePixels;
        return MagickGetImagePixels_f(wand, x, y, columns, rows, map, storage, pixels);
    }

    DisposeType MagickGetImageDispose_(MagickWand *wand)
    {
        typedef DisposeType (*MagickGetImageDispose_t)(MagickWand *);
        MagickGetImageDispose_t MagickGetImageDispose_f = (MagickGetImageDispose_t)m_MagickGetImageDispose;
        return MagickGetImageDispose_f(wand);
    }

    CompositeOperator MagickGetImageCompose_(MagickWand *wand)
    {
        typedef CompositeOperator (*MagickGetImageCompose_t)(MagickWand *);
        MagickGetImageCompose_t MagickGetImageCompose_f = (MagickGetImageCompose_t)m_MagickGetImageCompose;
        return MagickGetImageCompose_f(wand);
    }

    void MagickResetIterator_(MagickWand *wand)
    {
        typedef void (*MagickResetIterator_t)(MagickWand *);
        MagickResetIterator_t MagickResetIterator_f = (MagickResetIterator_t)m_MagickResetIterator;
        MagickResetIterator_f(wand);
    }

    unsigned char *MagickGetImageProfile_(MagickWand *wand, const char *name, unsigned long *length)
    {
        typedef unsigned char *(*MagickGetImageProfile_t)(MagickWand *, const char *, unsigned long *);
        MagickGetImageProfile_t MagickGetImageProfile_f = (MagickGetImageProfile_t)m_MagickGetImageProfile;
        return MagickGetImageProfile_f(wand, name, length);
    }

private:
    GraphicsMagickWandLib()
        : m_NewMagickWand(Q_NULLPTR)
        , m_DestroyMagickWand(Q_NULLPTR)
        , m_MagickReadImageBlob(Q_NULLPTR)
        , m_MagickGetNumberImages(Q_NULLPTR)
        , m_MagickGetImagePage(Q_NULLPTR)
        , m_MagickNextImage(Q_NULLPTR)
        , m_MagickGetImageDelay(Q_NULLPTR)
        , m_MagickGetImageIterations(Q_NULLPTR)
        , m_MagickGetImageWidth(Q_NULLPTR)
        , m_MagickGetImageHeight(Q_NULLPTR)
        , m_MagickGetImageOrientation(Q_NULLPTR)
        , m_MagickGetImagePixels(Q_NULLPTR)
        , m_MagickGetImageDispose(Q_NULLPTR)
        , m_MagickGetImageCompose(Q_NULLPTR)
        , m_MagickResetIterator(Q_NULLPTR)
        , m_MagickGetImageProfile(Q_NULLPTR)
    {
        if(!LibraryUtils::LoadQLibrary(m_library, GRAPHICSMAGICKWAND_LIBRARY_NAMES))
            return;

        m_NewMagickWand = m_library.resolve("NewMagickWand");
        m_DestroyMagickWand = m_library.resolve("DestroyMagickWand");
        m_MagickReadImageBlob = m_library.resolve("MagickReadImageBlob");
        m_MagickGetNumberImages = m_library.resolve("MagickGetNumberImages");
        m_MagickGetImagePage = m_library.resolve("MagickGetImagePage");
        m_MagickNextImage = m_library.resolve("MagickNextImage");
        m_MagickGetImageDelay = m_library.resolve("MagickGetImageDelay");
        m_MagickGetImageIterations = m_library.resolve("MagickGetImageIterations");
        m_MagickGetImageWidth = m_library.resolve("MagickGetImageWidth");
        m_MagickGetImageHeight = m_library.resolve("MagickGetImageHeight");
        m_MagickGetImageOrientation = m_library.resolve("MagickGetImageOrientation");
        m_MagickGetImagePixels = m_library.resolve("MagickGetImagePixels");
        m_MagickGetImageDispose = m_library.resolve("MagickGetImageDispose");
        m_MagickGetImageCompose = m_library.resolve("MagickGetImageCompose");
        m_MagickResetIterator = m_library.resolve("MagickResetIterator");
        m_MagickGetImageProfile = m_library.resolve("MagickGetImageProfile");
    }

    ~GraphicsMagickWandLib()
    {}

    bool isValid() const
    {
        return m_library.isLoaded()
                && m_NewMagickWand
                && m_DestroyMagickWand
                && m_MagickReadImageBlob
                && m_MagickGetNumberImages
                && m_MagickGetImagePage
                && m_MagickNextImage
                && m_MagickGetImageDelay
                && m_MagickGetImageIterations
                && m_MagickGetImageWidth
                && m_MagickGetImageHeight
                && m_MagickGetImageOrientation
                && m_MagickGetImagePixels
                && m_MagickGetImageDispose
                && m_MagickGetImageCompose
                && m_MagickResetIterator
                && m_MagickGetImageProfile
                ;
    }

    QLibrary m_library;
    QFunctionPointer m_NewMagickWand;
    QFunctionPointer m_DestroyMagickWand;
    QFunctionPointer m_MagickReadImageBlob;
    QFunctionPointer m_MagickGetNumberImages;
    QFunctionPointer m_MagickGetImagePage;
    QFunctionPointer m_MagickNextImage;
    QFunctionPointer m_MagickGetImageDelay;
    QFunctionPointer m_MagickGetImageIterations;
    QFunctionPointer m_MagickGetImageWidth;
    QFunctionPointer m_MagickGetImageHeight;
    QFunctionPointer m_MagickGetImageOrientation;
    QFunctionPointer m_MagickGetImagePixels;
    QFunctionPointer m_MagickGetImageDispose;
    QFunctionPointer m_MagickGetImageCompose;
    QFunctionPointer m_MagickResetIterator;
    QFunctionPointer m_MagickGetImageProfile;
};

#define InitializeMagick                GraphicsMagickLib::instance()->InitializeMagick_
#define DestroyMagick                   GraphicsMagickLib::instance()->DestroyMagick_
#define GetExceptionInfo                GraphicsMagickLib::instance()->GetExceptionInfo_
#define DestroyExceptionInfo            GraphicsMagickLib::instance()->DestroyExceptionInfo_
#define GetMagickInfoArray              GraphicsMagickLib::instance()->GetMagickInfoArray_
#define MagickFree                      GraphicsMagickLib::instance()->MagickFree_

#define NewMagickWand                   GraphicsMagickWandLib::instance()->NewMagickWand_
#define DestroyMagickWand               GraphicsMagickWandLib::instance()->DestroyMagickWand_
#define MagickReadImageBlob             GraphicsMagickWandLib::instance()->MagickReadImageBlob_
#define MagickGetNumberImages           GraphicsMagickWandLib::instance()->MagickGetNumberImages_
#define MagickGetImagePage              GraphicsMagickWandLib::instance()->MagickGetImagePage_
#define MagickNextImage                 GraphicsMagickWandLib::instance()->MagickNextImage_
#define MagickGetImageDelay             GraphicsMagickWandLib::instance()->MagickGetImageDelay_
#define MagickGetImageIterations        GraphicsMagickWandLib::instance()->MagickGetImageIterations_
#define MagickGetImageWidth             GraphicsMagickWandLib::instance()->MagickGetImageWidth_
#define MagickGetImageHeight            GraphicsMagickWandLib::instance()->MagickGetImageHeight_
#define MagickGetImageOrientation       GraphicsMagickWandLib::instance()->MagickGetImageOrientation_
#define MagickGetImagePixels            GraphicsMagickWandLib::instance()->MagickGetImagePixels_
#define MagickGetImageDispose           GraphicsMagickWandLib::instance()->MagickGetImageDispose_
#define MagickGetImageCompose           GraphicsMagickWandLib::instance()->MagickGetImageCompose_
#define MagickResetIterator             GraphicsMagickWandLib::instance()->MagickResetIterator_
#define MagickGetImageProfile           GraphicsMagickWandLib::instance()->MagickGetImageProfile_

bool isReady()
{
    return !!GraphicsMagickLib::instance() && !!GraphicsMagickWandLib::instance();
}

#else

bool isReady()
{
    return true;
}

#endif

// ====================================================================================================

struct GraphicsMagickGuard
{
    explicit GraphicsMagickGuard(const char *path)
    {
        InitializeMagick(path);
    }

    ~GraphicsMagickGuard()
    {
        DestroyMagick();
    }
};

class ScopedExceptionInfo
{
public:
    ScopedExceptionInfo()
    {
        GetExceptionInfo(&m_exception);
    }

    ~ScopedExceptionInfo()
    {
        DestroyExceptionInfo(&m_exception);
    }

    operator ExceptionInfo * ()
    {
        return &m_exception;
    }

    ExceptionInfo * operator -> ()
    {
        return &m_exception;
    }

private:
    ExceptionInfo m_exception;
};

struct MagickWandDeleter
{
    static inline void cleanup(MagickWand *mw)
    {
        if(mw)
            DestroyMagickWand(mw);
    }
};

// ====================================================================================================

class GraphicsMagickWandAnimationProvider : public AbstractAnimationProvider
{
    Q_DISABLE_COPY(GraphicsMagickWandAnimationProvider)

public:
    explicit GraphicsMagickWandAnimationProvider(const QString &filePath)
    {
        m_numLoops = 1;
        m_error = !readImage(filePath);
    }

private:
    bool readImage(const QString &filePath)
    {
        const MappedBuffer inBuffer(filePath);
        if(!inBuffer.isValid())
            return false;

        const GraphicsMagickGuard graphicsMagickGuard(Q_NULLPTR);
        QScopedPointer<MagickWand, MagickWandDeleter> mw(NewMagickWand());
        if(!MagickReadImageBlob(mw.data(), inBuffer.dataAs<const unsigned char *>(), inBuffer.sizeAs<size_t>()))
        {
            LOG_WARNING() << LOGGING_CTX << "MagickReadImageBlob error";
            return false;
        }

        const unsigned long num = MagickGetNumberImages(mw.data());
        MagickResetIterator(mw.data());
        long x = 0, y = 0;
        unsigned long width = 0, height = 0;
        MagickGetImagePage(mw.data(), &width, &height, &x, &y);
        FramesCompositor compositor;
        compositor.startComposition(width, height);
        for(unsigned long i = 0; i < num; ++i)
        {
            MagickNextImage(mw.data());
            QImage qImage = convertImage(mw.data());
            if(qImage.isNull())
            {
                LOG_WARNING() << LOGGING_CTX << "MagickGetImagePixels error";
                return false;
            }
            MagickGetImagePage(mw.data(), &width, &height, &x, &y);
            const QRect rect(static_cast<int>(x), static_cast<int>(y), qImage.width(), qImage.height());
            qImage = compositor.compositeFrame(qImage, rect, disposeType(mw.data()), blendType(mw.data()));
            const int delay = static_cast<int>(MagickGetImageDelay(mw.data()));
            m_frames.push_back(Frame(qImage, DelayCalculator::calculate(delay, DelayCalculator::MODE_CHROME)));
        }
        m_numFrames = static_cast<int>(num);
        m_numLoops = static_cast<int>(MagickGetImageIterations(mw.data()));
        return m_numFrames > 0;
    }

    QImage convertImage(MagickWand *mw) const
    {
        const unsigned long width = MagickGetImageWidth(mw);
        const unsigned long height = MagickGetImageHeight(mw);
        QImage qImage(static_cast<int>(width), static_cast<int>(height), QImage::Format_ARGB32);
        if(qImage.isNull())
            return qImage;

        qImage.fill(Qt::transparent);
        if(!MagickGetImagePixels(mw, 0, 0, width, height, ((QSysInfo::ByteOrder == QSysInfo::LittleEndian) ? "BGRA" : "ARGB"), CharPixel, qImage.bits()))
            return QImage();

        if(OrientationType orientation = MagickGetImageOrientation(mw))
        {
            LOG_DEBUG() << LOGGING_CTX << "Orientation found:" << orientation;
            ImageMetaData::applyExifOrientation(&qImage, static_cast<quint16>(orientation));
        }

        unsigned long length = 0;
        if(const unsigned char *datum = MagickGetImageProfile(mw, "ICC", &length))
        {
            LOG_DEBUG() << LOGGING_CTX << "ICC profile found";
            ICCProfile profile(QByteArray::fromRawData(reinterpret_cast<const char*>(datum), static_cast<int>(length)));
            profile.applyToImage(&qImage);
        }

        return qImage;
    }

    FramesCompositor::DisposeType disposeType(MagickWand *mw) const
    {
        switch(MagickGetImageDispose(mw))
        {
        case NoneDispose:
            return FramesCompositor::DISPOSE_NONE;
        case BackgroundDispose:
            return FramesCompositor::DISPOSE_BACKGROUND;
        case PreviousDispose:
            return FramesCompositor::DISPOSE_PREVIOUS;
        default:
            break;
        }
        return FramesCompositor::DISPOSE_NONE;
    }

    FramesCompositor::BlendType blendType(MagickWand *mw) const
    {
        if(MagickGetImageCompose(mw) == OverCompositeOp)
            return FramesCompositor::BLEND_OVER;
        return FramesCompositor::BLEND_NONE;
    }
};

// ====================================================================================================

class DecoderGraphicsMagickWand : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderGraphicsMagickWand");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        if(!isAvailable())
            return QStringList();

        const GraphicsMagickGuard graphicsMagickGuard(Q_NULLPTR);
        ScopedExceptionInfo exception;

        QStringList formatNames;
        MagickInfo **info = GetMagickInfoArray(exception);
        if(info)
        {
            for(size_t i = 0; info[i]; ++i)
                formatNames.append(QString::fromLatin1(info[i]->name).toLower());
            MagickFree(info);
        }
        return formatNames;
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
        IAnimationProvider *provider = new GraphicsMagickWandAnimationProvider(filePath);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createAnimatedItem(provider);
        IImageMetaData *metaData = item ? ImageMetaData::createMetaData(filePath) : Q_NULLPTR;
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), metaData));
    }
};

DecoderAutoRegistrator registrator(new DecoderGraphicsMagickWand, true);

// ====================================================================================================

} // namespace
