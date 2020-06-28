/*
   Copyright (C) 2019-2020 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (UPDATER_VERSION_H_INCLUDED)
#define UPDATER_VERSION_H_INCLUDED

#include <QString>

class QDebug;

class Version
{
public:
    enum Detail
    {
        DetailAuto = 0,
        DetailMajor = 1,
        DetailMinor = 2,
        DetailPatch = 3,
        DetailBuild = 4
    };

    explicit Version(int major = -1, int minor = -1, int patch = -1, int build = -1);
    explicit Version(const QString &version, const QString &delimeter = QString::fromLatin1("."));
    Version(const Version &version);
    ~Version();

    Version &operator = (const Version &other);
    bool operator == (const Version &other) const;
    bool operator != (const Version &other) const;
    bool operator < (const Version &other) const;
    bool operator <= (const Version &other) const;
    bool operator > (const Version &other) const;
    bool operator >= (const Version &other) const;

    bool isValid() const;

    bool hasMajor() const;
    bool hasMinor() const;
    bool hasPatch() const;
    bool hasBuild() const;

    int getMajor() const;
    int getMinor() const;
    int getPatch() const;
    int getBuild() const;

    void setMajor(int major);
    void setMinor(int minor);
    void setPatch(int patch);
    void setBuild(int build);

    Version shrinked(int detail) const;
    QString string(int detail = DetailAuto, const QString &delimeter = QString::fromLatin1(".")) const;

private:
    int m_major;
    int m_minor;
    int m_patch;
    int m_build;
};

QDebug operator << (QDebug debug, const Version &version);

#endif // UPDATER_VERSION_H_INCLUDED

