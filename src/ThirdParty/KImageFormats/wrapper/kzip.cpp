#include "kzip.h"

#include <algorithm>
#include <cstring>
#include <vector>

#include <QDataStream>
#include <QDebug>

#include <zlib.h>

QT_BEGIN_NAMESPACE

// https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT
// https://en.wikipedia.org/wiki/ZIP_(file_format)
// https://blog2k.ru/archives/3391
// https://blog2k.ru/archives/3392
// https://blog2k.ru/archives/3790

#define LOG_ERROR \
    qWarning() << __FUNCTION__ << "Error:"

#define CHECK_OR_RETURN_FALSE(arg) \
    do { \
        if(!(arg)) { \
            LOG_ERROR << #arg ; \
            return false; \
        } \
    } while(false)

struct KZip::Impl : public KArchiveDirectory
{
    struct EOCD64Locator
    {
        quint32 signature = 0;
        quint32 diskNumber = 0;
        quint64 eocd64Offset = 0;
        quint32 totalDiskCount = 0;

        static constexpr quint32 referenceSignature = 0x07064b50;
    };

    struct EOCD64
    {
        quint32 signature = 0;
        quint64 eocd64Size = 0;
        quint16 versionMadeBy = 0;
        quint16 versionToExtract = 0;
        quint32 diskNumber = 0;
        quint32 startDiskNumber = 0;
        quint64 numberCentralDirectoryRecord = 0;
        quint64 totalCentralDirectoryRecord = 0;
        quint64 sizeOfCentralDirectory = 0;
        quint64 centralDirectoryOffset = 0;
        QByteArray dataSector;

        static constexpr quint32 referenceSignature = 0x06064b50;
    };

    struct EOCD
    {
        quint32 signature = 0;
        quint16 diskNumber = 0;
        quint16 startDiskNumber = 0;
        quint16 numberCentralDirectoryRecord = 0;
        quint16 totalCentralDirectoryRecord = 0;
        quint32 sizeOfCentralDirectory = 0;
        quint32 centralDirectoryOffset= 0;
        quint16 commentLength = 0;
        QByteArray comment;
        EOCD64Locator eocd64Locator;
        EOCD64 eocd64;

        static constexpr quint32 referenceSignature = 0x06054b50;

        inline quint32 getDiskNumber() const
        {
            return eocd64.signature != 0 ? eocd64.diskNumber : diskNumber;
        }

        inline quint64 getNumberCentralDirectoryRecord() const
        {
            return eocd64.signature != 0 ? eocd64.numberCentralDirectoryRecord : numberCentralDirectoryRecord;
        }

        inline quint64 getCentralDirectoryOffset() const
        {
            return eocd64.signature != 0 ? eocd64.centralDirectoryOffset : centralDirectoryOffset;
        }
    };

    struct ExtraFieldRecord
    {
        quint16 headerId = 0;
        quint16 dataSize = 0;
        QByteArray data;

        inline bool isZip64() const
        {
            return headerId == 0x0001;
        }

        inline bool isUnicodePath() const
        {
            return headerId == 0x7075;
        }
    };

    struct ZIP64ExtendInformation
    {
        quint64 uncompressedSize = 0;
        quint64 compressedSize = 0;
        quint64 localFileHeaderOffset = 0;
        quint32 diskNumber = 0;
    };

    struct UnicodePathExtraField
    {
        quint8 version = 0;
        quint32 nameCrc32 = 0;
        QByteArray unicodeName;
    };

