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

#if !defined(IMAGE_METADATA_H_INCLUDED)
#define IMAGE_METADATA_H_INCLUDED

#include "Utils/ScopedPointer.h"

#include "../../IImageMetaData.h"

class QByteArray;
class QString;
class QImage;

class ImageMetaData : public IImageMetaData
{
    Q_DISABLE_COPY(ImageMetaData)

public:
    static ImageMetaData *createExifMetaData(const QString &filePath);
    static ImageMetaData *createExifMetaData(const QByteArray &rawExifData);

public:
    ImageMetaData();
    ~ImageMetaData();

    void applyExifOrientation(QImage *image) const;

    void addExifEntry(const QString &type, int tag, const QString &tagString, const QString &value);
    void addCustomEntry(const QString &type, const QString &tag, const QString &value);

public: // IImageMetaData
    QList<MetaDataType> types();
    MetaDataEntryList metaData(MetaDataType type);

protected:
    bool readExifData(const QString &filePath);
    bool readExifData(const QByteArray &rawExifData);

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // IMAGE_METADATA_H_INCLUDED
