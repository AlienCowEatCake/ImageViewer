/*
   Copyright (C) 2017-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <cstdio>
#include <cstdlib>

#if !defined (MNG_USE_SO)
#define MNG_USE_SO
#endif
#include <libmng.h>

#include <QFileInfo>
#include <QImage>
#include <QPixmap>
#include <QFile>
#include <QByteArray>
#include <QSysInfo>

#include "Utils/Global.h"
#include "Utils/Logging.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"
#include "Internal/Animation/DelayCalculator.h"
#include "Internal/Animation/IAnimationProvider.h"

namespace
{

// ====================================================================================================

class MngAnimationProvider : public IAnimationProvider
{
    Q_DISABLE_COPY(MngAnimationProvider)

public:
    explicit MngAnimationProvider(const QString &filePath);
    ~MngAnimationProvider();

    bool isValid() const Q_DECL_OVERRIDE;
    bool isSingleFrame() const Q_DECL_OVERRIDE;
    int nextImageDelay() const Q_DECL_OVERRIDE;
    bool jumpToNextImage() Q_DECL_OVERRIDE;
    QPixmap currentPixmap() const Q_DECL_OVERRIDE;
    QImage currentImage() const Q_DECL_OVERRIDE;

    mng_handle hMNG;
    QImage image;
    QFile file;
    bool error;
    int elapsed;
    int nextDelay;

    enum {
        TYPE_UNKNOWN         = 0,
        TYPE_SINGLE_FRAME    = 1,
        TYPE_MULTIPLE_FRAMES = 2
    } imageType;
};

// ====================================================================================================

mng_bool MNG_DECL proc_mng_error(mng_handle  hMNG,
                        mng_int32   iErrorcode,
                        mng_int8    iSeverity,
                        mng_chunkid iChunkname,
                        mng_uint32  /*iChunkseq*/,
                        mng_int32   iExtra1,
                        mng_int32   iExtra2,
                        mng_pchar   zErrortext)
{
    MngAnimationProvider *provider = reinterpret_cast<MngAnimationProvider*>(mng_get_userdata(hMNG));
    if(iSeverity > 2)
        provider->error = true;
    LOG_WARNING("%s MNG error %d: %s; chunk %c%c%c%c; subcode %d:%d",
            LOGGING_CTX,
            iErrorcode, zErrortext,
            (iChunkname >> 24) & 0xff,
            (iChunkname >> 16) & 0xff,
            (iChunkname >> 8)  & 0xff,
            (iChunkname >> 0)  & 0xff,
            iExtra1, iExtra2);
    return MNG_TRUE;
}

mng_ptr MNG_DECL proc_mng_alloc(mng_size_t iSize)
{
    return reinterpret_cast<mng_ptr>(calloc(1, iSize));
}

void MNG_DECL proc_mng_free(mng_ptr pPtr, mng_size_t /*iSize*/)
{
    free(pPtr);
}

mng_bool MNG_DECL proc_mng_openstream(mng_handle hMNG)
{
    MngAnimationProvider *provider = reinterpret_cast<MngAnimationProvider*>(mng_get_userdata(hMNG));
    return provider->file.open(QIODevice::ReadOnly) ? MNG_TRUE : MNG_FALSE;
}

mng_bool MNG_DECL proc_mng_closestream(mng_handle hMNG)
{
    MngAnimationProvider *provider = reinterpret_cast<MngAnimationProvider*>(mng_get_userdata(hMNG));
    provider->file.close();
    return MNG_TRUE;
}

mng_bool MNG_DECL proc_mng_readstream(mng_handle  hMNG,
                             mng_ptr     pBuf,
                             mng_uint32  iSize,
                             mng_uint32p pRead)
{
    MngAnimationProvider *provider = reinterpret_cast<MngAnimationProvider*>(mng_get_userdata(hMNG));
    *pRead = static_cast<mng_uint32>(provider->file.read(reinterpret_cast<char*>(pBuf), iSize));
    return (*pRead > 0) ? MNG_TRUE : MNG_FALSE;
}

mng_bool MNG_DECL proc_mng_processheader(mng_handle hMNG,
                                mng_uint32 iWidth,
                                mng_uint32 iHeight)
{
    MngAnimationProvider *provider = reinterpret_cast<MngAnimationProvider*>(mng_get_userdata(hMNG));
    const mng_uint32 canvasStyle = (QSysInfo::ByteOrder == QSysInfo::LittleEndian) ? MNG_CANVAS_BGRA8 : MNG_CANVAS_ARGB8;
    if(mng_set_canvasstyle(hMNG, canvasStyle) != MNG_NOERROR)
        return MNG_FALSE;
    provider->image = QImage(static_cast<int>(iWidth), static_cast<int>(iHeight), QImage::Format_ARGB32);
    if(provider->image.isNull())
    {
        LOG_WARNING() << LOGGING_CTX << "Invalid image size";
        provider->error = true;
        return MNG_FALSE;
    }
    provider->image.fill(0);
    return MNG_TRUE;
}

