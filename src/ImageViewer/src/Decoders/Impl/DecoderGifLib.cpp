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

#include <gif_lib.h>

namespace {
#if defined (DISPOSE_BACKGROUND)
const int GIFLIB_DISPOSE_BACKGROUND = DISPOSE_BACKGROUND;
#undef DISPOSE_BACKGROUND
#else
const int GIFLIB_DISPOSE_BACKGROUND = 2;
#endif
#if defined (DISPOSE_PREVIOUS)
const int GIFLIB_DISPOSE_PREVIOUS   = DISPOSE_PREVIOUS;
#undef DISPOSE_PREVIOUS
#else
const int GIFLIB_DISPOSE_PREVIOUS   = 3;
#endif
} // namespace

#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QVector>
#include <QDebug>

#include "Utils/ScopedPointer.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/Animation/AbstractAnimationProvider.h"
#include "Internal/Animation/DelayCalculator.h"
#include "Internal/Animation/FramesCompositor.h"

namespace {

// ====================================================================================================

int readProc(GifFileType *gifHandle, GifByteType *destBuffer, int length)
{
    if(!gifHandle || !destBuffer || length < 0)
        return -1;
    QIODevice *device = static_cast<QIODevice*>(gifHandle->UserData);
    return static_cast<int>(device->isReadable() ? device->read(reinterpret_cast<char*>(destBuffer), length) : -1);
}

GifFileType *dGifOpenWrapper(void *userData, InputFunc readFunc)
{
#if defined (GIFLIB_MAJOR) && (GIFLIB_MAJOR >= 5)
    int errorCode = E_GIF_SUCCEEDED;
    GifFileType* gifFile = DGifOpen(userData, readFunc, &errorCode);
    if(errorCode != E_GIF_SUCCEEDED)
        qWarning() << "GIFLIB Error:" << GifErrorString(errorCode);
    return gifFile;
#else
    return DGifOpen(userData, readFunc);
#endif
}

int dGifCloseFileWrapper(GifFileType *gifFile)
{
#if defined (GIFLIB_MAJOR) && (GIFLIB_MAJOR >= 5)
    int errorCode = E_GIF_SUCCEEDED;
    int result = DGifCloseFile(gifFile, &errorCode);
    if(errorCode != E_GIF_SUCCEEDED)
        qWarning() << "GIFLIB Error:" << GifErrorString(errorCode);
    return result;
#else
    return DGifCloseFile(gifFile);
#endif
}

QVector<QRgb> colorTableFromColorMapObject(ColorMapObject *colorMap, int transparentIndex = -1)
{
    QVector<QRgb> colorTable;
    if(colorMap)
    {
        for(int i = 0; i < colorMap->ColorCount; i++)
        {
            const GifColorType gifColor = colorMap->Colors[i];
            colorTable.append(qRgba(gifColor.Red, gifColor.Green, gifColor.Blue, i == transparentIndex ? 0 : 255));
        }
    }
    return colorTable;
}

struct FrameControlData
{
    int transparentIndex;
    FramesCompositor::DisposeType disposeType;
    int delayMs;

