/*
   Copyright (C) 2017-2020 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "ImageMetaData.h"

#include <cstdio>
#include <string>
#include <sstream>

#include <QImage>
#include <QTransform>
#include <QBuffer>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QDebug>

//#undef HAS_EXIV2
//#undef HAS_LIBEXIF
//#undef HAS_QTEXTENDED

#if defined (HAS_EXIV2)
#define USE_EXIV2
#elif defined (HAS_LIBEXIF)
#define USE_LIBEXIF
#elif defined (HAS_QTEXTENDED)
#define USE_QTEXTENDED
#endif

#if defined (HAS_EXIV2)
#include "Workarounds/BeginIgnoreDeprecated.h"
#include <exiv2/exiv2.hpp>
#include "Workarounds/EndIgnoreDeprecated.h"
#endif
#if defined (HAS_LIBEXIF)
#include <libexif/exif-data.h>
#include <libexif/exif-loader.h>
#endif
#if defined (HAS_QTEXTENDED)
#include "qexifimageheader.h"
#endif

#include "Utils/Global.h"

namespace {

#if defined (USE_EXIV2)

#if (EXIV2_MAJOR_VERSION == 0 && EXIV2_MINOR_VERSION < 27) || (EXIV2_MAJOR_VERSION == 0 && EXIV2_MINOR_VERSION == 27 && EXIV2_PATCH_VERSION < 99)
typedef Exiv2::BasicIo::AutoPtr Exiv2BasicIoPtr;
typedef Exiv2::Image::AutoPtr Exiv2ImagePtr;
#else
typedef Exiv2::BasicIo::UniquePtr Exiv2BasicIoPtr;
typedef Exiv2::Image::UniquePtr Exiv2ImagePtr;
#endif

class QFileIo : public Exiv2::BasicIo
{
    Q_DISABLE_COPY(QFileIo)

public:
    explicit QFileIo(const QString &path)
        : m_file(path)
        , m_mapped(Q_NULLPTR)
    {}

    //! @name Manipulators
    //@{
    /*!
      @brief Open the IO source using the default access mode. The
          default mode should allow for reading and writing.

      This method can also be used to "reopen" an IO source which will
      flush any unwritten data and reset the IO position to the start.
      Subclasses may provide custom methods to allow for
      opening IO sources differently.

      @return 0 if successful;<BR>
          Nonzero if failure.
     */
    int open() Q_DECL_OVERRIDE
    {
        if(m_file.isOpen())
            m_file.close();
        if(m_file.open(QIODevice::ReadOnly))
            return 0;
        qWarning() << "[Exiv2::QFileIo]" << "Error in open():" << m_file.errorString();
        return 1;
    }

    /*!
      @brief Close the IO source. After closing a BasicIo instance can not
          be read or written. Closing flushes any unwritten data. It is
          safe to call close on a closed instance.
      @return 0 if successful;<BR>
          Nonzero if failure.
     */
    int close() Q_DECL_OVERRIDE
    {
        m_file.close();
        return 0;
    }

    /*!
      @brief Write data to the IO source. Current IO position is advanced
          by the number of bytes written.
      @param data Pointer to data. Data must be at least \em wcount
          bytes long
      @param wcount Number of bytes to be written.
      @return Number of bytes written to IO source successfully;<BR>
          0 if failure;
     */
    long write(const Exiv2::byte * /*data*/, long /*wcount*/) Q_DECL_OVERRIDE
    {
        qWarning() << "[Exiv2::QFileIo]" << "Error in write(): Not allowed (RO only)";
        return 0;
    }

    /*!
      @brief Write data that is read from another BasicIo instance to
          the IO source. Current IO position is advanced by the number
          of bytes written.
      @param src Reference to another BasicIo instance. Reading start
          at the source's current IO position
      @return Number of bytes written to IO source successfully;<BR>
          0 if failure;
     */
    long write(Exiv2::BasicIo &/*src*/) Q_DECL_OVERRIDE
    {
        qWarning() << "[Exiv2::QFileIo]" << "Error in write(): Not allowed (RO only)";
        return 0;
    }

    /*!
      @brief Write one byte to the IO source. Current IO position is
          advanced by one byte.
      @param data The single byte to be written.
      @return The value of the byte written if successful;<BR>
          EOF if failure;
     */
    virtual int putb(Exiv2::byte /*data*/) Q_DECL_OVERRIDE
    {
        qWarning() << "[Exiv2::QFileIo]" << "Error in putb(): Not allowed (RO only)";
        return EOF;
    }

    /*!
      @brief Read data from the IO source. Reading starts at the current
          IO position and the position is advanced by the number of bytes
          read.
      @param rcount Maximum number of bytes to read. Fewer bytes may be
          read if \em rcount bytes are not available.
      @return DataBuf instance containing the bytes read. Use the
          DataBuf::size_ member to find the number of bytes read.
          DataBuf::size_ will be 0 on failure.
     */
    Exiv2::DataBuf read(long rcount) Q_DECL_OVERRIDE
    {
        const QByteArray ba = m_file.read(static_cast<qint64>(rcount));
        Exiv2::DataBuf buf(reinterpret_cast<const Exiv2::byte*>(ba.constData()), static_cast<long>(ba.size()));
        return buf;
    }

    /*!
      @brief Read data from the IO source. Reading starts at the current
          IO position and the position is advanced by the number of bytes
          read.
      @param buf Pointer to a block of memory into which the read data
          is stored. The memory block must be at least \em rcount bytes
          long.
      @param rcount Maximum number of bytes to read. Fewer bytes may be
          read if \em rcount bytes are not available.
      @return Number of bytes read from IO source successfully;<BR>
          0 if failure;
     */
    long read(Exiv2::byte *buf, long rcount) Q_DECL_OVERRIDE
    {
        return static_cast<long>(m_file.read(reinterpret_cast<char*>(buf), static_cast<qint64>(rcount)));
    }

    /*!
      @brief Read one byte from the IO source. Current IO position is
          advanced by one byte.
      @return The byte read from the IO source if successful;<BR>
          EOF if failure;
     */
    int getb() Q_DECL_OVERRIDE
    {
        Exiv2::byte byte = 0;
        if(m_file.read(reinterpret_cast<char*>(&byte), 1) == 1)
            return byte;
        qWarning() << "[Exiv2::QFileIo]" << "Error in getb():" << m_file.errorString();
        return EOF;
    }

    /*!
      @brief Remove all data from this object's IO source and then transfer
          data from the \em src BasicIo object into this object.

      The source object is invalidated by this operation and should not be
      used after this method returns. This method exists primarily to
      be used with the BasicIo::temporary() method.

      @param src Reference to another BasicIo instance. The entire contents
          of src are transferred to this object. The \em src object is
          invalidated by the method.
      @throw Error In case of failure
     */
    void transfer(BasicIo &/*src*/) Q_DECL_OVERRIDE
    {
        qWarning() << "[Exiv2::QFileIo]" << "Error in transfer(): Not allowed (RO only)";
#if (EXIV2_MAJOR_VERSION == 0 && EXIV2_MINOR_VERSION < 26)
        const int errorCode = 1;
#else
        const Exiv2::ErrorCode errorCode = Exiv2::kerErrorMessage;
#endif
        throw Exiv2::Error(errorCode, "Not allowed (RO only)");
    }

    /*!
      @brief Move the current IO position.
      @param offset Number of bytes to move the position relative
          to the starting position specified by \em pos
      @param pos Position from which the seek should start
      @return 0 if successful;<BR>
          Nonzero if failure;
     */