    struct CentralDirectoryFileHeader
    {
        quint32 signature = 0;
        quint16 versionMadeBy = 0;
        quint16 versionToExtract = 0;
        quint16 generalPurposeBitFlag = 0;
        quint16 compressionMethod = 0;
        quint16 modificationTime = 0;
        quint16 modificationDate = 0;
        quint32 crc32 = 0;
        quint32 compressedSize = 0;
        quint32 uncompressedSize = 0;
        quint16 filenameLength = 0;
        quint16 extraFieldLength = 0;
        quint16 fileCommentLength = 0;
        quint16 diskNumber = 0;
        quint16 internalFileAttributes = 0;
        quint32 externalFileAttributes = 0;
        quint32 localFileHeaderOffset = 0;
        QByteArray filename;
        std::vector<ExtraFieldRecord> extraField;
        QByteArray fileComment;
        ZIP64ExtendInformation zip64ExtInfo;
        UnicodePathExtraField unicodePath;

        static constexpr quint32 referenceSignature = 0x02014b50;

        inline bool isZip64() const
        {
            for(size_t i = 0; i < extraField.size(); ++i)
                if(extraField[i].isZip64())
                    return true;
            return false;
        }

        inline bool isUnicodePath() const
        {
            for(size_t i = 0; i < extraField.size(); ++i)
                if(extraField[i].isUnicodePath())
                    return true;
            return false;
        }

        inline quint64 getUncompressedSize() const
        {
            return uncompressedSize == 0xFFFFFFFF ? zip64ExtInfo.uncompressedSize : uncompressedSize;
        }

        inline quint64 getCompressedSize() const
        {
            return compressedSize == 0xFFFFFFFF ? zip64ExtInfo.compressedSize : compressedSize;
        }

        inline quint64 getLocalFileHeaderOffset() const
        {
            return localFileHeaderOffset == 0xFFFFFFFF ? zip64ExtInfo.localFileHeaderOffset : localFileHeaderOffset;
        }

        inline quint64 getDiskNumber() const
        {
            return diskNumber == 0xFFFF ? zip64ExtInfo.diskNumber : diskNumber;
        }
    };

    struct LocalFileHeader
    {
        quint32 signature = 0;
        quint16 versionToExtract = 0;
        quint16 generalPurposeBitFlag = 0;
        quint16 compressionMethod = 0;
        quint16 modificationTime = 0;
        quint16 modificationDate = 0;
        quint32 crc32 = 0;
        quint32 compressedSize = 0;
        quint32 uncompressedSize = 0;
        quint16 filenameLength = 0;
        quint16 extraFieldLength = 0;
        QByteArray filename;
        std::vector<ExtraFieldRecord> extraField;
        ZIP64ExtendInformation zip64ExtInfo;
        UnicodePathExtraField unicodePath;

        static constexpr quint32 referenceSignature = 0x04034b50;

        inline bool isZip64() const
        {
            for(size_t i = 0; i < extraField.size(); ++i)
                if(extraField[i].isZip64())
                    return true;
            return false;
        }

        inline bool isUnicodePath() const
        {
            for(size_t i = 0; i < extraField.size(); ++i)
                if(extraField[i].isUnicodePath())
                    return true;
            return false;
        }

        inline quint64 getUncompressedSize() const
        {
            return uncompressedSize == 0xFFFFFFFF ? zip64ExtInfo.uncompressedSize : uncompressedSize;
        }

        inline quint64 getCompressedSize() const
        {
            return compressedSize == 0xFFFFFFFF ? zip64ExtInfo.compressedSize : compressedSize;
        }
    };

    struct DataDescriptor
    {
        quint32 signature = 0;
        quint32 crc32 = 0;
        quint32 compressedSize = 0;
        quint32 uncompressedSize = 0;

        static constexpr quint32 referenceSignature = 0x08074b50;
    };

    struct DataDescriptor64
    {
        quint32 signature = 0;
        quint32 crc32 = 0;
        quint64 compressedSize = 0;
        quint64 uncompressedSize = 0;

        static constexpr quint32 referenceSignature = DataDescriptor::referenceSignature;
    };

    struct FileInfo : public KZipFileEntry
    {
        CentralDirectoryFileHeader cdfh;
        LocalFileHeader lfh;
        DataDescriptor dd;
        DataDescriptor64 dd64;