    FrameControlData()
        : transparentIndex(-1)
        , disposeType(FramesCompositor::DISPOSE_NONE)
        , delayMs(10)
    {}
};

FrameControlData getFrameControlData(GifFileType *gifFile, int frameIndex)
{
    FrameControlData controlData;
#if defined (GIFLIB_MAJOR) && (GIFLIB_MAJOR >= 5)
    GraphicsControlBlock gcb;
    DGifSavedExtensionToGCB(gifFile, frameIndex, &gcb);

    controlData.delayMs = gcb.DelayTime * 10;
    controlData.transparentIndex = gcb.TransparentColor;

    switch(gcb.DisposalMode)
    {
    case GIFLIB_DISPOSE_BACKGROUND:
        controlData.disposeType = FramesCompositor::DISPOSE_BACKGROUND;
        break;
    case GIFLIB_DISPOSE_PREVIOUS:
        controlData.disposeType = FramesCompositor::DISPOSE_PREVIOUS;
        break;
    default:
        controlData.disposeType = FramesCompositor::DISPOSE_NONE;
        break;
    }
#else
    const SavedImage *gifFrame = gifFile->SavedImages + frameIndex;
    for(int i = 0; i < gifFrame->ExtensionBlockCount; i++)
    {
        ExtensionBlock *eb = gifFrame->ExtensionBlocks + i;
        if(eb->Function == 0xF9 && eb->ByteCount == 4)
        {
            controlData.delayMs = eb->Bytes[1] * 10 + eb->Bytes[2];

            if((eb->Bytes[0] & 1) == 1)
                controlData.transparentIndex = eb->Bytes[3];

            switch((eb->Bytes[0] >> 2) & 0x07)
            {
            case GIFLIB_DISPOSE_BACKGROUND:
                controlData.disposeType = FramesCompositor::DISPOSE_BACKGROUND;
                break;
            case GIFLIB_DISPOSE_PREVIOUS:
                controlData.disposeType = FramesCompositor::DISPOSE_PREVIOUS;
                break;
            default:
                controlData.disposeType = FramesCompositor::DISPOSE_NONE;
                break;
            }
        }
    }
#endif
    return controlData;
}

// ====================================================================================================

class GifAnimationProvider : public AbstractAnimationProvider
{
    Q_DISABLE_COPY(GifAnimationProvider)

public:
    GifAnimationProvider(const QString &filePath)
    {
        m_error = !readGif(filePath);
    }

private:
    bool readGif(const QString &filePath)
    {
        QFile inFile(filePath);
        if(!inFile.open(QIODevice::ReadOnly))
        {
            qWarning() << "Can't open" << filePath;
            return false;
        }

        GifFileType *gifFile = dGifOpenWrapper(&inFile, &readProc);
        if(!gifFile)
        {
            qWarning() << "Can't open" << filePath;
            return false;
        }

        if(DGifSlurp(gifFile) == GIF_ERROR)
        {
            qWarning() << "Can't DGifSlurp" << filePath;
            dGifCloseFileWrapper(gifFile);
            return false;
        }

        const QSize screenSize(gifFile->SWidth, gifFile->SHeight);
        if(screenSize.isEmpty())
        {
            qWarning() << "Invalid image screen size";
            dGifCloseFileWrapper(gifFile);
            return false;
        }

        const QVector<QRgb> screenColorTable = colorTableFromColorMapObject(gifFile->SColorMap);

        FramesCompositor compositor;
        compositor.startComposition(screenSize);

        for(int frameIndex = 0; frameIndex < gifFile->ImageCount; frameIndex++)
        {
            const SavedImage *gifFrame = gifFile->SavedImages + frameIndex;
            const QRect frameRect = QRect(gifFrame->ImageDesc.Left, gifFrame->ImageDesc.Top, gifFrame->ImageDesc.Width, gifFrame->ImageDesc.Height);
            const FrameControlData frameControl = getFrameControlData(gifFile, frameIndex);

            QVector<QRgb> colorTable;
            if(gifFrame->ImageDesc.ColorMap)
                colorTable = colorTableFromColorMapObject(gifFrame->ImageDesc.ColorMap, frameControl.transparentIndex);
            else if(frameControl.transparentIndex >= 0 && frameControl.transparentIndex < screenColorTable.size())
                colorTable = colorTableFromColorMapObject(gifFile->SColorMap, frameControl.transparentIndex);
            else
                colorTable = screenColorTable;

            QImage frame(frameRect.width(), frameRect.height(), QImage::Format_Indexed8);
            if(frame.isNull())
            {
                qWarning() << "Invalid image size";
                dGifCloseFileWrapper(gifFile);
                return false;
            }

            frame.setColorTable(colorTable);
            if(frameControl.transparentIndex >= 0 && frameControl.transparentIndex < colorTable.size())
                frame.fill(static_cast<uint>(frameControl.transparentIndex));
            else if(!screenColorTable.isEmpty())
                frame.fill(static_cast<uint>(gifFile->SBackGroundColor));

#if !defined (GIFLIB_MAJOR) || (GIFLIB_MAJOR < 5)
            if(gifFrame->ImageDesc.Interlace)
            {
                static const int interlacedOffset[] = { 0, 4, 2, 1 };
                static const int interlacedJumps[]  = { 8, 8, 4, 2 };
                int line = 0;
                for(int i = 0; i < 4; i++)
                    for(int row = interlacedOffset[i]; row < frameRect.height(); row += interlacedJumps[i])
                        memcpy(frame.scanLine(row), gifFrame->RasterBits + (line++) * frameRect.width(), static_cast<size_t>(frameRect.width()));
            }
            else
#endif
            {
                for(int row = 0; row < frameRect.height(); row++)
                    memcpy(frame.scanLine(row), gifFrame->RasterBits + row * frameRect.width(), static_cast<size_t>(frameRect.width()));
            }

            frame = compositor.compositeFrame(frame.convertToFormat(QImage::Format_ARGB32), frameRect, frameControl.disposeType, FramesCompositor::BLEND_OVER);
            m_frames.push_back(Frame(frame, DelayCalculator::calculate(frameControl.delayMs, DelayCalculator::MODE_CHROME)));
        }

        dGifCloseFileWrapper(gifFile);
        m_numFrames = m_frames.size();
        return m_numFrames > 0;
    }
};

// ====================================================================================================

class DecoderGifLib : public IDecoder
{
public:
    QString name() const
    {
        return QString::fromLatin1("DecoderGifLib");
    }

    QStringList supportedFormats() const
    {
        return QStringList()
                << QString::fromLatin1("gif");
    }

    QStringList advancedFormats() const
    {
        return QStringList();
    }

    QGraphicsItem *loadImage(const QString &filePath)
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return NULL;
        return GraphicsItemsFactory::instance().createAnimatedItem(new GifAnimationProvider(filePath));
    }
};

DecoderAutoRegistrator registrator(new DecoderGifLib);

// ====================================================================================================

} // namespace