#if defined (_MSC_VER)
    int seek(int64_t offset, BasicIo::Position pos) Q_DECL_OVERRIDE
#else
    int seek(long offset, BasicIo::Position pos) Q_DECL_OVERRIDE
#endif
    {
        qint64 newIdx = 0;
        switch(pos)
        {
        case BasicIo::cur:
            newIdx = m_file.pos() + static_cast<qint64>(offset);
            break;
        case BasicIo::beg:
            newIdx = static_cast<qint64>(offset);
            break;
        case BasicIo::end:
            newIdx = m_file.size() + static_cast<qint64>(offset);
            break;
        }
        if(newIdx < 0)
        {
            qWarning() << "[Exiv2::QFileIo]" << "Error in seek(): position is negative";
            return 1;
        }
        if(m_file.seek(newIdx))
            return 0;
        qWarning() << "[Exiv2::QFileIo]" << "Error in seek():" << m_file.errorString();
        return 1;
    }

    /*!
      @brief Direct access to the IO data. For files, this is done by
             mapping the file into the process's address space; for memory
             blocks, this allows direct access to the memory block.
      @param isWriteable Set to true if the mapped area should be writeable
             (default is false).
      @return A pointer to the mapped area.
      @throw Error In case of failure.
     */
    Exiv2::byte *mmap(bool /*isWriteable*/ = false) Q_DECL_OVERRIDE
    {
        uchar *memory = m_file.map(0, m_file.size());
        if(!memory)
        {
            qWarning() << "[Exiv2::QFileIo]" << "Error in mmap():" << m_file.errorString();
#if (EXIV2_MAJOR_VERSION == 0 && EXIV2_MINOR_VERSION < 26)
            const int errorCode = 1;
#else
            const Exiv2::ErrorCode errorCode = Exiv2::kerErrorMessage;
#endif
            throw Exiv2::Error(errorCode, m_file.errorString().toLocal8Bit().data());
        }
        m_mapped = reinterpret_cast<Exiv2::byte*>(memory);
        return m_mapped;
    }

    /*!
      @brief Remove a mapping established with mmap(). If the mapped area
             is writeable, this ensures that changes are written back.
      @return 0 if successful;<BR>
              Nonzero if failure;
     */
    int munmap() Q_DECL_OVERRIDE
    {
        if(!m_file.unmap(reinterpret_cast<uchar*>(m_mapped)))
        {
            qWarning() << "[Exiv2::QFileIo]" << "Error in mmap():" << m_file.errorString();
            return 1;
        }
        m_mapped = Q_NULLPTR;
        return 0;
    }

    //@}

    //! @name Accessors
    //@{
    /*!
      @brief Get the current IO position.
      @return Offset from the start of IO if successful;<BR>
             -1 if failure;
     */
    long tell() const Q_DECL_OVERRIDE
    {
        if(m_file.isOpen())
            return static_cast<long>(m_file.pos());
        qWarning() << "[Exiv2::QFileIo]" << "Error in tell(): File is not open!";
        return -1;
    }

    /*!
      @brief Get the current size of the IO source in bytes.
      @return Size of the IO source in bytes;<BR>
             -1 if failure;
     */
#if (EXIV2_MAJOR_VERSION == 0 && EXIV2_MINOR_VERSION < 26)
    long size() const Q_DECL_OVERRIDE
    {
        return static_cast<long>(m_file.size());
    }
#else
    size_t size() const Q_DECL_OVERRIDE
    {
        return static_cast<size_t>(m_file.size());
    }
#endif

    //!Returns true if the IO source is open, otherwise false.
    bool isopen() const Q_DECL_OVERRIDE
    {
        return m_file.isOpen();
    }

    //!Returns 0 if the IO source is in a valid state, otherwise nonzero.
    int error() const Q_DECL_OVERRIDE
    {
        return m_file.error();
    }

    //!Returns true if the IO position has reached the end, otherwise false.
    bool eof() const Q_DECL_OVERRIDE
    {
        return m_file.atEnd();
    }

    /*!
      @brief Return the path to the IO resource. Often used to form
          comprehensive error messages where only a BasicIo instance is
          available.
     */
    std::string path() const Q_DECL_OVERRIDE
    {
        return m_file.fileName().toLocal8Bit().data();
    }

#if defined (EXV_UNICODE_PATH)
    /*!
      @brief Like path() but returns a unicode path in an std::wstring.
      @note This function is only available on Windows.
     */
    std::wstring wpath() const Q_DECL_OVERRIDE
    {
        return m_file.fileName().toStdWString();
    }
#endif

#if (EXIV2_MAJOR_VERSION == 0 && EXIV2_MINOR_VERSION < 26)
    /*!
      @brief Returns a temporary data storage location. This is often
          needed to rewrite an IO source.
      For example, data may be read from the original IO source, modified
      in some way, and then saved to the temporary instance. After the
      operation is complete, the BasicIo::transfer method can be used to
      replace the original IO source with the modified version. Subclasses
      are free to return any class that derives from BasicIo.
      @return An instance of BasicIo on success
      @throw Error In case of failure
     */
    Exiv2::BasicIo::AutoPtr temporary() const Q_DECL_OVERRIDE
    {
        return Exiv2::BasicIo::AutoPtr(new Exiv2::MemIo);
    }
