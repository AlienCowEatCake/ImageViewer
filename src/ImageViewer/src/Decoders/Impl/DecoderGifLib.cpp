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

#include "Utils/Global.h"
#include "Utils/ScopedPointer.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"
#include "Internal/Animation/AbstractAnimationProvider.h"
#include "Internal/Animation/DelayCalculator.h"
#include "Internal/Animation/FramesCompositor.h"
#include "Internal/Utils/CmsUtils.h"

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
    explicit GifAnimationProvider(const QString &filePath)
        : m_metaData(Q_NULLPTR)
    {
        m_numLoops = 1;
        m_error = !readGif(filePath);
    }

    ~GifAnimationProvider()
    {
        if(m_metaData)
            delete m_metaData;
    }

    ImageMetaData *takeMetaData()
    {
        ImageMetaData *result = m_metaData;
        m_metaData = Q_NULLPTR;
        return result;
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
        QScopedPointer<ICCProfile> iccProfile;

        FramesCompositor compositor;
        compositor.startComposition(screenSize);

        for(int frameIndex = 0; frameIndex < gifFile->ImageCount; frameIndex++)
        {
            const SavedImage *gifFrame = gifFile->SavedImages + frameIndex;

            for(int i = 0; i < gifFrame->ExtensionBlockCount; i++)
            {
                ExtensionBlock *extensionHeader = gifFrame->ExtensionBlocks + i;

                if(extensionHeader->Function == COMMENT_EXT_FUNC_CODE)
                {
                    // https://www.fileformat.info/format/gif/egff.htm#GIF.SPEC.COMEXB
                    // The Comment Extension block is used to insert a human-readable
                    // string of text into a GIF file or data stream. Each comment may
                    // contain up to 255 7-bit ASCII characters, including all the ASCII
                    // control codes. Any number of Comment Extension blocks may occur
                    // in a GIF file, and they may appear anywhere after the Global Color
                    // Table. It is suggested, however, that comments should appear before
                    // or after all image data in the GIF file.
                    const QByteArray commentData = QByteArray(reinterpret_cast<const char*>(extensionHeader->Bytes), extensionHeader->ByteCount);
                    if(!m_metaData)
                        m_metaData = new ImageMetaData;
                    m_metaData->addCustomEntry(QString::fromLatin1("Comment"), QString::fromLatin1("Comment Extension block #%1").arg(i), QString::fromLatin1(commentData));
                    qDebug() << "Found comment";
                    continue;
                }

                if(extensionHeader->Function != APPLICATION_EXT_FUNC_CODE)
                    continue;
                if(!extensionHeader->Bytes || extensionHeader->ByteCount != 11)
                    continue;
                if(i + 1 == gifFrame->ExtensionBlockCount) // Нужно для CONTINUE_EXT_FUNC_CODE
                    continue;
                if(!memcmp(extensionHeader->Bytes, "NETSCAPE2.0", 11) || !memcmp(extensionHeader->Bytes, "ANIMEXTS1.0", 11))
                {
                    ExtensionBlock *extensionData = extensionHeader + 1;
                    if(extensionData->Function != CONTINUE_EXT_FUNC_CODE)
                        continue;
                    if(!extensionData->Bytes || extensionData->ByteCount != 3)
                        continue;
                    if(extensionData->Bytes[0] != 1) // wrong marker
                        continue;
                    m_numLoops = extensionData->Bytes[1] | (extensionData->Bytes[2] << 8);
                    i++;
                    continue;
                }
                if(!memcmp(extensionHeader->Bytes, "XMP DataXMP", 11))
                {
                    QByteArray xmpData;
                    ExtensionBlock *extensionData = extensionHeader + 1;
                    while(i + 1 != gifFrame->ExtensionBlockCount && extensionData->Function == CONTINUE_EXT_FUNC_CODE)
                    {
                        // Usual case (including ICC profile): In each sub-block, the
                        // first byte specifies its size in bytes (0 to 255) and the
                        // rest of the bytes contain the data.
                        // Special case for XMP data: In each sub-block, the first byte
                        // is also part of the XMP payload. XMP in GIF also has a 257
                        // byte padding data. See the XMP specification for details.
                        const unsigned char c = static_cast<unsigned char>(extensionData->ByteCount);
                        xmpData += QByteArray(reinterpret_cast<const char*>(&c), 1);

                        xmpData += QByteArray(reinterpret_cast<const char*>(extensionData->Bytes), extensionData->ByteCount);
                        extensionData++;
                        i++;
                    }

                    // XMP padding data is 0x01, 0xff, 0xfe ... 0x01, 0x00.
                    const int xmpPadingSize = 257;
                    if(xmpData.size() > xmpPadingSize)
                        xmpData.resize(xmpData.size() - xmpPadingSize);

                    m_metaData = ImageMetaData::joinMetaData(takeMetaData(), ImageMetaData::createXmpMetaData(xmpData));
                    qDebug() << "Found XMP metadata";
                    continue;
                }
                if(!memcmp(extensionHeader->Bytes, "ICCRGBG1012", 11))
                {
                    QByteArray iccData;
                    ExtensionBlock *extensionData = extensionHeader + 1;
                    while(i + 1 != gifFrame->ExtensionBlockCount && extensionData->Function == CONTINUE_EXT_FUNC_CODE)
                    {
                        iccData += QByteArray(reinterpret_cast<const char*>(extensionData->Bytes), extensionData->ByteCount);
                        extensionData++;
                        i++;
                    }
                    iccProfile.reset(new ICCProfile(iccData));
                    qDebug() << "Found ICCP metadata";
                    continue;
                }
            }

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

            frame = frame.convertToFormat(QImage::Format_ARGB32);
            if(iccProfile)
                iccProfile->applyToImage(&frame);
            frame = compositor.compositeFrame(frame, frameRect, frameControl.disposeType, FramesCompositor::BLEND_OVER);
            m_frames.push_back(Frame(frame, DelayCalculator::calculate(frameControl.delayMs, DelayCalculator::MODE_CHROME)));
        }

        dGifCloseFileWrapper(gifFile);
        m_numFrames = m_frames.size();
        return m_numFrames > 0;
    }

private:
    ImageMetaData *m_metaData;
};

// ====================================================================================================

class DecoderGifLib : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderGifLib");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
                << QString::fromLatin1("gif");
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
        GifAnimationProvider *provider = new GifAnimationProvider(filePath);
        IImageMetaData *metaData = ImageMetaData::joinMetaData(ImageMetaData::createMetaData(filePath), provider->takeMetaData());
        QGraphicsItem *item = GraphicsItemsFactory::instance().createAnimatedItem(provider);
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), metaData));
    }
};

DecoderAutoRegistrator registrator(new DecoderGifLib);

// ====================================================================================================

} // namespace
