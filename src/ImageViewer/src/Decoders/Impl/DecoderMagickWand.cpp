/*
   Copyright (C) 2020-2021 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if defined (HAS_MAGICKWAND)
#define LINKED_MAGICKWAND
#endif

#include "Utils/Global.h"

#include <cstdlib>

#if defined (LINKED_MAGICKWAND)
#if QT_HAS_INCLUDE(<MagickWand/MagickWand.h>)
#include <MagickWand/MagickWand.h>
#else
#include <wand/MagickWand.h>
#endif
/// @note https://github.com/umlaeute/Gem/blob/v0.94/plugins/imageMAGICK/imageMAGICK.cpp#L52
#if (defined (MagickLibInterface) && (MagickLibInterface > 3)) || (defined (MagickLibVersion) && (MagickLibVersion >= 0x662))
typedef size_t magick_size_t;
typedef ssize_t magick_ssize_t;
#else
typedef unsigned long magick_size_t;
typedef long magick_ssize_t;
#endif
#else
typedef size_t magick_size_t;
#if !defined (_MSC_VER)
typedef ssize_t magick_ssize_t;
#else
#if defined (_WIN64)
typedef qint64 magick_ssize_t;
#else
typedef qint32 magick_ssize_t;
#endif
#endif
#endif

#include <QFileInfo>
#include <QImage>
#include <QByteArray>
#include <QDebug>
#include <QLibrary>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QFunctionPointer>
#else
typedef void* QFunctionPointer;
#endif

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

#if !defined (LINKED_MAGICKWAND)

const QStringList MAGICKCORE_LIBRARY_NAMES = QStringList()
        << QString::fromLatin1("libMagickCore-6")
        << QString::fromLatin1("libMagickCore-7")
        << QString::fromLatin1("cygMagickCore-6")
        << QString::fromLatin1("cygMagickCore-7")
           ;
const QStringList MAGICKWAND_LIBRARY_NAMES = QStringList()
        << QString::fromLatin1("libMagickWand-6")
        << QString::fromLatin1("libMagickWand-7")
        << QString::fromLatin1("cygMagickWand-6")
        << QString::fromLatin1("cygMagickWand-7")
           ;

enum MagickBooleanType
{
    MagickFalse = 0,
    MagickTrue = 1
};

enum StorageType
{
    CharPixel = 1,
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
    OverCompositeOp = 40
};

typedef int OrientationType;

struct ExceptionInfo;
struct MagickWand;

struct MagickInfo
{
    char *name;
};

class MagickCoreLib
{
public:
    static MagickCoreLib *instance()
    {
        static MagickCoreLib _;
        if(!_.isValid())
        {
            qWarning() << "Failed to load MagickCore";
            return Q_NULLPTR;
        }
        return &_;
    }

    ExceptionInfo *AcquireExceptionInfo_()
    {
        typedef ExceptionInfo *(*AcquireExceptionInfo_t)();
        AcquireExceptionInfo_t AcquireExceptionInfo_f = (AcquireExceptionInfo_t)m_AcquireExceptionInfo;
        return AcquireExceptionInfo_f();
    }

    ExceptionInfo *DestroyExceptionInfo_(ExceptionInfo *exception)
    {
        typedef ExceptionInfo *(*DestroyExceptionInfo_t)(ExceptionInfo *);
        DestroyExceptionInfo_t DestroyExceptionInfo_f = (DestroyExceptionInfo_t)m_DestroyExceptionInfo;
        return DestroyExceptionInfo_f(exception);
    }

    const MagickInfo **GetMagickInfoList_(const char *pattern, size_t *number_formats, ExceptionInfo *exception)
    {
        typedef const MagickInfo **(*GetMagickInfoList_t)(const char *, size_t *, ExceptionInfo *);
        GetMagickInfoList_t GetMagickInfoList_f = (GetMagickInfoList_t)m_GetMagickInfoList;
        return GetMagickInfoList_f(pattern, number_formats, exception);
    }

    void *RelinquishMagickMemory_(void *memory)
    {
        typedef void *(*RelinquishMagickMemory_t)(void *);
        RelinquishMagickMemory_t RelinquishMagickMemory_f = (RelinquishMagickMemory_t)m_RelinquishMagickMemory;
        return RelinquishMagickMemory_f(memory);
    }

private:
    MagickCoreLib()
        : m_AcquireExceptionInfo(Q_NULLPTR)
        , m_DestroyExceptionInfo(Q_NULLPTR)
        , m_GetMagickInfoList(Q_NULLPTR)
        , m_RelinquishMagickMemory(Q_NULLPTR)
    {
        if(!LibraryUtils::LoadQLibrary(m_library, MAGICKCORE_LIBRARY_NAMES))
            return;

        m_AcquireExceptionInfo = m_library.resolve("AcquireExceptionInfo");
        m_DestroyExceptionInfo = m_library.resolve("DestroyExceptionInfo");
        m_GetMagickInfoList = m_library.resolve("GetMagickInfoList");
        m_RelinquishMagickMemory = m_library.resolve("RelinquishMagickMemory");
    }

    ~MagickCoreLib()
    {}

    bool isValid() const
    {
        return m_library.isLoaded()
                && m_AcquireExceptionInfo
                && m_DestroyExceptionInfo
                && m_GetMagickInfoList
                && m_RelinquishMagickMemory
                ;
    }

    QLibrary m_library;
    QFunctionPointer m_AcquireExceptionInfo;
    QFunctionPointer m_DestroyExceptionInfo;
    QFunctionPointer m_GetMagickInfoList;
    QFunctionPointer m_RelinquishMagickMemory;
};

class MagickWandLib
{
public:
    static MagickWandLib *instance()
    {
        static MagickWandLib _;
        if(!_.isValid())
        {
            qWarning() << "Failed to load MagickWand";
            return Q_NULLPTR;
        }
        return &_;
    }

    void MagickWandGenesis_()
    {
        typedef void (*MagickWandGenesis_t)();
        MagickWandGenesis_t MagickWandGenesis_f = (MagickWandGenesis_t)m_MagickWandGenesis;
        MagickWandGenesis_f();
    }

    void MagickWandTerminus_()
    {
        typedef void (*MagickWandTerminus_t)();
        MagickWandTerminus_t MagickWandTerminus_f = (MagickWandTerminus_t)m_MagickWandTerminus;
        MagickWandTerminus_f();
    }

    MagickWand *NewMagickWand_()
    {
        typedef MagickWand *(*NewMagickWand_t)();
        NewMagickWand_t NewMagickWand_f = (NewMagickWand_t)m_NewMagickWand;
        return NewMagickWand_f();
    }

    MagickWand *DestroyMagickWand_(MagickWand *wand)
    {
        typedef MagickWand *(*DestroyMagickWand_t)(MagickWand *);
        DestroyMagickWand_t DestroyMagickWand_f = (DestroyMagickWand_t)m_DestroyMagickWand;
        return DestroyMagickWand_f(wand);
    }

    MagickBooleanType MagickReadImageBlob_(MagickWand *wand, const void *blob, const size_t length)
    {
        typedef MagickBooleanType (*MagickReadImageBlob_t)(MagickWand *, const void *, const size_t);
        MagickReadImageBlob_t MagickReadImageBlob_f = (MagickReadImageBlob_t)m_MagickReadImageBlob;
        return MagickReadImageBlob_f(wand, blob, length);
    }

    size_t MagickGetNumberImages_(MagickWand *wand)
    {
        typedef size_t (*MagickGetNumberImages_t)(MagickWand *);
        MagickGetNumberImages_t MagickGetNumberImages_f = (MagickGetNumberImages_t)m_MagickGetNumberImages;
        return MagickGetNumberImages_f(wand);
    }

    MagickBooleanType MagickGetImagePage_(MagickWand *wand, size_t *width, size_t *height, magick_ssize_t *x, magick_ssize_t *y)
    {
        typedef MagickBooleanType (*MagickGetImagePage_t)(MagickWand *, size_t *, size_t *, magick_ssize_t *, magick_ssize_t *);
        MagickGetImagePage_t MagickGetImagePage_f = (MagickGetImagePage_t)m_MagickGetImagePage;
        return MagickGetImagePage_f(wand, width, height, x, y);
    }

    MagickBooleanType MagickNextImage_(MagickWand *wand)
    {
        typedef MagickBooleanType (*MagickNextImage_t)(MagickWand *);
        MagickNextImage_t MagickNextImage_f = (MagickNextImage_t)m_MagickNextImage;
        return MagickNextImage_f(wand);
    }

    size_t MagickGetImageDelay_(MagickWand *wand)
    {
        typedef size_t (*MagickGetImageDelay_t)(MagickWand *);
        MagickGetImageDelay_t MagickGetImageDelay_f = (MagickGetImageDelay_t)m_MagickGetImageDelay;
        return MagickGetImageDelay_f(wand);
    }

    size_t MagickGetImageTicksPerSecond_(MagickWand *wand)
    {
        typedef size_t (*MagickGetImageTicksPerSecond_t)(MagickWand *);
        MagickGetImageTicksPerSecond_t MagickGetImageTicksPerSecond_f = (MagickGetImageTicksPerSecond_t)m_MagickGetImageTicksPerSecond;
        return MagickGetImageTicksPerSecond_f(wand);
    }

    size_t MagickGetImageIterations_(MagickWand *wand)
    {
        typedef size_t (*MagickGetImageIterations_t)(MagickWand *);
        MagickGetImageIterations_t MagickGetImageIterations_f = (MagickGetImageIterations_t)m_MagickGetImageIterations;
        return MagickGetImageIterations_f(wand);
    }

    size_t MagickGetImageWidth_(MagickWand *wand)
    {
        typedef size_t (*MagickGetImageWidth_t)(MagickWand *);
        MagickGetImageWidth_t MagickGetImageWidth_f = (MagickGetImageWidth_t)m_MagickGetImageWidth;
        return MagickGetImageWidth_f(wand);
    }

    size_t MagickGetImageHeight_(MagickWand *wand)
    {
        typedef size_t (*MagickGetImageHeight_t)(MagickWand *);
        MagickGetImageHeight_t MagickGetImageHeight_f = (MagickGetImageHeight_t)m_MagickGetImageHeight;
        return MagickGetImageHeight_f(wand);
    }

    OrientationType MagickGetImageOrientation_(MagickWand *wand)
    {
        typedef OrientationType (*MagickGetImageOrientation_t)(MagickWand *);
        MagickGetImageOrientation_t MagickGetImageOrientation_f = (MagickGetImageOrientation_t)m_MagickGetImageOrientation;
        return MagickGetImageOrientation_f(wand);
    }

    MagickBooleanType MagickExportImagePixels_(MagickWand *wand, const magick_ssize_t x, const magick_ssize_t y, const size_t columns, const size_t rows, const char *map, const StorageType storage, void *pixels)
    {
        typedef MagickBooleanType (*MagickExportImagePixels_t)(MagickWand *, const magick_ssize_t, const magick_ssize_t, const size_t, const size_t, const char *, const StorageType, void *);
        MagickExportImagePixels_t MagickExportImagePixels_f = (MagickExportImagePixels_t)m_MagickExportImagePixels;
        return MagickExportImagePixels_f(wand, x, y, columns, rows, map, storage, pixels);
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

    unsigned char *MagickGetImageProfile_(MagickWand *wand, const char *name, size_t *length)
    {
        typedef unsigned char *(*MagickGetImageProfile_t)(MagickWand *, const char *, size_t *);
        MagickGetImageProfile_t MagickGetImageProfile_f = (MagickGetImageProfile_t)m_MagickGetImageProfile;
        return MagickGetImageProfile_f(wand, name, length);
    }

private:
    MagickWandLib()
        : m_MagickWandGenesis(Q_NULLPTR)
        , m_MagickWandTerminus(Q_NULLPTR)
        , m_NewMagickWand(Q_NULLPTR)
        , m_DestroyMagickWand(Q_NULLPTR)
        , m_MagickReadImageBlob(Q_NULLPTR)
        , m_MagickGetNumberImages(Q_NULLPTR)
        , m_MagickGetImagePage(Q_NULLPTR)
        , m_MagickNextImage(Q_NULLPTR)
        , m_MagickGetImageDelay(Q_NULLPTR)
        , m_MagickGetImageTicksPerSecond(Q_NULLPTR)
        , m_MagickGetImageIterations(Q_NULLPTR)
        , m_MagickGetImageWidth(Q_NULLPTR)
        , m_MagickGetImageHeight(Q_NULLPTR)
        , m_MagickGetImageOrientation(Q_NULLPTR)
        , m_MagickExportImagePixels(Q_NULLPTR)
        , m_MagickGetImageDispose(Q_NULLPTR)
        , m_MagickGetImageCompose(Q_NULLPTR)
        , m_MagickResetIterator(Q_NULLPTR)
        , m_MagickGetImageProfile(Q_NULLPTR)
    {
        if(!LibraryUtils::LoadQLibrary(m_library, MAGICKWAND_LIBRARY_NAMES))
            return;

        m_MagickWandGenesis = m_library.resolve("MagickWandGenesis");
        m_MagickWandTerminus = m_library.resolve("MagickWandTerminus");
        m_NewMagickWand = m_library.resolve("NewMagickWand");
        m_DestroyMagickWand = m_library.resolve("DestroyMagickWand");
        m_MagickReadImageBlob = m_library.resolve("MagickReadImageBlob");
        m_MagickGetNumberImages = m_library.resolve("MagickGetNumberImages");
        m_MagickGetImagePage = m_library.resolve("MagickGetImagePage");
        m_MagickNextImage = m_library.resolve("MagickNextImage");
        m_MagickGetImageDelay = m_library.resolve("MagickGetImageDelay");
        m_MagickGetImageTicksPerSecond = m_library.resolve("MagickGetImageTicksPerSecond");
        m_MagickGetImageIterations = m_library.resolve("MagickGetImageIterations");
        m_MagickGetImageWidth = m_library.resolve("MagickGetImageWidth");
        m_MagickGetImageHeight = m_library.resolve("MagickGetImageHeight");
        m_MagickGetImageOrientation = m_library.resolve("MagickGetImageOrientation");
        m_MagickExportImagePixels = m_library.resolve("MagickExportImagePixels");
        m_MagickGetImageDispose = m_library.resolve("MagickGetImageDispose");
        m_MagickGetImageCompose = m_library.resolve("MagickGetImageCompose");
        m_MagickResetIterator = m_library.resolve("MagickResetIterator");
        m_MagickGetImageProfile = m_library.resolve("MagickGetImageProfile");
    }

    ~MagickWandLib()
    {}

    bool isValid() const
    {
        return m_library.isLoaded()
                && m_MagickWandGenesis
                && m_MagickWandTerminus
                && m_NewMagickWand
                && m_DestroyMagickWand
                && m_MagickReadImageBlob
                && m_MagickGetNumberImages
                && m_MagickGetImagePage
                && m_MagickNextImage
                && m_MagickGetImageDelay
                && m_MagickGetImageTicksPerSecond
                && m_MagickGetImageIterations
                && m_MagickGetImageWidth
                && m_MagickGetImageHeight
                && m_MagickGetImageOrientation
                && m_MagickExportImagePixels
                && m_MagickGetImageDispose
                && m_MagickGetImageCompose
                && m_MagickResetIterator
                && m_MagickGetImageProfile
                ;
    }

    QLibrary m_library;
    QFunctionPointer m_MagickWandGenesis;
    QFunctionPointer m_MagickWandTerminus;
    QFunctionPointer m_NewMagickWand;
    QFunctionPointer m_DestroyMagickWand;
    QFunctionPointer m_MagickReadImageBlob;
    QFunctionPointer m_MagickGetNumberImages;
    QFunctionPointer m_MagickGetImagePage;
    QFunctionPointer m_MagickNextImage;
    QFunctionPointer m_MagickGetImageDelay;
    QFunctionPointer m_MagickGetImageTicksPerSecond;
    QFunctionPointer m_MagickGetImageIterations;
    QFunctionPointer m_MagickGetImageWidth;
    QFunctionPointer m_MagickGetImageHeight;
    QFunctionPointer m_MagickGetImageOrientation;
    QFunctionPointer m_MagickExportImagePixels;
    QFunctionPointer m_MagickGetImageDispose;
    QFunctionPointer m_MagickGetImageCompose;
    QFunctionPointer m_MagickResetIterator;
    QFunctionPointer m_MagickGetImageProfile;
};

#define AcquireExceptionInfo            MagickCoreLib::instance()->AcquireExceptionInfo_
#define DestroyExceptionInfo            MagickCoreLib::instance()->DestroyExceptionInfo_
#define GetMagickInfoList               MagickCoreLib::instance()->GetMagickInfoList_
#define RelinquishMagickMemory          MagickCoreLib::instance()->RelinquishMagickMemory_

#define MagickWandGenesis               MagickWandLib::instance()->MagickWandGenesis_
#define MagickWandTerminus              MagickWandLib::instance()->MagickWandTerminus_
#define NewMagickWand                   MagickWandLib::instance()->NewMagickWand_
#define DestroyMagickWand               MagickWandLib::instance()->DestroyMagickWand_
#define MagickReadImageBlob             MagickWandLib::instance()->MagickReadImageBlob_
#define MagickGetNumberImages           MagickWandLib::instance()->MagickGetNumberImages_
#define MagickGetImagePage              MagickWandLib::instance()->MagickGetImagePage_
#define MagickNextImage                 MagickWandLib::instance()->MagickNextImage_
#define MagickGetImageDelay             MagickWandLib::instance()->MagickGetImageDelay_
#define MagickGetImageTicksPerSecond    MagickWandLib::instance()->MagickGetImageTicksPerSecond_
#define MagickGetImageIterations        MagickWandLib::instance()->MagickGetImageIterations_
#define MagickGetImageWidth             MagickWandLib::instance()->MagickGetImageWidth_
#define MagickGetImageHeight            MagickWandLib::instance()->MagickGetImageHeight_
#define MagickGetImageOrientation       MagickWandLib::instance()->MagickGetImageOrientation_
#define MagickExportImagePixels         MagickWandLib::instance()->MagickExportImagePixels_
#define MagickGetImageDispose           MagickWandLib::instance()->MagickGetImageDispose_
#define MagickGetImageCompose           MagickWandLib::instance()->MagickGetImageCompose_
#define MagickResetIterator             MagickWandLib::instance()->MagickResetIterator_
#define MagickGetImageProfile           MagickWandLib::instance()->MagickGetImageProfile_

bool isReady()
{
    return !!MagickCoreLib::instance() && !!MagickWandLib::instance();
}

#else

bool isReady()
{
    return true;
}

#endif

// ====================================================================================================

struct MagickWandGuard
{
    MagickWandGuard()
    {
        MagickWandGenesis();
    }

    ~MagickWandGuard()
    {
        MagickWandTerminus();
    }
};

struct ExceptionInfoDeleter
{
    static inline void cleanup(ExceptionInfo *exception)
    {
        if(exception)
            DestroyExceptionInfo(exception);
    }
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

class MagickWandAnimationProvider : public AbstractAnimationProvider
{
    Q_DISABLE_COPY(MagickWandAnimationProvider)

public:
    explicit MagickWandAnimationProvider(const QString &filePath)
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

        const MagickWandGuard magickWandGuard;
        QScopedPointer<MagickWand, MagickWandDeleter> mw(NewMagickWand());
        if(!MagickReadImageBlob(mw.data(), inBuffer.dataAs<const void*>(), inBuffer.sizeAs<size_t>()))
        {
            qWarning() << "[DecoderMagickWand] MagickReadImageBlob error";
            return false;
        }

        const size_t num = MagickGetNumberImages(mw.data());
        MagickResetIterator(mw.data());
        magick_ssize_t x = 0, y = 0;
        magick_size_t width = 0, height = 0;
        MagickGetImagePage(mw.data(), &width, &height, &x, &y);
        FramesCompositor compositor;
        compositor.startComposition(width, height);
        for(size_t i = 0; i < num; ++i)
        {
            MagickNextImage(mw.data());
            QImage qImage = convertImage(mw.data());
            if(qImage.isNull())
            {
                qWarning() << "[DecoderMagickWand] MagickGetImagePixels error";
                return false;
            }
            MagickGetImagePage(mw.data(), &width, &height, &x, &y);
            const QRect rect(static_cast<int>(x), static_cast<int>(y), qImage.width(), qImage.height());
            qImage = compositor.compositeFrame(qImage, rect, disposeType(mw.data()), blendType(mw.data()));
            const int delay = static_cast<int>(MagickGetImageDelay(mw.data()) * 1000 / MagickGetImageTicksPerSecond(mw.data()));
            m_frames.push_back(Frame(qImage, DelayCalculator::calculate(delay, DelayCalculator::MODE_CHROME)));
        }
        m_numFrames = static_cast<int>(num);
        m_numLoops = static_cast<int>(MagickGetImageIterations(mw.data()));
        return m_numFrames > 0;
    }

    QImage convertImage(MagickWand *mw) const
    {
        const size_t width = MagickGetImageWidth(mw);
        const size_t height = MagickGetImageHeight(mw);
        QImage qImage(static_cast<int>(width), static_cast<int>(height), QImage::Format_ARGB32);
        if(qImage.isNull())
            return qImage;

        qImage.fill(Qt::transparent);
        if(!MagickExportImagePixels(mw, 0, 0, width, height, "BGRA", CharPixel, qImage.bits()))
            return QImage();

        if(OrientationType orientation = MagickGetImageOrientation(mw))
        {
            qDebug() << "[DecoderMagickWand] Orientation found:" << orientation;
            ImageMetaData::applyExifOrientation(&qImage, static_cast<quint16>(orientation));
        }

        size_t length = 0;
        if(const unsigned char *datum = MagickGetImageProfile(mw, "ICC", &length))
        {
            qDebug() << "[DecoderMagickWand] ICC profile found";
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

class DecoderMagickWand : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderMagickWand");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        const MagickWandGuard magickWandGuard;
        QScopedPointer<ExceptionInfo, ExceptionInfoDeleter> exception(AcquireExceptionInfo());

        QStringList formatNames;
        magick_size_t num = 0;
        const MagickInfo **info = GetMagickInfoList("*", &num, exception.data());
        if(info)
        {
            for(size_t i = 0; i < num; i++)
                formatNames.append(QString::fromLatin1(info[i]->name).toLower());
            RelinquishMagickMemory(info);
        }
        return formatNames;
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
        IAnimationProvider *provider = new MagickWandAnimationProvider(filePath);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createAnimatedItem(provider);
        IImageMetaData *metaData = item ? ImageMetaData::createMetaData(filePath) : Q_NULLPTR;
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), metaData));
    }
};

DecoderAutoRegistrator registrator(new DecoderMagickWand, true);

// ====================================================================================================

} // namespace