#endif

    //@}

private:
    QFile m_file;
    Exiv2::byte *m_mapped;
};

QString formatValue(const std::string &value)
{
    return QString::fromUtf8(value.c_str());
}

QString formatValue(const Exiv2::Value &value)
{
    std::ostringstream sst;
    sst << value;
    return formatValue(sst.str());
}

void fillExifMetaData(const Exiv2::ExifData &data, IImageMetaData::MetaDataEntryListMap &entryListMap)
{
    try
    {
        for(Exiv2::ExifData::const_iterator it = data.begin(), end = data.end(); it != end; ++it)
        {
            const IImageMetaData::MetaDataType type = QString::fromUtf8(it->familyName()) + QString::fromLatin1(" ") + QString::fromUtf8(it->ifdName());
            IImageMetaData::MetaDataEntryList list = entryListMap[type];
            list.append(IImageMetaData::MetaDataEntry(
                            QString::fromUtf8(it->tagName().c_str()),
                            QString::fromUtf8(it->tagLabel().c_str()),
                            QString(),
                            formatValue(it->value())
                            ));
            entryListMap[type] = list;
        }
    }
    catch(...)
    {}
}

void fillIptcMetaData(const Exiv2::IptcData &data, IImageMetaData::MetaDataEntryListMap &entryListMap)
{
    try
    {
        for(Exiv2::IptcData::const_iterator it = data.begin(), end = data.end(); it != end; ++it)
        {
            const IImageMetaData::MetaDataType type = QString::fromUtf8(it->familyName());
            IImageMetaData::MetaDataEntryList list = entryListMap[type];
            list.append(IImageMetaData::MetaDataEntry(
                            QString::fromUtf8(it->tagName().c_str()),
                            QString::fromUtf8(it->tagLabel().c_str()),
                            QString(),
                            formatValue(it->value())
                            ));
            entryListMap[type] = list;
        }
    }
    catch(...)
    {}
}

void fillXmpMetaData(const Exiv2::XmpData &data, IImageMetaData::MetaDataEntryListMap &entryListMap)
{
    try
    {
        for(Exiv2::XmpData::const_iterator it = data.begin(), end = data.end(); it != end; ++it)
        {
            const IImageMetaData::MetaDataType type = QString::fromUtf8(it->familyName());
            IImageMetaData::MetaDataEntryList list = entryListMap[type];
            list.append(IImageMetaData::MetaDataEntry(
                            QString::fromUtf8(it->tagName().c_str()),
                            QString::fromUtf8(it->tagLabel().c_str()),
                            QString(),
                            formatValue(it->value())
                            ));
            entryListMap[type] = list;
        }
    }
    catch(...)
    {}
}

void fillCommentMetaData(const std::string &data, IImageMetaData::MetaDataEntryListMap &entryListMap)
{
    if(data.empty())
        return;
    const IImageMetaData::MetaDataType type = QString::fromLatin1("Comment");
    IImageMetaData::MetaDataEntryList list = entryListMap[type];
    list.append(IImageMetaData::MetaDataEntry(type, formatValue(data)));
    entryListMap[type] = list;
}

quint16 getOrientation(const Exiv2::ExifData &data)
{
    try
    {
        Exiv2::ExifData::const_iterator it = Exiv2::orientation(data);
        if(it != data.end())
            return static_cast<quint16>(it->toLong());
    }
    catch(...)
    {}
    return 1;
}

#elif defined (USE_LIBEXIF)

IImageMetaData::MetaDataType getMetaDataType(ExifIfd ifd)
{
    return QString::fromLatin1("EXIF IFD %1").arg(QString::fromUtf8(exif_ifd_get_name(ifd)));
}

#elif defined (USE_QTEXTENDED)

template<typename T>
QString toStringValue(const T &value)
{
    return QString::fromLatin1("%1").arg(value);
}

template<>
QString toStringValue(const QString &value)
{
    return value;
}

template<>
QString toStringValue(const QByteArray &value)
{
    return QString::fromLatin1(value.toHex());
}

template<>
QString toStringValue(const QExifURational &value)
{
    return QString::fromLatin1("%1/%2").arg(value.first).arg(value.second);
}

template<>
QString toStringValue(const QExifSRational &value)
{
    return QString::fromLatin1("%1/%2").arg(value.first).arg(value.second);
}

template<typename T>
QString toStringValue(const QVector<T> &value)
{
    QStringList result;
    for(typename QVector<T>::ConstIterator it = value.begin(), itEnd = value.end(); it != itEnd; ++it)
        result.append(toStringValue(*it));
    return QString::fromLatin1("{%1}").arg(result.join(QString::fromLatin1(", ")));
}

template<>
QString toStringValue(const QExifValue &value)
{
    const bool isVector = value.count() > 1;
    switch(value.type())
    {
    case QExifValue::Byte:
        return isVector ? toStringValue(value.toByteVector()) : toStringValue(value.toByte());
    case QExifValue::Ascii:
        return toStringValue(value.toString());
    case QExifValue::Short:
        return isVector ? toStringValue(value.toShortVector()) : toStringValue(value.toShort());
    case QExifValue::Long:
        return isVector ? toStringValue(value.toLongVector()) : toStringValue(value.toLong());
    case QExifValue::Rational:
        return isVector ? toStringValue(value.toRationalVector()) : toStringValue(value.toRational());
    case QExifValue::Undefined:
        return toStringValue(value.toByteArray());
    case QExifValue::SignedLong:
        return isVector ? toStringValue(value.toSignedLongVector()) : toStringValue(value.toSignedLong());
    case QExifValue::SignedRational:
        return isVector ? toStringValue(value.toSignedRationalVector()) : toStringValue(value.toSignedRational());
    default:
        break;
    }
    return QString::fromLatin1("-");
}