mng_ptr MNG_DECL proc_mng_getcanvasline(mng_handle hMNG,
                               mng_uint32 iLinenr)
{
    MngAnimationProvider *provider = reinterpret_cast<MngAnimationProvider*>(mng_get_userdata(hMNG));
    return reinterpret_cast<mng_ptr>(provider->image.scanLine(static_cast<int>(iLinenr)));
}

mng_bool MNG_DECL proc_mng_refresh(mng_handle /*hMNG*/,
                          mng_uint32 /*iX*/,
                          mng_uint32 /*iY*/,
                          mng_uint32 /*iWidth*/,
                          mng_uint32 /*iHeight*/)
{
    return MNG_TRUE;
}

mng_uint32 MNG_DECL proc_mng_gettickcount(mng_handle hMNG)
{
    MngAnimationProvider *provider = reinterpret_cast<MngAnimationProvider*>(mng_get_userdata(hMNG));
    return static_cast<mng_uint32>(provider->elapsed);
}

mng_bool MNG_DECL proc_mng_settimer(mng_handle hMNG,
                           mng_uint32 iMsecs)
{
    MngAnimationProvider *provider = reinterpret_cast<MngAnimationProvider*>(mng_get_userdata(hMNG));
    provider->elapsed += iMsecs;
    provider->nextDelay = DelayCalculator::calculate(static_cast<int>(iMsecs), DelayCalculator::MODE_NORMAL);
    return MNG_TRUE;
}

mng_bool MNG_DECL proc_mng_trace(mng_handle /*hMNG*/,
                        mng_int32  iFuncnr,
                        mng_int32  iFuncseq,
                        mng_pchar  zFuncname)
{
    LOG_DEBUG("%s mng trace: iFuncnr: %d iFuncseq: %d zFuncname: %s", LOGGING_CTX, iFuncnr, iFuncseq, zFuncname);
    return MNG_TRUE;
}

// ====================================================================================================

MngAnimationProvider::MngAnimationProvider(const QString &filePath)
    : hMNG(mng_initialize(reinterpret_cast<mng_ptr>(this), proc_mng_alloc, proc_mng_free, proc_mng_trace))
    , file(filePath)
    , error(false)
    , elapsed(0)
    , nextDelay(0)
    , imageType(TYPE_UNKNOWN)
{
    if(hMNG)
    {
        mng_setcb_errorproc(hMNG, proc_mng_error);
        mng_setcb_openstream(hMNG, proc_mng_openstream);
        mng_setcb_closestream(hMNG, proc_mng_closestream);
        mng_setcb_readdata(hMNG, proc_mng_readstream);
        mng_setcb_processheader(hMNG, proc_mng_processheader);
        mng_setcb_getcanvasline(hMNG, proc_mng_getcanvasline);
        mng_setcb_refresh(hMNG, proc_mng_refresh);
        mng_setcb_gettickcount(hMNG, proc_mng_gettickcount);
        mng_setcb_settimer(hMNG, proc_mng_settimer);
        mng_set_doprogressive(hMNG, MNG_FALSE);
        mng_set_suspensionmode(hMNG, MNG_TRUE);
        mng_readdisplay(hMNG);
    }
}

MngAnimationProvider::~MngAnimationProvider()
{
    mng_cleanup(&hMNG);
}

bool MngAnimationProvider::isValid() const
{
    return hMNG && !error;
}

bool MngAnimationProvider::isSingleFrame() const
{
    return imageType == TYPE_SINGLE_FRAME || (imageType == TYPE_UNKNOWN && !file.isOpen() && isValid());
}

int MngAnimationProvider::nextImageDelay() const
{
    return nextDelay;
}

bool MngAnimationProvider::jumpToNextImage()
{
    if(imageType == TYPE_UNKNOWN && isValid())
        imageType = (file.isOpen() ? TYPE_MULTIPLE_FRAMES : TYPE_SINGLE_FRAME);
    mng_display_resume(hMNG);
    return true;
}

QPixmap MngAnimationProvider::currentPixmap() const
{
    return QPixmap::fromImage(currentImage());
}

QImage MngAnimationProvider::currentImage() const
{
    return image;
}

// ====================================================================================================

class DecoderLibMng : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderLibMng");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
                << QString::fromLatin1("mng")
#if !defined (MNG_NO_INCLUDE_JNG)
                << QString::fromLatin1("jng")
#endif
                ;
    }

    QStringList advancedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList();
    }

    bool isAvailable() const Q_DECL_OVERRIDE
    {
        return true;
    }

    QSharedPointer<IImageData> loadImage(const QString &filePath) Q_DECL_OVERRIDE
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return QSharedPointer<IImageData>();
        IAnimationProvider *provider = new MngAnimationProvider(filePath);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createAnimatedItem(provider);
        IImageMetaData *metaData = item ? ImageMetaData::createMetaData(filePath) : Q_NULLPTR;
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), metaData));
    }
};

DecoderAutoRegistrator registrator(new DecoderLibMng);

// ====================================================================================================

} // namespace
