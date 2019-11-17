/*
   Copyright (C) 2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <cstring>
#include <exception>
#include <vector>

#include <QImage>
#include <QFileInfo>
#include <QFile>
#include <QDebug>

#include <OpenEXR/ImfArray.h>
#include <OpenEXR/ImfIO.h>
#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/Iex.h>

#if defined (OPENEXR_VERSION_MAJOR) && defined (OPENEXR_VERSION_MINOR) && defined (OPENEXR_VERSION_PATCH)
#define OPENEXR_VERSION_GREATER_OR_EQUAL(MAJOR, MINOR, PATCH) \
    QT_VERSION_CHECK((OPENEXR_VERSION_MAJOR), (OPENEXR_VERSION_MINOR), (OPENEXR_VERSION_PATCH)) >= \
    QT_VERSION_CHECK((MAJOR), (MINOR), (PATCH))
#else
#define OPENEXR_VERSION_GREATER_OR_EQUAL(MAJOR, MINOR, PATCH) 0
#endif

#include <OpenEXR/ImfAttribute.h>
#include <OpenEXR/ImfBoxAttribute.h>
#include <OpenEXR/ImfChannelListAttribute.h>
#include <OpenEXR/ImfChromaticitiesAttribute.h>
#include <OpenEXR/ImfCompressionAttribute.h>
#if OPENEXR_VERSION_GREATER_OR_EQUAL(2, 1, 0)
#include <OpenEXR/ImfDeepImageStateAttribute.h>
#endif
#include <OpenEXR/ImfDoubleAttribute.h>
#include <OpenEXR/ImfEnvmapAttribute.h>
#include <OpenEXR/ImfFloatAttribute.h>
//#include <OpenEXR/ImfFloatVertorAttribute.h>
#include <OpenEXR/ImfIntAttribute.h>
#include <OpenEXR/ImfKeyCodeAttribute.h>
#include <OpenEXR/ImfLineOrderAttribute.h>
#include <OpenEXR/ImfMatrixAttribute.h>
#include <OpenEXR/ImfOpaqueAttribute.h>
#include <OpenEXR/ImfPreviewImageAttribute.h>
#include <OpenEXR/ImfRationalAttribute.h>
#include <OpenEXR/ImfStringAttribute.h>
#if OPENEXR_VERSION_GREATER_OR_EQUAL(1, 7, 0)
#include <OpenEXR/ImfStringVectorAttribute.h>
#endif
#include <OpenEXR/ImfTileDescriptionAttribute.h>
#include <OpenEXR/ImfTimeCodeAttribute.h>
#include <OpenEXR/ImfVecAttribute.h>

#include "Utils/Global.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"
#include "Internal/PayloadWithMetaData.h"

#if OPENEXR_VERSION_GREATER_OR_EQUAL(2, 2, 0) && defined (_MSC_VER)
template <>
void OPENEXR_IMF_INTERNAL_NAMESPACE::TypedAttribute<std::vector<float> >::writeValueTo (OPENEXR_IMF_INTERNAL_NAMESPACE::OStream &os, int version) const;
template <>
void OPENEXR_IMF_INTERNAL_NAMESPACE::TypedAttribute<std::vector<float> >::readValueFrom (OPENEXR_IMF_INTERNAL_NAMESPACE::IStream &is, int size, int version);
#endif

namespace {

using namespace Iex;
using namespace Imath;
using namespace Imf;

// ====================================================================================================

class QFileIStream : public IStream
{
public:
    explicit QFileIStream(const QString &filePath)
        : IStream(filePath.toLocal8Bit())
        , m_file(filePath)
    {
        m_file.open(QIODevice::ReadOnly);
    }

    bool isValid() const
    {
        return m_file.isOpen();
    }

    /// @brief Read from the stream:
    /// read(c,n) reads n bytes from the stream, and stores
    /// them in array c.  If the stream contains less than n
    /// bytes, or if an I/O error occurs, read(c,n) throws
    /// an exception.  If read(c,n) reads the last byte from
    /// the file it returns false, otherwise it returns true.
    bool read(char c[/*n*/], int n) Q_DECL_OVERRIDE
    {
        const qint64 maxSize = static_cast<qint64>(n);
        const qint64 readSize = m_file.read(c, maxSize);
        if(readSize < maxSize)
            THROW(InputExc, "Early end of file: read " << maxSize << " out of " << readSize << " requested bytes.");
        return !m_file.atEnd();
    }

    /// @brief Get the current reading position, in bytes from the
    /// beginning of the file.  If the next call to read() will
    /// read the first byte in the file, tellg() returns 0.
    Int64 tellg() Q_DECL_OVERRIDE
    {
        return static_cast<Int64>(m_file.pos());
    }

    /// @brief Set the current reading position.
    /// After calling seekg(i), tellg() returns i.
    void seekg(Int64 pos) Q_DECL_OVERRIDE
    {
        m_file.seek(static_cast<qint64>(pos));
    }