/// @see http://www.cipa.jp/std/documents/e/DC-008-Translation-2016-E.pdf
IImageMetaData::MetaDataEntry makeEntry(int tagID, const QExifValue &value)
{
    switch(tagID)
    {
    // ImageTag
    case QExifImageHeader::ImageWidth: // 0x0100
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("ImageWidth"), toStringValue(value));
    case QExifImageHeader::ImageLength: // 0x0101
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("ImageLength"), toStringValue(value));
    case QExifImageHeader::BitsPerSample: // 0x0102
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("BitsPerSample"), toStringValue(value));
    case QExifImageHeader::Compression: // 0x0103
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("Compression"), toStringValue(value));
    case QExifImageHeader::PhotometricInterpretation: // 0x0106
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("PhotometricInterpretation"), toStringValue(value));
    case QExifImageHeader::Orientation: // 0x0112
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("Orientation"), toStringValue(value));
    case QExifImageHeader::SamplesPerPixel: // 0x0115
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("SamplesPerPixel"), toStringValue(value));
    case QExifImageHeader::PlanarConfiguration: // 0x011C
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("PlanarConfiguration"), toStringValue(value));
    case QExifImageHeader::YCbCrSubSampling: // 0x0212
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("YCbCrSubSampling"), toStringValue(value));
    case QExifImageHeader::XResolution: // 0x011A
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("XResolution"), toStringValue(value));
    case QExifImageHeader::YResolution: // 0x011B
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("YResolution"), toStringValue(value));
    case QExifImageHeader::ResolutionUnit: // 0x0128
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("ResolutionUnit"), toStringValue(value));
    case QExifImageHeader::StripOffsets: // 0x0111
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("StripOffsets"), toStringValue(value));
    case QExifImageHeader::RowsPerStrip: // 0x0116
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("RowsPerStrip"), toStringValue(value));
    case QExifImageHeader::StripByteCounts: // 0x0117
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("StripByteCounts"), toStringValue(value));
    case QExifImageHeader::TransferFunction: // 0x012D
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("TransferFunction"), toStringValue(value));
    case QExifImageHeader::WhitePoint: // 0x013E
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("WhitePoint"), toStringValue(value));
    case QExifImageHeader::PrimaryChromaciticies: // 0x013F
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("PrimaryChromaciticies"), toStringValue(value));
    case QExifImageHeader::YCbCrCoefficients: // 0x0211
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("YCbCrCoefficients"), toStringValue(value));
    case QExifImageHeader::ReferenceBlackWhite: // 0x0214
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("ReferenceBlackWhite"), toStringValue(value));
    case QExifImageHeader::DateTime: // 0x0132
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("DateTime"), toStringValue(value));
    case QExifImageHeader::ImageDescription: // 0x010E
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("ImageDescription"), toStringValue(value));
    case QExifImageHeader::Make: // 0x010F
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("Make"), toStringValue(value));
    case QExifImageHeader::Model: // 0x0110
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("Model"), toStringValue(value));
    case QExifImageHeader::Software: // 0x0131
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("Software"), toStringValue(value));
    case QExifImageHeader::Artist: // 0x013B
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("Artist"), toStringValue(value));
    case QExifImageHeader::Copyright: // 0x8298
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("Copyright"), toStringValue(value));
    // ExifExtendedTag
    case QExifImageHeader::ExifVersion: // 0x9000
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("ExifVersion"), toStringValue(value));
    case QExifImageHeader::FlashPixVersion: // 0xA000
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("FlashPixVersion"), toStringValue(value));
    case QExifImageHeader::ColorSpace: // 0xA001
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("ColorSpace"), toStringValue(value));
    case QExifImageHeader::ComponentsConfiguration: // 0x9101
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("ComponentsConfiguration"), toStringValue(value));
    case QExifImageHeader::CompressedBitsPerPixel: // 0x9102
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("CompressedBitsPerPixel"), toStringValue(value));
    case QExifImageHeader::PixelXDimension: // 0xA002
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("PixelXDimension"), toStringValue(value));
    case QExifImageHeader::PixelYDimension: // 0xA003
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("PixelYDimension"), toStringValue(value));
    case QExifImageHeader::MakerNote: // 0x927C
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("MakerNote"), toStringValue(value));
    case QExifImageHeader::UserComment: // 0x9286
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("UserComment"), toStringValue(value));
    case QExifImageHeader::RelatedSoundFile: // 0xA004
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("RelatedSoundFile"), toStringValue(value));
    case QExifImageHeader::DateTimeOriginal: // 0x9003
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("DateTimeOriginal"), toStringValue(value));
    case QExifImageHeader::DateTimeDigitized: // 0x9004
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("DateTimeDigitized"), toStringValue(value));
    case QExifImageHeader::SubSecTime: // 0x9290
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("SubSecTime"), toStringValue(value));
    case QExifImageHeader::SubSecTimeOriginal: // 0x9291
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("SubSecTimeOriginal"), toStringValue(value));
    case QExifImageHeader::SubSecTimeDigitized: // 0x9292
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("SubSecTimeDigitized"), toStringValue(value));
    case QExifImageHeader::ImageUniqueId: // 0xA420
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("ImageUniqueId"), toStringValue(value));
    case QExifImageHeader::ExposureTime: // 0x829A
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("ExposureTime"), toStringValue(value));
    case QExifImageHeader::FNumber: // 0x829D
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("FNumber"), toStringValue(value));
    case QExifImageHeader::ExposureProgram: // 0x8822
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("ExposureProgram"), toStringValue(value));
    case QExifImageHeader::SpectralSensitivity: // 0x8824
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("SpectralSensitivity"), toStringValue(value));
    case QExifImageHeader::ISOSpeedRatings: // 0x8827
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("ISOSpeedRatings"), toStringValue(value));
    case QExifImageHeader::Oecf: // 0x8828
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("Oecf"), toStringValue(value));
    case QExifImageHeader::ShutterSpeedValue: // 0x9201
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("ShutterSpeedValue"), toStringValue(value));
    case QExifImageHeader::ApertureValue: // 0x9202
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("ApertureValue"), toStringValue(value));
    case QExifImageHeader::BrightnessValue: // 0x9203
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("BrightnessValue"), toStringValue(value));
    case QExifImageHeader::ExposureBiasValue: // 0x9204
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("ExposureBiasValue"), toStringValue(value));
    case QExifImageHeader::MaxApertureValue: // 0x9205
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("MaxApertureValue"), toStringValue(value));
    case QExifImageHeader::SubjectDistance: // 0x9206
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("SubjectDistance"), toStringValue(value));
    case QExifImageHeader::MeteringMode: // 0x9207
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("MeteringMode"), toStringValue(value));
    case QExifImageHeader::LightSource: // 0x9208
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("LightSource"), toStringValue(value));
    case QExifImageHeader::Flash: // 0x9209
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("Flash"), toStringValue(value));
    case QExifImageHeader::FocalLength: // 0x920A
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("FocalLength"), toStringValue(value));
    case QExifImageHeader::SubjectArea: // 0x9214
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("SubjectArea"), toStringValue(value));
    case QExifImageHeader::FlashEnergy: // 0xA20B
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("FlashEnergy"), toStringValue(value));
    case QExifImageHeader::SpatialFrequencyResponse: // 0xA20C
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("SpatialFrequencyResponse"), toStringValue(value));
    case QExifImageHeader::FocalPlaneXResolution: // 0xA20E
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("FocalPlaneXResolution"), toStringValue(value));
    case QExifImageHeader::FocalPlaneYResolution: // 0xA20F
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("FocalPlaneYResolution"), toStringValue(value));
    case QExifImageHeader::FocalPlaneResolutionUnit: // 0xA210
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("FocalPlaneResolutionUnit"), toStringValue(value));
    case QExifImageHeader::SubjectLocation: // 0xA214
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("SubjectLocation"), toStringValue(value));
    case QExifImageHeader::ExposureIndex: // 0xA215
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("ExposureIndex"), toStringValue(value));
    case QExifImageHeader::SensingMethod: // 0xA217
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("SensingMethod"), toStringValue(value));
    case QExifImageHeader::FileSource: // 0xA300
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("FileSource"), toStringValue(value));
    case QExifImageHeader::SceneType: // 0xA301
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("SceneType"), toStringValue(value));
    case QExifImageHeader::CfaPattern: // 0xA302
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("CfaPattern"), toStringValue(value));
    case QExifImageHeader::CustomRendered: // 0xA401
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("CustomRendered"), toStringValue(value));
    case QExifImageHeader::ExposureMode: // 0xA402
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("ExposureMode"), toStringValue(value));
    case QExifImageHeader::WhiteBalance: // 0xA403
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("WhiteBalance"), toStringValue(value));
    case QExifImageHeader::DigitalZoomRatio: // 0xA404
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("DigitalZoomRatio"), toStringValue(value));
    case QExifImageHeader::FocalLengthIn35mmFilm: // 0xA405
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("FocalLengthIn35mmFilm"), toStringValue(value));
    case QExifImageHeader::SceneCaptureType: // 0xA406
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("SceneCaptureType"), toStringValue(value));
    case QExifImageHeader::GainControl: // 0xA407
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GainControl"), toStringValue(value));
    case QExifImageHeader::Contrast: // 0xA408
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("Contrast"), toStringValue(value));
    case QExifImageHeader::Saturation: // 0xA409
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("Saturation"), toStringValue(value));
    case QExifImageHeader::Sharpness: // 0xA40A
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("Sharpness"), toStringValue(value));
    case QExifImageHeader::DeviceSettingDescription: // 0xA40B
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("DeviceSettingDescription"), toStringValue(value));
    case QExifImageHeader::SubjectDistanceRange: // 0x40C
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("SubjectDistanceRange"), toStringValue(value));
    // GpsTag
    case QExifImageHeader::GpsVersionId: // 0x0000
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsVersionId"), toStringValue(value));
    case QExifImageHeader::GpsLatitudeRef: // 0x0001
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsLatitudeRef"), toStringValue(value));
    case QExifImageHeader::GpsLatitude: // 0x0002
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsLatitude"), toStringValue(value));
    case QExifImageHeader::GpsLongitudeRef: // 0x0003
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsLongitudeRef"), toStringValue(value));
    case QExifImageHeader::GpsLongitude: // 0x0004
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsLongitude"), toStringValue(value));
    case QExifImageHeader::GpsAltitudeRef: // 0x0005
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsAltitudeRef"), toStringValue(value));
    case QExifImageHeader::GpsAltitude: // 0x0006
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsAltitude"), toStringValue(value));
    case QExifImageHeader::GpsTimeStamp: // 0x0007
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsTimeStamp"), toStringValue(value));
    case QExifImageHeader::GpsSatellites: // 0x0008
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsSatellites"), toStringValue(value));
    case QExifImageHeader::GpsStatus: // 0x0009
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsStatus"), toStringValue(value));
    case QExifImageHeader::GpsMeasureMode: // 0x000A
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsMeasureMode"), toStringValue(value));
    case QExifImageHeader::GpsDop: // 0x000B
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsDop"), toStringValue(value));
    case QExifImageHeader::GpsSpeedRef: // 0x000C
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsSpeedRef"), toStringValue(value));
    case QExifImageHeader::GpsSpeed: // 0x000D
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsSpeed"), toStringValue(value));
    case QExifImageHeader::GpsTrackRef: // 0x000E
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsTrackRef"), toStringValue(value));
    case QExifImageHeader::GpsTrack: // 0x000F
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsTrack"), toStringValue(value));
    case QExifImageHeader::GpsImageDirectionRef: // 0x0010
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsImageDirectionRef"), toStringValue(value));
    case QExifImageHeader::GpsImageDirection: // 0x0011
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsImageDirection"), toStringValue(value));
    case QExifImageHeader::GpsMapDatum: // 0x0012
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsMapDatum"), toStringValue(value));
    case QExifImageHeader::GpsDestLatitudeRef: // 0x0013
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsDestLatitudeRef"), toStringValue(value));
    case QExifImageHeader::GpsDestLatitude: // 0x0014
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsDestLatitude"), toStringValue(value));
    case QExifImageHeader::GpsDestLongitudeRef: // 0x0015
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsDestLongitudeRef"), toStringValue(value));
    case QExifImageHeader::GpsDestLongitude: // 0x0016
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsDestLongitude"), toStringValue(value));
    case QExifImageHeader::GpsDestBearingRef: // 0x0017
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsDestBearingRef"), toStringValue(value));
    case QExifImageHeader::GpsDestBearing: // 0x0018
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsDestBearing"), toStringValue(value));
    case QExifImageHeader::GpsDestDistanceRef: // 0x0019
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsDestDistanceRef"), toStringValue(value));
    case QExifImageHeader::GpsDestDistance: // 0x001A
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsDestDistance"), toStringValue(value));
    case QExifImageHeader::GpsProcessingMethod: // 0x001B
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsProcessingMethod"), toStringValue(value));
    case QExifImageHeader::GpsAreaInformation: // 0x001C
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsAreaInformation"), toStringValue(value));
    case QExifImageHeader::GpsDateStamp: // 0x001D
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsDateStamp"), toStringValue(value));
    case QExifImageHeader::GpsDifferential: // 0x001E
        return IImageMetaData::MetaDataEntry(QString::fromLatin1("GpsDifferential"), toStringValue(value));
    default:
        break;
    }
    return IImageMetaData::MetaDataEntry(QString::fromLatin1("Unknown (Dec: %1, Hex: 0x%2)"), toStringValue(value));
}