        QString filename;
        QIODevice *dev = Q_NULLPTR;
        qint64 dataPos = 0;
        qint64 dataEndPos = 0;

        bool isFile() const Q_DECL_OVERRIDE
        {
            return !filename.endsWith(QLatin1Char('/'));
        }

        QByteArray data() const Q_DECL_OVERRIDE
        {
            if(!dev)
                return QByteArray();

            if(!dev->seek(dataPos))
                return QByteArray();

            if(lfh.compressionMethod == 0)
            {
                return dev->read(dataEndPos - dataPos);
            }
            else if(lfh.compressionMethod == Z_DEFLATED)
            {
                z_stream zs;
                memset(&zs, 0, sizeof(zs));
                inflateInit2(&zs, -MAX_WBITS);
                QByteArray compressed = dev->read(dataEndPos - dataPos);
                zs.avail_in = compressed.size();
                zs.next_in = reinterpret_cast<Bytef*>(compressed.data());
                QByteArray uncompressed;
                if(lfh.generalPurposeBitFlag & (1 << 3))
                {
                    QByteArray chunk;
                    chunk.resize(1024);
                    while(true)
                    {
                        zs.avail_out = chunk.size();
                        zs.next_out = reinterpret_cast<Bytef*>(chunk.data());
                        const int res = inflate(&zs, Z_BLOCK);
                        if(zs.avail_out == 0)
                            uncompressed += chunk;
                        else
                            uncompressed += QByteArray::fromRawData(chunk.constData(), chunk.size() - zs.avail_out);
                        if(res == Z_OK || (res == Z_BUF_ERROR && zs.avail_out == 0))
                            continue;
                        if(res != Z_STREAM_END)
                        {
                            LOG_ERROR << "inflate failed, res =" << res;
                            uncompressed.clear();
                        }
                        break;
                    }
                }
                else
                {
                    uncompressed.resize(lfh.getUncompressedSize());
                    zs.avail_out = uncompressed.size();
                    zs.next_out = reinterpret_cast<Bytef*>(uncompressed.data());
                    const int res = inflate(&zs, Z_FINISH);
                    if(res != Z_OK && res != Z_STREAM_END)
                    {
                        LOG_ERROR << "inflate failed, res =" << res;
                        uncompressed.clear();
                    }
                }
                inflateEnd(&zs);
                return uncompressed;
            }
            else
            {
                LOG_ERROR << "invalid compression method" << lfh.compressionMethod;
            }
            return QByteArray();
        }
    };

    QIODevice *dev;
    bool isValid = false;
    EOCD eocd;
    std::vector<FileInfo> fileInfo;

    Impl(QIODevice *dev)
        : dev(dev)
    {
    }

    ~Impl()
    {
        if(dev && dev->isOpen())
            dev->close();
    }

    bool isFile() const Q_DECL_OVERRIDE
    {
        return false;
    }

    const KArchiveEntry *entry(const QString &name) const Q_DECL_OVERRIDE
    {
        if(!isValid)
            return Q_NULLPTR;

        for(size_t i = 0; i < fileInfo.size(); ++i)
        {
            if(fileInfo[i].filename == name)
                return &fileInfo[i];
        }
        return Q_NULLPTR;
    }