private:
    QFile m_file;
};

// ====================================================================================================

IImageMetaData *readExrMetaData(const QString &filePath, const Header &header)
{
    ImageMetaData *metaData = ImageMetaData::createMetaData(filePath);
    if(!metaData)
        metaData = new ImageMetaData();

    const QString headerMetaDataType = QString::fromLatin1("OpenEXR Header");
    for(Header::ConstIterator it = header.begin(), itEnd = header.end(); it != itEnd; ++it)
    {
        const Attribute *attribute = &it.attribute();
        const QString name = QString::fromUtf8(it.name());
        QString value = QString::fromLatin1("Unknown attribute type (%1)").arg(QString::fromLatin1(attribute->typeName()));
        if(const ChannelListAttribute *channelListAttribute = dynamic_cast<const ChannelListAttribute*>(attribute))
        {
            const ChannelList &channelList = channelListAttribute->value();
            QStringList namesList;
            for(ChannelList::ConstIterator cIt = channelList.begin(), cItEnd = channelList.end(); cIt != cItEnd; ++cIt)
                namesList.append(QString::fromUtf8(cIt.name()));
            value = QString::fromLatin1("\"") + namesList.join(QString::fromLatin1("\", \"")) + QString::fromLatin1("\"");
        }
        else if(const Box2iAttribute *box2iAttribute = dynamic_cast<const Box2iAttribute*>(attribute))
        {
            const Box2i &box2i = box2iAttribute->value();
            value = QString::fromLatin1("Box2i: min(%1, %2), max(%3, %4)")
                    .arg(box2i.min.x).arg(box2i.min.y)
                    .arg(box2i.max.x).arg(box2i.max.y);
        }
        else if(const Box2fAttribute *box2fAttribute = dynamic_cast<const Box2fAttribute*>(attribute))
        {
            const Box2f &box2f = box2fAttribute->value();
            value = QString::fromLatin1("Box2f: min(%1, %2), max(%3, %4)")
                    .arg(static_cast<qreal>(box2f.min.x)).arg(static_cast<qreal>(box2f.min.y))
                    .arg(static_cast<qreal>(box2f.max.x)).arg(static_cast<qreal>(box2f.max.y));
        }
        else if(const ChromaticitiesAttribute *chromaticitiesAttribute = dynamic_cast<const ChromaticitiesAttribute*>(attribute))
        {
            const Chromaticities &chromaticities = chromaticitiesAttribute->value();
            value = QString::fromLatin1("Chromaticities: red(%1, %2), green(%3, %4), blue(%5, %6), white(%7, %8)")
                    .arg(static_cast<qreal>(chromaticities.red[0])).arg(static_cast<qreal>(chromaticities.red[1]))
                    .arg(static_cast<qreal>(chromaticities.green[0])).arg(static_cast<qreal>(chromaticities.green[1]))
                    .arg(static_cast<qreal>(chromaticities.blue[0])).arg(static_cast<qreal>(chromaticities.blue[1]))
                    .arg(static_cast<qreal>(chromaticities.white[0])).arg(static_cast<qreal>(chromaticities.white[1]));
        }
        else if(const CompressionAttribute *compressionAttribute = dynamic_cast<const CompressionAttribute*>(attribute))
        {
            const Compression &compression = compressionAttribute->value();
            switch(compression)
            {
#define ADD_CASE(VALUE) case(VALUE): value = QString::fromLatin1(#VALUE); break
            ADD_CASE(NO_COMPRESSION);
            ADD_CASE(RLE_COMPRESSION);
            ADD_CASE(ZIPS_COMPRESSION);
            ADD_CASE(ZIP_COMPRESSION);
            ADD_CASE(PIZ_COMPRESSION);
            ADD_CASE(PXR24_COMPRESSION);
            ADD_CASE(B44_COMPRESSION);
            ADD_CASE(B44A_COMPRESSION);
#if OPENEXR_VERSION_GREATER_OR_EQUAL(2, 2, 0)
            ADD_CASE(DWAA_COMPRESSION);
            ADD_CASE(DWAB_COMPRESSION);
#endif
#undef ADD_CASE
            default:
                value = QString::fromLatin1("Unknown");
                break;
            }
        }
#if OPENEXR_VERSION_GREATER_OR_EQUAL(2, 1, 0)
        else if(const DeepImageStateAttribute *deepImageStateAttribute = dynamic_cast<const DeepImageStateAttribute*>(attribute))
        {
            const DeepImageState &deepImageState = deepImageStateAttribute->value();
            switch(deepImageState)
            {
#define ADD_CASE(VALUE) case(VALUE): value = QString::fromLatin1(#VALUE); break
            ADD_CASE(DIS_MESSY);
            ADD_CASE(DIS_SORTED);
            ADD_CASE(DIS_NON_OVERLAPPING);
            ADD_CASE(DIS_TIDY);
#undef ADD_CASE
            default:
                value = QString::fromLatin1("Unknown");
                break;
            }
        }
#endif
        else if(const DoubleAttribute *doubleAttribute = dynamic_cast<const DoubleAttribute*>(attribute))
        {
            value = QString::number(static_cast<qreal>(doubleAttribute->value()));
        }
        else if(const EnvmapAttribute *envmapAttribute = dynamic_cast<const EnvmapAttribute*>(attribute))
        {
            const Envmap &envmap = envmapAttribute->value();
            switch(envmap)
            {
#define ADD_CASE(VALUE) case(VALUE): value = QString::fromLatin1(#VALUE); break
            ADD_CASE(ENVMAP_LATLONG);
            ADD_CASE(ENVMAP_CUBE);
#undef ADD_CASE
            default:
                value = QString::fromLatin1("Unknown");
                break;
            }
        }
        else if(const FloatAttribute *floatAttribute = dynamic_cast<const FloatAttribute*>(attribute))
        {
            value = QString::number(static_cast<qreal>(floatAttribute->value()));
        }
#if OPENEXR_VERSION_GREATER_OR_EQUAL(2, 2, 0)
        else if(const TypedAttribute<std::vector<float> > *floatVectorAttribute = dynamic_cast<const TypedAttribute<std::vector<float> > *>(attribute))
        {
            const std::vector<float> &floatVector = floatVectorAttribute->value();
            QStringList list;
            for(std::vector<float>::const_iterator vIt = floatVector.begin(), vItEnd = floatVector.end(); vIt != vItEnd; ++vIt)
                list.append(QString::number(static_cast<qreal>(*vIt)));
            value = list.join(QString::fromLatin1(", "));
        }
#endif
        else if(const IntAttribute *intAttribute = dynamic_cast<const IntAttribute*>(attribute))
        {
            value = QString::number(intAttribute->value());
        }
        else if(const KeyCodeAttribute *keyCodeAttribute = dynamic_cast<const KeyCodeAttribute*>(attribute))
        {
            const KeyCode &keyCode = keyCodeAttribute->value();
            value = QString::fromLatin1("filmMfcCode: %1, filmType: %2, prefix: %3, count: %4, perfOffset: %5, perfsPerFrame: %6, perfsPerCount: %7")
                    .arg(keyCode.filmMfcCode())
                    .arg(keyCode.filmType())
                    .arg(keyCode.prefix())
                    .arg(keyCode.count())
                    .arg(keyCode.perfOffset())
                    .arg(keyCode.perfsPerFrame())
                    .arg(keyCode.perfsPerCount());
        }
        else if(const LineOrderAttribute *lineOrderAttribute = dynamic_cast<const LineOrderAttribute*>(attribute))
        {
            const LineOrder &lineOrder = lineOrderAttribute->value();
            switch(lineOrder)
            {
#define ADD_CASE(VALUE) case(VALUE): value = QString::fromLatin1(#VALUE); break
            ADD_CASE(INCREASING_Y);
            ADD_CASE(DECREASING_Y);
            ADD_CASE(RANDOM_Y);
#undef ADD_CASE
            default:
                value = QString::fromLatin1("Unknown");
                break;
            }
        }
        else if(const M33fAttribute *m33fAttribute = dynamic_cast<const M33fAttribute*>(attribute))
        {
            const M33f &m33f = m33fAttribute->value();
            value = QString::fromLatin1("{{%1, %2, %3}, {%4, %5, %6}, {%7, %8, %9}}")
                    .arg(static_cast<qreal>(m33f[0][0])).arg(static_cast<qreal>(m33f[0][1])).arg(static_cast<qreal>(m33f[0][2]))
                    .arg(static_cast<qreal>(m33f[1][0])).arg(static_cast<qreal>(m33f[1][1])).arg(static_cast<qreal>(m33f[1][2]))
                    .arg(static_cast<qreal>(m33f[2][0])).arg(static_cast<qreal>(m33f[2][1])).arg(static_cast<qreal>(m33f[2][2]));
        }
#if OPENEXR_VERSION_GREATER_OR_EQUAL(1, 7, 0)
        else if(const M33dAttribute *m33dAttribute = dynamic_cast<const M33dAttribute*>(attribute))
        {
            const M33d &m33d = m33dAttribute->value();
            value = QString::fromLatin1("{{%1, %2, %3}, {%4, %5, %6}, {%7, %8, %9}}")
                    .arg(static_cast<qreal>(m33d[0][0])).arg(static_cast<qreal>(m33d[0][1])).arg(static_cast<qreal>(m33d[0][2]))
                    .arg(static_cast<qreal>(m33d[1][0])).arg(static_cast<qreal>(m33d[1][1])).arg(static_cast<qreal>(m33d[1][2]))
                    .arg(static_cast<qreal>(m33d[2][0])).arg(static_cast<qreal>(m33d[2][1])).arg(static_cast<qreal>(m33d[2][2]));
        }
#endif
        else if(const M44fAttribute *m44fAttribute = dynamic_cast<const M44fAttribute*>(attribute))
        {
            const M44f &m44f = m44fAttribute->value();
            value = QString::fromLatin1("{{%1, %2, %3, %4}, {%5, %6, %7, %8}, {%9, %10, %11, %12}, {%13, %14, %15, %16}}")
                    .arg(static_cast<qreal>(m44f[0][0])).arg(static_cast<qreal>(m44f[0][1])).arg(static_cast<qreal>(m44f[0][2])).arg(static_cast<qreal>(m44f[0][3]))
                    .arg(static_cast<qreal>(m44f[1][0])).arg(static_cast<qreal>(m44f[1][1])).arg(static_cast<qreal>(m44f[1][2])).arg(static_cast<qreal>(m44f[1][3]))
                    .arg(static_cast<qreal>(m44f[2][0])).arg(static_cast<qreal>(m44f[2][1])).arg(static_cast<qreal>(m44f[2][2])).arg(static_cast<qreal>(m44f[2][3]))
                    .arg(static_cast<qreal>(m44f[3][0])).arg(static_cast<qreal>(m44f[3][1])).arg(static_cast<qreal>(m44f[3][2])).arg(static_cast<qreal>(m44f[3][3]));
        }
#if OPENEXR_VERSION_GREATER_OR_EQUAL(1, 7, 0)
        else if(const M44dAttribute *m44dAttribute = dynamic_cast<const M44dAttribute*>(attribute))
        {
            const M44d &m44d = m44dAttribute->value();
            value = QString::fromLatin1("{{%1, %2, %3, %4}, {%5, %6, %7, %8}, {%9, %10, %11, %12}, {%13, %14, %15, %16}}")
                    .arg(static_cast<qreal>(m44d[0][0])).arg(static_cast<qreal>(m44d[0][1])).arg(static_cast<qreal>(m44d[0][2])).arg(static_cast<qreal>(m44d[0][3]))
                    .arg(static_cast<qreal>(m44d[1][0])).arg(static_cast<qreal>(m44d[1][1])).arg(static_cast<qreal>(m44d[1][2])).arg(static_cast<qreal>(m44d[1][3]))
                    .arg(static_cast<qreal>(m44d[2][0])).arg(static_cast<qreal>(m44d[2][1])).arg(static_cast<qreal>(m44d[2][2])).arg(static_cast<qreal>(m44d[2][3]))
                    .arg(static_cast<qreal>(m44d[3][0])).arg(static_cast<qreal>(m44d[3][1])).arg(static_cast<qreal>(m44d[3][2])).arg(static_cast<qreal>(m44d[3][3]));
        }
#endif
        else if(const OpaqueAttribute *opaqueAttribute = dynamic_cast<const OpaqueAttribute*>(attribute))
        {
            Q_UNUSED(opaqueAttribute);
            value = QString::fromLatin1("Inaccessible");
        }
        else if(const PreviewImageAttribute *previewImageAttribute = dynamic_cast<const PreviewImageAttribute*>(attribute))
        {
            const PreviewImage &previewImage = previewImageAttribute->value();
            value = QString::fromLatin1("%1x%2").arg(previewImage.width()).arg(previewImage.height());
        }
        else if(const RationalAttribute *rationalAttribute = dynamic_cast<const RationalAttribute*>(attribute))
        {
            const Rational &rational = rationalAttribute->value();
            value = QString::fromLatin1("%1/%2").arg(rational.n).arg(rational.d);
        }
        else if(const StringAttribute *stringAttribute = dynamic_cast<const StringAttribute*>(attribute))
        {
            value = QString::fromUtf8(stringAttribute->value().c_str());
        }
#if OPENEXR_VERSION_GREATER_OR_EQUAL(1, 7, 0)
        else if(const StringVectorAttribute *stringVectorAttribute = dynamic_cast<const StringVectorAttribute*>(attribute))
        {
            const StringVector &stringVector = stringVectorAttribute->value();
            QStringList list;
            for(StringVector::const_iterator vIt = stringVector.begin(), vItEnd = stringVector.end(); vIt != vItEnd; ++vIt)
                list.append(QString::fromUtf8(vIt->c_str()));
            value = QString::fromLatin1("\"") + list.join(QString::fromLatin1("\", \"")) + QString::fromLatin1("\"");
        }
#endif
        else if(const TileDescriptionAttribute *tileDescriptionAttribute = dynamic_cast<const TileDescriptionAttribute*>(attribute))
        {
            const TileDescription &tileDescription = tileDescriptionAttribute->value();
            QString mode;
            switch(tileDescription.mode)
            {
#define ADD_CASE(VALUE) case(VALUE): mode = QString::fromLatin1(#VALUE); break
            ADD_CASE(ONE_LEVEL);
            ADD_CASE(MIPMAP_LEVELS);
            ADD_CASE(RIPMAP_LEVELS);
#undef ADD_CASE
            default:
                mode = QString::fromLatin1("Unknown");
                break;
            }
            QString roundingMode;
            switch(tileDescription.roundingMode)
            {
#define ADD_CASE(VALUE) case(VALUE): roundingMode = QString::fromLatin1(#VALUE); break
            ADD_CASE(ROUND_DOWN);
            ADD_CASE(ROUND_UP);
#undef ADD_CASE
            default:
                roundingMode = QString::fromLatin1("Unknown");
                break;
            }
            value = QString::fromLatin1("xSize: %1, ySize: %2, mode: %3, roundingMode: %4")
                    .arg(tileDescription.xSize)
                    .arg(tileDescription.ySize)
                    .arg(mode)
                    .arg(roundingMode);
        }
        else if(const TimeCodeAttribute *timeCodeAttribute = dynamic_cast<const TimeCodeAttribute*>(attribute))
        {
            const TimeCode &timeCode = timeCodeAttribute->value();
            value = QString::fromLatin1("%1:%2:%3:%4")
                    .arg(timeCode.hours())
                    .arg(timeCode.minutes())
                    .arg(timeCode.seconds())
                    .arg(timeCode.frame());
        }
        else if(const V2iAttribute *v2iAttribute = dynamic_cast<const V2iAttribute*>(attribute))
        {
            const V2i &v2i = v2iAttribute->value();
            value = QString::fromLatin1("%1, %2").arg(static_cast<int>(v2i[0])).arg(static_cast<int>(v2i[1]));
        }
        else if(const V2fAttribute *v2fAttribute = dynamic_cast<const V2fAttribute*>(attribute))
        {
            const V2f &v2f = v2fAttribute->value();
            value = QString::fromLatin1("%1, %2").arg(static_cast<qreal>(v2f[0])).arg(static_cast<qreal>(v2f[1]));
        }
#if OPENEXR_VERSION_GREATER_OR_EQUAL(1, 7, 0)
        else if(const V2dAttribute *v2dAttribute = dynamic_cast<const V2dAttribute*>(attribute))
        {
            const V2d &v2d = v2dAttribute->value();
            value = QString::fromLatin1("%1, %2").arg(static_cast<qreal>(v2d[0])).arg(static_cast<qreal>(v2d[1]));
        }
#endif
        else if(const V3iAttribute *v3iAttribute = dynamic_cast<const V3iAttribute*>(attribute))
        {
            const V3i &v3i = v3iAttribute->value();
            value = QString::fromLatin1("%1, %2, %3").arg(static_cast<int>(v3i[0])).arg(static_cast<int>(v3i[1])).arg(static_cast<int>(v3i[2]));
        }
        else if(const V3fAttribute *v3fAttribute = dynamic_cast<const V3fAttribute*>(attribute))
        {
            const V3f &v3f = v3fAttribute->value();
            value = QString::fromLatin1("%1, %2, %3").arg(static_cast<qreal>(v3f[0])).arg(static_cast<qreal>(v3f[1])).arg(static_cast<qreal>(v3f[2]));
        }
#if OPENEXR_VERSION_GREATER_OR_EQUAL(1, 7, 0)
        else if(const V3dAttribute *v3dAttribute = dynamic_cast<const V3dAttribute*>(attribute))
        {
            const V3d &v3d = v3dAttribute->value();
            value = QString::fromLatin1("%1, %2, %3").arg(static_cast<qreal>(v3d[0])).arg(static_cast<qreal>(v3d[1])).arg(static_cast<qreal>(v3d[2]));
        }
#endif

        metaData->addCustomEntry(headerMetaDataType, name, value);
    }

    return metaData;
}