#endif

} // namespace

struct ImageMetaData::Impl
{
    IImageMetaData::MetaDataEntryListMap entryListMap;
    bool isEntryListLoaded;
#if defined (USE_EXIV2)
    Exiv2ImagePtr image;
    Exiv2::ExifData exifData;
    Exiv2::XmpData xmpData;
#elif defined (USE_LIBEXIF)
    ExifData *exifData;
#elif defined (USE_QTEXTENDED)
    QExifImageHeader exifHeader;
#endif

    Impl()
        : isEntryListLoaded(false)
    {
#if defined (USE_LIBEXIF)
        exifData = Q_NULLPTR;
#endif
    }

    ~Impl()
    {
#if defined (USE_LIBEXIF)
        if(exifData)
            exif_data_unref(exifData);
#endif
    }

    quint16 getExifOrientation() const
    {
        quint16 orientation = 1;
#if defined (USE_EXIV2)
        if(image.get())
        {
            orientation = getOrientation(image->exifData());
            qDebug() << "EXIF orientation =" << orientation;
        }
        else if(!exifData.empty())
        {
            orientation = getOrientation(exifData);
            qDebug() << "EXIF orientation =" << orientation;
        }
#elif defined (USE_LIBEXIF)
        if(!exifData)
            return orientation;
        ExifEntry *entry = exif_content_get_entry(exifData->ifd[EXIF_IFD_0], EXIF_TAG_ORIENTATION);
        if(entry && entry->parent && entry->parent->parent && entry->format == EXIF_FORMAT_SHORT && entry->components == 1)
        {
            orientation = exif_get_short(entry->data, exif_data_get_byte_order(entry->parent->parent));
            qDebug() << "EXIF orientation =" << orientation;
        }
#elif defined (USE_QTEXTENDED)
        if(exifHeader.contains(QExifImageHeader::Orientation))
        {
            orientation = exifHeader.value(QExifImageHeader::Orientation).toShort();
            qDebug() << "EXIF orientation =" << orientation;
        }
#endif
        return orientation;
    }

#if defined (USE_EXIV2)

