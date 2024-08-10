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

#include "ReleaseVersion.h"

#include <cassert>

#include <QDebug>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QRegularExpression>
#else
#include <QRegExp>
#endif
#include <QStringList>

#include "Utils/Global.h"
#include "Utils/StringUtils.h"

//#define UPDATER_RELEASE_VERSION_DEBUG

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
typedef QRegularExpression QRE;
#else
typedef QRegExp QRE;
#endif

ReleaseVersion::ReleaseVersion(const QString &version, const QString &delimeter)
{
    const QStringList list = version.split(delimeter);
    for(QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it)
    {
        Field field;
        field.value = *it;
        static const QRE re = QRE(QString::fromLatin1("[^\\d]"));
        if(field.value.contains(QString::fromLatin1("alpha"), Qt::CaseInsensitive))
            field.type = FieldTypeAlpha;
        else if(field.value.contains(QString::fromLatin1("beta"), Qt::CaseInsensitive))
            field.type = FieldTypeBeta;
        else if(field.value.contains(QString::fromLatin1("rc"), Qt::CaseInsensitive))
            field.type = FieldTypeRc;
        else if(field.value.contains(re))
            field.type = FieldTypeCustom;
        else if(!field.value.isEmpty())
            field.type = FieldTypeRegular;
        else
            field.type = FieldTypeUnknown;
        m_fields.append(field);
    }

#if defined (UPDATER_RELEASE_VERSION_DEBUG)
    static bool tested = false;
    if(!tested)
    {
        tested = true;
        assert(ReleaseVersion::fromTag(QString::fromLatin1("v1.2.3")) == ReleaseVersion(QString::fromLatin1("1.2.3")));
        assert(ReleaseVersion(QString::fromLatin1("1.2")) < ReleaseVersion(QString::fromLatin1("1.2.0")));
        assert(ReleaseVersion(QString::fromLatin1("1.2.3")) < ReleaseVersion(QString::fromLatin1("1.2.4beta")));
        assert(ReleaseVersion(QString::fromLatin1("1.2.3")) > ReleaseVersion(QString::fromLatin1("1.2.3beta")));
        assert(ReleaseVersion(QString::fromLatin1("1.2.3beta1")) < ReleaseVersion(QString::fromLatin1("1.2.3beta2")));
        assert(ReleaseVersion(QString::fromLatin1("1.2.3-beta1")) < ReleaseVersion(QString::fromLatin1("1.2.3-beta2")));
        assert(ReleaseVersion(QString::fromLatin1("1.2.3.beta1")) < ReleaseVersion(QString::fromLatin1("1.2.3.beta2")));
        assert(ReleaseVersion(QString::fromLatin1("1.2.3.beta1")) < ReleaseVersion(QString::fromLatin1("1.2.3")));
    }
#endif
}

ReleaseVersion::~ReleaseVersion()
{}

ReleaseVersion::ReleaseVersion(const ReleaseVersion &version)
    : m_fields(version.m_fields)
{}

ReleaseVersion &ReleaseVersion::operator = (const ReleaseVersion &other)
{
    m_fields = other.m_fields;
    return *this;
}

ReleaseVersion ReleaseVersion::fromTag(const QString &tag, const QString &delimeter)
{
    static const QRE re = QRE(QString::fromLatin1("[\\d]"));
    const int versionPos = static_cast<int>(tag.indexOf(re));
    if(versionPos < 0)
        return ReleaseVersion();
    return ReleaseVersion(tag.mid(versionPos), delimeter);
}

bool ReleaseVersion::isValid() const
{
    return m_fields.size() > 0 && m_fields[0].type == FieldTypeRegular;
}

QString ReleaseVersion::toString(const QString &delimeter) const
{
    QString result;
    if(isValid())
    {
        for(QList<Field>::ConstIterator it = m_fields.constBegin(); it != m_fields.constEnd(); ++ it)
        {
            if(!result.isEmpty())
                result.append(delimeter);
            result.append(it->value);
        }
    }
    return result;
}

bool ReleaseVersion::operator == (const ReleaseVersion &other) const
{
    return m_fields == other.m_fields;
}

bool ReleaseVersion::operator != (const ReleaseVersion &other) const
{
    return !((*this) == other);
}

bool ReleaseVersion::operator < (const ReleaseVersion &other) const
{
    QList<Field>::ConstIterator it1 = m_fields.constBegin(), it2 = other.m_fields.constBegin();
    for(; it1 != m_fields.constEnd() && it2 != other.m_fields.constEnd(); ++it1, ++it2)
    {
        if((*it1) < (*it2))
            return true;
        if((*it1) > (*it2))
            return false;
    }
    if(it1 == m_fields.constEnd() && it2 != other.m_fields.constEnd())
        return it2->type >= FieldTypeRegular;
    if(it1 != m_fields.constEnd() && it2 == other.m_fields.constEnd())
        return it1->type < FieldTypeRegular;
    return false;
}

bool ReleaseVersion::operator <= (const ReleaseVersion &other) const
{
    return ((*this) == other) || ((*this) < other);
}

bool ReleaseVersion::operator > (const ReleaseVersion &other) const
{
    return ((*this) != other) && !((*this) < other);
}

bool ReleaseVersion::operator >= (const ReleaseVersion &other) const
{
    return ((*this) == other) || ((*this) > other);
}

bool ReleaseVersion::Field::operator == (const Field &other) const
{
    return value == other.value;
}

bool ReleaseVersion::Field::operator != (const Field &other) const
{
    return !((*this) == other);
}

bool ReleaseVersion::Field::operator < (const Field &other) const
{
    if((*this) == other)
        return false;

    // 1.2.3 < 1.2.4beta
    QString num1, num2;
    for(QString::ConstIterator it = value.constBegin(); it != value.constEnd() && it->isNumber(); ++it)
        num1.append(*it);
    for(QString::ConstIterator it = other.value.constBegin(); it != other.value.constEnd() && it->isNumber(); ++it)
        num2.append(*it);
    if(num1.length() < num2.length())
        return true;
    if(num1.length() > num2.length())
        return false;
    if(!num1.isEmpty() && !num2.isEmpty() && num1 != num2)
        return num1 < num2;

    // 1.2.3 > 1.2.3beta
    if(type < other.type)
        return true;
    if(type > other.type)
        return false;

    // 1.2.3beta1 < 1.2.3beta2
    return StringUtils::NumericLessThan(value, other.value);
}

bool ReleaseVersion::Field::operator <= (const Field &other) const
{
    return ((*this) == other) || ((*this) < other);
}

bool ReleaseVersion::Field::operator > (const Field &other) const
{
    return ((*this) != other) && !((*this) < other);
}

bool ReleaseVersion::Field::operator >= (const Field &other) const
{
    return ((*this) == other) || ((*this) > other);
}

QDebug operator << (QDebug debug, const ReleaseVersion &version)
{
    debug << (QString::fromLatin1("Version(%1)").arg(version.toString())).toLatin1().data();
    return debug;
}