    bool open(QIODevice::OpenMode mode)
    {
        CHECK_OR_RETURN_FALSE(mode == QIODevice::ReadOnly);
        CHECK_OR_RETURN_FALSE(dev);

        isValid = false;
        if(dev->isOpen())
            dev->close();

        CHECK_OR_RETURN_FALSE(dev->open(mode));
        CHECK_OR_RETURN_FALSE(dev->isReadable());
        CHECK_OR_RETURN_FALSE(!dev->isSequential());

        CHECK_OR_RETURN_FALSE(seekToEocd());
        CHECK_OR_RETURN_FALSE(readEocd());
        CHECK_OR_RETURN_FALSE(dev->seek(eocd.getCentralDirectoryOffset()));

        fileInfo.clear();
        fileInfo.resize(eocd.getNumberCentralDirectoryRecord());
        for(size_t i = 0; i < fileInfo.size(); ++i)
            CHECK_OR_RETURN_FALSE(readCentralDirectoryFileHeader(fileInfo[i].cdfh));

        for(size_t i = 0; i < fileInfo.size(); ++i)
        {
            if(fileInfo[i].cdfh.getDiskNumber() != eocd.getDiskNumber())
            {
                fileInfo[i].dev = Q_NULLPTR;
                fileInfo[i].dataPos = 0;
                fileInfo[i].dataEndPos = 0;
                continue;
            }

            CHECK_OR_RETURN_FALSE(dev->seek(fileInfo[i].cdfh.getLocalFileHeaderOffset()));
            CHECK_OR_RETURN_FALSE(readLocalFileHeader(fileInfo[i].lfh));

            fileInfo[i].dev = dev;
            fileInfo[i].dataPos = dev->pos();

            if(fileInfo[i].cdfh.generalPurposeBitFlag & (1 << 11)) // Language encoding flag (EFS)
                fileInfo[i].filename = QString::fromUtf8(fileInfo[i].cdfh.filename);
            else if(fileInfo[i].cdfh.isUnicodePath() && !fileInfo[i].cdfh.unicodePath.unicodeName.isEmpty())
                fileInfo[i].filename = QString::fromUtf8(fileInfo[i].cdfh.unicodePath.unicodeName);
            else
                fileInfo[i].filename = QString::fromLocal8Bit(fileInfo[i].cdfh.filename);
        }

        for(size_t i = 0; i < fileInfo.size(); ++i)
        {
            if(!fileInfo[i].dev)
                continue;

            if(fileInfo[i].lfh.compressionMethod == 0)
                fileInfo[i].dataEndPos = fileInfo[i].dataPos + fileInfo[i].lfh.getUncompressedSize();
            else
                fileInfo[i].dataEndPos = fileInfo[i].dataPos + fileInfo[i].lfh.getCompressedSize();

            if(fileInfo[i].lfh.generalPurposeBitFlag & (1 << 3))
            {
                fileInfo[i].dataEndPos = std::min<qint64>(std::max<qint64>(fileInfo[i].dataPos, eocd.getCentralDirectoryOffset()), dev->size());
                for(size_t j = 0; j < fileInfo.size(); ++j)
                {
                    if(fileInfo[j].dataPos > fileInfo[i].dataPos)
                        fileInfo[i].dataEndPos = std::min<qint64>(fileInfo[i].dataEndPos, fileInfo[j].dataPos);
                }
                if(fileInfo[i].lfh.isZip64())
                {
                    constexpr qint64 eocd64SizeWithoutSignature = sizeof(fileInfo[i].dd64.crc32) +
                                                                  sizeof(fileInfo[i].dd64.compressedSize) +
                                                                  sizeof(fileInfo[i].dd64.uncompressedSize);
                    fileInfo[i].dataEndPos -= eocd64SizeWithoutSignature;
                }
                else
                {
                    constexpr qint64 eocdSizeWithoutSignature = sizeof(fileInfo[i].dd.crc32) +
                                                                sizeof(fileInfo[i].dd.compressedSize) +
                                                                sizeof(fileInfo[i].dd.uncompressedSize);
                    fileInfo[i].dataEndPos -= eocdSizeWithoutSignature;
                }

                // It is not compression-specific code, but it is very slow.
                // Deflated data can be extracted without this information.
                if(fileInfo[i].lfh.compressionMethod == 0)
                {
                    CHECK_OR_RETURN_FALSE(dev->seek(fileInfo[i].dataPos));
                    QDataStream ds(dev);
                    ds.setByteOrder(QDataStream::LittleEndian);
                    while(dev->pos() < fileInfo[i].dataEndPos)
                    {
                        quint32 signature = 0;
                        CHECK_OR_RETURN_FALSE(!((ds >> signature).atEnd()));
                        if(signature == DataDescriptor::referenceSignature)
                        {
                            CHECK_OR_RETURN_FALSE(dev->seek(dev->pos() - sizeof(signature)));
                            if(fileInfo[i].lfh.isZip64())
                            {
                                if(readDataDescriptor64(fileInfo[i].dd64))
                                {
                                    if(fileInfo[i].lfh.compressionMethod == 0)
                                        fileInfo[i].dataEndPos = fileInfo[i].dataPos + fileInfo[i].dd64.uncompressedSize;
                                    else
                                        fileInfo[i].dataEndPos = fileInfo[i].dataPos + fileInfo[i].dd64.compressedSize;
                                }
                            }
                            else
                            {
                                if(readDataDescriptor(fileInfo[i].dd))
                                {
                                    if(fileInfo[i].lfh.compressionMethod == 0)
                                        fileInfo[i].dataEndPos = fileInfo[i].dataPos + fileInfo[i].dd.uncompressedSize;
                                    else
                                        fileInfo[i].dataEndPos = fileInfo[i].dataPos + fileInfo[i].dd.compressedSize;
                                }
                            }
                            break;
                        }
                        CHECK_OR_RETURN_FALSE(dev->seek(dev->pos() - sizeof(signature) + 1));
                    }
                }
            }
        }

        isValid = true;
        return true;
    }