    void fillMetaData()
    {
        if(image.get())
        {
            fillExifMetaData(image->exifData(), entryListMap);
            fillIptcMetaData(image->iptcData(), entryListMap);
            fillXmpMetaData(image->xmpData(), entryListMap);
            fillCommentMetaData(image->comment(), entryListMap);
        }
        else
        {
            if(!exifData.empty())
                fillExifMetaData(exifData, entryListMap);
            if(!xmpData.empty())
                fillXmpMetaData(xmpData, entryListMap);
        }
    }

#elif defined (USE_LIBEXIF)

    void fillMetaData()
    {
        if(!exifData)
            return;
        for(int i = 0; i < EXIF_IFD_COUNT; i++)
        {
            if(!exifData->ifd[i] || !exifData->ifd[i]->count)
                continue;
            const IImageMetaData::MetaDataType type = getMetaDataType(static_cast<ExifIfd>(i));
            IImageMetaData::MetaDataEntryList list = entryListMap[type];
            for(unsigned int j = 0, count = exifData->ifd[i]->count; j < count; j++)
            {
                ExifEntry *e = exifData->ifd[i]->entries[j];
                char value[8192];
                list.append(IImageMetaData::MetaDataEntry(
                                QString::fromUtf8(exif_tag_get_name_in_ifd(e->tag, static_cast<ExifIfd>(i))),
                                QString::fromUtf8(exif_tag_get_title_in_ifd(e->tag, static_cast<ExifIfd>(i))),
                                QString::fromUtf8(exif_tag_get_description_in_ifd(e->tag, static_cast<ExifIfd>(i))),
                                QString::fromUtf8(exif_entry_get_value(e, value, sizeof(value)))
                                ));
            }
            entryListMap[type] = list;
        }
    }

#elif defined (USE_QTEXTENDED)

    void fillMetaData()
    {
        const QList<QExifImageHeader::ImageTag> imageTags = exifHeader.imageTags();
        if(!imageTags.empty())
        {
            const IImageMetaData::MetaDataType imageTagsType = QString::fromLatin1("EXIF IFD 0");
            IImageMetaData::MetaDataEntryList imageTagsData = entryListMap[imageTagsType];
            for(QList<QExifImageHeader::ImageTag>::ConstIterator it = imageTags.constBegin(); it != imageTags.constEnd(); ++it)
                imageTagsData.append(makeEntry(*it, exifHeader.value(*it)));
            entryListMap[imageTagsType] = imageTagsData;
        }

        const QList<QExifImageHeader::ExifExtendedTag> extendedTags = exifHeader.extendedTags();
        if(!extendedTags.empty())
        {
            const IImageMetaData::MetaDataType extendedTagsType = QString::fromLatin1("EXIF IFD EXIF");
            IImageMetaData::MetaDataEntryList extendedTagsData = entryListMap[extendedTagsType];
            for(QList<QExifImageHeader::ExifExtendedTag>::ConstIterator it = extendedTags.constBegin(); it != extendedTags.constEnd(); ++it)
                extendedTagsData.append(makeEntry(*it, exifHeader.value(*it)));
            entryListMap[extendedTagsType] = extendedTagsData;
        }

        const QList<QExifImageHeader::GpsTag> gpsTags = exifHeader.gpsTags();
        if(!gpsTags.empty())
        {
            const IImageMetaData::MetaDataType gpsTagsType = QString::fromLatin1("EXIF IFD GPS");
            IImageMetaData::MetaDataEntryList gpsTagsData = entryListMap[gpsTagsType];
            for(QList<QExifImageHeader::GpsTag>::ConstIterator it = gpsTags.constBegin(); it != gpsTags.constEnd(); ++it)
                gpsTagsData.append(makeEntry(*it, exifHeader.value(*it)));
            entryListMap[gpsTagsType] = gpsTagsData;
        }
    }

#else

