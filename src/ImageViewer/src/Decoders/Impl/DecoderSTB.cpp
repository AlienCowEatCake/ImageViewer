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

#include <cassert>

#include <QImage>
#include <QFileInfo>

#include <stb_image.h>

#include "Utils/Global.h"
#include "Utils/Logging.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"
#include "Internal/PayloadWithMetaData.h"
#include "Internal/Animation/AbstractAnimationProvider.h"
#include "Internal/Animation/DelayCalculator.h"
#include "Internal/Utils/MappedBuffer.h"

namespace {

class STBAnimationProvider : public AbstractAnimationProvider
{
    Q_DISABLE_COPY(STBAnimationProvider)

public:
    STBAnimationProvider()
    {}

    PayloadWithMetaData<bool> readFile(const QString &filePath)
    {
        const MappedBuffer inBuffer(filePath);
        if(!inBuffer.isValid())
            return false;

        ::stbi_convert_iphone_png_to_rgb(1);
        ::stbi_set_unpremultiply_on_load(1);

        int *delays = Q_NULLPTR;
        int x = 0, y = 0, z = 0, n = 0;
        stbi_uc *data = Q_NULLPTR;
        if(filePath.endsWith(QString::fromLatin1(".gif"), Qt::CaseInsensitive))
        {
            data = ::stbi_load_gif_from_memory(inBuffer.dataAs<stbi_uc*>(), inBuffer.sizeAs<int>(), &delays, &x, &y, &z, &n, 0);
        }
        else
        {
            z = 1;
            data = ::stbi_load_from_memory(inBuffer.dataAs<stbi_uc*>(), inBuffer.sizeAs<int>(), &x, &y, &n, 0);
        }
        if(!data)
        {
            LOG_WARNING() << LOGGING_CTX << ::stbi_failure_reason();
            return false;
        }

        ImageMetaData *metaData = ImageMetaData::createMetaData(filePath);
        for(int index = 0; index < z; ++index)
        {
            QImage image(x, y, QImage::Format_ARGB32);
            for(int i = 0; i < image.height(); i++)
            {
                QRgb *line = reinterpret_cast<QRgb*>(image.scanLine(i));
                for(int j = 0; j < image.width(); j++)
                {
                    stbi_uc *source = data + ((i + y * index) * x + j) * n;
                    switch(n)
                    {
                    case 1: // grey
                        line[j] = qRgb(source[0], source[0], source[0]);
                        break;
                    case 2: // grey, alpha
                        line[j] = qRgba(source[0], source[0], source[0], source[1]);
                        break;
                    case 3: // red, green, blue
                        line[j] = qRgb(source[0], source[1], source[2]);
                        break;
                    case 4: // red, green, blue, alpha
                        line[j] = qRgba(source[0], source[1], source[2], source[3]);
                        break;
                    default:
                        assert(false);
                        image = QImage();
                        break;
                    }
                }
            }
            if(image.isNull())
            {
                LOG_WARNING() << LOGGING_CTX << "Invalid image size";
                m_frames.clear();
                break;
            }
            if(metaData)
                metaData->applyExifOrientation(&image);
            m_frames.push_back(Frame(image, DelayCalculator::calculate(delays ? delays[index] : 0, DelayCalculator::MODE_CHROME)));
        }

        if(delays)
            ::stbi_image_free(delays);
        ::stbi_image_free(data);

        m_numFrames = m_frames.size();
        m_error = m_numFrames <= 0;
        return PayloadWithMetaData<bool>(isValid(), metaData);
    }
};

class DecoderSTB : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderSTB");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
                << QString::fromLatin1("jpg")
                << QString::fromLatin1("jpeg")
                << QString::fromLatin1("jpe")   /// @todo Check this!
                << QString::fromLatin1("png")
                << QString::fromLatin1("bmp")
                << QString::fromLatin1("dib")   /// @todo Check this!
                << QString::fromLatin1("psd")
                << QString::fromLatin1("tga")
                << QString::fromLatin1("targa") /// @todo Check this!
                << QString::fromLatin1("gif")
                << QString::fromLatin1("hdr")
                << QString::fromLatin1("pic")
                << QString::fromLatin1("pbm")   /// @todo Check this!
                << QString::fromLatin1("ppm")
                << QString::fromLatin1("pgm")
                << QString::fromLatin1("pnm");  /// @todo Check this!
    }

    QStringList advancedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
                << QString::fromLatin1("heic"); // iPhone 7 Plus, iPhone 8 Plus, iPhone XS Max, etc.
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
        STBAnimationProvider *provider = new STBAnimationProvider();
        const PayloadWithMetaData<bool> readResult = provider->readFile(filePath);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createAnimatedItem(provider);
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), readResult.metaData()));
    }
};

DecoderAutoRegistrator registrator(new DecoderSTB, true);

} // namespace