// ====================================================================================================

PayloadWithMetaData<QImage> readExrFile(const QString &filePath)
{
    QFileIStream is(filePath);
    if(!is.isValid())
    {
        qWarning() << "Can't open" << filePath;
        return QImage();
    }

    try
    {
        RgbaInputFile in(is);
        const Header &header = in.header();

        const Box2i &displayWindow = header.displayWindow();
        const QRect displayRect(displayWindow.min.x, displayWindow.min.y,
                                displayWindow.max.x - displayWindow.min.x + 1, displayWindow.max.y - displayWindow.min.y + 1);
        const Box2i &dataWindow = header.dataWindow();
        const QRect dataRect(dataWindow.min.x, dataWindow.min.y,
                             dataWindow.max.x - dataWindow.min.x + 1, dataWindow.max.y - dataWindow.min.y + 1);
        const QRect intersection = displayRect.intersected(dataRect);

        Array<Rgba> pixels;
        const long pixelsSize = static_cast<long>(dataRect.width()) * static_cast<long>(dataRect.height());
        pixels.resizeErase(pixelsSize);
        memset(pixels, 0, static_cast<size_t>(pixelsSize) * (sizeof(Rgba)));
        in.setFrameBuffer(pixels - dataRect.x() - dataRect.y() * dataRect.width(), 1, static_cast<size_t>(dataRect.width()));
        in.readPixels(dataRect.top(), dataRect.bottom());

        QImage image(displayRect.size(), QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        for(int y = 0; y < intersection.height(); ++y)
        {
            const int displayY = y + intersection.y() - displayRect.y();
            const int dataY = y + intersection.y() - dataRect.y();
            QRgb *rgb = reinterpret_cast<QRgb*>(image.scanLine(displayY));
            for(int x = 0; x < intersection.width(); ++x)
            {
                const int displayX = x + intersection.x() - displayRect.x();
                const int dataX = x + intersection.x() - dataRect.x();
                const Rgba &halfData = pixels[dataY * dataRect.width() + dataX];
                const int r = qBound(0, static_cast<int>(halfData.r * 255), 255);
                const int g = qBound(0, static_cast<int>(halfData.g * 255), 255);
                const int b = qBound(0, static_cast<int>(halfData.b * 255), 255);
                const int a = qBound(0, static_cast<int>(halfData.a * 255), 255);
                rgb[displayX] = qRgba(r, g, b, a);
            }
        }
        return PayloadWithMetaData<QImage>(image, readExrMetaData(filePath, header));
    }
    catch(BaseExc &e)
    {
        qWarning() << "ERROR:" << e.what();
    }
    catch(...)
    {
        qWarning() << "ERROR: Unknown exception";
    }
    return QImage();
}

// ====================================================================================================

class DecoderOpenEXR : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderOpenEXR");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
                << QString::fromLatin1("exr");
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
        const PayloadWithMetaData<QImage> readData = readExrFile(filePath);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createImageItem(readData);
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), readData.metaData()));
    }
};

DecoderAutoRegistrator registrator(new DecoderOpenEXR);

// ====================================================================================================

} // namespace