    void fillMetaData()
    {}

#endif

    void ensureMetaDataFilled()
    {
        if(isEntryListLoaded)
            return;
        fillMetaData();
        isEntryListLoaded = true;
    }
};

ImageMetaData *ImageMetaData::createMetaData(const QString &filePath)
{
    ImageMetaData *metaData = new ImageMetaData();
    if(metaData->readFile(filePath))
        return metaData;
    delete metaData;
    return Q_NULLPTR;
}

ImageMetaData *ImageMetaData::createMetaData(const QByteArray &fileData)
{
    ImageMetaData *metaData = new ImageMetaData();
    if(metaData->readFile(fileData))
        return metaData;
    delete metaData;
    return Q_NULLPTR;
}

ImageMetaData *ImageMetaData::createExifMetaData(const QByteArray &rawExifData)
{
    ImageMetaData *metaData = new ImageMetaData();
    if(metaData->readExifData(rawExifData))
        return metaData;
    delete metaData;
    return Q_NULLPTR;
}

ImageMetaData *ImageMetaData::createXmpMetaData(const QByteArray &rawXmpData)
{
    ImageMetaData *metaData = new ImageMetaData();
    if(metaData->readXmpData(rawXmpData))
        return metaData;
    delete metaData;
    return Q_NULLPTR;
}

ImageMetaData *ImageMetaData::joinMetaData(ImageMetaData *first, ImageMetaData *second)
{
    if(!first)
        return second;
    if(!second)
        return first;

    first->m_impl->ensureMetaDataFilled();
    const QList<MetaDataType> secondTypes = second->types();
    for(QList<MetaDataType>::ConstIterator it = secondTypes.constBegin(), itEnd = secondTypes.constEnd(); it != itEnd; ++it)
    {
        const MetaDataEntryList secondMetaData = second->metaData(*it);
        for(MetaDataEntryList::ConstIterator jt = secondMetaData.constBegin(), jtEnd = secondMetaData.constEnd(); jt != jtEnd; ++jt)
            first->addCustomEntry(*it, *jt);
    }

    delete second;
    return first;
}

// https://bugreports.qt.io/browse/QTBUG-37946
// https://codereview.qt-project.org/#/c/110668/2
// https://github.com/qt/qtbase/blob/v5.4.0/src/gui/image/qjpeghandler.cpp
void ImageMetaData::applyExifOrientation(QImage *image, quint16 orientation)
{
    if(!image || image->isNull())
        return;

    // This is not an optimized implementation, but easiest to maintain
    QTransform transform;

    switch(orientation)
    {
        case 1: // normal
            break;
        case 2: // mirror horizontal
            *image = image->mirrored(true, false);
            break;
        case 3: // rotate 180
            transform.rotate(180);
            *image = image->transformed(transform);
            break;
        case 4: // mirror vertical
            *image = image->mirrored(false, true);
            break;
        case 5: // mirror horizontal and rotate 270 CCW
            *image = image->mirrored(true, false);
            transform.rotate(270);
            *image = image->transformed(transform);
            break;
        case 6: // rotate 90 CW
            transform.rotate(90);
            *image = image->transformed(transform);
            break;
        case 7: // mirror horizontal and rotate 90 CW
            *image = image->mirrored(true, false);
            transform.rotate(90);
            *image = image->transformed(transform);
            break;
        case 8: // rotate 270 CW
            transform.rotate(-90);
            *image = image->transformed(transform);
            break;
        default:
            qWarning("This should never happen");
    }
}

ImageMetaData::ImageMetaData()
    : m_impl(new Impl())
{}

ImageMetaData::~ImageMetaData()
{}

void ImageMetaData::applyExifOrientation(QImage *image) const
{
    applyExifOrientation(image, m_impl->getExifOrientation());
}

void ImageMetaData::addExifEntry(const QString &type, int tag, const QString &tagString, const QString &value)
{
#if defined (HAS_LIBEXIF)
    const char *name = exif_tag_get_name(static_cast<ExifTag>(tag));
    const char *title = exif_tag_get_title(static_cast<ExifTag>(tag));
    const char *description = exif_tag_get_description(static_cast<ExifTag>(tag));
    if(name && title)
    {
        addCustomEntry(type, IImageMetaData::MetaDataEntry(QString::fromUtf8(name), QString::fromUtf8(title), QString::fromUtf8(description), value));
        return;
    }
#else
    Q_UNUSED(tag)
#endif
    addCustomEntry(type, tagString, value);
}

void ImageMetaData::addCustomEntry(const QString &type, const QString &tag, const QString &value)
{
    addCustomEntry(type, IImageMetaData::MetaDataEntry(tag, value));
}

void ImageMetaData::addCustomEntry(const QString &type, const IImageMetaData::MetaDataEntry &entry)
{
    IImageMetaData::MetaDataEntryList &list = m_impl->entryListMap[type];
    if(!list.contains(entry))
        list.append(entry);
}

QList<IImageMetaData::MetaDataType> ImageMetaData::types()
{
    m_impl->ensureMetaDataFilled();
    return m_impl->entryListMap.keys();
}

IImageMetaData::MetaDataEntryList ImageMetaData::metaData(IImageMetaData::MetaDataType type)
{
    m_impl->ensureMetaDataFilled();
    return m_impl->entryListMap.value(type);
}

