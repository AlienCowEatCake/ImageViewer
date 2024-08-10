/*
   Copyright (C) 2019-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

   This file is part of the `QtUtils' library.

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

#if !defined (UPDATER_RELEASE_VERSION_H_INCLUDED)
#define UPDATER_RELEASE_VERSION_H_INCLUDED

#include <QList>
#include <QString>

class QDebug;

class ReleaseVersion
{
public:
    explicit ReleaseVersion(const QString &version = QString(), const QString &delimeter = QString::fromLatin1("."));
    ReleaseVersion(const ReleaseVersion &version);
    ~ReleaseVersion();
    ReleaseVersion &operator = (const ReleaseVersion &other);

    static ReleaseVersion fromTag(const QString &tag, const QString &delimeter = QString::fromLatin1("."));

    bool isValid() const;
    QString toString(const QString &delimeter = QString::fromLatin1(".")) const;

    bool operator == (const ReleaseVersion &other) const;
    bool operator != (const ReleaseVersion &other) const;
    bool operator < (const ReleaseVersion &other) const;
    bool operator <= (const ReleaseVersion &other) const;
    bool operator > (const ReleaseVersion &other) const;
    bool operator >= (const ReleaseVersion &other) const;

private:
    enum FieldType
    {
        FieldTypeUnknown    = 0,
        FieldTypeAlpha      = 1,
        FieldTypeBeta       = 2,
        FieldTypeRc         = 3,
        FieldTypeCustom     = 4,
        FieldTypeRegular    = 5
    };

    struct Field
    {
        FieldType type;
        QString value;

        bool operator == (const Field &other) const;
        bool operator != (const Field &other) const;
        bool operator < (const Field &other) const;
        bool operator <= (const Field &other) const;
        bool operator > (const Field &other) const;
        bool operator >= (const Field &other) const;
    };

    QList<Field> m_fields;
};

QDebug operator << (QDebug debug, const ReleaseVersion &version);

#endif // UPDATER_RELEASE_VERSION_H_INCLUDED