    bool seekToEocd()
    {
        constexpr qint64 minEocdSize = 22;
        constexpr qint64 maxEocdSize = 0xffff + minEocdSize;
        CHECK_OR_RETURN_FALSE(dev->size() >= minEocdSize);
        CHECK_OR_RETURN_FALSE(dev->seek(dev->size() - minEocdSize));
        QDataStream ds(dev);
        ds.setByteOrder(QDataStream::LittleEndian);
        for(qint64 i = 0; i < maxEocdSize - minEocdSize; ++i)
        {
            quint32 signature = 0;
            CHECK_OR_RETURN_FALSE(!((ds >> signature).atEnd()));
            if(signature == EOCD::referenceSignature)
            {
                CHECK_OR_RETURN_FALSE(dev->seek(dev->pos() - sizeof(signature)));
                return true;
            }
            CHECK_OR_RETURN_FALSE(dev->seek(dev->pos() - sizeof(signature) - 1));
        }
        return false;
    }

    bool readEocd()
    {
        const qint64 eocdPos = dev->pos();
        QDataStream ds(dev);
        ds.setByteOrder(QDataStream::LittleEndian);
        CHECK_OR_RETURN_FALSE(!((ds >> eocd.signature).atEnd()));
        CHECK_OR_RETURN_FALSE(eocd.signature == EOCD::referenceSignature);
        CHECK_OR_RETURN_FALSE(!((ds >> eocd.diskNumber).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> eocd.startDiskNumber).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> eocd.numberCentralDirectoryRecord).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> eocd.totalCentralDirectoryRecord).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> eocd.sizeOfCentralDirectory).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> eocd.centralDirectoryOffset).atEnd()));
        eocd.commentLength = 0;
        ds >> eocd.commentLength; // May be at end
        eocd.comment.resize(eocd.commentLength);
        CHECK_OR_RETURN_FALSE(ds.readRawData(eocd.comment.data(), eocd.comment.size()) >= 0);
        CHECK_OR_RETURN_FALSE(eocd.diskNumber == eocd.startDiskNumber);
        CHECK_OR_RETURN_FALSE(eocd.numberCentralDirectoryRecord == eocd.totalCentralDirectoryRecord);
        constexpr qint64 eocd64LocatorOffset = sizeof(eocd.eocd64Locator.signature) +
                                               sizeof(eocd.eocd64Locator.diskNumber) +
                                               sizeof(eocd.eocd64Locator.eocd64Offset) +
                                               sizeof(eocd.eocd64Locator.totalDiskCount);
        CHECK_OR_RETURN_FALSE(dev->seek(eocdPos - eocd64LocatorOffset));
        CHECK_OR_RETURN_FALSE(!((ds >> eocd.eocd64Locator.signature).atEnd()));
        if(eocd.eocd64Locator.signature == EOCD64Locator::referenceSignature)
        {
            CHECK_OR_RETURN_FALSE(!((ds >> eocd.eocd64Locator.diskNumber).atEnd()));
            CHECK_OR_RETURN_FALSE(!((ds >> eocd.eocd64Locator.eocd64Offset).atEnd()));
            CHECK_OR_RETURN_FALSE(!((ds >> eocd.eocd64Locator.totalDiskCount).atEnd()));
            CHECK_OR_RETURN_FALSE(dev->seek(eocd.eocd64Locator.eocd64Offset));
            CHECK_OR_RETURN_FALSE(!((ds >> eocd.eocd64.signature).atEnd()));
            CHECK_OR_RETURN_FALSE(eocd.eocd64.signature == EOCD64::referenceSignature);
            CHECK_OR_RETURN_FALSE(!((ds >> eocd.eocd64.eocd64Size).atEnd()));
            CHECK_OR_RETURN_FALSE(!((ds >> eocd.eocd64.versionMadeBy).atEnd()));
            CHECK_OR_RETURN_FALSE(!((ds >> eocd.eocd64.versionToExtract).atEnd()));
            CHECK_OR_RETURN_FALSE(!((ds >> eocd.eocd64.diskNumber).atEnd()));
            CHECK_OR_RETURN_FALSE(!((ds >> eocd.eocd64.startDiskNumber).atEnd()));
            CHECK_OR_RETURN_FALSE(!((ds >> eocd.eocd64.numberCentralDirectoryRecord).atEnd()));
            CHECK_OR_RETURN_FALSE(!((ds >> eocd.eocd64.totalCentralDirectoryRecord).atEnd()));
            CHECK_OR_RETURN_FALSE(!((ds >> eocd.eocd64.sizeOfCentralDirectory).atEnd()));
            CHECK_OR_RETURN_FALSE(!((ds >> eocd.eocd64.centralDirectoryOffset).atEnd()));
            CHECK_OR_RETURN_FALSE(eocd.eocd64.diskNumber == eocd.eocd64.startDiskNumber);
            CHECK_OR_RETURN_FALSE(eocd.eocd64.numberCentralDirectoryRecord == eocd.eocd64.totalCentralDirectoryRecord);
            constexpr qint64 eocd64MinSize = sizeof(eocd.eocd64.versionMadeBy) +
                                             sizeof(eocd.eocd64.versionToExtract) +
                                             sizeof(eocd.eocd64.diskNumber) +
                                             sizeof(eocd.eocd64.startDiskNumber) +
                                             sizeof(eocd.eocd64.numberCentralDirectoryRecord) +
                                             sizeof(eocd.eocd64.totalCentralDirectoryRecord) +
                                             sizeof(eocd.eocd64.sizeOfCentralDirectory) +
                                             sizeof(eocd.eocd64.centralDirectoryOffset);
            CHECK_OR_RETURN_FALSE(eocd.eocd64.eocd64Size >= eocd64MinSize);
            eocd.eocd64.dataSector.resize(eocd.eocd64.eocd64Size - eocd64MinSize);
            CHECK_OR_RETURN_FALSE(ds.readRawData(eocd.eocd64.dataSector.data(), eocd.eocd64.dataSector.size()) >= 0);
        }
        else
        {
            eocd.eocd64Locator = EOCD64Locator();
            eocd.eocd64 = EOCD64();
        }
        return true;
    }

    bool readCentralDirectoryFileHeader(CentralDirectoryFileHeader &cdfh)
    {
        QDataStream ds(dev);
        ds.setByteOrder(QDataStream::LittleEndian);
        CHECK_OR_RETURN_FALSE(!((ds >> cdfh.signature).atEnd()));
        CHECK_OR_RETURN_FALSE(cdfh.signature == CentralDirectoryFileHeader::referenceSignature);
        CHECK_OR_RETURN_FALSE(!((ds >> cdfh.versionMadeBy).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> cdfh.versionToExtract).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> cdfh.generalPurposeBitFlag).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> cdfh.compressionMethod).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> cdfh.modificationTime).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> cdfh.modificationDate).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> cdfh.crc32).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> cdfh.compressedSize).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> cdfh.uncompressedSize).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> cdfh.filenameLength).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> cdfh.extraFieldLength).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> cdfh.fileCommentLength).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> cdfh.diskNumber).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> cdfh.internalFileAttributes).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> cdfh.externalFileAttributes).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> cdfh.localFileHeaderOffset).atEnd()));
        cdfh.filename.resize(cdfh.filenameLength);
        CHECK_OR_RETURN_FALSE(ds.readRawData(cdfh.filename.data(), cdfh.filename.size()) >= 0);
        for(quint16 i = 0; i < cdfh.extraFieldLength;)
        {
            ExtraFieldRecord extra;
            CHECK_OR_RETURN_FALSE(!((ds >> extra.headerId).atEnd()));
            i += sizeof(extra.headerId);
            CHECK_OR_RETURN_FALSE(!((ds >> extra.dataSize).atEnd()));
            i += sizeof(extra.dataSize);
            CHECK_OR_RETURN_FALSE(i + extra.dataSize <= cdfh.extraFieldLength);
            extra.data.resize(extra.dataSize);
            CHECK_OR_RETURN_FALSE(ds.readRawData(extra.data.data(), extra.data.size()) >= 0);
            i += extra.dataSize;
            if(extra.isZip64())
            {
                QDataStream ds64(extra.data);
                ds64.setByteOrder(QDataStream::LittleEndian);
                if(cdfh.uncompressedSize == 0xFFFFFFFF)
                    ds64 >> cdfh.zip64ExtInfo.uncompressedSize;
                if(cdfh.compressedSize == 0xFFFFFFFF)
                    ds64 >> cdfh.zip64ExtInfo.compressedSize;
                if(cdfh.localFileHeaderOffset  == 0xFFFFFFFF)
                    ds64 >> cdfh.zip64ExtInfo.localFileHeaderOffset;
                if(cdfh.diskNumber  == 0xFFFF)
                    ds64 >> cdfh.zip64ExtInfo.diskNumber;
            }
            else if(extra.isUnicodePath())
            {
                QDataStream dsU8(extra.data);
                dsU8.setByteOrder(QDataStream::LittleEndian);
                dsU8 >> cdfh.unicodePath.version;
                if(cdfh.unicodePath.version == 1)
                {
                    dsU8 >> cdfh.unicodePath.nameCrc32;
                    cdfh.unicodePath.unicodeName.resize(extra.dataSize - sizeof(cdfh.unicodePath.version) - sizeof(cdfh.unicodePath.nameCrc32));
                    dsU8.readRawData(cdfh.unicodePath.unicodeName.data(), cdfh.unicodePath.unicodeName.size());
                }
            }
            cdfh.extraField.emplace_back(extra);
        }
        cdfh.fileComment.resize(cdfh.fileCommentLength);
        CHECK_OR_RETURN_FALSE(ds.readRawData(cdfh.fileComment.data(), cdfh.fileComment.size()) >= 0);
        return true;
    }

    bool readLocalFileHeader(LocalFileHeader &lfh)
    {
        QDataStream ds(dev);
        ds.setByteOrder(QDataStream::LittleEndian);
        CHECK_OR_RETURN_FALSE(!((ds >> lfh.signature).atEnd()));
        CHECK_OR_RETURN_FALSE(lfh.signature == LocalFileHeader::referenceSignature);
        CHECK_OR_RETURN_FALSE(!((ds >> lfh.versionToExtract).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> lfh.generalPurposeBitFlag).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> lfh.compressionMethod).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> lfh.modificationTime).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> lfh.modificationDate).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> lfh.crc32).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> lfh.compressedSize).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> lfh.uncompressedSize).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> lfh.filenameLength).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> lfh.extraFieldLength).atEnd()));
        lfh.filename.resize(lfh.filenameLength);
        CHECK_OR_RETURN_FALSE(ds.readRawData(lfh.filename.data(), lfh.filename.size()) >= 0);
        for(quint16 i = 0; i < lfh.extraFieldLength;)
        {
            ExtraFieldRecord extra;
            CHECK_OR_RETURN_FALSE(!((ds >> extra.headerId).atEnd()));
            i += sizeof(extra.headerId);
            CHECK_OR_RETURN_FALSE(!((ds >> extra.dataSize).atEnd()));
            i += sizeof(extra.dataSize);
            CHECK_OR_RETURN_FALSE(i + extra.dataSize <= lfh.extraFieldLength);
            extra.data.resize(extra.dataSize);
            CHECK_OR_RETURN_FALSE(ds.readRawData(extra.data.data(), extra.data.size()) >= 0);
            i += extra.dataSize;
            if(extra.isZip64())
            {
                QDataStream ds64(extra.data);
                ds64.setByteOrder(QDataStream::LittleEndian);
                if(lfh.uncompressedSize == 0xFFFFFFFF)
                    ds64 >> lfh.zip64ExtInfo.uncompressedSize;
                if(lfh.compressedSize == 0xFFFFFFFF)
                    ds64 >> lfh.zip64ExtInfo.compressedSize;
            }
            else if(extra.isUnicodePath())
            {
                QDataStream dsU8(extra.data);
                dsU8.setByteOrder(QDataStream::LittleEndian);
                dsU8 >> lfh.unicodePath.version;
                if(lfh.unicodePath.version == 1)
                {
                    dsU8 >> lfh.unicodePath.nameCrc32;
                    lfh.unicodePath.unicodeName.resize(extra.dataSize - sizeof(lfh.unicodePath.version) - sizeof(lfh.unicodePath.nameCrc32));
                    dsU8.readRawData(lfh.unicodePath.unicodeName.data(), lfh.unicodePath.unicodeName.size());
                }
            }
            lfh.extraField.emplace_back(extra);
        }
        return true;
    }

    bool readDataDescriptor(DataDescriptor &dd)
    {
        QDataStream ds(dev);
        ds.setByteOrder(QDataStream::LittleEndian);
        CHECK_OR_RETURN_FALSE(!((ds >> dd.signature).atEnd()));
        CHECK_OR_RETURN_FALSE(dd.signature == DataDescriptor::referenceSignature);
        CHECK_OR_RETURN_FALSE(!((ds >> dd.crc32).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> dd.compressedSize).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> dd.uncompressedSize).atEnd()));
        return true;
    }

    bool readDataDescriptor64(DataDescriptor64 &dd64)
    {
        QDataStream ds(dev);
        ds.setByteOrder(QDataStream::LittleEndian);
        CHECK_OR_RETURN_FALSE(!((ds >> dd64.signature).atEnd()));
        CHECK_OR_RETURN_FALSE(dd64.signature == DataDescriptor64::referenceSignature);
        CHECK_OR_RETURN_FALSE(!((ds >> dd64.crc32).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> dd64.compressedSize).atEnd()));
        CHECK_OR_RETURN_FALSE(!((ds >> dd64.uncompressedSize).atEnd()));
        return true;
    }
};

KZip::KZip(QIODevice *dev)
    : m_impl(new Impl(dev))
{}

KZip::~KZip()
{}

bool KZip::open(QIODevice::OpenMode mode)
{
    return m_impl->open(mode);
}

const KArchiveDirectory *KZip::directory() const
{
    return m_impl.data();
}

QT_END_NAMESPACE