bool ImageMetaData::readFile(const QString &filePath)
{
    m_impl->entryListMap.clear();
#if defined (USE_EXIV2)
    try
    {
        m_impl->image.reset();
        m_impl->exifData.clear();
        m_impl->xmpData.clear();
        m_impl->image = Exiv2::ImageFactory::open(Exiv2BasicIoPtr(new QFileIo(filePath)));
        if(!m_impl->image.get())
            return false;
        m_impl->image->readMetadata();
        if(m_impl->image->exifData().empty() && m_impl->image->iptcData().empty() && m_impl->image->xmpData().empty())
            return false;
        if(!m_impl->image->exifData().empty())
            qDebug() << "EXIF data detected";
        if(!m_impl->image->iptcData().empty())
            qDebug() << "IPTC data detected";
        if(!m_impl->image->xmpData().empty())
            qDebug() << "XMP data detected";
        return true;
    }
    catch(...)
    {
        return false;
    }
#elif defined (USE_LIBEXIF)
    if(m_impl->exifData)
        exif_data_unref(m_impl->exifData);
    m_impl->exifData = exif_data_new_from_file(filePath.toLocal8Bit());
    if(!m_impl->exifData)
        return false;
//#if defined (QT_DEBUG)
//    fflush(stdout);
//    fflush(stderr);
//    exif_data_dump(m_impl->exifData);
//    fflush(stdout);
//    fflush(stderr);
//#endif
    qDebug() << "EXIF header detected";
    return true;
#elif defined (USE_QTEXTENDED)
    m_impl->exifHeader.clear();
    if(!m_impl->exifHeader.loadFromJpeg(filePath))
        return false;
    qDebug() << "EXIF header detected";
    return true;
#else
    Q_UNUSED(filePath);
    return false;
#endif
}

bool ImageMetaData::readFile(const QByteArray &fileData)
{
    m_impl->entryListMap.clear();
#if defined (USE_EXIV2)
    try
    {
        m_impl->image.reset();
        m_impl->exifData.clear();
        m_impl->xmpData.clear();
        const Exiv2::byte *data = reinterpret_cast<const Exiv2::byte*>(fileData.constData());
        const long dataSize = static_cast<long>(fileData.size());
        m_impl->image = Exiv2::ImageFactory::open(data, dataSize);
        if(!m_impl->image.get())
            return false;
        m_impl->image->readMetadata();
        if(m_impl->image->exifData().empty() && m_impl->image->iptcData().empty() && m_impl->image->xmpData().empty())
            return false;
        if(!m_impl->image->exifData().empty())
            qDebug() << "EXIF data detected";
        if(!m_impl->image->iptcData().empty())
            qDebug() << "IPTC data detected";
        if(!m_impl->image->xmpData().empty())
            qDebug() << "XMP data detected";
        return true;
    }
    catch(...)
    {
        return false;
    }
#elif defined (USE_LIBEXIF)
    if(m_impl->exifData)
        exif_data_unref(m_impl->exifData);
    ExifLoader *loader = exif_loader_new();
    unsigned char *buf = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(fileData.constData()));
    unsigned int len = static_cast<unsigned int>(fileData.size());
    exif_loader_write(loader, buf, len);
    m_impl->exifData = exif_loader_get_data(loader);
    exif_loader_unref(loader);
    if(!m_impl->exifData)
        return false;
//#if defined (QT_DEBUG)
//    fflush(stdout);
//    fflush(stderr);
//    exif_data_dump(m_impl->exifData);
//    fflush(stdout);
//    fflush(stderr);
//#endif
    qDebug() << "EXIF header detected";
    return true;
#elif defined (USE_QTEXTENDED)
    m_impl->exifHeader.clear();
    QBuffer buffer(const_cast<QByteArray*>(&fileData));
    if(!buffer.open(QIODevice::ReadOnly))
        return false;
    if(!m_impl->exifHeader.loadFromJpeg(&buffer))
        return false;
    qDebug() << "EXIF header detected";
    return true;
#else
    Q_UNUSED(fileData);
    return false;
#endif
}

bool ImageMetaData::readExifData(const QByteArray &rawExifData)
{
    m_impl->entryListMap.clear();
#if defined (USE_EXIV2)
    try
    {
        m_impl->image.reset();
        m_impl->exifData.clear();
        m_impl->xmpData.clear();
        const Exiv2::byte *data = reinterpret_cast<const Exiv2::byte*>(rawExifData.constData());
        const uint32_t dataSize = static_cast<uint32_t>(rawExifData.size());
        Exiv2::ExifParser::decode(m_impl->exifData, data, dataSize);
        if(m_impl->exifData.empty())
            return false;
        qDebug() << "EXIF data detected";
        return true;
    }
    catch(...)
    {
        return false;
    }
#elif defined (USE_LIBEXIF)
    if(m_impl->exifData)
        exif_data_unref(m_impl->exifData);
    const QByteArray rawExifDataWithHeader = QByteArray("Exif\0\0", 6) + rawExifData;
    const unsigned char *data = reinterpret_cast<const unsigned char*>(rawExifDataWithHeader.data());
    const unsigned int dataSize = static_cast<unsigned int>(rawExifDataWithHeader.size());
    m_impl->exifData = exif_data_new_from_data(data, dataSize);
    if(!m_impl->exifData)
        return false;
//#if defined (QT_DEBUG)
//    fflush(stdout);
//    fflush(stderr);
//    exif_data_dump(m_impl->exifData);
//    fflush(stdout);
//    fflush(stderr);
//#endif
    qDebug() << "EXIF header detected";
    return true;
#elif defined (USE_QTEXTENDED)
    m_impl->exifHeader.clear();
    QBuffer buffer(const_cast<QByteArray*>(&rawExifData));
    if(!buffer.open(QIODevice::ReadOnly))
        return false;
    if(!m_impl->exifHeader.read(&buffer))
        return false;
    qDebug() << "EXIF header detected";
    return true;
#else
    Q_UNUSED(rawExifData);
    return false;
#endif
}

bool ImageMetaData::readXmpData(const QByteArray &rawXmpData)
{
    m_impl->entryListMap.clear();
#if defined (USE_EXIV2)
    try
    {
        m_impl->image.reset();
        m_impl->exifData.clear();
        m_impl->xmpData.clear();
        const std::string packet = std::string(reinterpret_cast<const char*>(rawXmpData.constData()), static_cast<size_t>(rawXmpData.size()));
        Exiv2::XmpParser::decode(m_impl->xmpData, packet);
        if(m_impl->xmpData.empty())
            return false;
        qDebug() << "XMP data detected";
        return true;
    }
    catch(...)
    {
        return false;
    }
#else
    Q_UNUSED(rawXmpData);
    return false;
#endif
}
