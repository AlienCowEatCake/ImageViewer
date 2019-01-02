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

#if !defined(IIMAGEMETADATA_H_INCLUDED)
#define IIMAGEMETADATA_H_INCLUDED

#include <QMap>
#include <QList>
#include <QPair>

class QString;

class IImageMetaData
{
public:
    struct MetaDataEntry
    {
        QString tagName;
        QString tagTitle;
        QString tagDescription;
        QString value;

        MetaDataEntry()
        {}

        MetaDataEntry(const QString &tagName, const QString &tagTitle, const QString &tagDescription, const QString &value)
            : tagName(tagName)
            , tagTitle(tagTitle)
            , tagDescription(tagDescription)
            , value(value)
        {}
    };
    typedef QList<MetaDataEntry> MetaDataEntryList;
    typedef QString MetaDataType;
    typedef QMap<MetaDataType, MetaDataEntryList> MetaDataEntryListMap;

public:
    virtual ~IImageMetaData() {}

    virtual QList<MetaDataType> types() const = 0;
    virtual MetaDataEntryList metaData(MetaDataType type) const = 0;
};

#endif // IIMAGEMETADATA_H_INCLUDED
